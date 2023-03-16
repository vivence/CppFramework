
#ifndef OBJECT_H
#define OBJECT_H

#include "core.h"
#include "noncopyable.h"
#include "dis_new.h"

CORE_NAMESPACE_BEG

//----------- 适配ML项目 --------------->
namespace _container_details {
	template<typename _T, bool has_default_ctor>
    struct _default_construct;
}
//--------------------------------------<

class object : noncopyable, dis_new {
    void* operator new(size_t, void* mem) noexcept { return mem; }
    inline void operator delete(void*, void* mem){}
	//----------- 适配ML项目 --------------->
	template<typename _T, bool has_default_ctor>
	friend struct _container_details::_default_construct;
	//--------------------------------------<

    friend class object_factory;

protected:
    object() {};
    virtual ~object() {}

private:
    virtual void* get_this() const = 0;
};

CORE_NAMESPACE_END

#endif