
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

class _cast_utils {
protected:
	template<typename _T, typename _P, bool _IsConvertible>
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
	template<typename _T, typename _P>
	struct _caster<_T, _P, true> {
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
};

template<typename _T, typename _P>
class cast_utils : _cast_utils {
	
public:
	static constexpr bool IsConvertible = std::is_convertible<_T*, _P*>::value;
	static _P* cast(_T* p)
	{
		return _caster<_T, _P, IsConvertible>::cast(p);
	}
	static _P& cast(_T& v)
	{
		return _caster<_T, _P, IsConvertible>::cast(v);
	}
};

template <class _Ty, _Ty _Val>
struct integral_constant {
	static constexpr _Ty value = _Val;

	using value_type = _Ty;
	using type = integral_constant;

	constexpr operator value_type() const noexcept {
		return value;
	}

	constexpr value_type operator()() const noexcept {
		return value;
	}
};

template <bool _Val>
using bool_constant = integral_constant<bool, _Val>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

CORE_NAMESPACE_END

#endif
