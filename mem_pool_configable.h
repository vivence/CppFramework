
#ifndef MEM_POOL_CONFIGABLE_H
#define MEM_POOL_CONFIGABLE_H

#include "core.h"
#include "noncopyable.h"
#include "mem_cell.h"
#include "mem_pool_config.h"
#include "mem_raw_pool.h"
#include "environment.h"
#include "bug_reporter.h"
#include <memory>

CORE_NAMESPACE_BEG

inline constexpr static size_t mem_pool_config_cell_unit_size()
{
	constexpr size_t mem_cell_size = sizeof(mem_cell);
	constexpr size_t unit_size = sizeof(size_t);
	return unit_size * (mem_cell_size / unit_size + (0 < mem_cell_size % unit_size ? 1 : 0));
}

class test_mem_pool;

template<size_t _CellUnitSize, size_t _BlockMaxSize>
class mem_pool_configable : noncopyable {
	friend class test_mem_pool;

	using _config = mem_pool_config<_CellUnitSize, _BlockMaxSize>;

	using _pool_pointer_type = std::unique_ptr<mem_raw_pool>;

	_pool_pointer_type _pools[mem_cell::PoolCount];
	size_t _cleanup_index = 0;

	mem_raw_pool* _get_pool(void* user_mem)
	{
		auto i = mem_cell::get_cell(user_mem).get_pool_index();
		return mem_cell::PoolCount > i ? _pools[i].get() : nullptr;
	}

	template<size_t _PoolIndex>
	struct _pool_creater {
		static void create(_pool_pointer_type pools[])
		{
			pools[_PoolIndex] = _pool_pointer_type(new mem_raw_pool(
				_config::calc::cell_size_by_pool_index(_PoolIndex), 
				_config::calc::cell_count_by_pool_index(_PoolIndex)));
			_pool_creater<_PoolIndex - 1>::create(pools);
		}
	};
	template<>
	struct _pool_creater<0> {
		static void create(_pool_pointer_type pools[])
		{
			pools[0] = _pool_pointer_type(new mem_raw_pool(
				_config::calc::cell_size_by_pool_index(0),
				_config::calc::cell_count_by_pool_index(0)));
		}
	};

public:
	mem_pool_configable()
	{
		_pool_creater<mem_cell::PoolCount - 1>::create(_pools);
	}

public:
	template<typename _T>
	void* alloc()
	{
		using type_meta = typename _config::template type_meta<_T>;
		static_assert(type_meta::pool_index < mem_cell::PoolCount, "pool_index is too large");
		auto user_mem = _pools[type_meta::pool_index]->alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(type_meta::pool_index);
		}
		return user_mem;
	}
	void* alloc(size_t user_mem_size);
	void* realloc(void* user_mem, size_t user_mem_size);
	bool free(void* user_mem);

	void cleanup_step();

public:
	struct info_for_global {
		static const size_t cell_unit_size = _CellUnitSize;
		static const size_t block_max_size = _BlockMaxSize;
		static const size_t pool_max_count = mem_cell::PoolCount;
		static const size_t cell_raw_size_min = sizeof(mem_cell);
		static const size_t cell_head_size = sizeof(mem_cell::head_type);
		static const size_t user_mem_offset_in_cell = mem_cell::UserMemOffset;
		static const size_t min_cell_size = _config::calc::cell_size(0);
		static const size_t min_cell_user_mem_size = min_cell_size - mem_cell::UserMemOffset;
		static const size_t min_cell_pool_index = _config::calc::pool_index(min_cell_user_mem_size);
		static const size_t min_cell_count = _config::calc::cell_count_by_pool_index(min_cell_pool_index);
		static const size_t max_cell_size = _config::calc::cell_size_by_pool_index(mem_cell::PoolCount - 1);
		static const size_t max_cell_user_mem_size = _config::calc::cell_size_by_pool_index(mem_cell::PoolCount - 1) - mem_cell::UserMemOffset;
		static const size_t max_cell_count = _config::calc::cell_count_by_pool_index(mem_cell::PoolCount - 1);
		static const size_t max_type_size = max_cell_size - user_mem_offset_in_cell;
	};
	template<typename _T>
	struct info_for_type {
		using type_meta = typename _config::template type_meta<_T>;
		static const size_t type_size = sizeof(_T);
		static const size_t cell_size = type_meta::cell_size;
		static const size_t cell_count_in_block = type_meta::cell_count;
		static const size_t pool_index = type_meta::pool_index;
	};
};

template<size_t _CellUnitSize, size_t _BlockMaxSize>
void* mem_pool_configable<_CellUnitSize, _BlockMaxSize>::alloc(size_t user_mem_size)
{
	auto pool_index = _config::calc::pool_index(user_mem_size);
	if (pool_index < mem_cell::PoolCount)
	{
		auto user_mem = _pools[pool_index]->alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(pool_index);
		}
		return user_mem;
	}
	else
	{
		environment::get_cur_bug_reporter().report(BUG_TAG_MEM_POOL, "pool_index is too large")
	}
	return nullptr;
}

template<size_t _CellUnitSize, size_t _BlockMaxSize>
void* mem_pool_configable<_CellUnitSize, _BlockMaxSize>::realloc(void* user_mem, size_t user_mem_size)
{
	auto old_pool_index = mem_cell::get_cell(user_mem).get_pool_index();
	auto new_pool_index = _config::calc::pool_index(user_mem_size);
	if (old_pool_index == new_pool_index)
	{
		return user_mem;
	}
	free(user_mem);
	return alloc(user_mem_size);
}

template<size_t _CellUnitSize, size_t _BlockMaxSize>
bool mem_pool_configable<_CellUnitSize, _BlockMaxSize>::free(void* user_mem)
{
	auto p_pool = _get_pool(user_mem);
	if (nullptr == p_pool)
	{
		return false;
	}
	return p_pool->free(user_mem);
}

template<size_t _CellUnitSize, size_t _BlockMaxSize>
void mem_pool_configable<_CellUnitSize, _BlockMaxSize>::cleanup_step()
{
	mem_raw_pool* p_pool = nullptr;
	for (size_t i = _cleanup_index; mem_cell::PoolCount > i; ++i)
	{
		p_pool = _pools[i].get();
		if (nullptr != p_pool && 0 < p_pool->cleanup_free_blocks())
		{
			_cleanup_index = i + 1;
			if (mem_cell::PoolCount == _cleanup_index)
			{
				_cleanup_index = 0;
			}
			return;
		}
	}

	for (size_t i = 0; i < _cleanup_index; ++i)
	{
		if (nullptr != p_pool && 0 < p_pool->cleanup_free_blocks())
		{
			_cleanup_index = i + 1;
			return;
		}
	}
}

CORE_NAMESPACE_END

#endif