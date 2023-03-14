
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "mem_pool_configable.h"

CORE_NAMESPACE_BEG

using mem_pool = mem_pool_configable<sizeof(size_t) * 2, 1 * 1024 * 1024>;
using large_mem_pool = mem_pool_configable<
	mem_pool::info_for_global::max_cell_size + mem_pool::info_for_global::cell_unit_size, 
	mem_pool::info_for_global::block_max_size>;

CORE_NAMESPACE_END

#endif