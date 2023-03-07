
#ifndef SFINAE_MACROS_H
#define SFINAE_MACROS_H

#include <type_traits>

#define ENABLE_IF_DEF(_V) typename std::enable_if<_V>::type*
#define ENABLE_IF(_V) ENABLE_IF_DEF(_V) = nullptr

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
