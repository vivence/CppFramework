
#ifndef OBJECT_PTR_UTILS_H
#define OBJECT_PTR_UTILS_H

#include "core.h"
#include "object_shared_ptr.h"
#include "sfinae_macros.h"
#include <type_traits>

CORE_NAMESPACE_BEG

struct object_ptr_utils {
	template<typename _T> struct is_shared_ptr : std::false_type {};
	template<typename _T> struct is_shared_ptr<object_shared_ptr<_T>> : std::true_type {};

	//----------- ÊÊÅäMLÏîÄ¿ --------------->
	template<class _T>
	inline static object_shared_ptr<_T> to_ptr(_T* p)
	{
		return object_shared_ptr<_T>(p);
	}
	template<class _T>
	inline static object_shared_ptr<_T> to_ptr(const _T* p)
	{
		return object_shared_ptr<_T>(p);
	}
	template<class _T>
	inline static object_shared_ptr<_T> to_ptr(_T* p, int extra_ref_count)
	{
		return object_shared_ptr<_T>(p, extra_ref_count);
	}
	template<class _T>
	inline static object_shared_ptr<_T> to_ptr(const _T* p, int extra_ref_count)
	{
		return object_shared_ptr<_T>(p, extra_ref_count);
	}
	template<class _T>
	inline static _T** get_addr(object_shared_ptr<_T>& p)
	{
		return &p._p;
	}
	//--------------------------------------<

private:
	template<typename _D, typename _T>
	inline static object_shared_ptr<typename _D::value_type> _dync_cast(_T* p)
	{
		return object_shared_ptr<typename _D::value_type>(dynamic_cast<typename _D::value_type*>(p));
	}

	template<typename _D, typename _T>
	inline static object_shared_ptr<typename _D::value_type> _dync_cast(const _T* p)
	{
		return _dync_cast<_D>(const_cast<_T*>(p));
	}

public:
	template<typename _D, typename _T>
	inline static object_shared_ptr<typename _D::value_type> dync_cast(object_shared_ptr<_T>& p)
	{
		return _dync_cast<_D>(p.operator->());
	}

	template<typename _D, typename _T>
	inline static object_shared_ptr<typename _D::value_type> dync_cast(const object_shared_ptr<_T>& p)
	{
		return _dync_cast<_D>(const_cast<_T*>(p.operator->()));
	}
};

CORE_NAMESPACE_END

#endif
