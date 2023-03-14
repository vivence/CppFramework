
#ifndef GENERAL_ENVIRONMENT_H
#define GENERAL_ENVIRONMENT_H

#include "core.h"
#include "environment.h"
#include <memory>

CORE_NAMESPACE_BEG

class general_environment : public environment {
	using _bug_reporter_pointer_type = std::unique_ptr<bug_reporter>;
	_bug_reporter_pointer_type _p_real_bug_reporter;
	bug_reporter& _bug_reporter;
	object_factory& _obj_factory;

public:
	explicit general_environment(environment& prev_env = environment::get_current_env())
		: environment()
		, _p_real_bug_reporter(nullptr)
		, _bug_reporter(prev_env.get_bug_reporter())
		, _obj_factory(prev_env.get_object_factory())
	{ 

	}
	template<typename _BR = bug_reporter>
	explicit general_environment(environment& prev_env = environment::get_current_env())
		: environment()
		, _p_real_bug_reporter(new _BR())
		, _bug_reporter(*_p_real_bug_reporter)
		, _obj_factory(prev_env.get_object_factory())
	{
	}
	virtual ~general_environment() = default;

public:
	virtual bug_reporter& get_bug_reporter() override { return _bug_reporter; }
	virtual object_factory& get_object_factory() override { return _obj_factory; }

public:
	void be_current()
	{
		_s_current_env = this;
	}
};

CORE_NAMESPACE_END

#endif