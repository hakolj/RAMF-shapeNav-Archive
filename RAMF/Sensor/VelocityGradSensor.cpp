#include "VelocityGradSensor.h"

using namespace std;
using namespace sensor;

//---------------------FluidVorticity2DSensor------------------------------//
void FluidVorticity2DSensor::initialize(const std::string &cfgContent, const Config &config)
{
	direction = config.Read<int>("Direction", 2);
	coordSys = coord::CartCoordSys(2, direction);
}

void FluidVorticity2DSensor::getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate)
{
	auto itf = dynamic_pointer_cast<GetFlowInfoAble>(amatter);
	for (unsigned i = 0; i < amatter->amatternum; i++)
	{
		// fluid vel grad in inertial frame
		vec3d gradU = itf->getGradU(false)[i];
		vec3d gradV = itf->getGradV(false)[i];
		vec3d gradW = itf->getGradW(false)[i];

		vec3d vorticity = vec3d::Zero();
		vorticity(0) = gradW(1) - gradV(2);
		vorticity(1) = gradU(2) - gradW(0);
		vorticity(2) = gradV(0) - gradU(1);

		// normal direction
		int normDir = (coordSys[1] + 1) % 3;
		newstate[i][0] = vorticity[normDir];
	}
	return;
}