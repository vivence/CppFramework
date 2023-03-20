#ifndef OBJECT_MANAGER_TEMP_H
#define OBJECT_MANAGER_TEMP_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_factory.h"
#include "object_temp_ref.h"

CORE_NAMESPACE_BEG

/// <summary>
/// temp means auto destroy in frame end
/// </summary>
template<typename _TObj>
class object_manager_temp final : noncopyable {
	static_assert(std::is_base_of<object, _TObj>::value, "_TObj must be inherit from object");
public:
	using temp_ref = object_temp_ref<_TObj>;
	using ref = _TObj*;

private:
	object_factory& _obj_factory;

public:
	explicit object_manager_temp(object_factory& obj_factory)
		: _obj_factory(obj_factory)
	{
	}
	~object_manager_temp() {}

public:
	template<typename ..._Args>
	temp_ref& create(_Args&&... args);
	template<typename _T, typename ..._Args>
	object_temp_ref<_T>& create(_Args&&... args);
};

template<typename _TObj>
template<typename ..._Args>
typename object_manager_temp<_TObj>::temp_ref& object_manager_temp<_TObj>::create(_Args&&... args)
{
	return create<_TObj>(std::forward<_Args>(args)...);
}

template<typename _TObj>
template<typename _T, typename ..._Args>
object_temp_ref<_T>& object_manager_temp<_TObj>::create(_Args&&... args)
{
	static_assert(std::is_base_of<_TObj, _T>::value, "_T must be inherit from _TObj");

	_T* obj_ref = _obj_factory.new_obj<_T>(std::forward<_Args>(args)...);
	_obj_factory.delete_obj(obj_ref); // auto destroy in frame end
	return _obj_factory.get_temp_ref(obj_ref);
}

CORE_NAMESPACE_END

#endif