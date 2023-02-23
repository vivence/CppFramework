
#ifndef TEST_MEM_POOL_H
#define TEST_MEM_POOL_H

#include "core.h"
#include <ostream>

CORE_NAMESPACE_BEG

class mem_raw_pool;

class test_mem_pool {
	std::ostream& _out;

public:
	explicit test_mem_pool(std::ostream& out) : _out(out) {}

public:
	bool test_alloc();
	bool test_free();
	bool test_cleanup_step();

public:
	void test_performance();

private:
	static size_t _get_free_cell_count(const mem_raw_pool& raw_pool);
};

CORE_NAMESPACE_END

#endif
