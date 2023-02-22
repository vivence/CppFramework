
#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_factory.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include "containers.h"
#include <type_traits>
#include <utility>
#include <map>

CORE_NAMESPACE_BEG

template<typename _TID, typename _TObj>
class object_manager final : noncopyable {
public:
	typedef _TID id_type;
    using weak_ref = object_weak_ref<_TObj>;
    using temp_ref = object_temp_ref<_TObj>;
    using ref = _TObj*;

    using weak_ref_array = vector<weak_ref>;

private:
	using _map_type = std::map<id_type, ref>;

private:
    _map_type _map;
    object_factory& _obj_factory;

public:
    explicit object_manager(object_factory& obj_factory) : _obj_factory(obj_factory) {};
	~object_manager();

public:
    template<typename ..._Args>
    weak_ref create(id_type id, _Args&&... args);
    template<typename _T, typename ..._Args>
    object_weak_ref<_T> create(id_type id, _Args&&... args);

    /// <summary>
    /// deconstruct in frame end
    /// </summary>
    void destroy(id_type id);

    /// <summary>
    /// deconstruct immediately
    /// </summary>
    void destroy_immediately(id_type id);

    weak_ref try_get(id_type id) const;
    temp_ref& try_get_temp(id_type id) const;

    /// <summary>
    /// DO NOT call object_manager::create or object_manager::destroy in pred
    /// </summary>
    /// <param name="pred">is functional: bool pred(object_temp_ref &amp; obj_temp_ref)</param>
    /// <returns>object_weak_ref&lt;_TObj&gt;</returns>
    template<typename _P>
    weak_ref find_if(_P pred) const;

    /// <summary>
    /// DO NOT call object_manager::create or object_manager::destroy in pred
    /// </summary>
    /// <param name="pred">is functional: bool pred(object_temp_ref &amp; obj_temp_ref)</param>
    /// <returns>object_temp_ref&lt;_TObj&gt;</returns>
    template<typename _P>
    temp_ref& find_temp_if(_P pred) const;

    template<typename _P>
    weak_ref_array find_all_if(_P pred) const;

    /// <summary>
    /// DO NOT call object_manager::create or object_manager::destroy in func
    /// </summary>
    /// <param name="func">is functional: void func(object_temp_ref &amp; obj_temp_ref)</param>
    template<typename _F>
    void for_each(_F func) const;

    /// <summary>
    /// DO NOT call object_manager::create or object_manager::destroy in func
    /// </summary>
    /// <param name="func">is functional: bool func(object_temp_ref &amp; obj_temp_ref), return whether to continue</param>
    template<typename _P>
    void for_each_interruptible(_P pred) const;

private:
    void _destroy(id_type id, void (object_factory::* delete_fuc)(object*));
};

template<typename _TID, typename _TObj>
object_manager<_TID, _TObj>::~object_manager()
{
    for (auto& kv : _map)
    {
        _obj_factory.delete_obj(kv.second);
    }
    _map.clear();
}

template<typename _TID, typename _TObj>
template<typename ..._Args>
typename object_manager<_TID, _TObj>::weak_ref object_manager<_TID, _TObj>::create(id_type id, _Args&&... args)
{
    return create<_TObj>(id, std::forward<_Args>(args)...);
}

template<typename _TID, typename _TObj>
template<typename _T, typename ..._Args>
object_weak_ref<_T> object_manager<_TID, _TObj>::create(id_type id, _Args&&... args)
{
    static_assert(std::is_base_of<_TObj, _T>::value, "_T must be inherit from _TObj");

    _T* p_obj = nullptr;

    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        p_obj = _obj_factory.new_obj<_T>(id, std::forward<_Args>(args)...);
        _map.insert(std::make_pair(id, ref(p_obj)));
    }

    return _obj_factory.get_weak_ref(p_obj);
}

template<typename _TID, typename _TObj>
void object_manager<_TID, _TObj>::destroy(id_type id)
{
    _destroy(id, &object_factory::delete_obj);
}

template<typename _TID, typename _TObj>
void object_manager<_TID, _TObj>::destroy_immediately(id_type id)
{
    _destroy(id, &object_factory::delete_obj_immediately);
}

template<typename _TID, typename _TObj>
typename object_manager<_TID, _TObj>::weak_ref object_manager<_TID, _TObj>::try_get(id_type id) const
{
    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        return weak_ref::null_ref;
    }
    
    return _obj_factory.get_weak_ref(iter->second);
}

template<typename _TID, typename _TObj>
typename object_manager<_TID, _TObj>::temp_ref& object_manager<_TID, _TObj>::try_get_temp(id_type id) const
{
    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        return temp_ref::null_ref;
    }

    return _obj_factory.get_temp_ref(iter->second);
}

template<typename _TID, typename _TObj>
template<typename _P>
typename object_manager<_TID, _TObj>::weak_ref object_manager<_TID, _TObj>::find_if(_P pred) const
{
    for (const auto& kv : _map)
    {
        if (pred(_obj_factory.get_temp_ref(kv.second)))
        {
            return _obj_factory.get_weak_ref(kv.second);
        }
    }
    return weak_ref::null_ref;
}

template<typename _TID, typename _TObj>
template<typename _P>
typename object_manager<_TID, _TObj>::temp_ref& object_manager<_TID, _TObj>::find_temp_if(_P pred) const
{
    for (const auto& kv : _map)
    {
        auto& obj_temp_ref = _obj_factory.get_temp_ref(kv.second);
        if (pred(obj_temp_ref))
        {
            return obj_temp_ref;
        }
    }
    return temp_ref::null_ref;
}

template<typename _TID, typename _TObj>
template<typename _P>
typename object_manager<_TID, _TObj>::weak_ref_array object_manager<_TID, _TObj>::find_all_if(_P pred) const
{
    weak_ref_array objs;
    for (const auto& kv : _map)
    {
        auto& obj_temp_ref = _obj_factory.get_temp_ref(kv.second);
        if (pred(obj_temp_ref))
        {
            objs.push_back(std::move(_obj_factory.get_weak_ref(kv.second)));
        }
    }
    return std::move(objs);
}

template<typename _TID, typename _TObj>
template<typename _F>
void object_manager<_TID, _TObj>::for_each(_F func) const
{
    for (const auto& kv : _map)
    {
        func(_obj_factory.get_temp_ref(kv.second));
    }
}

template<typename _TID, typename _TObj>
template<typename _P>
void object_manager<_TID, _TObj>::for_each_interruptible(_P pred) const
{
    for (const auto& kv : _map)
    {
        if (!pred(_obj_factory.get_temp_ref(kv.second)))
        {
            break;
        }
    }
}

template<typename _TID, typename _TObj>
void object_manager<_TID, _TObj>::_destroy(id_type id, void (object_factory::* delete_fuc)(object*))
{
    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        return;
    }

    auto obj_ref = iter->second;
    _map.erase(iter);

    (_obj_factory.*delete_fuc)(obj_ref);
}

CORE_NAMESPACE_END

#endif