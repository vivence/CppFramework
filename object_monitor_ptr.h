
#ifndef OBJECT_MONITOR_PTR_H
#define OBJECT_MONITOR_PTR_H

#include "core.h"
#include "utils.h"
#include "dis_new.h"
#include "object.h"
#include "sfinae_macros.h"
#include "environment.h"
#include "object_factory.h"
#include "bug_reporter.h"
#include <utility>
#include <stdlib.h>

CORE_NAMESPACE_BEG

struct object_ptr_utils;

struct info_for_monitor_ptr {
	int ref_count = 0;
	bool cs_refed = false;
	bool destroyed = false;
	virtual ~info_for_monitor_ptr() {}
};

template<typename _T>
class object_monitor_ptr : dis_new {
public: // support external placement new
	void* operator new(size_t, void* mem) noexcept { return mem; }
	inline void operator delete(void*, void* mem) {}

private:
	struct type_offset {
		static const int INVALID_VALUE = 9999;
		static int value;
	};

	_T* _p;

private: // monitor
	info_for_monitor_ptr* _get_info() const
	{
		if (nullptr == _p)
		{
			return nullptr;
		}
		return reinterpret_cast<info_for_monitor_ptr*>((intptr_t)_p + type_offset::value);
	}
	int _add_ref() const
	{
		auto p_info = _get_info();
		if (nullptr != p_info)
		{
			return ++p_info->ref_count;
		}
		return 0;
	}
	int _remove_ref() const
	{
		auto p_info = _get_info();
		if (nullptr != p_info)
		{
			return --p_info->ref_count;
		}
		return 0;
	}

public: // typedef
	using value_type = _T;

public: // create and destroy
	template<typename ..._Args>
	static object_monitor_ptr create(_Args&&... args)
	{
		if (type_offset::INVALID_VALUE == type_offset::value)
		{
			type_offset::value = (int)offsetof(_T, ref_count) - (int)offsetof(info_for_monitor_ptr, ref_count);
		}
		return object_monitor_ptr(environment::get_cur_object_factory().new_obj<_T>(std::forward<_Args>(args)...));
	}

