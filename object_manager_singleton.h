#ifndef OBJECT_MANAGER_SINGLETON_H
#define OBJECT_MANAGER_SINGLETON_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_factory.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"

CORE_NAMESPACE_BEG

template<typename _TObj>
class object_manager_singleton final : noncopyable {
	static_assert(std::is_base_of<object, _TObj>::value, "_TObj must be inherit from object");
public:
    using weak_ref = object_weak_ref<_TObj>;
    using temp_ref = object_temp_ref<_TObj>;
    using ref = _TObj*;

private:
    ref _instance;
    object_factory& _obj_factory;

public:
    explicit object_manager_singleton(object_factory& obj_factory) 
        : _instance(nullptr), _obj_factory(obj_factory) 
    {
    }
	~object_manager_singleton()
	{
		if (nullptr != _instance)
		{
			_obj_factory.delete_obj(_instance);
			_instance = nullptr;
		}
	}

public:
    template<typename ..._Args>
	weak_ref get_instance(_Args&&... args)
	{
		if (nullptr != _instance)
		{
			return _obj_factory.get_weak_ref(_instance);
		}

		auto p_obj = _obj_factory.new_obj<_TObj>(std::forward<_Args>(args)...);
		return _obj_factory.get_weak_ref(p_obj);
	}

    /// <summary>
    /// deconstruct in frame end
    /// </summary>
	inline void destroy_instance()
	{
		_destroy(&object_factory::delete_obj);
	}

    /// <summary>
    /// deconstruct immediately
    /// </summary>
    inline void destroy_instance_immediately()
	{
		_destroy(&object_factory::delete_obj_immediately);
	}

private:
    inline void _destroy(void (object_factory::* delete_fuc)(object*))
	{
		if (nullptr == _instance)
		{
			return;
		}

		(_obj_factory.*delete_fuc)(_instance);
		_instance = nullptr;
	}
};

CORE_NAMESPACE_END

#endif