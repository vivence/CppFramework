
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "mem_pool_configable.h"

CORE_NAMESPACE_BEG

using mem_pool = mem_pool_configable<sizeof(size_t), 1 * 1024 * 1024>;
using large_mem_pool = mem_pool_configable<1 * 1024 * 1024, 0>; // cell count always 1

CORE_NAMESPACE_END

#endif