
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
	
public:
	mem_pool() = default;

public:
	// 备忘：应该使block里每个cell都在block里内部连接（通过cell head可以找到block index），block之间也相互连接，保持pool只有一个head
	template<size_t _ID, typename _T>
	void* alloc()
	{
		static_assert(_config::pool_meta<_T>::PoolIndex < mem_cell::PoolCount, "PoolIndex is too large");
		return _pool_creater<_ID, _config::pool_meta<_T>::PoolIndex, _config::cell_meta<_T>::Size, _config::cell_meta<_T>::Count>::get_pool(_pools).alloc();
	}

	void free(void* user_mem) 
	{ 
		// todo
	}
};

CORE_NAMESPACE_END

#endif