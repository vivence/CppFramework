
#ifndef DIS_NEW_H
#define DIS_NEW_H

#include "core.h"

CORE_NAMESPACE_BEG

class dis_new {
protected:
    void* operator new(size_t) = delete;
    void operator delete(void*) = delete;
};

CORE_NAMESPACE_END

#endif