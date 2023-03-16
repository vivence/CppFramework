
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

CORE_NAMESPACE_BEG

struct object_ptr_utils;

template<typename _T>
class object_monitor_ptr;

template<typename _T>
class object_monitor_ptr : dis_new {

public: // support external placement new
	void* operator new(size_t, void* mem) noexcept { return mem; }
	inline void operator delete(void*, void* mem) {}

public:
	struct _obj_monitored : public object {
		virtual void* get_this() const { return (void*)this; }
		static const size_t OBJECT_OFFSET = offsetof(_obj_monitored, obj_mem);

		//---info--->
		int ref_count;
		bool destroyed;
		//----------<
		char obj_mem[sizeof(_T)];

		template<typename ..._Args>
		explicit _obj_monitored(_Args&&... args)
			: ref_count(0)
			, destroyed(false)
		{
			auto p = new (&obj_mem[0]) _T(std::forward<_Args>(args)...);
		}
		~_obj_monitored()
		{
			get_obj()->~_T();
		}

		_T* get_obj()
		{
			return reinterpret_cast<_T*>(&obj_mem[0]);
		}

		static _obj_monitored* get(_T* p)
		{
			return reinterpret_cast<_obj_monitored*>(((intptr_t)(p->get_this()) - OBJECT_OFFSET));
		}

		static int add_ref(_T* p)
		{
			if (nullptr == p)
			{
				return 0;
			}
			return ++(get(p)->ref_count);
		}

		template<typename _T>
		static int remove_ref(_T* p)
		{
			if (nullptr == p)
			{
				return 0;
			}
			return --(get(p)->ref_count);
		}
	};

	_T* _p;

public: // typedef
	using value_type = _T;

public: // create and destory
	template<typename ..._Args>
	static object_monitor_ptr create(_Args&&... args)
	{
		auto p_obj_monitored = environment::get_cur_object_factory().new_obj<_obj_monitored>(std::forward<_Args>(args)...);
		object_monitor_ptr ptr(p_obj_monitored->get_obj());
		return std::move(ptr);
	}

	static bool destroy(object_monitor_ptr ptr)
	{
		if (nullptr != ptr._p)
		{
			_obj_monitored::get(ptr._p)->destroyed = true;
		}
		return true;
	}

private:
	void _check_valid() const
	{
		if (nullptr != _p && _obj_monitored::get(_p)->destroyed)
		{
			environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr access destroyed object");
		}
	}

private: // not allowed
	object_monitor_ptr* operator&() = delete;

private: // private constructors
	friend struct object_ptr_utils;
	explicit object_monitor_ptr(_T* p) noexcept : _p(p) { _check_valid(); _obj_monitored::add_ref(_p); }
	explicit object_monitor_ptr(const _T* p) noexcept : object_monitor_ptr(const_cast<_T*>(p)) {}

public: // default constructors
	object_monitor_ptr() : _p(nullptr) {}
	object_monitor_ptr(nullptr_t) : _p(nullptr) {}

public:
	~object_monitor_ptr()
	{
		if (nullptr != _p && 0 >= _obj_monitored::remove_ref(_p))
		{
			auto p_obj_monitored = _obj_monitored::get(_p);
			if (p_obj_monitored->destroyed)
			{
				environment::get_cur_object_factory().delete_obj(p_obj_monitored);
			}
			else
			{
				environment::get_cur_bug_reporter().report(BUG_TAG_MONITOR_PTR, "monitor_ptr leak");
			}
		}
	}

public: // implicit constructors 
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr(object_monitor_ptr<_D>& p) noexcept : object_monitor_ptr(static_cast<_T*>(p.operator->())) {}
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr(const object_monitor_ptr<_D>& p) noexcept : object_monitor_ptr(static_cast<_T*>(const_cast<_D*>(p.operator->()))) {}
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr(object_monitor_ptr<_D>&& p) noexcept 
		: _p(static_cast<_T*>(p.operator->())) 
	{
		_check_valid();
		p._p = nullptr;
	}

public: // implicit conversions
	//operator bool() const { return nullptr != _p; } // Ambiguity of function overloading

private: // private assign
	object_monitor_ptr& operator =(_T* p) noexcept
	{
		if (_p == p) { return *this; }
		object_monitor_ptr(p).swap(*this);
		return *this;
	}
	object_monitor_ptr& operator =(const _T* p) noexcept { *this = const_cast<_T*>(p); return *this; }

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr& operator =(_D* p) noexcept { *this = static_cast<_T*>(p); return *this; }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr& operator =(const _D* p) noexcept { *this = const_cast<_D*>(p); return *this; }

public: // copy and assign
	object_monitor_ptr(const object_monitor_ptr& other) noexcept : object_monitor_ptr(other._p) {}
	object_monitor_ptr& operator =(const object_monitor_ptr& other) noexcept
	{
		if (other == *this) { return *this; }
		object_monitor_ptr(other).swap(*this);
		return *this;
	}
	object_monitor_ptr& operator =(nullptr_t) noexcept
	{
		if (nullptr == _p) { return *this; }
		object_monitor_ptr(nullptr).swap(*this);
		return *this;
	}

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr& operator =(object_monitor_ptr<_D>& p) noexcept { *this = p.operator->(); return *this; }
	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr& operator =(const object_monitor_ptr<_D>& p) noexcept { *this = p.operator->(); return *this; }

public: // move and assign
	object_monitor_ptr(object_monitor_ptr&& other) noexcept : _p(other._p) { _check_valid(); other._p = nullptr; }
	object_monitor_ptr& operator =(object_monitor_ptr&& other) noexcept
	{
		if (other == *this) { return *this; }
		object_monitor_ptr(std::move(other)).swap(*this);
		return *this;
	}

	template<typename _D, ENABLE_IF_CONVERTIBLE(_D*, _T*)>
	object_monitor_ptr& operator =(object_monitor_ptr<_D>&& other)
	{ 
		if (other == *this) { return *this; }
		object_monitor_ptr(std::move(other)).swap(*this);
		return *this;
	}

public: // swap
	void swap(object_monitor_ptr& other) noexcept { std::swap(_p, other._p); }

public: // work as raw pointer
	_T* operator->() { _check_valid(); return _p; }
	const _T* operator->() const { _check_valid(); return _p; }

	_T& operator*() { _check_valid(); return *_p; }
	const _T& operator*() const { _check_valid(); return *_p; }

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

	template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE_DEF(_DD*, _TT*)>
	friend bool operator ==(const _DD* p, const object_monitor_ptr<_TT>& wp);
	template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE_DEF(_DD*, _TT*)>
	friend bool operator !=(const _DD* p, const object_monitor_ptr<_TT>& wp);
};

template<typename _TT>
inline bool operator ==(nullptr_t, const object_monitor_ptr<_TT>& wp) { return nullptr == wp._p; }
template<typename _TT>
inline bool operator !=(nullptr_t, const object_monitor_ptr<_TT>& wp) { return nullptr != wp._p; }

template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE(_DD*, _TT*)>
inline bool operator ==(const _DD* p, const object_monitor_ptr<_TT>& wp) { return p == wp._p; }
template<typename _DD, typename _TT, ENABLE_IF_CONVERTIBLE(_DD*, _TT*)>
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