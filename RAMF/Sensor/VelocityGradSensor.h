#ifndef VELOCITYGRADSENSOR_H
#define VELOCITYGRADSENSOR_H

#include "Sensor.h"
#include "CoordSys.h"
namespace sensor
{
	class FluidVorticity2DSensor : public Sensor
	{
	public:
		int dimension = 2;
		int direction = 0;
		coord::CartCoordSys coordSys;

		virtual int dim() const
		{
			return 1;
		};

		virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate) override;
		virtual void initialize(const std::string &cfgContent, const Config &config) override;
		FluidVorticity2DSensor() {};
	};
}
#endif // !1