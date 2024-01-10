
#ifndef OBJECT_SHARED_REF_H
#define OBJECT_SHARED_REF_H

#include "core.h"
#include "noncopyable.h"
#include "ref.h"
#include <type_traits>
#include <utility>

#if ENABLE_REF_SAFE_CHECK
#include "environment.h"
#include "bug_reporter.h"
#endif // REF_SAFE_CHECK

CORE_NAMESPACE_BEG

class object;

class support_shared_ref : noncopyable {
	friend class object_factory;
	template<typename _T, typename _Deleter>
	friend class object_shared_ref;

	int _ref_count;

protected:
	inline support_shared_ref() : _ref_count(0) {}
	virtual ~support_shared_ref() {}
};

template<typename _T, typename _Deleter>
class object_shared_ref final : public ref<_T> {
	static_assert(std::is_base_of<support_shared_ref, _T>::value, "_T must be inherit from support_shared_ref");

	_T* _p;

public:
	static object_shared_ref null_ref;

private:
	friend class object_factory;
	friend struct object_ref_utils;
	inline explicit object_shared_ref(_T* p = nullptr) : ref<_T>(p) { _add_ref(); }
	inline explicit object_shared_ref(const ref<_T>& obj_ref) : ref<_T>(obj_ref) { _add_ref(); }

public:
	~object_shared_ref()
	{
		if (nullptr != _p && 0 >= _remove_ref())
		{
			_Deleter::delete_obj(_p);
		}
	}

public:
	inline object_shared_ref(const object_shared_ref& other) : _p(other._p) { _add_ref(); }
	inline object_shared_ref(object_shared_ref&& other) : _p(other._p) { other._p = nullptr; }

	inline object_shared_ref& operator=(const object_shared_ref& other)
	{
		if (other._p != _p)
		{
			object_shared_ref(other).swap(*this);
		}
		return *this;
	}
	inline object_shared_ref& operator=(object_shared_ref&& other)
	{
		if (other._p != _p)
		{
			object_shared_ref(std::move(other)).swap(*this);
		}
		return *this;
	}

public:
	inline void swap(object_shared_ref& other)
	{
		std::swap(_p, other._p);
	}

private:
	inline int _add_ref()
	{
		if (nullptr == _p)
		{
			return 0;
		}
		return ++(static_cast<support_shared_ref*>(_p)->_ref_count);
	}
	inline int _remove_ref()
	{
		if (nullptr == _p)
		{
			return 0;
		}
		return --(static_cast<support_shared_ref*>(_p)->_ref_count);
	}

#if ENABLE_REF_SAFE_CHECK
public:
	inline const _T* operator->() const override
	{
		return _safe_ref();
	}
	inline _T* operator->() override
	{
		return _safe_ref();
	}
private:
	inline _T* _safe_ref() const
	{
		if (nullptr == *this)
		{
			environment::get_current_env().get_bug_reporter().report(BUG_TAG_SHARED_REF, "shared_ref is nullptr!");
		}
		return _p;
	}
#endif // REF_SAFE_CHECK
};

template<typename _T, typename _Deleter>
object_shared_ref<_T, _Deleter> object_shared_ref<_T, _Deleter>::null_ref(nullptr);

CORE_NAMESPACE_END

#endif