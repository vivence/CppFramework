
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "core.h"
#include "noncopyable.h"
#include "mem_cell.h"
#include "mem_pool_config.h"
#include "mem_raw_pool.h"
#include <memory>

CORE_NAMESPACE_BEG

class test_mem_pool;

class mem_pool : noncopyable {
	friend class test_mem_pool;

	enum { _CellUnitSize = sizeof(size_t), _BlockMaxSize = 1 * 1024 * 1024};
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
		using cell_meta_for_pool = _config::cell_meta_for_pool<_PoolIndex>;
		static void create(_pool_pointer_type pools[])
		{
			pools[_PoolIndex] = _pool_pointer_type(new mem_raw_pool(cell_meta_for_pool::Size, cell_meta_for_pool::Count));
			_pool_creater<_PoolIndex - 1>::create(pools);
		}
	};
	template<>
	struct _pool_creater<0> {
		using cell_meta_for_pool = _config::cell_meta_for_pool<0>;
		static void create(_pool_pointer_type pools[])
		{
			pools[0] = _pool_pointer_type(new mem_raw_pool(cell_meta_for_pool::Size, cell_meta_for_pool::Count));
		}
	};
	
public:
	mem_pool()
	{
		_pool_creater<mem_cell::PoolCount - 1>::create(_pools);
	}

public:
	template<typename _T>
	void* alloc()
	{
		static_assert(_config::pool_meta<_T>::PoolIndex < mem_cell::PoolCount, "PoolIndex is too large");
		auto user_mem = _pools[_config::pool_meta<_T>::PoolIndex]->alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(_config::pool_meta<_T>::PoolIndex);
		}
		return user_mem;
	}
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
	};
	template<typename _T>
	struct info_for_type {
		static const size_t cell_raw_size = sizeof(_T);
		static const size_t cell_size = _config::cell_meta<_T>::Size;
		static const size_t cell_count_in_block = _config::cell_meta<_T>::Count;
		static const size_t pool_index = _config::pool_meta<_T>::PoolIndex;
	};
};

CORE_NAMESPACE_END

#endif