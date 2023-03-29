
#ifndef CORE_H
#define CORE_H

#include <cstdint>
#include <cstddef>

#define CORE_NAMESPACE_BEG namespace core{
#define CORE_NAMESPACE_END }
#define CORE_NAMESPACE_USING using namespace core;
#define CORE core::

#define ENABLE_REF_SAFE_CHECK 1
#define ENABLE_MEM_POOL_CLEANUP 1

const int BUG_TAG_MEM_RAW_POOL = 1;
const int BUG_TAG_MEM_POOL = 2;

#if ENABLE_REF_SAFE_CHECK
const int BUG_TAG_TEMP_REF = 10;
const int BUG_TAG_WEAK_REF = 11;
const int BUG_TAG_SHARED_REF = 12;
const int BUG_TAG_OBJECT_FACTORY = 13;
#endif // REF_SAFE_CHECK

const int BUG_TAG_MONITOR_PTR = 21;

#endif
