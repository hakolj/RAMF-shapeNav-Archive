#include "pch.h"
#include "Sensor.h"

#include "VelocitySensor.h"
#include "VelocityGradSensor.h"
#include "StrainSensor.h"
#include "OrientationSensor.h"
#include "CompositeSensor.h"

using namespace std;
using namespace sensor;
shared_ptr<Sensor> Sensor::Create(std::string cfgContent)
{
	// read config
	std::istringstream iss(cfgContent);
	Config config(iss, "SENSOR");

	shared_ptr<Sensor> psensor;
	string sensorname = config.Read("type", string("_EMPTY"));

	psensor = SensorSelector(sensorname);
	psensor->initialize(cfgContent, config);
	cout << "Sensor: " << sensorname << " created." << endl;
	return psensor;
}

std::shared_ptr<Sensor> Sensor::SensorSelector(std::string sensorname)
{
	shared_ptr<Sensor> psensor;
	if (sensorname == "FluidVelocity2DPolar")
	{
		psensor = make_shared<FluidVelocity2DPolarSensor>();
	}
	else if (sensorname == "OrientationPolor2D")
	{
		psensor = make_shared<OrientationPolor2DSensor>();
	}
	else if (sensorname == "Strain2D")
	{
		psensor = make_shared<Strain2DSensor>();
	}
	
	else if (sensorname == "FluidVorticity2D")
	{
		psensor = make_shared<FluidVorticity2DSensor>();
	}
	else if (sensorname == "CompositeSensor")
	{
		psensor = make_shared<CompositeSensor>();
	}
	else if (sensorname == "_EMPTY")
	{
		std::string errmsg = "Sensor type is not defined. Check config file.";
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}

	else
	{
		std::string errmsg = "Undefined sensor name. Check config file.";
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}
	return psensor;
}