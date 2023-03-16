
#ifndef OBJECT_PTR_UTILS_H
#define OBJECT_PTR_UTILS_H

#include "core.h"
#include "object_monitor_ptr.h"
#include "sfinae_macros.h"
#include "environment.h"
#include "object_factory.h"
#include <type_traits>

CORE_NAMESPACE_BEG

struct object_ptr_utils {
};


#define EnableMPtr 1
#define EnableObjFactory EnableMPtr + 1

#if EnableMPtr
template<typename _T>
using MPtr = object_monitor_ptr<_T>;
#define GetRawPtr(p) p.operator->()
#else
template<typename T>
using MPtr = T*;
#define GetRawPtr(p) (p)
#endif

#define NewDeclareClass(T) class T; \
						   using T##_P = CORE MPtr<T> 

#endif

CORE_NAMESPACE_END