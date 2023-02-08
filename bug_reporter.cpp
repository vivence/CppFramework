#include "bug_reporter.h"
#include <iostream>

CORE_NAMESPACE_BEG

void bug_reporter::report(int tag, const char* message)
{
	std::cout << "bug[" << tag << "] " << message << std::endl;
}

CORE_NAMESPACE_END