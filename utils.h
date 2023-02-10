
#ifndef UTILS_H
#define UTILS_H

#include "core.h"

CORE_NAMESPACE_BEG

template<size_t _V1, size_t _V2, bool _BMax>
struct return_max {};
template<size_t _V1, size_t _V2>
struct return_max<_V1, _V2, true> { enum { Value = _V1 }; };
template<size_t _V1, size_t _V2>
struct return_max<_V1, _V2, false> { enum { Value = _V2 }; };

template<size_t _V1, size_t _V2>
struct get_max {
	enum { Value = return_max<_V1, _V2, (_V1 > _V2)>::Value };
};

CORE_NAMESPACE_END

#endif
