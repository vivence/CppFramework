
#ifndef UTILS_H
#define UTILS_H

#include "core.h"
#include <string>
#include <sstream>

CORE_NAMESPACE_BEG

struct string_format_utils {
	static size_t format_size(std::stringstream& ss, size_t size, size_t formatSize, const char* suffix);
	static std::string format_size(size_t size);
	static void format_count(std::stringstream& ss, size_t size);
	static std::string format_count(size_t size);
};

CORE_NAMESPACE_END

#endif
