#include "pch.h"
#include "ActiveMatter.h"
#include "InertialessSwimmer.h"
#include "Config.h"
#include "PointVehicle.h"
#include "Balloon.h"
#include "HomogeneousSwarm.h"
#include "Airship.h"
#include "AirshipSwarm.h"

using namespace std;

ActiveMatter::ActiveMatter(unsigned amatternum) : amatternum(amatternum){};

shared_ptr<ActiveMatter> ActiveMatter::Create(const std::string &cfgContent)
{
	// read config
	std::istringstream iss(cfgContent);
	Config config(iss, "ACTIVEMATTER");
	shared_ptr<ActiveMatter> pamatter;
	cout << "pamatter use count = " << pamatter.use_count() << endl;

	string amattername = config.Read("type", string("_EMPTY"));
	unsigned amatternum = config.Read<unsigned>("amatternum");
	pamatter = ActiveMatterSelector(amattername, amatternum);

	pamatter->initialize(cfgContent, config);
	cout << "ActiveMatter: " << amattername << " created." << endl;

	return pamatter;
}

std::shared_ptr<ActiveMatter> ActiveMatter::ActiveMatterSelector(const std::string &amattername, int amatternum)
{
	shared_ptr<ActiveMatter> pamatter;
	if (amattername == "InertialessSwimmer")
	{
		pamatter = make_shared<InertialessSwimmer>(amatternum);
	}
	else if (amattername == "PointVehicle")
	{
		pamatter = make_shared<PointVehicle>(amatternum);
	}
	else if (amattername == "Balloon")
	{
		pamatter = make_shared<Balloon>(amatternum);
	}
	else if (amattername == "HomogeneousSwarm")
	{
		pamatter = make_shared<HomogeneousSwarm>(amatternum);
	}
	else if (amattername == "Airship")
	{
		pamatter = make_shared<Airship>(amatternum);
	}
	else if (amattername == "AirshipSwarm")
	{
		pamatter = make_shared<AirshipSwarm>(amatternum);
	}
	else if (amattername == "_EMPTY")
	{
		string errmsg = "ActiveMatter type is not defined. Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}
	else
	{
		string errmsg = "Undefined activematter name: " + amattername + " Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}

	return pamatter;
}
