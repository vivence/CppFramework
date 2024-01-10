#ifndef OBJECT_MANAGER_WITHOUT_ID_H
#define OBJECT_MANAGER_WITHOUT_ID_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_factory.h"
#include "object_weak_ref.h"
#include <set>

CORE_NAMESPACE_BEG

template<typename _TObj>
class object_manager_without_id final : noncopyable {
	static_assert(std::is_base_of<object, _TObj>::value, "_TObj must be inherit from object");
public:
	using weak_ref = object_weak_ref<_TObj>;
	using ref = _TObj*;

private:
	using _set_type = std::set<ref>;

	_set_type _objs;
	object_factory& _obj_factory;

public:
	explicit object_manager_without_id(object_factory& obj_factory)
		: _objs(), _obj_factory(obj_factory)
	{
	}
	~object_manager_without_id()
	{
		for (auto& v : _objs)
		{
			_obj_factory.delete_obj(v);
		}
		_objs.clear();
	}

public:
	template<typename ..._Args>
	inline weak_ref create(_Args&&... args)
	{
		return create<_TObj, _Args...>(std::forward<_Args>(args)...);
	}
	template<typename _T, typename ..._Args>
	inline object_weak_ref<_T> create(_Args&&... args)
	{
		static_assert(std::is_base_of<_TObj, _T>::value, "_T must be inherit from _TObj");

		_T* obj_ref = _obj_factory.new_obj<_T>(std::forward<_Args>(args)...);
		_objs.insert(obj_ref);
		return _obj_factory.get_weak_ref(obj_ref);
	}

	/// <summary>
	/// deconstruct in frame end
	/// </summary>
	void destroy(weak_ref w_ref)
	{
		_destroy(w_ref.operator->(), &object_factory::_delete_obj);
	}

	/// <summary>
	/// deconstruct immediately
	/// </summary>
	void destroy_immediately(weak_ref w_ref)
	{
		_destroy(w_ref.operator->(), &object_factory::_delete_obj_immediately);
	}

private:
	inline void _destroy(ref obj_ref, void (object_factory::* delete_fuc)(object*))
	{
		if (0 == _objs.erase(obj_ref))
		{
			return;
		}

		(_obj_factory.*delete_fuc)(obj_ref);
	}
};

CORE_NAMESPACE_END

#endif