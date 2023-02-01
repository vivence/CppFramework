
#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

#include "core.h"

CORE_NAMESPACE_BEG

class noncopyable {
protected:
    noncopyable() = default;
private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

CORE_NAMESPACE_END

#endif