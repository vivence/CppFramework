
#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "core.h"
#include "utils.h"
#include "noncopyable.h"
#include "mem_pool.h"
#include "object.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include "sfinae_macros.h"
#include <type_traits>
#include <vector>
#include <memory>
#include <initializer_list>

CORE_NAMESPACE_BEG

class temp_ref_mem_pool;

template<typename _TID, typename _TObj>
class object_manager;

template<typename _TObj>
class object_manager_singleton;

template<typename _T>
class object_monitor_ptr;

class object_factory final : noncopyable {
	using _object_array_type = std::vector<object*>;
	using _temp_ref_mem_pool_pointer_type = std::unique_ptr<temp_ref_mem_pool>;

	mem_pool _mem_pool;
	_temp_ref_mem_pool_pointer_type _p_temp_ref_pool;

	support_weak_ref::_id_type _next_object_id = support_weak_ref::_FIRST_ID;
	_object_array_type _delay_destroy_objs;

public:
	static const size_t DefaultTempRefPoolCellCount = 1000;
	explicit object_factory(size_t temp_ref_pool_cell_count = DefaultTempRefPoolCellCount);
	~object_factory();

public:
	void on_frame_end();
	void cleanup_mem_step() { _mem_pool.cleanup_step(); }

private: // private functions
	void _handle_delay_destroy();
	void* _alloc_temp_ref_mem();
	void _recyle_temp_refs();
	template<typename _T, ENABLE_IF_CONVERTIBLE(_T, support_weak_ref)>
	void _init_obj(_T* p, void* user_mem)
	{
		auto p_weak_obj = static_cast<support_weak_ref*>(p);
		p_weak_obj->_instance_id = _next_object_id++;
	}
	template<typename _T, ENABLE_IF_NOT_CONVERTIBLE(_T, support_weak_ref)>
	void _init_obj(_T* p, void* user_mem)
	{
	}
	void _delete_obj(object* p_obj);
	void _delete_obj_immediately(object* p_obj);

private: // friend functions
	friend struct object_ref_utils;
	friend struct object_ptr_utils;

	template<typename _TID, typename _TObj>
	friend class object_manager;
	template<typename _TObj>
	friend class object_manager_singleton;

	template<typename _T>
	friend class object_monitor_ptr;

	template<typename _T, typename ..._Args>
	_T* new_obj(_Args&&... args);
	template<typename _T, typename _E>
	_T* new_obj(std::initializer_list<_E> list);
	template<typename _T>
	bool delete_obj(_T* p)
	{
		auto p_obj = cast_utils<_T, object>::cast(p);
		if (nullptr == p_obj)
		{
			return false;
		}
		_delete_obj(p_obj);
		return true;
	}
	template<typename _T>
	bool delete_obj_immediately(_T* p)
	{
		auto p_obj = cast_utils<_T, object>::cast(p);
		if (nullptr == p_obj)
		{
			return false;
		}
		_delete_obj_immediately(p_obj);
		return true;
	}

	template<typename _T>
	object_weak_ref<_T> get_weak_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return object_weak_ref<_T>(p_obj);
	}
	template<typename _T>
	object_weak_ref<_T> get_weak_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return object_weak_ref<_T>(obj_ref);
	}

	template<typename _T>
	object_temp_ref<_T>& get_temp_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return *new(_alloc_temp_ref_mem()) object_temp_ref<_T>(p_obj);
	}
	template<typename _T>
	object_temp_ref<_T>& get_temp_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return *new(_alloc_temp_ref_mem()) object_temp_ref<_T>(obj_ref);
	}
};

template<typename _T, typename ..._Args>
_T* object_factory::new_obj(_Args&&... args)
{
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	void* user_mem = _mem_pool.alloc<_T>();
	if (nullptr == user_mem)
	{
		return nullptr;
	}

	auto p = new(user_mem) _T(std::forward<_Args>(args)...);
	if (nullptr == p)
	{
		return nullptr;
	}

	_init_obj(p, user_mem);
	return p;
}

template<typename _T, typename _E>
_T* object_factory::new_obj(std::initializer_list<_E> list)
{
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	void* user_mem = _mem_pool.alloc<_T>();
	if (nullptr == user_mem)
	{
		return nullptr;
	}

	auto p = new(user_mem) _T(list);
	if (nullptr == p)
	{
		return nullptr;
	}

	_init_obj(p, user_mem);
	return p;
}

CORE_NAMESPACE_END

#endif