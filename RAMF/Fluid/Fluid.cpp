#include "pch.h"
#include "Fluid.h"

#include "LoadedFlow.h"
#include "MultiscaleStochasticFlow.h"

using namespace std;

std::shared_ptr<Fluid> Fluid::Create(const std::string &cfgContent)
{
	using namespace fluid;
	// read config
	std::istringstream iss(cfgContent);
	Config config(iss, "FLUID");
	shared_ptr<Fluid> pf;
	string fluidname = config.Read<string>("type", string("_EMPTY"));

	
	if (fluidname == "LoadedFlow")
	{
		pf = LoadedFlow::makeInstance(config);
	}
	else if (fluidname == "MultiscaleStochasticFlow")
	{
		pf = MultiscaleStochasticFlow::makeInstance(config);
	}
	else if (fluidname == "_EMPTY")
	{
		string errmsg = "Fluid type is not defined. Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}
	else
	{
		string errmsg = "Undefined Fluid name. Check config file.";
		cout << errmsg << endl;
		throw runtime_error(errmsg);
	}
	pf->initialize(cfgContent, config);
	cout << "Fluid: " << fluidname << " created." << endl;
	return pf;
}
