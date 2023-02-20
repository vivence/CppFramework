#include "mem_raw_pool.h"
#include "mem_cell.h"

CORE_NAMESPACE_BEG

mem_raw_pool::~mem_raw_pool()
{
	for (auto block : _blocks)
	{
		::operator delete(block);
	}
	_blocks.clear();

	_free_head = nullptr;
	_cell_size = 0;
	_cell_count = 0;
}

void* mem_raw_pool::alloc()
{
	return (void*)_pop_cell().user_mem;
}

void mem_raw_pool::free(void* user_mem)
{
	_push_cell(mem_cell::get_cell(user_mem));
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
			::operator delete(block);
			// 3.
			_blocks.pop_back();
			// 4.
			++cleanup_count;
		}
	}
	return cleanup_count;
}

void mem_raw_pool::_push_cell(mem_cell& c)
{
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
	return *p_cell;
}

void mem_raw_pool::_new_block()
{
	// 1.
	void* block = ::operator new(_cell_size * _cell_count);

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

CORE_NAMESPACE_END