#include "test_mem_pool.h"
#include "mem_pool.h"
#ifdef TEST_GC
#include "gc/gc.h"
#endif
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>

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
	void Remove(void* mem) { _mems.erase(std::remove(_mems.begin(), _mems.end(), mem)); }
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
		_out << console_text::RED;
		_out << "test_alloc failed: raw count is invalid, " << raw_pool_count << " != " << mem_cell::PoolCount << std::endl;
		_out << console_text::RESET;
		return false;
	}
	_out << "test_alloc check pool count: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;

	// check block increase
	auto pool_index = mem_pool::info_for_type<int>::pool_index;
	if (nullptr == raw_pools[pool_index])
	{
		_out << console_text::RED;
		_out << "test_alloc failed: raw pool[" << pool_index << "] is nullptr" << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto& raw_pool = *raw_pools[pool_index];
	auto cell_count_in_block = mem_pool::info_for_type<int>::cell_count_in_block;
	auto block_count = raw_pool._blocks.size();
	if (0 != block_count)
	{
		_out << console_text::RED;
		_out << "test_alloc failed: block_count is not 0, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	for (size_t i = 0; i < cell_count_in_block; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	block_count = raw_pool._blocks.size();
	if (1 != block_count)
	{
		_out << console_text::RED;
		_out << "test_alloc failed: block_count is not 1, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << console_text::RED;
		_out << "test_alloc failed: block_count is not 2, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	_out << "test_alloc check block increase: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;
	
	return true;
}

bool test_mem_pool::test_realloc()
{
	mem_pool pool;
	_AutoFree auto_free(pool);
	auto& raw_pools = pool._pools;

	// check pool count
	auto raw_pool_count = sizeof(raw_pools) / sizeof(raw_pools[0]);
	if (mem_cell::PoolCount != raw_pool_count)
	{
		_out << console_text::RED;
		_out << "test_realloc failed: raw count is invalid, " << raw_pool_count << " != " << mem_cell::PoolCount << std::endl;
		_out << console_text::RESET;
		return false;
	}
	_out << "test_realloc check pool count: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;

	// check realloc
	auto mem = pool.alloc(1);
	auto_free.Add(mem);
	auto min_size = mem_pool::info_for_global::min_cell_user_mem_size;
	for (size_t i = 2; i <= min_size; ++i)
	{
		auto new_mem = pool.realloc(mem, i);
		if (new_mem != mem)
		{
			_out << console_text::RED;
			_out << "test_realloc failed: new_mem != mem, user_mem_size = " << i << std::endl;
			_out << console_text::RESET;
			auto_free.Add(new_mem);
			return false;
		}
	}
	auto new_size = min_size + 1;
	auto new_mem = pool.realloc(mem, new_size);
	if (new_mem == mem)
	{
		_out << console_text::RED;
		_out << "test_realloc failed: new_mem == mem, user_mem_size = " << new_size << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Add(new_mem);

	_out << "test_realloc: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;

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
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block - 1 != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block - 1 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	for (size_t i = 0; i < free_cell_count; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	free_cell_count = _get_free_cell_count(raw_pool);
	if (0 != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Add(pool.alloc<int>());
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block - 1 != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block - 1 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block * 2 != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_free failed: free_cell_count is not " << cell_count_in_block * 2 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}

	_out << "test_free check free cell count: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;
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
		_out << console_text::RED;
		_out << "test_cleanup_step failed: block_count is not 0, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}

	for (size_t i = 0; i < cell_count_in_block; ++i)
	{
		auto_free.Add(pool.alloc<int>());
	}
	auto mem = pool.alloc<int>();
	auto_free.Add(mem);
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << console_text::RED;
		_out << "test_alloc failed: block_count is not 2, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto p_mem_freed = pool.get_pool_mem_freed_ptr(mem);
	if (nullptr == p_mem_freed)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: p_mem_freed is nullptr" << std::endl;
		_out << console_text::RESET;
		return false;
	}
	if (*p_mem_freed)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: *p_mem_freed is true " << std::endl;
		_out << console_text::RESET;
		return false;
	}
	auto_free.Remove(mem);
	pool.free(mem);
	mem = nullptr;
	block_count = raw_pool._blocks.size();
	if (2 != block_count)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: block_count is not 2, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	if (*p_mem_freed)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: *p_mem_freed is true " << std::endl;
		_out << console_text::RESET;
		return false;
	}

	pool.cleanup_step();
	block_count = raw_pool._blocks.size();
	if (1 != block_count)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: block_count is not 1, it is " << block_count << std::endl;
		_out << console_text::RESET;
		return false;
	}
	if (!*p_mem_freed)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: *p_mem_freed is false " << std::endl;
		_out << console_text::RESET;
		return false;
	}
	_out << "test_cleanup_step check block count: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;

	// check free cell count
	auto free_cell_count = _get_free_cell_count(raw_pool);
	if (0 != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: free_cell_count is not " << 0 << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}

	auto_free.Clear();
	free_cell_count = _get_free_cell_count(raw_pool);
	if (cell_count_in_block != free_cell_count)
	{
		_out << console_text::RED;
		_out << "test_cleanup_step failed: free_cell_count is not " << cell_count_in_block << ", it is " << free_cell_count << std::endl;
		_out << console_text::RESET;
		return false;
	}

	_out << "test_cleanup_step check free cell count: " << console_text::GREEN << "OK" << console_text::RESET << std::endl;
	return true;
}

void _test_new_performance(size_t test_count)
{
	for (size_t i = 0; i < test_count; i++)
	{
		auto p = ::operator new(sizeof(int));
		//::operator delete(p);
	}
}

#ifdef TEST_GC
void _test_gc_performance(size_t test_count)
{
	for (size_t i = 0; i < test_count; i++)
	{
		auto p = GC_malloc(sizeof(int));
		//GC_free(p);
	}
}
#endif // TEST_GC

void _test_template_alloc_performance(mem_pool& mp, size_t test_count)
{
	for (size_t i = 0; i < test_count; i++)
	{
		auto p = mp.alloc<int>();
		//mp.free(p);
	}
}

void _test_alloc_performance(mem_pool& mp, size_t test_count)
{
	for (size_t i = 0; i < test_count; i++)
	{
		auto p = mp.alloc(sizeof(int));
		//mp.free(p);
	}
}

void test_mem_pool::test_performance()
{
#ifdef TEST_GC
	GC_init();
#endif // TEST_GC

	clock_t start, end;
	size_t test_count = 10000 * 100;
	_out << "test_count: " << string_format_utils::format_count(test_count) << std::endl;
	_out << "clocks per second: " << string_format_utils::format_count(CLOCKS_PER_SEC) << std::endl;

	mem_pool mp;

	start = clock();
	_test_new_performance(test_count);
	end = clock();
	auto spent_0 = end - start;
	_out << "new spent clocks: " << spent_0 << std::endl;

#ifdef TEST_GC
	start = clock();
	_test_gc_performance(test_count);
	end = clock();
	auto spent_1 = end - start;
	_out << "gc spent clocks: " << spent_1 << std::endl;
#endif // TEST_GC

	start = clock();
	_test_template_alloc_performance(mp, test_count);
	end = clock();
	auto spent_2 = end - start;
	_out << "template alloc spent clocks: " << spent_2 << std::endl;

	start = clock();
	_test_alloc_performance(mp, test_count);
	end = clock();
	auto spent_3 = end - start;
	_out << "alloc spent clocks: " << spent_3 << std::endl;

	_out << "template alloc diff to new = " << spent_2 - spent_0;
	_out << ", spent percent = " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << spent_2 * 100.0 / spent_0 << "%" << std::endl;
	
#ifdef TEST_GC
	_out << "template alloc diff to gc = " << spent_2 - spent_1;
	_out << ", spent percent = " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << spent_2 * 100.0 / spent_1 << "%" << std::endl;
#endif // TEST_GC

	_out << "template alloc diff to alloc = " << spent_2 - spent_3;
	_out << ", spent percent = " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << spent_2 * 100.0 / spent_3 << "%" << std::endl;

#ifdef TEST_GC
	GC_deinit();
#endif // TEST_GC
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