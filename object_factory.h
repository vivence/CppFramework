
#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "core.h"
#include "noncopyable.h"
#include "mem_pool.h"
#include "object.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include <type_traits>
#include <vector>
#include <memory>

CORE_NAMESPACE_BEG

class temp_mem_pool;

template<typename _TID, typename _TObj>
class object_manager;

template<typename _TObj>
class object_manager_singleton;

template<typename _T>
class object_shared_ptr;

class object_factory final : noncopyable {
	class _object_deletor_base {
	public:
		virtual void delete_obj(mem_pool& mp) = 0;
	};
	template<typename _T>
	class _object_deletor : public _object_deletor_base
	{
		_T* _p;
	public:
		explicit _object_deletor(_T* p) : _p(p) {}
		virtual void delete_obj(mem_pool& mp)
		{
			do_delete(mp, _p);
		}
	public:
		static void do_delete(mem_pool& mp, _T* p)
		{
#if REF_SAFE_CHECK
			object_temp_ref_destroyed_pointers::add_destroyed_pointer(p);
#endif // REF_SAFE_CHECK
			p->~_T();
			mp.free(p);
		}
	};

	using _object_array_type = std::vector<_object_deletor_base*>;
	using _temp_ref_mem_pool_pointer_type = std::unique_ptr<temp_mem_pool>;
	using _object_deletor_mem_pool_pointer_type = std::unique_ptr<temp_mem_pool>;

	mem_pool _mem_pool;
	_temp_ref_mem_pool_pointer_type _p_temp_ref_pool;
	_object_deletor_mem_pool_pointer_type _p_object_deletor_pool;

	object::_id_type _next_object_id = object::_FIRST_ID;
	_object_array_type _delay_destroy_objs;

public:
	static const size_t DefaultTempRefPoolCellCount = 1000;
	static const size_t DefaultObjectDeletorPoolCellCount = 1000;
	explicit object_factory(size_t temp_ref_pool_cell_count = DefaultTempRefPoolCellCount, size_t object_deletor_pool_cell_count = DefaultObjectDeletorPoolCellCount);
	~object_factory();

public:
	void on_frame_end();
	void cleanup_mem_step() { _mem_pool.cleanup_step(); }

private: // private functions
	void _handle_delay_destroy();
	void* _alloc_temp_ref_mem();
	void _recyle_temp_refs();
	void* _alloc_object_deletor();
	void _recyle_object_deletors();

private: // friend functions
	template<typename _TID, typename _TObj>
	friend class object_manager;
	template<typename _TObj>
	friend class object_manager_singleton;
	template<typename _T>
	friend class object_shared_ptr;

	template<typename _T, typename ..._Args>
	_T* new_obj(_Args&&... args);
	template<typename _T>
	void delete_obj(_T* p);
	template<typename _T>
	void delete_obj_immediately(_T* p);

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

	void* mem = _mem_pool.alloc<_T>();
	if (nullptr == mem)
	{
		return nullptr;
	}

	auto p = new(mem) _T(std::forward<_Args>(args)...);
	if (nullptr == p)
	{
		return nullptr;
	}

	auto p_obj = static_cast<object*>(p);
	p_obj->_instance_id = _next_object_id++;
	return p;
}

template<typename _T>
void object_factory::delete_obj(_T* p)
{
	auto p_deletor = new(_alloc_object_deletor()) _object_deletor<_T>(p);
	_delay_destroy_objs.push_back(p_deletor);
}
template<typename _T>
void object_factory::delete_obj_immediately(_T* p)
{
	_object_deletor<_T>::do_delete(_mem_pool, p);
}

CORE_NAMESPACE_END

#endif