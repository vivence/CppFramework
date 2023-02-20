#include "test_mem_pool.h"
#include "mem_pool.h"
#include <vector>

/*
* mem_pool单元测试用例
* alloc
*   block 扩容
*   free link 变更
* free
*   free link变更
* cleanup_step
*   block 缩容
*/

CORE_NAMESPACE_BEG

struct _LargeData {
	size_t d1;
	size_t d2;
};

class _AutoFree : noncopyable {
	mem_pool& _pool;
	std::vector<void*> _mems;
public:
	explicit _AutoFree(mem_pool& pool) : _pool(pool)
	{

	}
	~_AutoFree() 
	{
		Clear();
	}
public:
	void Add(void* mem) { _mems.push_back(mem); }
	void Clear()
	{
		for (auto mem : _mems)
		{
			_pool.free(mem);
		}
		_mems.clear();
	}
};

bool test_mem_pool::test_alloc()
{
	mem_pool pool;
	_AutoFree auto_free(pool);
	auto& raw_pools = pool._pools;

	// check pool count
	auto raw_pool_count = sizeof(raw_pools) / sizeof(raw_pools[0]);
	if (mem_cell::PoolCount != raw_pool_count)
	{
		_out << "test_alloc failed: raw count is invalid, " << raw_pool_count << " != " << mem_cell::PoolCount << std::endl;
		return false;
	}
	_out << "test_alloc check pool count: OK" << std::endl;

	// check block increase
	auto pool_index = mem_pool::info_for_type<int>::pool_index;
	if (nullptr == raw_pools[pool_index])
	{
		_out << "test_alloc failed: raw pool[" << pool_index << "] is nullptr" << std::endl;
		return false;
	}
	auto& raw_pool = *raw_pools[pool_index];
	auto cell_count_in_block = mem_pool::info_for_type<int>::cell_count_in_block;
	auto block_count = raw_pool._blocks.size();
	if (0 != block_count)
	{
		_out << "test_alloc failed: block_count is not 0, it is " << block_count << std::endl;
		return false;
	}
	for (size_t i = 0; i < cell_count_in_block; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	block_count = raw_pool._blocks.size();
	if (1 != block_count)
	{
		_out << "test_alloc failed: block_count is not 1, it is " << block_count << std::endl;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << "test_alloc failed: block_count is not 2, it is " << block_count << std::endl;
		return false;
	}
	_out << "test_alloc check block increase: OK" << std::endl;
	
	return true;
}

bool test_mem_pool::test_free()
{
	mem_pool pool;
	_AutoFree auto_free(pool);

	auto& raw_pools = pool._pools;
	auto pool_index = mem_pool::info_for_type<int>::pool_index;
	auto& raw_pool = *raw_pools[pool_index];
	auto cell_count_in_block = mem_pool::info_for_type<int>::cell_count_in_block;

	auto free_cell_count = _get_free_cell_count(raw_pool);
	if (0 != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block - 1 != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block - 1 << ", it is " << free_cell_count << std::endl;
		return false;
	}
	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block << ", it is " << free_cell_count << std::endl;
		return false;
	}
	for (size_t i = 0; i < free_cell_count; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	free_cell_count = _get_free_cell_count(raw_pool);
	if (0 != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block - 1 != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block - 1 << ", it is " << free_cell_count << std::endl;
		return false;
	}
	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block * 2 != free_cell_count)
	{
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block * 2 << ", it is " << free_cell_count << std::endl;
		return false;
	}

	_out << "test_free check free cell count: OK" << std::endl;
	return true;
}

bool test_mem_pool::test_cleanup_step()
{
	mem_pool pool;
	_AutoFree auto_free(pool);

	auto& raw_pools = pool._pools;
	auto pool_index = mem_pool::info_for_type<int>::pool_index;
	auto& raw_pool = *raw_pools[pool_index];
	auto cell_count_in_block = mem_pool::info_for_type<int>::cell_count_in_block;

	// check block count
	auto block_count = raw_pool._blocks.size();
	if (0 != block_count)
	{
		_out << "test_alloc failed: block_count is not 0, it is " << block_count << std::endl;
		return false;
	}

	for (size_t i = 0; i < cell_count_in_block; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	auto mem = pool.alloc<int>();
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << "test_alloc failed: block_count is not 2, it is " << block_count << std::endl;
		return false;
	}
	pool.free(mem);
	mem = nullptr;
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << "test_alloc failed: block_count is not 2, it is " << block_count << std::endl;
		return false;
	}

	pool.cleanup_step();
	block_count = raw_pool._blocks.size();
	if (1 != block_count)
	{
		_out << "test_alloc failed: block_count is not 1, it is " << block_count << std::endl;
		return false;
	}
	_out << "test_cleanup_step check block count: OK" << std::endl;

	// check free cell count
	auto free_cell_count = _get_free_cell_count(raw_pool);
	if (0 != free_cell_count)
	{
		_out << "test_cleanup_step failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		return false;
	}

	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block != free_cell_count)
	{
		_out << "test_cleanup_step failed: free_cell_count is not " << cell_count_in_block << ", it is " << free_cell_count << std::endl;
		return false;
	}

	_out << "test_cleanup_step check free cell count: OK" << std::endl;
	return true;
}

size_t test_mem_pool::_get_free_cell_count(const mem_raw_pool& raw_pool)
{
	size_t free_cell_count = 0;
	auto p_cell = raw_pool._free_head;
	while (nullptr != p_cell)
	{
		++free_cell_count;
		p_cell = p_cell->p_next_cell;
	}
	return free_cell_count;
}


CORE_NAMESPACE_END