#include "mem_raw_pool.h"
#include "mem_cell.h"
#include "environment.h"
#include "bug_reporter.h"
#include <string.h>

// 部分类并没有对所有字段进行构造初始化，依赖了内存分配器自动置零
// 这个机制会导致分配消耗更大，字段初始化值应该由类自身去保证，以减少内存分配器的开销
#define CLEAN_MEM 1

CORE_NAMESPACE_BEG

typedef void* (*fp_mem_alloc_type)(size_t size);
typedef void (*fp_mem_free_type)(void* mem);

static fp_mem_alloc_type s_fp_mem_alloc = ::operator new;
static fp_mem_free_type s_fp_mem_free = ::operator delete;

inline bool* _new_block_freed_state()
{
	return new bool(false);
}
inline void _delete_block_freed_state(bool* p_state)
{
	delete p_state;
}

mem_raw_pool::~mem_raw_pool()
{
	for (auto block : _blocks)
	{
		s_fp_mem_free(block);
	}
	_blocks.clear();

	_free_head = nullptr;
	_cell_size = 0;
	_cell_count = 0;

	for (auto p_state : _blocks_freed_state)
	{
		_delete_block_freed_state(p_state);
	}
	_blocks_freed_state.clear();
	_blocks_freed_state_map.clear();
}

void* mem_raw_pool::alloc()
{
	return (void*)_pop_cell().user_mem;
}

bool mem_raw_pool::free(void* user_mem)
{
	auto& c = mem_cell::get_cell(user_mem);
	if (!c.is_used())
	{
		environment::get_current_env().get_bug_reporter().report(
			BUG_TAG_MEM_RAW_POOL,
			"mem_raw_pool free failed: user_mem is not return from alloc()!");
		return false;
	}
	_push_cell(c);
	return true;
}

/// <summary>
/// 如果此函数耗时过高，请参看<see cref="_block_is_free"/>和<see cref="_pop_block_cells_from_free_link"/>的优化注释
/// </summary>
size_t mem_raw_pool::cleanup_free_blocks()
{
	size_t cleanup_count = 0;
	for (int i = (int)_blocks.size() - 1; i >= 0; --i)
	{
		const auto block = _blocks[i];
		if (_block_is_free(block))
		{
			// 1.
			_pop_block_cells_from_free_link(block);
			// 2.
			s_fp_mem_free(block);
			// 3.
			_blocks.pop_back();
			// 4.
			_try_set_block_freed_state(block, true);
			// 5.
			++cleanup_count;
		}
	}
	return cleanup_count;
}

void mem_raw_pool::_push_cell(mem_cell& c)
{
	c.mark_unused();
#if CLEAN_MEM
	memset(c.user_mem, 0, _cell_size - mem_cell::UserMemOffset);
#endif
	c.p_next_cell = _free_head;
	_free_head = &c;
}
mem_cell& mem_raw_pool::_pop_cell()
{
	if (nullptr == _free_head)
	{
		_new_block();
	}

	auto p_cell = _free_head;
	_free_head = _free_head->p_next_cell;
	p_cell->mark_used();
#if CLEAN_MEM
	memset(p_cell->user_mem, 0, _cell_size - mem_cell::UserMemOffset);
#endif
	return *p_cell;
}

void mem_raw_pool::_new_block()
{
	// 1.
	void* block = s_fp_mem_alloc(_cell_size * _cell_count);

	// 2.
	_push_block_cells_into_free_link(block);

	// 3.
	_blocks.push_back(block);
}

void mem_raw_pool::_push_block_cells_into_free_link(void* block)
{
	mem_cell* p_cell = (mem_cell*)block;
	for (size_t i = 0; i < _cell_count; ++i)
	{
		_push_cell(*p_cell);
		p_cell = (mem_cell*)((intptr_t)p_cell + _cell_size);
	}
}

/// <summary>
/// 如果此函数耗时过高，可以为每个block增加一个计数，在push和pop时记录，由计数来确定是否free
/// </summary>
bool mem_raw_pool::_block_is_free(void* block)
{
	mem_cell* p_cell = (mem_cell*)block;
	for (size_t i = 0; i < _cell_count; ++i)
	{
		if (p_cell->is_used())
		{
			// not free
			return false;
		}
		p_cell = (mem_cell*)((intptr_t)block + _cell_size);
	}
	return true;
}

/// <summary>
/// 如果此函数耗时过高，可以在cell中增加前向节点指针将单向列表改为双向列表，以提升节点断开的执行效率
/// </summary>
void mem_raw_pool::_pop_block_cells_from_free_link(void* block)
{
	if (nullptr == _free_head)
	{
		return;
	}
	const intptr_t block_mem_beg = (intptr_t)block;
	const intptr_t block_mem_end = block_mem_beg + _cell_size * _cell_count;

	// 1. find out new free head
	while (nullptr != _free_head)
	{
		if (!_cell_is_in_block(_free_head, block_mem_beg, block_mem_end))
		{
			break;
		}
		_free_head = _free_head->p_next_cell;
	}
	if (nullptr == _free_head)
	{
		return;
	}

	// 2. find out other free cells and remove them
	mem_cell* p_prev_cell = _free_head;
	mem_cell* p_cell = p_prev_cell->p_next_cell;
	while (nullptr != p_cell)
	{
		if (_cell_is_in_block(p_cell, block_mem_beg, block_mem_end))
		{
			// remove
			p_prev_cell->p_next_cell = p_cell->p_next_cell;
		}
		else
		{
			p_prev_cell = p_cell;
		}
		p_cell = p_cell->p_next_cell;
	}
}

bool* mem_raw_pool::_get_block_freed_state(void* block)
{
	auto iter = _blocks_freed_state_map.find(block);
	if (_blocks_freed_state_map.end() == iter)
	{
		auto p_state = _new_block_freed_state();
		_blocks_freed_state.push_back(p_state);
		_blocks_freed_state_map[block] = p_state;
		return p_state;
	}
	return iter->second;
}

bool mem_raw_pool::_try_set_block_freed_state(void* block, bool state)
{
	auto iter = _blocks_freed_state_map.find(block);
	if (_blocks_freed_state_map.end() == iter)
	{
		return false;
	}
	*iter->second = state;
	return true;
}

bool* mem_raw_pool::get_pool_mem_freed_ptr(void* user_mem)
{
	auto block_size = _cell_size * _cell_count;
	for (auto block : _blocks)
	{
		if ((intptr_t)block <= (intptr_t)user_mem && ((intptr_t)block + (intptr_t)block_size) > (intptr_t)user_mem)
		{
			return _get_block_freed_state(block);
		}
	}
	environment::get_current_env().get_bug_reporter().report(
		BUG_TAG_MEM_RAW_POOL,
		"mem_raw_pool get_pool_mem_freed_ptr failed: user_mem is not in this pool!");
	return nullptr;
}

CORE_NAMESPACE_END