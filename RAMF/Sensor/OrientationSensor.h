#ifndef ORIENTATIONSENSOR_H
#define ORIENTATIONSENSOR_H
#include "Sensor.h"
#include "CoordSys.h"
namespace sensor
{
    

    class OrientationPolor2DSensor : public Sensor
    {
    public:
        int direction = 0;
        coord::CartCoordSys coordSys;
        virtual inline int dim() const { return 2; }
        virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate);
        virtual void initialize(const std::string &path, const Config &config);
        virtual void reset(std::shared_ptr<Task> task) override;

    protected:
        bool originFromTask; // whether the origin ()
        vec3d origin;        // the origin of the coordinate system, if not from task, it is (0,0,0)
    };

} // namespace sensor

#endif
