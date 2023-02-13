
#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include <type_traits>
#include <vector>

CORE_NAMESPACE_BEG

class object_factory final : noncopyable {
	using _object_array_type = std::vector<object*>;

	object::_id_type _next_object_id = 1;
	_object_array_type _delay_destroy_objs;

	char* _test_buf;
	char* _mem;

	char* _temp_buf;
	char* _temp_mem;

public:
	object_factory() : _test_buf(new char[10000]), _mem(&_test_buf[0]), _temp_buf(new char[10000]), _temp_mem(&_temp_buf[0]){}
	~object_factory();

public:
	void on_frame_end();

private: // private functions
	void _handle_delay_destroy();
	void _recyle_temp_refs();

private: // friend functions
	template<typename _TID, typename _TObj>
	friend class object_manager;

	template<typename _T, typename ..._Args>
	_T* new_obj(_Args&&... args);
	void delete_obj(object* p_obj);
	void delete_obj_immediately(object* p_obj);

	template<typename _T>
	object_weak_ref<_T> get_weak_ref(_T* p_obj);

	template<typename _T>
	object_temp_ref<_T>& get_temp_ref(_T* p_obj);
};

template<typename _T, typename ..._Args>
_T* object_factory::new_obj(_Args&&... args)
{
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	// ��ʱʵ��
	auto p = new(_mem) _T(std::forward<_Args>(args)...);
	if (nullptr == p) return p;
	_mem += sizeof(_T);

	auto p_obj = static_cast<object*>(p);
	p_obj->_instance_id = _next_object_id++;
	return p;
}

template<typename _T>
object_weak_ref<_T> object_factory::get_weak_ref(_T* p_obj)
{
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
	return object_weak_ref<_T>(p_obj);
}

template<typename _T>
object_temp_ref<_T>& object_factory::get_temp_ref(_T* p_obj)
{
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	// ��ʱʵ��
	auto p_ref = new(_temp_mem) object_temp_ref<_T>(p_obj);
	_temp_mem += sizeof(object_temp_ref<_T>);

	return *p_ref;
}

CORE_NAMESPACE_END

#endif