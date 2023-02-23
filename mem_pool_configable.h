
#ifndef MEM_POOL_CONFIGABLE_H
#define MEM_POOL_CONFIGABLE_H

#include "core.h"
#include "noncopyable.h"
#include "mem_cell.h"
#include "mem_pool_config.h"
#include "mem_raw_pool.h"
#include <memory>

CORE_NAMESPACE_BEG

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
		using cell_meta_for_pool = typename _config::template cell_meta_for_pool<_PoolIndex>;
		static void create(_pool_pointer_type pools[])
		{
			pools[_PoolIndex] = _pool_pointer_type(new mem_raw_pool(cell_meta_for_pool::Size, cell_meta_for_pool::Count));
			_pool_creater<_PoolIndex - 1>::create(pools);
		}
	};
	template<>
	struct _pool_creater<0> {
		using cell_meta_for_pool = typename _config::template cell_meta_for_pool<0>;
		static void create(_pool_pointer_type pools[])
		{
			pools[0] = _pool_pointer_type(new mem_raw_pool(cell_meta_for_pool::Size, cell_meta_for_pool::Count));
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
		static_assert(typename _config::template pool_meta<_T>::PoolIndex < mem_cell::PoolCount, "PoolIndex is too large");
		auto user_mem = _pools[_config::template pool_meta<_T>::PoolIndex]->alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(_config::template pool_meta<_T>::PoolIndex);
		}
		return user_mem;
	}
	void* alloc(size_t user_mem_size);
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
		static const size_t min_cell_size = _config::cell_meta_for_pool<0>::Size;
		static const size_t min_cell_count = _config::cell_meta_for_pool<0>::Count;
		static const size_t max_cell_size = _config::cell_meta_for_pool<mem_cell::PoolCount - 1>::Size;
		static const size_t max_cell_count = _config::cell_meta_for_pool<mem_cell::PoolCount - 1>::Count;
		static const size_t max_type_size = max_cell_size - user_mem_offset_in_cell;
	};
	template<typename _T>
	struct info_for_type {
		static const size_t cell_raw_size = sizeof(_T);
		static const size_t cell_size = _config::template cell_meta<_T>::Size;
		static const size_t cell_count_in_block = _config::template cell_meta<_T>::Count;
		static const size_t pool_index = _config::template pool_meta<_T>::PoolIndex;
	};
};

template<size_t _CellUnitSize, size_t _BlockMaxSize>
void* mem_pool_configable<_CellUnitSize, _BlockMaxSize>::alloc(size_t user_mem_size)
{
	auto pool_index = _config::get_pool_index(user_mem_size);
	if (pool_index < mem_cell::PoolCount)
	{
		auto user_mem = _pools[pool_index]->alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(pool_index);
		}
		return user_mem;
	}
	return nullptr;
}

template<size_t _CellUnitSize, size_t _BlockMaxSize>
bool mem_pool_configable<_CellUnitSize, _BlockMaxSize>::free(void* user_mem)
{
	auto p_pool = _get_pool(user_mem);
	if (nullptr == p_pool)
	{
		return false;
	}
	p_pool->free(user_mem);
	return true;
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