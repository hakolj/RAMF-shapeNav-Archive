#include "pch.h"
#include "Actor.h"
#include "Fop.h"

#include "DeformationActor.h"

using namespace std;
using namespace actor;
shared_ptr<Actor> Actor::Create(const std::string &cfgContent)
{
	// read config
	std::istringstream iss(cfgContent);
	Config config(iss, "ACTOR");
	shared_ptr<Actor> pactor;
	string actorname = config.Read("type", string("_EMPTY"));
	pactor = ActorSelector(actorname);
	pactor->initialize(cfgContent, config);
	cout << "Actor: " << actorname << " created." << endl;
	return pactor;
}

std::shared_ptr<Actor> Actor::ActorSelector(std::string actorname)
{
	std::shared_ptr<Actor> pactor;
	if (actorname == "NULLActor")
	{
		pactor = make_shared<NullActor>();
	}
	else if (actorname == "DeformationActor")
	{
		pactor = make_shared<DeformationActor>();
	}
	else if (actorname == "DeformationContActor")
	{
		pactor = make_shared<DeformationContActor>();
	}
	else if (actorname == "_EMPTY")
	{
		string errmsg = "Actor type is not defined. Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}

	else
	{
		string errmsg = "Undefined actor name:" + actorname + " Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}
	return pactor;
}

void Actor::SetActionRescalingFactor(const Config &config)
{
	std::string str{""};
	str = config.Read<std::string>("action.Scale", "");
	if (str != "")
	{
		actionScale = Fop::loadvec1d<double>(str);
		if (actionScale.size() != dim())
		{
			throw(std::runtime_error("Actor: Error: action.Scale does not equal to action dimension!"));
		}
	}
	else
	{
		actionScale = std::vector<double>(dim(), 1.0);
	}

	str = config.Read<std::string>("action.Offset", "");
	if (str != "")
	{
		actionOffset = Fop::loadvec1d<double>(str);
		if (actionOffset.size() != dim())
		{
			throw(std::runtime_error("Actor: Error: action.Offset does not equal to action dimension!"));
		}
	}
	else
	{
		actionOffset = std::vector<double>(dim(), 0.0);
	}
}