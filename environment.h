
#ifndef ENVIRNOMENT_H
#define ENVIRNOMENT_H

#include "core.h"
#include "noncopyable.h"

CORE_NAMESPACE_BEG

class bug_reporter;
class object_factory;

class environment : noncopyable {
    static bool _init_default_enviroment();

public:
    inline static environment& get_current_env()
    { 
        static bool inited = _init_default_enviroment();
        return *_s_current_env; 
	}
    inline static bug_reporter& get_cur_bug_reporter() { return get_current_env().get_bug_reporter(); }
    inline static object_factory& get_cur_object_factory() { return get_current_env().get_object_factory(); }

protected:
    static environment* _s_current_env;

protected:
    environment() { _s_current_env = this; }
    virtual ~environment() = default;

public:
    virtual bug_reporter& get_bug_reporter() = 0;
    virtual object_factory& get_object_factory() = 0;
};

CORE_NAMESPACE_END

#endif