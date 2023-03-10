
#ifndef OBJECT_WEAK_REF_H
#define OBJECT_WEAK_REF_H

#include "core.h"
#include "dis_new.h"
#include "ref.h"
#include "object.h"
#include "environment.h"
#include <type_traits>

CORE_NAMESPACE_BEG

template<typename _T>
class object_weak_ref : dis_new {
    static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	object::_id_type _obj_id;
    _T* _p;

private:
    friend class object_factory;
    explicit object_weak_ref(_T* p)
        : _p(p)
        , _obj_id((nullptr != p) ? static_cast<object*>(p)->_instance_id : object::_INVALID_ID)
    {
    }
    explicit object_weak_ref(const ref<_T>& obj_ref)
        : _p(const_cast<_T*>(obj_ref.operator->()))
        , _obj_id((nullptr != obj_ref) ? obj_ref->_instance_id : object::_INVALID_ID)
    {
    }

public:
    object_weak_ref() : _obj_id(object::_INVALID_ID), _p(nullptr) {}

public:
    /// <summary>
    /// called after checking not equal to nullptr, or may throw null pointer exception
    /// </summary>
    const _T* operator->() const throw() { return _safe_ref(); }
    /// <summary>
    /// called after checking not equal to nullptr, or may throw null pointer exception
    /// </summary>
    _T* operator->() throw() { return _safe_ref(); }
private:
    _T* _safe_ref() const
    {
        if (nullptr == *this)
        {
            environment::get_current_env().get_bug_reporter().report(BUG_TAG_WEAK_REF, "weak_ref is nullptr!");
        }
        return _p;
    }

public:
	static object_weak_ref null_ref;

public:
    inline bool operator ==(const _T* p) const { return _p == p && (nullptr == _p || _p->_instance_id == _obj_id); }
    inline bool operator !=(const _T* p) const { return !operator ==(p); }
    inline bool operator ==(const ref<_T>& rhs) const { return rhs == _p && (nullptr == _p || _p->_instance_id == _obj_id); }
    inline bool operator !=(const ref<_T>& rhs) const { return !operator ==(rhs); }
    inline bool operator ==(const object_weak_ref& rhs) const { return rhs._obj_id == _obj_id && rhs._p == _p; }
    inline bool operator !=(const object_weak_ref & rhs) const { return !operator ==(rhs); }
    inline bool operator ==(std::nullptr_t p) const { return p == _p || _p->_instance_id != _obj_id; }
    inline bool operator !=(std::nullptr_t p) const { return !operator ==(p); }
    template<typename _TT>
    friend bool operator ==(std::nullptr_t p, const object_weak_ref<_TT>& wp);
    template<typename _TT>
    friend bool operator !=(std::nullptr_t p, const object_weak_ref<_TT>& wp);

private:
    object_weak_ref* operator&() = delete;
};

template<typename _T>
object_weak_ref<_T> object_weak_ref<_T>::null_ref(nullptr);

template<typename _T>
inline bool operator ==(std::nullptr_t p, const object_weak_ref<_T>& wp) { return wp.operator ==(p); }
template<typename _T>
inline bool operator !=(std::nullptr_t p, const object_weak_ref<_T>& wp) { return wp.operator !=(p); }

CORE_NAMESPACE_END

#endif