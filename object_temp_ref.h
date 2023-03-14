#ifndef OBJECT_TEMP_REF_H
#define OBJECT_TEMP_REF_H

#include "core.h"
#include "noncopyable.h"
#include "ref.h"
#include <type_traits>

#if REF_SAFE_CHECK
#include "environment.h"
#include "bug_reporter.h"
#include <set>
#endif // REF_SAFE_CHECK

CORE_NAMESPACE_BEG

#if REF_SAFE_CHECK
class object;
class object_temp_ref_destroyed_pointers {
	object_temp_ref_destroyed_pointers() = delete;
	friend class object_factory;
	template<typename _T>
	friend class object_temp_ref;

	using _pointer_set_type = std::set<object*>;
	static _pointer_set_type _s_destroyed_pointers;
	static void add_destroyed_pointer(object* p)
	{
		_s_destroyed_pointers.insert(p);
	}
	static void clear_destroyed_pointers()
	{
		_s_destroyed_pointers.clear();
	}
	static void check_and_report(object* p)
	{
		if (_s_destroyed_pointers.end() != _s_destroyed_pointers.find(p))
		{
			enviroment::get_current_env().get_bug_reporter().report(BUG_TAG_TEMP_REF, "temp_ref access destroyed pointer!");
		}
	}
};
#endif // REF_SAFE_CHECK

template<typename _T>
class object_temp_ref : public ref<_T>, noncopyable {
	static_assert(std::is_base_of<object, _T>::value, "_T must be inherit from object");

	void* operator new(size_t, void* mem) noexcept { return mem; }
	inline void operator delete(void*, void* mem) {}
	friend class object_factory;

public:
	static object_temp_ref null_ref;

private:
	friend class object_factory;
	explicit object_temp_ref(_T* p) : ref<_T>(p) {}
	explicit object_temp_ref(const ref<_T>& obj_ref) : ref<_T>(obj_ref) {}

#if REF_SAFE_CHECK
public:
	const _T* operator->() const override 
	{ 
		return _safe_ref();
	}
	_T* operator->() override
	{
		return _safe_ref();
	}
private:
	_T* _safe_ref() const
	{
		object_temp_ref_destroyed_pointers::check_and_report(static_cast<object*>(ref<_T>::_p));
		return ref<_T>::_p;
	}
#endif // REF_SAFE_CHECK
};

template<typename _T>
object_temp_ref<_T> object_temp_ref<_T>::null_ref(nullptr);

CORE_NAMESPACE_END

#endif