
#ifndef SFINAE_MACROS_H
#define SFINAE_MACROS_H

#include <type_traits>

CORE_NAMESPACE_BEG

template<bool _Test>
using enable_if_int = std::enable_if_t<_Test, int>;

template<typename _T>
using enable_if_is_class_int = enable_if_int<std::is_class<_T>::value>;

template<typename _T, typename _P>
using enable_if_convertible_int = enable_if_int<std::is_convertible<_T, _P>::value>;

template<typename _T, typename _P>
using enable_if_not_convertible_int = enable_if_int<!std::is_convertible<_T, _P>::value>;

template<typename _T>
using enable_if_is_pointer_int = enable_if_int<std::is_pointer<_T>::value>;

template<typename _T>
using enable_if_is_not_pointer_int = enable_if_int<!std::is_pointer<_T>::value>;

template<template<typename> typename T, typename TT>
std::true_type is_base_of_template(T<TT>) {}
template<template<typename> typename T>
std::false_type is_base_of_template(...) {}

CORE_NAMESPACE_END

#define ENABLE_IF_DEF(_V) typename std::enable_if<_V>::type*
#define ENABLE_IF(_V) ENABLE_IF_DEF(_V) = nullptr

#define ENABLE_IF_NOT_DEF(_V) typename std::enable_if<!_V>::type*
#define ENABLE_IF_NOT(_V) ENABLE_IF_NOT_DEF(_V) = nullptr

#define ENABLE_IF_CLASS_DEF(_T) typename std::enable_if<std::is_class<_T>::value>::type*
#define ENABLE_IF_CLASS(_T) ENABLE_IF_CLASS_DEF(_T) = nullptr

#define ENABLE_IF_CONVERTIBLE_DEF(_T, _P) typename std::enable_if<std::is_convertible<_T, _P>::value>::type*
#define ENABLE_IF_CONVERTIBLE(_T, _P) ENABLE_IF_CONVERTIBLE_DEF(_T, _P) = nullptr

#define ENABLE_IF_NOT_CONVERTIBLE_DEF(_T, _P) typename std::enable_if<!std::is_convertible<_T, _P>::value>::type*
#define ENABLE_IF_NOT_CONVERTIBLE(_T, _P) ENABLE_IF_NOT_CONVERTIBLE_DEF(_T, _P) = nullptr

#define ENABLE_IF_RAW_POINTER_DEF(_T) typename std::enable_if<std::is_pointer<_T>::value>::type*
#define ENABLE_IF_RAW_POINTER(_T) ENABLE_IF_RAW_POINTER_DEF(_T) = nullptr

#define ENABLE_IF_NOT_RAW_POINTER_DEF(_T) typename std::enable_if<!std::is_pointer<_T>::value>::type*
#define ENABLE_IF_NOT_RAW_POINTER(_T) ENABLE_IF_NOT_RAW_POINTER_DEF(_T) = nullptr

#endif
