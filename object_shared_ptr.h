
#ifndef OBJECT_SHARED_REF_H
#define OBJECT_SHARED_REF_H

#include "core.h"
#include "utils.h"
#include "dis_new.h"
#include "object.h"
#include "sfinae_macros.h"
#include "environment.h"
#include "object_factory.h"
#include <utility>
#include <initializer_list>

CORE_NAMESPACE_BEG

class ref_info {
	int _ref_count = 0;
	int _add_ref() { return ++_ref_count; }
	int _remove_ref() { return --_ref_count;}

public:
	virtual ~ref_info() {}

public:
	template<typename _T>
	static int add_ref(_T* p)
	{
		if (nullptr == p)
		{
			return 0;
		}
		auto p_ref = cast_utils<_T, ref_info>::cast(p);
		if (nullptr == p_ref)
		{
			return 0;
		}
		return p_ref->_add_ref();
	}

	template<typename _T>
	static int remove_ref(_T* p)
	{
		if (nullptr == p)
		{
			return 0;
		}
		auto p_ref = cast_utils<_T, ref_info>::cast(p);
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

struct object_ptr_utils;
//----------- 适配ML项目 --------------->
namespace _container_details {
	template<typename _T, bool has_default_ctor>
	struct _default_construct;
}
//--------------------------------------<

template<typename _T>
class object_shared_ptr : dis_new {
	void* operator new(size_t, void* mem) noexcept { return mem; }
	inline void operator delete(void*, void* mem) {}
	//----------- 适配ML项目 --------------->
	template<typename _T, bool has_default_ctor>
	friend struct _container_details::_default_construct;
	//--------------------------------------<
	_T* _p;

public: // typedef
	using value_type = _T;

private: // not allowed
	object_shared_ptr* operator&() = delete;

private: // private constructors
	friend struct object_ptr_utils;
	explicit object_shared_ptr(_T* p) : _p(p) { ref_info::add_ref(_p); }
	explicit object_shared_ptr(const _T* p) : _p(const_cast<_T*>(p)) { ref_info::add_ref(_p); }
	//----------- 适配ML项目 --------------->
	object_shared_ptr(_T* p, int extra_ref_count) : _p(p) 
	{ 
		ref_info::add_ref(_p); 
		for (int i = 0; i < extra_ref_count; ++i)
		{
			ref_info::add_ref(_p);
		}
	}
	object_shared_ptr(const _T* p, int extra_ref_count) : object_shared_ptr(const_cast<_T*>(p), extra_ref_count) {}
	//--------------------------------------<

public: // default constructors
	object_shared_ptr() : _p(nullptr) {}
	object_shared_ptr(nullptr_t) : _p(nullptr) {}

public: // explicit constructors
	template<typename ..._Args>
	explicit object_shared_ptr(object_shared_ptr_tag_t, _Args&&... args)
		: _p(environment::get_current_env().get_object_factory().new_obj<_T>(std::forward<_Args>(args)...))
	{ 
		ref_info::add_ref(_p); 
	}
	template<typename _E>
	explicit object_shared_ptr(object_shared_ptr_tag_t, std::initializer_list<_E> list)
		: _p(environment::get_current_env().get_object_factory().new_obj<_T>(list))
	{
		ref_info::add_ref(_p);
	}

public:
	~object_shared_ptr() 
	{ 
		if (nullptr != _p && 0 >= ref_info::remove_ref(_p))
		{
			environment::get_current_env().get_object_factory().delete_obj(_p);
		}
	}

public: // implicit constructors 
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(object_shared_ptr<_D>& p) : _p(static_cast<_T*>(p.operator->())) { ref_info::add_ref(_p); }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_shared_ptr(const object_shared_ptr<_D>& p) : _p(static_cast<_T*>(const_cast<_D*>(p.operator->()))) { ref_info::add_ref(_p); }

public: // implicit conversions
	//operator bool() const { return nullptr != _p; } // Ambiguity of function overloading

private: // private assign
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

public: // copy and assign
	object_shared_ptr(const object_shared_ptr& other) noexcept : _p(other._p) { ref_info::add_ref(_p); }
	object_shared_ptr& operator =(const object_shared_ptr& other) noexcept 
	{ 
		if (other == *this) { return *this; }
		object_shared_ptr(other).swap(*this);
		return *this;
	}
	object_shared_ptr& operator =(nullptr_t)
	{
		if (nullptr == _p) { return *this; }
		object_shared_ptr(nullptr).swap(*this);
		return *this;
	}
	
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

	template<typename _U>
	inline bool operator ==(const object_shared_ptr<_U>& rhs) const { return _p == rhs.operator->(); }
	template<typename _U>
	inline bool operator !=(const object_shared_ptr<_U>& rhs) const { return _p != rhs.operator->(); }

	inline bool operator ==(nullptr_t) const { return _p == nullptr; }
	inline bool operator !=(nullptr_t) const { return _p != nullptr; }

	inline bool operator ==(const _T* p) const { return _p == p; }
	inline bool operator !=(const _T* p) const { return _p != p; }

public: // comparators as right hand
	template<typename _TT>
	friend bool operator ==(nullptr_t, const object_shared_ptr<_TT>& wp);
	template<typename _TT>
	friend bool operator !=(nullptr_t, const object_shared_ptr<_TT>& wp);

	template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE_DEF(_DD*, _TT*)>
	friend bool operator ==(const _DD* p, const object_shared_ptr<_TT>& wp);
	template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE_DEF(_DD*, _TT*)>
	friend bool operator !=(const _DD* p, const object_shared_ptr<_TT>& wp);
};

template<typename _TT>
inline bool operator ==(nullptr_t, const object_shared_ptr<_TT>& wp) { return nullptr == wp._p; }
template<typename _TT>
inline bool operator !=(nullptr_t, const object_shared_ptr<_TT>& wp) { return nullptr != wp._p; }

template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE(_DD*, _TT*)>
inline bool operator ==(const _DD* p, const object_shared_ptr<_TT>& wp) { return p == wp._p; }
template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE(_DD*, _TT*)>
inline bool operator !=(const _DD* p, const object_shared_ptr<_TT>& wp) { return p != wp._p; }

CORE_NAMESPACE_END

namespace std {

	template <typename _T>
	struct remove_pointer<CORE object_shared_ptr<_T>>
	{
		using type = _T;
	};
}

#endif