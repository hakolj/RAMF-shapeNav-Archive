#include "OrientationSensor.h"
#include "Fop.h"
#include "MatrixOp.h"
using namespace sensor;

////////////////////////OrientationPolor2DSensor////////////////

void OrientationPolor2DSensor::initialize(const std::string &path, const Config &config)
{
    direction = config.Read<int>("Direction");
    coordSys = coord::CartCoordSys(2, direction);
    originFromTask = config.Read<bool>("originFromTask", false);
    if (!originFromTask)
    {
        origin = vec3d::Zero(); // default origin is (0,0,0)
    }
}

void OrientationPolor2DSensor::getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate)
{
    auto itf = std::dynamic_pointer_cast<GetTransRotAble>(amatter);
    for (unsigned i = 0; i < amatter->amatternum; i++)
    {
        vec3d pos = itf->getPos()[i];
        vec3d p2 = itf->getP2()[i];
        vec3d vecr = vec3d::Zero(); // vec in r direction (2d)
        vec3d vect = vec3d::Zero(); // vec in theta direction (2d)
        vecr(coordSys[0]) = pos(coordSys[0]) - origin(coordSys[0]);
        vecr(coordSys[1]) = pos(coordSys[1]) - origin(coordSys[1]);
        vecr.normalize();

        vect(coordSys[0]) = -vecr(coordSys[1]);
        vect(coordSys[1]) = vecr(coordSys[0]);

        newstate[i][0] = p2.dot(vecr);
        newstate[i][1] = p2.dot(vect);
    }
}

void OrientationPolor2DSensor::reset(std::shared_ptr<Task> task)
{
    if (originFromTask)
    {
        auto itf = std::dynamic_pointer_cast<const GetTargetable>(task);
        origin = itf->getTarget();
    }
}
