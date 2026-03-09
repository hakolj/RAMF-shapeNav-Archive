#ifndef STRAINRATESENSOR_H
#define STRAINRATESENSOR_H
#include "Sensor.h"
#include "CoordSys.h"
namespace sensor
{


 class Strain2DSensor : public Sensor
    {
    public:
        int direction = 0;
        coord::CartCoordSys coordSys;
        bool islocalFrame = false; // whether the strain is measured at local frame of the agent
        virtual inline int dim() const override { return 2; }
        virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate) override;
        virtual void initialize(const std::string &path, const Config &config) override;
    };


}
#endif
