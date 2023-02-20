
#ifndef MEM_POOL_CONFIG_H
#define MEM_POOL_CONFIG_H

#include "core.h"
#include "utils.h"

CORE_NAMESPACE_BEG

template<size_t _CellUnitSize, size_t _BlockMaxSize>
class mem_pool_config {

	template<size_t _TSize> // raw cell size is the min multiples of _CellUnitSize
	struct _cell_size { enum { Value = (_TSize / _CellUnitSize + 1) * _CellUnitSize }; };

	template<size_t _CellCount>
	struct _cell_count { enum { Value = _CellCount }; };
	template<>
	struct _cell_count<0> { enum { Value = 1 }; }; // cell min cout is 1

public:
	template<typename _T>
	class cell_meta {
		// cell raw min size is sizeof(_cell)
		enum { _RawSize = get_max<sizeof(_T) + mem_cell::UserMemOffset, sizeof(mem_cell)>::Value };
	public:
		enum {
			Size = _cell_size<_RawSize>::Value,
			Count = _cell_count<_BlockMaxSize / Size>::Value
		};
	};

	template<typename _T>
	struct pool_meta {
		enum { PoolIndex = cell_meta<_T>::Size / _CellUnitSize - 1 };
	};

	template<size_t _PoolIndex>
	struct cell_meta_for_pool {
		enum {
			Size = (_PoolIndex + 1) * _CellUnitSize,
			Count = _cell_count<_BlockMaxSize / Size>::Value
		};
	};
};

CORE_NAMESPACE_END

#endif
