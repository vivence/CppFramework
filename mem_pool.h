
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "mem_pool_configable.h"

CORE_NAMESPACE_BEG

using mem_pool = mem_pool_configable<mem_pool_config_cell_unit_size(), 1 * 1024 * 1024>;
using large_mem_pool = mem_pool_configable<
	mem_pool::info_for_global::max_cell_size + mem_pool::info_for_global::cell_unit_size, 
	mem_pool::info_for_global::block_max_size>;

struct mem_pool_utils {
	static mem_pool* p_mem_pool;
	template<typename _T>
	inline static void* alloc()
	{
		return p_mem_pool->alloc<_T>();
	}
	inline static void* alloc(size_t user_mem_size)
	{
		return p_mem_pool->alloc(user_mem_size);
	}
	inline static void* realloc(void* user_mem, size_t user_mem_size)
	{
		return p_mem_pool->realloc(user_mem, user_mem_size);
	}
	inline static bool free(void* user_mem)
	{
		return p_mem_pool->free(user_mem);
	}
};

CORE_NAMESPACE_END

#endif