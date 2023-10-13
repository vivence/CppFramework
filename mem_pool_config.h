
#ifndef MEM_POOL_CONFIG_H
#define MEM_POOL_CONFIG_H

#include "core.h"
#include "utils.h"
#include <utility>

CORE_NAMESPACE_BEG

template<size_t _CellUnitSize, size_t _BlockMaxSize>
struct mem_pool_config {
	static_assert(sizeof(mem_cell) <= _CellUnitSize, "_CellUnitSize must be bigger than sizeof(mem_cell)");
	enum {
		CellUnitSize = _CellUnitSize,
		BlockMaxSize = _BlockMaxSize
	};

	struct calc {

		// cell raw size is not less than sizeof(mem_cell)
		constexpr static size_t cell_raw_size(size_t user_mem_size)
		{
			// (std::max): fxxk the macro "max"  
			return (std::max)(user_mem_size + mem_cell::UserMemOffset, sizeof(mem_cell));
		}

		// all indexes are divided according to multiples of CellUnitSize
		constexpr static size_t pool_index(size_t user_mem_size)
		{
			auto raw_size = cell_raw_size(user_mem_size);
			return (raw_size - 1) / _CellUnitSize;
		}

		// cell size is the min multiples of CellUnitSize
		constexpr static size_t cell_size(size_t user_mem_size)
		{
			return _CellUnitSize * (pool_index(user_mem_size) + 1);
		}

		// cell count is (BlockMaxSize / cell size), but min count is 1
		constexpr static size_t cell_count_by_cell_size(size_t c_size)
		{
			auto count = _BlockMaxSize / c_size;
			if (0 == count)
			{
				count = 1;
			}
			return count;
		}

		constexpr static size_t cell_count(size_t user_mem_size)
		{
			return cell_count_by_cell_size(cell_size(user_mem_size));
		}

		constexpr static size_t cell_size_by_pool_index(size_t p_index)
		{
			return _CellUnitSize * (p_index + 1);
		}

		constexpr static size_t cell_count_by_pool_index(size_t p_index)
		{
			return cell_count_by_cell_size(cell_size_by_pool_index(p_index));
		}
	};

	template<typename _T>
	struct type_meta {
		static const size_t cell_size = calc::cell_size(sizeof(_T));
		static const size_t cell_count = calc::cell_count(sizeof(_T));
		static const size_t pool_index = calc::pool_index(sizeof(_T));
	};
};

CORE_NAMESPACE_END

#endif
