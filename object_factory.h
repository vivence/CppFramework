
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

class temp_ref_mem_pool;

class object_factory final : noncopyable {
	using _object_array_type = std::vector<object*>;
	using _temp_ref_mem_pool_pointer_type = std::unique_ptr<temp_ref_mem_pool>;

	mem_pool _mem_pool;
	_temp_ref_mem_pool_pointer_type _p_temp_ref_pool;

	object::_id_type _next_object_id = 1;
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

private: // friend functions
	template<typename _TID, typename _TObj>
	friend class object_manager;

	template<typename _T, typename ..._Args>
	_T* new_obj(_Args&&... args);
	void delete_obj(object* p_obj);
	void delete_obj_immediately(object* p_obj);

	template<typename _T>
	object_weak_ref<_T> get_weak_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return object_weak_ref<_T>(p_obj);
	}

	template<typename _T>
	object_temp_ref<_T>& get_temp_ref(_T* p_obj)
	{
		static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
		return *new(_alloc_temp_ref_mem()) object_temp_ref<_T>(p_obj);
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
	auto p_obj = static_cast<object*>(p);
	p_obj->_instance_id = _next_object_id++;
	return p;
}

CORE_NAMESPACE_END

#endif