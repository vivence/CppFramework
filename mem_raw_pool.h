
#ifndef MEM_RAW_POOL_H
#define MEM_RAW_POOL_H

#include "core.h"
#include "noncopyable.h"
#include <vector>

CORE_NAMESPACE_BEG

struct mem_cell;

class mem_raw_pool : noncopyable {
	using _block_array_type = std::vector<void*>;
	_block_array_type _blocks;
	mem_cell* _free_head;

	size_t _cell_size;
	size_t _cell_count;

public:
	mem_raw_pool(size_t c_size, size_t c_count)
		: _cell_size(c_size)
		, _cell_count(c_count)
		, _free_head(nullptr)
		, _blocks()
	{

	}
	~mem_raw_pool();

public:
	void* alloc();
	void free(void* user_mem);
	// NOTICE!! this function is expensive
	void cleanup_free_blocks();

private:
	void _push_cell(mem_cell& c);
	mem_cell& _pop_cell();
	void _new_block();
	void _push_block_cells_into_free_link(void* block);
	bool _block_is_free(void* block);
	void _pop_block_cells_from_free_link(void* block);
	static bool _cell_is_in_block(mem_cell* p_cell, const intptr_t block_mem_beg, const intptr_t block_mem_end)
	{
		return block_mem_beg <= (intptr_t)p_cell && block_mem_end > (intptr_t)p_cell;
	}
};

CORE_NAMESPACE_END

#endif
