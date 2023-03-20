#ifndef SRC_CODE_LOCATION_H
#define SRC_CODE_LOCATION_H

#include "core.h"

CORE_NAMESPACE_BEG

struct src_code_location {
	int line = -1;
	int column = -1;
	const char* file = "";
	const char* function = "";

	static src_code_location current(
		const int _Line_ = __builtin_LINE(),
		const int _Column_ = __builtin_COLUMN(), 
		const char* const _File_ = __builtin_FILE(),
		const char* const _Function_ = __builtin_FUNCTION()) noexcept 
	{
		src_code_location cur;
		cur.line = _Line_;
		cur.column = _Column_;
		cur.file = _File_;
		cur.function = _Function_;
		return cur;
	}
};

CORE_NAMESPACE_END

#endif