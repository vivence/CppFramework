
#include "object_temp_ref.h"

CORE_NAMESPACE_BEG

#if ENABLE_REF_SAFE_CHECK

typename object_temp_ref_destroyed_pointers::_pointer_set_type object_temp_ref_destroyed_pointers::_s_destroyed_pointers;

#endif // REF_SAFE_CHECK

CORE_NAMESPACE_END