	static bool destroy(object_monitor_ptr ptr)
	{
		auto p_info = ptr._get_info();
		if (nullptr != p_info)
		{
			p_info->destroyed = true;
			if (p_info->cs_refed)
			{
				environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr destroyed but still be referenced by c#");
			}
		}
		return true;
	}

private:
	inline void _check_valid() const
	{
		auto p_info = _get_info();
		if (nullptr != p_info && p_info->destroyed)
		{
			environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr access destroyed object");
		}
	}

private: // not allowed
	object_monitor_ptr* operator&() = delete;

private: // private constructors
	friend struct object_ptr_utils;
	template<typename _U>
	friend class object_monitor_ptr;
	inline explicit object_monitor_ptr(_T* p) noexcept : _p(p) { _check_valid(); _add_ref(); }
	inline explicit object_monitor_ptr(const _T* p) noexcept : object_monitor_ptr(const_cast<_T*>(p)) {}

public: // default constructors
	inline object_monitor_ptr() : _p(nullptr) {}
	inline object_monitor_ptr(nullptr_t) : _p(nullptr) {}

public:
	~object_monitor_ptr()
	{
		if (0 >= _remove_ref())
		{
			auto p_info = _get_info();
			if (nullptr != p_info)
			{
				if (p_info->cs_refed)
				{
					environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr (there's no pointer but still be referenced by c#)");
				}
				if (p_info->destroyed)
				{
					environment::get_cur_object_factory().delete_obj(_p);
				}
				else
				{
					environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr leak (there's no pointer but not destroyed)");
				}
			}
		}
	}

public: // implicit constructors 
	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr(object_monitor_ptr<_D>& p) noexcept : object_monitor_ptr(static_cast<_T*>(p.operator->())) {}
	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr(const object_monitor_ptr<_D>& p) noexcept : object_monitor_ptr(static_cast<_T*>(const_cast<_D*>(p.operator->()))) {}
	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr(object_monitor_ptr<_D>&& p) noexcept
		: _p(static_cast<_T*>(p.operator->())) 
	{
		_check_valid();
		p._p = nullptr;
	}

public: // implicit conversions
	//inline operator bool() const { return nullptr != _p; } // Ambiguity of function overloading

private: // private assign
	inline object_monitor_ptr& operator =(_T* p) noexcept
	{
		if (_p == p) { return *this; }
		object_monitor_ptr(p).swap(*this);
		return *this;
	}
	inline object_monitor_ptr& operator =(const _T* p) noexcept { *this = const_cast<_T*>(p); return *this; }

	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr& operator =(_D* p) noexcept { *this = static_cast<_T*>(p); return *this; }
	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr& operator =(const _D* p) noexcept { *this = const_cast<_D*>(p); return *this; }

public: // copy and assign
	inline object_monitor_ptr(const object_monitor_ptr& other) noexcept : object_monitor_ptr(other._p) {}
	inline object_monitor_ptr& operator =(const object_monitor_ptr& other) noexcept
	{
		if (other == *this) { return *this; }
		object_monitor_ptr(other).swap(*this);
		return *this;
	}
	inline object_monitor_ptr& operator =(nullptr_t) noexcept
	{
		if (nullptr == _p) { return *this; }
		object_monitor_ptr(nullptr).swap(*this);
		return *this;
	}

	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr& operator =(object_monitor_ptr<_D>& p) noexcept { *this = p.operator->(); return *this; }
	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr& operator =(const object_monitor_ptr<_D>& p) noexcept { *this = p.operator->(); return *this; }

public: // move and assign
	object_monitor_ptr(object_monitor_ptr&& other) noexcept : _p(other._p) { _check_valid(); other._p = nullptr; }
	inline object_monitor_ptr& operator =(object_monitor_ptr&& other) noexcept
	{
		if (other == *this) { return *this; }
		object_monitor_ptr(std::move(other)).swap(*this);
		return *this;
	}

	template<typename _D, enable_if_convertible_int<_D*, _T*> = 0>
	inline object_monitor_ptr& operator =(object_monitor_ptr<_D>&& other)
	{ 
		if (other == *this) { return *this; }
		object_monitor_ptr(std::move(other)).swap(*this);
		return *this;
	}

public: // swap
	inline void swap(object_monitor_ptr& other) noexcept { std::swap(_p, other._p); }

public: // work as raw pointer
	inline _T* operator->() { _check_valid(); return _p; }
	inline const _T* operator->() const { _check_valid(); return _p; }

	inline _T& operator*() { _check_valid(); return *_p; }
	inline const _T& operator*() const { _check_valid(); return *_p; }

public: // comparators as left hand
	inline bool operator ==(const object_monitor_ptr& rhs) const { return _p == rhs._p; }
	inline bool operator !=(const object_monitor_ptr& rhs) const { return _p != rhs._p; }

	template<typename _U>
	inline bool operator ==(const object_monitor_ptr<_U>& rhs) const { return _p == rhs.operator->(); }
	template<typename _U>
	inline bool operator !=(const object_monitor_ptr<_U>& rhs) const { return _p != rhs.operator->(); }

	inline bool operator ==(nullptr_t) const { return _p == nullptr; }
	inline bool operator !=(nullptr_t) const { return _p != nullptr; }

	inline bool operator ==(const _T* p) const { return _p == p; }
	inline bool operator !=(const _T* p) const { return _p != p; }

public: // comparators as right hand
	template<typename _TT>
	friend bool operator ==(nullptr_t, const object_monitor_ptr<_TT>& wp);
	template<typename _TT>
	friend bool operator !=(nullptr_t, const object_monitor_ptr<_TT>& wp);

	template<typename _DD, typename _TT, enable_if_convertible_int<_DD*, _TT*>>
	friend bool operator ==(const _DD* p, const object_monitor_ptr<_TT>& wp);
	template<typename _DD, typename _TT, enable_if_convertible_int<_DD*, _TT*>>
	friend bool operator !=(const _DD* p, const object_monitor_ptr<_TT>& wp);
};

template<typename _T>
int object_monitor_ptr<_T>::type_offset::value = object_monitor_ptr<_T>::type_offset::INVALID_VALUE;

template<typename _TT>
inline bool operator ==(nullptr_t, const object_monitor_ptr<_TT>& wp) { return nullptr == wp._p; }
template<typename _TT>
inline bool operator !=(nullptr_t, const object_monitor_ptr<_TT>& wp) { return nullptr != wp._p; }

template<typename _DD, typename _TT, enable_if_convertible_int<_DD*, _TT*> = 0>
inline bool operator ==(const _DD* p, const object_monitor_ptr<_TT>& wp) { return p == wp._p; }
template<typename _DD, typename _TT, enable_if_convertible_int<_DD*, _TT*> = 0>
inline bool operator !=(const _DD* p, const object_monitor_ptr<_TT>& wp) { return p != wp._p; }

CORE_NAMESPACE_END

namespace std {

	template <typename _T>
	struct remove_pointer<CORE object_monitor_ptr<_T>>
	{
		using type = _T;
	};
}

#endif