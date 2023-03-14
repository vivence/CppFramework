
#ifndef ENVIRNOMENT_H
#define ENVIRNOMENT_H

#include "core.h"
#include "noncopyable.h"

CORE_NAMESPACE_BEG

class bug_reporter;
class object_factory;

class enviroment : noncopyable {
    static bool _init_default_enviroment();

public:
    static enviroment& get_current_env()
    { 
        static bool inited = _init_default_enviroment();
        return *_s_current_env; 
    }

protected:
    static enviroment* _s_current_env;

protected:
    enviroment() { _s_current_env = this; }
    virtual ~enviroment() = default;

public:
    virtual bug_reporter& get_bug_reporter() = 0;
    virtual object_factory& get_object_factory() = 0;
};

CORE_NAMESPACE_END

#endif