
#ifndef OBJECT_H
#define OBJECT_H

#include "core.h"
#include "noncopyable.h"
#include "dis_new.h"

CORE_NAMESPACE_BEG

class object : noncopyable, dis_new {
    void* operator new(size_t, void* mem) noexcept { return mem; }
    inline void operator delete(void*, void* mem){}

    friend class object_factory;

protected:
    object() {};
    virtual ~object() {}

public:
    virtual void* get_this() const = 0;
};

CORE_NAMESPACE_END

#endif