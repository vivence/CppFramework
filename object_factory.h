
#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "core.h"
#include "utils.h"
#include "noncopyable.h"
#include "mem_pool.h"
#include "object.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include "object_shared_ref.h"
#include "sfinae_macros.h"
#include <type_traits>
#include <vector>
#include <memory>
#include <utility>
#include <initializer_list>
#if ENABLE_REF_SAFE_CHECK
#include <set>
#endif // ENABLE_REF_SAFE_CHECK

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
	inline void cleanup_mem_step() { _mem_pool.cleanup_step(); }


#if ENABLE_REF_SAFE_CHECK
private:
	using _object_set_type = std::set<object*>;
	_object_set_type _extern_retained_objs;
public:
	void extern_retain(object* p_obj);
	void extern_release(object* p_obj);
#else
public:
	inline void extern_retain(object*) {}
	inline void extern_release(object*) {}
#endif // ENABLE_REF_SAFE_CHECK

private: // private functions
	void _handle_delay_destroy();
	void* _alloc_temp_ref_mem();
	void _recyle_temp_refs();
	template<typename _T, enable_if_convertible_int<_T, support_weak_ref> = 0>
	inline void _init_obj(_T* p, void* user_mem)
	{
		auto p_weak_obj = static_cast<support_weak_ref*>(p);
		p_weak_obj->_instance_id = _next_object_id++;

		auto p_obj = static_cast<object*>(p);
		p_obj->_mem = user_mem;
	}
	template<typename _T, enable_if_not_convertible_int<_T, support_weak_ref> = 0>
	inline void _init_obj(_T* p, void* user_mem)
	{
		auto p_obj = static_cast<object*>(p);
		p_obj->_mem = user_mem;
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
	template<typename _TObj>
	friend class object_manager_without_id;

	template<typename _T>
	friend class object_monitor_ptr;

	struct _shared_ref_deleter { static bool delete_obj(support_shared_ref* p_obj); };
	template<typename _T>
	using object_shared_ref = object_shared_ref<_T, _shared_ref_deleter>;

	template<typename _T, typename ..._Args>
	_T* new_obj(_Args&&... args);
	template<typename _T, typename _E>
	_T* new_obj(std::initializer_list<_E> list);
	template<typename _T>
	inline bool delete_obj(_T* p)
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
	inline bool delete_obj_immediately(_T* p)
	{
		auto p_obj = cast_utils<_T, object>::cast(p);
		if (nullptr == p_obj)
		{
			return false;
		}
		_delete_obj_immediately(p_obj);
		return true;
	}

#if ENABLE_MEM_POOL_CLEANUP
	template<typename _T>
	inline object_weak_ref<_T> get_weak_ref(_T* p)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		if (nullptr == p)
		{
			return object_weak_ref<_T>(p);
		}
		return object_weak_ref<_T>(p, _mem_pool.get_pool_mem_freed_ptr(cast_utils<_T, object>::cast(p)->_mem));
	}
	template<typename _T>
	inline object_weak_ref<_T> get_weak_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		if (nullptr == obj_ref)
		{
			return object_weak_ref<_T>(obj_ref);
		}
		return object_weak_ref<_T>(obj_ref, _mem_pool.get_pool_mem_freed_ptr(cast_utils<_T, object>::cast(obj_ref.operator->())->_mem));
	}
#else
	template<typename _T>
	inline object_weak_ref<_T> get_weak_ref(_T* p)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return object_weak_ref<_T>(p);
	}
	template<typename _T>
	inline object_weak_ref<_T> get_weak_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return object_weak_ref<_T>(obj_ref);
	}
#endif // ENABLE_MEM_POOL_CLEANUP

	template<typename _T>
	inline object_temp_ref<_T>& get_temp_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return *new(_alloc_temp_ref_mem()) object_temp_ref<_T>(p_obj);
	}
	template<typename _T>
	inline object_temp_ref<_T>& get_temp_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return *new(_alloc_temp_ref_mem()) object_temp_ref<_T>(obj_ref);
	}

	template<typename _T>
	inline object_shared_ref<_T> get_shared_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return std::move(object_shared_ref<_T>(p_obj));
	}
	template<typename _T>
	inline object_shared_ref<_T> get_shared_ref(const ref<_T>& obj_ref)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return std::move(object_shared_ref<_T>(obj_ref));
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