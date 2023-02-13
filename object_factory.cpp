
#include "object_factory.h"

#if REF_SAFE_CHECK
#include <iostream> // 临时实现
#endif // REF_SAFE_CHECK

CORE_NAMESPACE_BEG

object_factory::~object_factory()
{
	_handle_delay_destroy();
	_recyle_temp_refs();
	// 临时实现
	delete[] _temp_buf;
	_temp_buf = nullptr;
}

void object_factory::delete_obj(object* p_obj)
{
	_delay_destroy_objs.push_back(p_obj);
}

void object_factory::delete_obj_immediately(object* p_obj)
{
#if REF_SAFE_CHECK
	object_temp_ref_destroyed_pointers::add_destroyed_pointer(p_obj);
#endif // REF_SAFE_CHECK
	p_obj->~object();
	_mem_pool.free(p_obj);
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
			delete_obj_immediately(temp[i]);
		}
		temp.clear();
	}
}

void object_factory::_recyle_temp_refs()
{
	// 临时实现
	_temp_mem = &_temp_buf[0];
}

CORE_NAMESPACE_END