
#ifndef OBJECT_PTR_UTILS_H
#define OBJECT_PTR_UTILS_H

#include "core.h"
#include "utils.h"
#include "object_monitor_ptr.h"
#include "sfinae_macros.h"
#include "environment.h"
#include "object_factory.h"
#include <type_traits>

CORE_NAMESPACE_BEG

//template <int _V>
//using int_constant = std::integral_constant<int, _V>;
//
//template<typename _T>
//struct offset_of_object : public int_constant<offsetof(object, real_mem) - offsetof(_T, real_mem)> {};

template<typename _T>
struct is_gc_disabled : public false_type {};

template<typename _T>
struct is_monitor_ptr : public false_type {};
template<typename _T>
struct is_monitor_ptr<object_monitor_ptr<_T>> : public true_type {};

template<typename _T>
struct is_mptr_type : public bool_constant<is_monitor_ptr<_T>::value> {};

template<typename _T>
struct is_object_ptr_type : public bool_constant<std::is_base_of<object, typename std::remove_pointer<_T>::type>::value> {};

template<typename _T>
struct is_gc_disabled_ptr : public bool_constant<is_mptr_type<_T>::value || is_object_ptr_type<_T>::value> {};

#define EnableMPtr 1 // enable monitor ptr

#if EnableMPtr
template<typename _T>
using mptr_type = object_monitor_ptr<_T>;
#define GetRawPtr(ptr) CORE object_ptr_utils::get_raw(ptr);
#else
template<typename T>
using mptr_type = T*;
#define GetRawPtr(ptr) ptr
#endif

struct object_ptr_utils {
	template<typename _T, typename ..._Args, ENABLE_IF(is_gc_disabled<_T>::value)>
	inline static mptr_type<_T> create(_Args&&... args)
	{
#if EnableMPtr
		return std::move(mptr_type<_T>::create(std::forward<_Args>(args)...));
#else
		return environment::get_cur_object_factory().new_obj<_T>(std::forward<_Args>(args)...);
#endif
	}

	template<typename _P, typename ..._Args, ENABLE_IF(is_gc_disabled_ptr<_P>::value)>
	inline static _P create_by_ptr_type(_Args&&... args)
	{
#if EnableMPtr
		return std::move(_P::create(std::forward<_Args>(args)...));
#else
		return environment::get_cur_object_factory().new_obj<typename std::remove_pointer<_P>::type>(std::forward<_Args>(args)...);
#endif
	}

	template<typename _P, ENABLE_IF(is_gc_disabled_ptr<_P>::value)>
	inline static bool destroy(_P ptr)
	{
		
#if EnableMPtr
		return _P::destroy(ptr);
#else
		return environment::get_cur_object_factory().delete_obj(ptr);
#endif
	}

	template<typename _T>
	inline static _T* get_raw(object_monitor_ptr<_T> ptr)
	{
		return ptr.operator->();
	}
	template<typename _T>
	inline static _T* get_raw(_T* ptr)
	{
		return ptr;
	}
};

struct object_empty_class {};
#if EnableMPtr
struct object_empty_class_1 {};
#define NewBindableClass(...) \
BindableClass(__VA_ARGS__), \
public std::conditional<std::is_base_of<CORE object, __VA_ARGS__>::value, CORE object_empty_class, CORE object>::type, \
public std::conditional<std::is_base_of<CORE info_for_monitor_ptr, __VA_ARGS__>::value, CORE object_empty_class_1, CORE info_for_monitor_ptr>::type
#else
#define NewBindableClass(...) \
BindableClass(__VA_ARGS__), \
public std::conditional<std::is_base_of<CORE object, __VA_ARGS__>::value, CORE object_empty_class, CORE object>::type
#endif

#define DeclareDisableGC(T) \
protected: \
virtual ~T() {} \
public: \
void* get_this() const override { return (void*)this; } \
void* operator new(size_t, void* mem) noexcept { return mem; } \
inline void operator delete(void*, void* mem) {} \
void* operator new(size_t) noexcept { return nullptr; }; \
void operator delete(void*) {}; \
private:

#define DefineDisableGC(T) \
CORE_NAMESPACE_BEG \
template<> \
struct is_gc_disabled<T> : public true_type {}; \
CORE_NAMESPACE_END 

#define MPtrDeclareClass(T) class T; \
						   using T##_P = CORE mptr_type<T> 

CORE_NAMESPACE_END

#endif