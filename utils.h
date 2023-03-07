
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

template<typename _T, typename _P>
class cast_utils {
	template<bool _IsConvertible>
	struct _caster {
		static _P* cast(_T* p)
		{
			return dynamic_cast<_P*>(p);
		}
		static const _P* cast(const _T* p)
		{
			return dynamic_cast<const _P*>(p);
		}
		static _P& cast(_T& v)
		{
			return dynamic_cast<_P&>(v);
		}
		static const _P& cast(const _T& v)
		{
			return dynamic_cast<const _P&>(v);
		}
	};
	template<>
	struct _caster<true> {
		static _P* cast(_T* p)
		{
			return static_cast<_P*>(p);
		}
		static const _P* cast(const _T* p)
		{
			return static_cast<const _P*>(p);
		}
		static _P& cast(_T& v)
		{
			return static_cast<_P&>(v);
		}
		static const _P& cast(const _T& v)
		{
			return static_cast<const _P&>(v);
		}
	};
public:
	static _P* cast(_T* p)
	{
		return _caster<std::is_convertible<_T, _P>::value>::cast(p);
	}
	static _P& cast(_T& v)
	{
		return _caster<std::is_convertible<_T, _P>::value>::cast(v);
	}
};


CORE_NAMESPACE_END

#endif
