
#include "object_factory.h"
#include <vector>

#if REF_SAFE_CHECK
#include <iostream> // ¡Ÿ ± µœ÷
#endif // REF_SAFE_CHECK

CORE_NAMESPACE_BEG

class temp_mem_pool : noncopyable {
	using _block_array_type = std::vector<void*>;
	size_t _cell_size;
	size_t _cell_count;
	_block_array_type _blocks;
	void* _free_mem;
	size_t _next_block_index;
	size_t _used_count_in_cur_block;

public:
	temp_mem_pool(size_t cell_size, size_t cell_count)
		: _cell_size(cell_size)
		, _cell_count(cell_count)
		, _blocks()
		, _free_mem(nullptr)
		, _next_block_index(0)
		, _used_count_in_cur_block(0)
	{

	}
	~temp_mem_pool();

public:
	void* alloc();
	void reset();

private:
	void _new_block();
	void* _pop_cell();
};

temp_mem_pool::~temp_mem_pool()
{
	for (auto block : _blocks)
	{
		::operator delete(block);
	}
	_blocks.clear();
	_free_mem = nullptr;
	_next_block_index = 0;
	_used_count_in_cur_block = 0;
}

void* temp_mem_pool::alloc()
{
	if (nullptr == _free_mem)
	{
		_new_block();
	}
	return _pop_cell();
}
void temp_mem_pool::reset()
{
	_free_mem = nullptr;
	_next_block_index = 0;
	_used_count_in_cur_block = 0;
}

void temp_mem_pool::_new_block()
{
	void* block = nullptr;
	if (_blocks.size() > _next_block_index)
	{
		block = _blocks[_next_block_index];
	}
	else
	{
		block = ::operator new(_cell_size * _cell_count);
		_blocks.push_back(block);
	}
	++_next_block_index;

	_free_mem = block;
	_used_count_in_cur_block = 0;
}
void* temp_mem_pool::_pop_cell()
{
	void* mem = _free_mem;
	if (_cell_count == ++_used_count_in_cur_block)
	{
		_free_mem = nullptr;
	}
	else
	{
		_free_mem = (void*)((intptr_t)_free_mem + _cell_size);
	}
	return mem;
}

// --------------------------------------------------

object_factory::object_factory(size_t temp_ref_pool_cell_count, size_t object_deletor_pool_cell_count)
	: _p_temp_ref_pool(new temp_mem_pool(sizeof(object_temp_ref<object>), temp_ref_pool_cell_count))
	, _p_object_deletor_pool(new temp_mem_pool(sizeof(_object_deletor<object>), object_deletor_pool_cell_count))
{

}

object_factory::~object_factory()
{
	_handle_delay_destroy();
	_recyle_temp_refs();
}

void object_factory::on_frame_end()
{
	_handle_delay_destroy();
	_recyle_temp_refs();

#if REF_SAFE_CHECK
	object_temp_ref_destroyed_pointers::clear_destroyed_pointers();
#endif // REF_SAFE_CHECK
}

void object_factory::_handle_delay_destroy()
{
	static _object_array_type temp;
	auto obj_count = _delay_destroy_objs.size();
	if (0 < obj_count)
	{
		_delay_destroy_objs.swap(temp);
		for (size_t i = 0; i < obj_count; ++i)
		{
			temp[i]->delete_obj(_mem_pool);
		}
		temp.clear();
		_recyle_object_deletors();
	}
}

void* object_factory::_alloc_temp_ref_mem()
{
	return _p_temp_ref_pool->alloc();
}

void object_factory::_recyle_temp_refs()
{
	_p_temp_ref_pool->reset();
}


void* object_factory::_alloc_object_deletor()
{
	return _p_object_deletor_pool->alloc();
}
void object_factory::_recyle_object_deletors()
{
	_p_object_deletor_pool->reset();
}

CORE_NAMESPACE_END