#include "environment.h"
#include "bug_reporter.h"
#include "object_factory.h"

CORE_NAMESPACE_BEG

class default_enviroment : public environment {
	bug_reporter _bug_reporter;
	object_factory _obj_factory;

public:
	bug_reporter& get_bug_reporter() override { return _bug_reporter; }
	object_factory& get_object_factory() override { return _obj_factory; }

public:
	static default_enviroment& get_singleton()
	{
		static default_enviroment instance;
		return instance;
	}
};

environment* environment::_s_current_env = nullptr;

bool environment::_init_default_enviroment()
{
	if (nullptr != _s_current_env)
	{
		return false;
	}
	_s_current_env = &default_enviroment::get_singleton();
	return true;
}

CORE_NAMESPACE_END