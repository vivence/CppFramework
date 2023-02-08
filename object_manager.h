
#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "core.h"
#include "noncopyable.h"
#include "object.h"
#include "object_factory.h"
#include "object_weak_ref.h"
#include "object_temp_ref.h"
#include <type_traits>
#include <map>

CORE_NAMESPACE_BEG

template<typename _TID, typename _TObj>
class object_manager final : noncopyable {
public:
	typedef _TID id_type;
    using weak_ref = object_weak_ref<_TObj>;
    using temp_ref = object_temp_ref<_TObj>;

private:
	using _map_type = std::map<id_type, _TObj*>;

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
    void destroy(id_type id);
    void destroy_immediately(id_type id);

    weak_ref try_get(id_type id) const;
    temp_ref& try_get_temp(id_type id) const;
    // for_each
    // find_if
    // find_all_if

private:
    void _destroy(id_type id, void (object_factory::* delete_fuc)(object*));
};

template<typename _TID, typename _TObj>
object_manager<_TID, _TObj>::~object_manager()
{
    for (const auto& kv : _map)
    {
        _obj_factory.delete_obj(kv.second);
    }
    _map.clear();
}

template<typename _TID, typename _TObj>
template<typename ..._Args>
object_weak_ref<_TObj> object_manager<_TID, _TObj>::create(id_type id, _Args&&... args)
{
    return create<_TObj>(id, std::forward<_Args>(args)...);
}

template<typename _TID, typename _TObj>
template<typename _T, typename ..._Args>
object_weak_ref<_T> object_manager<_TID, _TObj>::create(id_type id, _Args&&... args)
{
    static_assert(std::is_base_of<_TObj, _T>::value, "_T must be inherit from _TObj");

    _T* pObj = nullptr;

    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        pObj = _obj_factory.new_obj<_T>(id, std::forward<_Args>(args)...);
        _map.insert(std::make_pair(id, pObj));
    }

    return _obj_factory.get_weak_ref(pObj);
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
void object_manager<_TID, _TObj>::_destroy(id_type id, void (object_factory::* delete_fuc)(object*))
{
    auto iter = _map.find(id);
    if (_map.end() == iter)
    {
        return;
    }

    auto p_obj = iter->second;
    _map.erase(iter);

    (_obj_factory.*delete_fuc)(p_obj);
}

CORE_NAMESPACE_END

#endif