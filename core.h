
#ifndef CORE_H
#define CORE_H

#include <cstddef>

#define CORE_NAMESPACE_BEG namespace core{
#define CORE_NAMESPACE_END }
#define CORE_NAMESPACE_USING using namespace core;
#define CORE core::

#define REF_SAFE_CHECK 1 // ≤‚ ‘”√

#if REF_SAFE_CHECK

const int BUG_TAG_TEMP_REF = 1;
const int BUG_TAG_WEAK_REF = 2;

#endif // REF_SAFE_CHECK

#endif
