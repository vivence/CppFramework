
#ifndef UTILS_H
#define UTILS_H

#include "core.h"
#include <string>
#include <sstream>

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

template<size_t _V>
struct zero_or_one {
	enum { Value = 1 };
};

template<>
struct zero_or_one<0> {
	enum { Value = 0 };
};

struct string_format_utils {
	static size_t format_size(std::stringstream& ss, size_t size, size_t formatSize, const char* suffix);
	static std::string format_size(size_t size);
	static void format_count(std::stringstream& ss, size_t size);
	static std::string format_count(size_t size);
};

CORE_NAMESPACE_END

#endif
