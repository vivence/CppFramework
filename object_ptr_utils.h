
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

	template<class _T>
	inline static object_shared_ptr<_T> to_ptr(_T* p)
	{
		return object_shared_ptr<_T>(p);
	}

	//template<typename _T, ENABLE_IF_NOT_RAW_POINTER(_T)>
	//inline static _T* get_raw(object_shared_ptr<_T>& p)
	//{
	//	return p.operator->();
	//}

	//template<typename _T, ENABLE_IF_NOT_RAW_POINTER(_T)>
	//inline static _T* get_raw(const object_shared_ptr<_T>& p)
	//{
	//	return const_cast<_T*>(p.operator->());
	//}

	//template<typename _T>
	//inline static _T* get_raw(_T* p)
	//{
	//	return p;
	//}

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

	//template<typename _D, typename _T, ENABLE_IF_RAW_POINTER(_D)>
	//inline static _D dync_cast(object_shared_ptr<_T>& p)
	//{
	//	return dynamic_cast<_D>(p.operator->());
	//}

	//template<typename _D, typename _T, ENABLE_IF_RAW_POINTER(_D)>
	//inline static _D dync_cast(const object_shared_ptr<_T>& p)
	//{
	//	return dync_cast<_D>(const_cast<_T*>(p.operator->()));
	//}

	//template<typename _D, typename _T, ENABLE_IF_RAW_POINTER(_D)>
	//inline static _D dync_cast(_T* p)
	//{
	//	return dynamic_cast<_D>(p);
	//}

	//template<typename _D, typename _T, ENABLE_IF_RAW_POINTER(_D)>
	//inline static const _D dync_cast(const _T* p)
	//{
	//	return dynamic_cast<const _D>(p);
	//}
};

CORE_NAMESPACE_END

#endif
