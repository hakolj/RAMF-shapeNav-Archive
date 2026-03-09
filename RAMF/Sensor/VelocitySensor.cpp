#include "pch.h"
#include "VelocitySensor.h"

using namespace std;
using namespace sensor;


//---------------------FluidVelocity2DPolarSensor------------------------------//
void FluidVelocity2DPolarSensor::getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate)
{
	auto itf_am = dynamic_pointer_cast<GetTransRotAble>(amatter);
	auto itf = dynamic_pointer_cast<GetFlowInfoAble>(amatter);
	// vectors3d pos = itf->getPos();

	for (unsigned i = 0; i < amatter->amatternum; i++)
	{
		vec3d pos = itf_am->getPos()[i];
		vec3d fvel = itf->getFluidVel(false)[i];
		// get vec e_r and e_theta in 2d
		vec3d vecr = vec3d::Zero(); // vec in r direction (2d)
		vecr(coordSys[0]) = pos(coordSys[0]) - origin(coordSys[0]);
		vecr(coordSys[1]) = pos(coordSys[1]) - origin(coordSys[1]);
		vecr.normalize();

		vec3d vect = vec3d::Zero(); // vec in theta direction (2d)
		vect(coordSys[0]) = -vecr(coordSys[1]);
		vect(coordSys[1]) = vecr(coordSys[0]);

		// get the velocity in r and theta direction
		double vr = fvel.dot(vecr);
		double vt = fvel.dot(vect);
		newstate[i][0] = vr;
		newstate[i][1] = vt;
	}
}
void FluidVelocity2DPolarSensor::initialize(const std::string &path, const Config &config)
{
	dimension = 2;
	direction = config.Read<int>("Direction", 0);
	coordSys = coord::CartCoordSys(dimension, direction); // in order to change cart coord sys to poloar 2d coord sys
	originFromTask = config.Read<bool>("originFromTask", false);
	if (!originFromTask)
	{
		origin = vec3d::Zero(); // default origin is (0,0,0)
	}
}

void FluidVelocity2DPolarSensor::reset(std::shared_ptr<Task> task)
{
	if (originFromTask)
	{
		auto itf = std::dynamic_pointer_cast<const GetTargetable>(task);
		origin = itf->getTarget();
	}
}