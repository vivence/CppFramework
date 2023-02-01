#ifndef OBJECT_TEMP_REF_H
#define OBJECT_TEMP_REF_H

#include "core.h"
#include "noncopyable.h"
#include "ref.h"

CORE_NAMESPACE_BEG

template<typename _T>
class object_temp_ref : public ref<_T>, noncopyable {
	void* operator new(size_t, void* mem) noexcept { return mem; }
	inline void operator delete(void*, void* mem) {}
	friend class object_factory;

public:
	static object_temp_ref null_ref;

private:
	friend class object_factory;
	explicit object_temp_ref(_T* p) : ref<_T>(p) {}

private:

};

template<typename _T>
object_temp_ref<_T> object_temp_ref<_T>::null_ref(nullptr);

CORE_NAMESPACE_END

#endif