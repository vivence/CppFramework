#include "mem_pool.h"
#include "mem_cell.h"

CORE_NAMESPACE_BEG

bool mem_pool::free(void* user_mem)
{
	auto p_pool = _get_pool(user_mem);
	if (nullptr == p_pool)
	{
		return false;
	}
	p_pool->free(user_mem);
	return true;
}

CORE_NAMESPACE_END