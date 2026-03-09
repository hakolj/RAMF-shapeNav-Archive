#include "pch.h"
#include "Task.h"


#include "EscapeFromCenter.h"

using namespace std;
shared_ptr<Task> Task::Create(const std::string &cfgContent)
{
	// read config
	std::istringstream iss(cfgContent);
	Config config(iss, "TASK");
	shared_ptr<Task> ptask;
	string taskname = config.Read<string>("type", string("_EMPTY"));

    if (taskname == "EscapeFromCenter")
	{
		ptask = make_shared<EscapeFromCenter>();
	}
	else if (taskname == "_EMPTY")
	{
		std::string errmsg = "Task type is not defined. Check config file.";
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}
	else
	{
		std::string errmsg = "Undefined Task name. Check config file.";
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}
	ptask->initialize(cfgContent, config);
	cout << "Task: " << taskname << " created." << endl;
	return ptask;
}
