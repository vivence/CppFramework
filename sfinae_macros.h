
#ifndef SFINAE_MACROS_H
#define SFINAE_MACROS_H

#include <type_traits>

#define ENABLE_IF_CLASS(_T) typename std::enable_if<std::is_class<_T>::value>::type* = nullptr
#define ENABLE_IF_CONVERTIBLE(_T, _P) typename std::enable_if<std::is_convertible<_T, _P>::value>::type* = nullptr

#endif
