#ifndef VELOCITYSENSOR_H
#define VELOCITYSENSOR_H

#include "Sensor.h"
#include "Fop.h"
#include "CoordSys.h"
namespace sensor
{
	class FluidVelocity2DPolarSensor : public Sensor
	{
	public:
		coord::CartCoordSys coordSys;
		int dimension = 2;

		// dim = 1: direction = 0, 1, 2 -> pos: x, y, z;
		// dim = 2: direction = 0, 1, 2 -> pos yz, zx, xy
		// dim = 3: direction = 0, 1, 2 -> pos: xyz;
		int direction = 2;

		virtual int dim() const
		{
			return dimension;
		};

		virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate);
		virtual void initialize(const std::string &path, const Config &config);
		virtual void reset(std::shared_ptr<Task> task) override;
		FluidVelocity2DPolarSensor() {};

	protected:
		bool originFromTask; // whether the origin (0,0) is from task
		vec3d origin;		 // the origin of the coordinate system, if not from task, it is (0,0,0)
	};
}
#endif