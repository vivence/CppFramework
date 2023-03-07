#include "bug_reporter.h"
#include <iostream>

CORE_NAMESPACE_BEG

void bug_reporter::report(int tag, const char* message)
{
	std::cerr << "bug[" << tag << "] " << message << std::endl;
}

CORE_NAMESPACE_END