
#ifndef OBJECT_SHARED_REF_H
#define OBJECT_SHARED_REF_H

#include "core.h"
#include "dis_new.h"
#include "object.h"
#include "sfinae_macros.h"
#include "enviroment.h"
#include "object_factory.h"
#include <utility>

CORE_NAMESPACE_BEG

class ref_info {
	int _ref_count = 0;
	int _add_ref() { return ++_ref_count; }
	int _remove_ref() { return --_ref_count;}

public:
	virtual ~ref_info() {}

public:
	template<typename _T, ENABLE_IF_CLASS(_T)>
	static int add_ref(_T* p)
	{
		if (nullptr == p)
		{
			return 0;
		}
		auto p_ref = dynamic_cast<ref_info*>(p);
		if (nullptr == p_ref)
		{
			return 0;
		}
		return p_ref->_add_ref();
	}
	template<typename _T, ENABLE_IF_CLASS(_T)>
	static int remove_ref(_T* p)
	{
		if (nullptr == p)
		{
			return 0;
		}
		auto p_ref = dynamic_cast<ref_info*>(p);
		if (nullptr == p_ref)
		{
			return 0;
		}
		return p_ref->_remove_ref();
	}
};

struct object_shared_ptr_tag_t {
	static object_shared_ptr_tag_t tag_new()
	{
		static object_shared_ptr_tag_t tag;
		return tag;
	}
};

template<typename _T>
class object_shared_ptr : dis_new {
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");
	static_assert(std::is_base_of<ref_info, _T>::value, "_T must be inherit from ref_info");
	_T* _p;

public: // typedef
	using value_type = _T;

private: // not allowed
	object_shared_ptr* operator&() = delete;
public: // default constructors
	object_shared_ptr() : _p(nullptr) {}
	object_shared_ptr(nullptr_t) : _p(nullptr) {}

public: // explicit constructors
	template<typename ..._Args>
	explicit object_shared_ptr(object_shared_ptr_tag_t, _Args&&... args)
		: _p(enviroment::get_current_env().get_object_factory().new_obj<_T>(std::forward<_Args>(args)...))
	{ 
		ref_info::add_ref(_p); 
	}

public:
	~object_shared_ptr() 
	{ 
		if (nullptr != _p && 0 >= ref_info::remove_ref(_p))
		{
			enviroment::get_current_env().get_object_factory().delete_obj(_p);
		}
	}

public: // implicit constructors 
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(_D* p) : _p(static_cast<_T*>(p)) { ref_info::add_ref(_p); }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(const _D* p) : _p(static_cast<_T*>(const_cast<_D*>(p))) { ref_info::add_ref(_p); }

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(object_shared_ptr<_D>& p) : _p(static_cast<_T*>(p.operator->())) { ref_info::add_ref(_p); }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(const object_shared_ptr<_D>& p) : _p(static_cast<_T*>(const_cast<_D*>(p.operator->()))) { ref_info::add_ref(_p); }

public: // implicit conversions
	operator bool() const { return nullptr != _p; }

public: // copy and assign
	object_shared_ptr(const object_shared_ptr& other) noexcept : _p(other._p) { ref_info::add_ref(_p); }
	object_shared_ptr& operator =(const object_shared_ptr& other) noexcept { *this = other._p; return *this; }
	object_shared_ptr& operator =(_T* p)
	{
		if (_p == p) { return *this; }
		object_shared_ptr(p).swap(*this);
		return *this;
	}
	object_shared_ptr& operator =(const _T* p) { *this = const_cast<_T*>(p); return *this; }

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr& operator =(_D* p) { *this = static_cast<_T*>(p); return *this; }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr& operator =(const _D* p) { *this = const_cast<_D*>(p); return *this; }

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr& operator =(object_shared_ptr<_D>& p) { *this = p.operator->(); return *this; }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr& operator =(const object_shared_ptr<_D>& p) { *this = p.operator->(); return *this; }

	void swap(object_shared_ptr& other) noexcept { std::swap(_p, other._p); }

public: // work as raw pointer
	_T* operator->() { return _p; }
	const _T* operator->() const { return _p; }

	_T& operator*() { return *_p; }
	const _T& operator*() const { return *_p; }

public: // comparators as left hand
	inline bool operator ==(const object_shared_ptr& rhs) const { return _p == rhs._p; }
	inline bool operator !=(const object_shared_ptr& rhs) const { return _p != rhs._p; }

	inline bool operator ==(nullptr_t) const { return _p == nullptr; }
	inline bool operator !=(nullptr_t) const { return _p != nullptr; }

	inline bool operator ==(const _T* p) const { return _p == p; }
	inline bool operator !=(const _T* p) const { return _p != p; }

public: // comparators as right hand
	template<typename _TT>
	friend bool operator ==(nullptr_t, const object_shared_ptr<_TT>& wp);
	template<typename _TT>
	friend bool operator !=(nullptr_t, const object_shared_ptr<_TT>& wp);

	template<typename _TT>
	friend bool operator ==(const _TT* p, const object_shared_ptr<_TT>& wp);
	template<typename _TT>
	friend bool operator !=(const _TT* p, const object_shared_ptr<_TT>& wp);
};

template<typename _TT>
inline bool operator ==(nullptr_t, const object_shared_ptr<_TT>& wp) { return nullptr == wp._p; }
template<typename _TT>
inline bool operator !=(nullptr_t, const object_shared_ptr<_TT>& wp) { return nullptr != wp._p; }

template<typename _TT>
inline bool operator ==(const _TT* p, const object_shared_ptr<_TT>& wp) { return p == wp._p; }
template<typename _TT>
inline bool operator !=(const _TT* p, const object_shared_ptr<_TT>& wp) { return p != wp._p; }

CORE_NAMESPACE_END

namespace std {

	template <typename _T>
	struct remove_pointer<CORE object_shared_ptr<_T>>
	{
		using type = _T;
	};
}

#endif