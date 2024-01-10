
#ifndef OBJECT_WEAK_REF_H
#define OBJECT_WEAK_REF_H

#include "core.h"
#include "dis_new.h"
#include "ref.h"
#include "environment.h"
#include <type_traits>

CORE_NAMESPACE_BEG

class support_weak_ref : noncopyable {
	friend class object_factory;
	template<typename _T>
	friend class object_weak_ref;
	typedef int _id_type;

	static const _id_type _INVALID_ID = 0;
	static const _id_type _FIRST_ID = 1;

	_id_type _instance_id;

protected:
    inline support_weak_ref() : _instance_id(_INVALID_ID) {};
	virtual ~support_weak_ref() { _instance_id = _INVALID_ID; }
};

template<typename _T>
class object_weak_ref final : dis_new {
    static_assert(std::is_base_of<support_weak_ref, _T>::value, "_T must be inherit from support_weak_ref");

	_T* _p;
#if ENABLE_MEM_POOL_CLEANUP
	bool* _p_pool_mem_freed;
#endif // ENABLE_MEM_POOL_CLEANUP
    support_weak_ref::_id_type _obj_id;

public:
	static object_weak_ref null_ref;

private:
    friend class object_factory;
    inline explicit object_weak_ref(_T* p, bool* p_pool_mem_freed = nullptr)
        : _p(p)
#if ENABLE_MEM_POOL_CLEANUP
		, _p_pool_mem_freed(p_pool_mem_freed)
#endif // ENABLE_MEM_POOL_CLEANUP
        , _obj_id((nullptr != p) ? static_cast<support_weak_ref*>(p)->_instance_id : support_weak_ref::_INVALID_ID)
    {
    }
    inline explicit object_weak_ref(const ref<_T>& obj_ref, bool* p_pool_mem_freed = nullptr)
		: _p(const_cast<_T*>(obj_ref.operator->()))
#if ENABLE_MEM_POOL_CLEANUP
		, _p_pool_mem_freed(p_pool_mem_freed)
#endif // ENABLE_MEM_POOL_CLEANUP
        , _obj_id((nullptr != obj_ref) ? obj_ref->_instance_id : support_weak_ref::_INVALID_ID)
    {
    }

public:
    inline object_weak_ref() : _obj_id(support_weak_ref::_INVALID_ID), _p(nullptr) {}

public:
    /// <summary>
    /// called after checking not equal to nullptr, or may throw null pointer exception
    /// </summary>
    inline const _T* operator->() const throw() { return _safe_ref(); }
    /// <summary>
    /// called after checking not equal to nullptr, or may throw null pointer exception
    /// </summary>
    inline _T* operator->() throw() { return _safe_ref(); }
private:
    inline _T* _safe_ref() const
    {
#if ENABLE_REF_SAFE_CHECK
		if (nullptr == *this)
		{
			environment::get_current_env().get_bug_reporter().report(BUG_TAG_WEAK_REF, "weak_ref is nullptr!");
		}
#endif
        return _p;
    }

public:
    inline bool operator ==(const _T* p) const { return _p == p && (nullptr == p || p->_instance_id == _obj_id); }
    inline bool operator !=(const _T* p) const { return !operator ==(p); }
    inline bool operator ==(const ref<_T>& rhs) const { return operator ==(rhs.operator->()); }
    inline bool operator !=(const ref<_T>& rhs) const { return !operator ==(rhs.operator->()); }
    inline bool operator ==(const object_weak_ref& rhs) const { return rhs._obj_id == _obj_id && rhs._p == _p; }
    inline bool operator !=(const object_weak_ref& rhs) const { return !operator ==(rhs); }
    inline bool operator ==(std::nullptr_t p) const { return p == _p || _pool_mem_freed() || _p->_instance_id != _obj_id; }
    inline bool operator !=(std::nullptr_t p) const { return !operator ==(p); }
    template<typename _TT>
    friend bool operator ==(std::nullptr_t p, const object_weak_ref<_TT>& wp);
    template<typename _TT>
    friend bool operator !=(std::nullptr_t p, const object_weak_ref<_TT>& wp);

private:
#if ENABLE_MEM_POOL_CLEANUP
	inline bool _pool_mem_freed() const { return nullptr != _p_pool_mem_freed && *_p_pool_mem_freed; }
#else
    inline constexpr bool _pool_mem_freed() const { return false; }
#endif

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