
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "core.h"
#include "noncopyable.h"
#include "mem_cell.h"
#include "mem_pool_config.h"
#include "mem_raw_pool.h"
#include <memory>

CORE_NAMESPACE_BEG

class mem_pool : noncopyable {
	enum { _CellUnitSize = sizeof(size_t), _BlockMaxSize = 1 * 1024 * 1024};
	using _config = mem_pool_config<_CellUnitSize, _BlockMaxSize>;

	using _pool_pointer_type = std::unique_ptr<mem_raw_pool>;

	template<size_t _ID, size_t _PoolIndex, size_t _CellSize, size_t _CellCount>
	class _pool_creater {
		explicit _pool_creater(mem_pool::_pool_pointer_type pools[]) { pools[_PoolIndex] = std::unique_ptr<mem_raw_pool>(new mem_raw_pool(_CellSize, _CellCount)); }
	public:
		static mem_raw_pool& get_pool(_pool_pointer_type pools[])
		{
			static _pool_creater instance(pools);
			return *(pools[_PoolIndex]);
		}
	};

	_pool_pointer_type _pools[mem_cell::PoolCount];
	size_t _cleanup_index = 0;

	mem_raw_pool* _get_pool(void* user_mem)
	{
		auto i = mem_cell::get_cell(user_mem).get_pool_index();
		return mem_cell::PoolCount > i ? _pools[i].get() : nullptr;
	}
	
public:
	mem_pool() = default;

public:
	template<size_t _ID, typename _T>
	void* alloc()
	{
		static_assert(_config::pool_meta<_T>::PoolIndex < mem_cell::PoolCount, "PoolIndex is too large");
		auto user_mem = _pool_creater<_ID, _config::pool_meta<_T>::PoolIndex, _config::cell_meta<_T>::Size, _config::cell_meta<_T>::Count>::get_pool(_pools).alloc();
		if (nullptr != user_mem)
		{
			mem_cell::get_cell(user_mem).set_pool_index(_config::pool_meta<_T>::PoolIndex);
		}
		return user_mem;
	}
	template<typename _T>
	void* alloc() { return alloc<0, _T>(); }
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

template<size_t _ID>
class mem_pool_for_id : noncopyable {
	mem_pool _pool;
public:
	template<typename _T>
	void* alloc() { return _pool.alloc<_ID, _T>(); }
	bool free(void* user_mem) { _pool.free(); }
};

CORE_NAMESPACE_END

#endif