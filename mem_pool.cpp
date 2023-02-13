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

void mem_pool::cleanup_step()
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