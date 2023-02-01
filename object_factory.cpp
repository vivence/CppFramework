
#include "object_factory.h"

CORE_NAMESPACE_BEG

void object_factory::delete_obj(object* p_obj)
{
	p_obj->~object();
}

CORE_NAMESPACE_END