#ifndef OBJECT_REF_UTILS_H
#define OBJECT_REF_UTILS_H

#include "core.h"
#include "sfinae_macros.h"
#include "environment.h"
#include "object_temp_ref.h"
#include "object_weak_ref.h"
#include "object_shared_ref.h"

CORE_NAMESPACE_BEG

struct object_ref_utils {

	//---------- cast ------------>

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_B>& cast(object_temp_ref<_D>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_D>& cast(object_temp_ref<_B>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_B> cast(object_weak_ref<_D> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_D> cast(object_weak_ref<_B> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_B> cast(object_shared_ref<_D> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_D> cast(object_shared_ref<_B> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	//----------------------------<


	//---------- convert ------------>

	// temp_ref --> weak_ref
	template<typename _T>
	static object_weak_ref<_T> to_weak(object_temp_ref<_T>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_B> to_weak(object_temp_ref<_D>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_D> to_weak(object_temp_ref<_B>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	// shared_ref --> weak_ref
	template<typename _T>
	static object_weak_ref<_T> to_weak(object_shared_ref<_T>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_B> to_weak(object_shared_ref<_D>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_weak_ref<_D> to_weak(object_shared_ref<_B>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_weak_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	// weak_ref --> temp_ref
	template<typename _T>
	static object_temp_ref<_T>& to_temp(object_weak_ref<_T> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_B>& to_temp(object_weak_ref<_D> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_D>& to_temp(object_weak_ref<_B> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	// shared_ref --> temp_ref
	template<typename _T>
	static object_temp_ref<_T>& to_temp(object_shared_ref<_T> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_B>& to_temp(object_shared_ref<_D> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_temp_ref<_D>& to_temp(object_shared_ref<_B> obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_temp_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	// temp_ref --> shared_ref
	template<typename _T>
	static object_shared_ref<_T> to_weak(object_temp_ref<_T>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_B> to_weak(object_temp_ref<_D>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_D> to_weak(object_temp_ref<_B>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	// weak_ref --> shared_ref
	template<typename _T>
	static object_shared_ref<_T> to_weak(object_weak_ref<_T>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(obj_ref._p);
	}

	template<typename _B, typename _D, ENABLE_IF_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_B> to_weak(object_weak_ref<_D>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(static_cast<_B*>(obj_ref._p));
	}

	template<typename _D, typename _B, ENABLE_IF_NOT_CONVERTIBLE(_D, _B)>
	static object_shared_ref<_D> to_weak(object_weak_ref<_B>& obj_ref, object_factory& obj_factory = _get_object_factory())
	{
		return obj_factory.get_shared_ref(dynamic_cast<_D*>(obj_ref._p));
	}

	//-------------------------------<

private:
	inline static object_factory& _get_object_factory()
	{
		return environment::get_cur_object_factory();
	}
};

CORE_NAMESPACE_END

#endif