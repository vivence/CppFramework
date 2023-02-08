
#ifndef BUG_REPORTER_H
#define BUG_REPORTER_H

#include "core.h"

CORE_NAMESPACE_BEG

class bug_reporter {
public:
    virtual ~bug_reporter() { }

public:
    virtual void report(int tag, const char* message);
};

CORE_NAMESPACE_END

#endif