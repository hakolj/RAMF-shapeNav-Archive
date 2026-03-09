#include "StrainSensor.h"
#include "Fop.h"
#include <Eigen/Core>
using namespace std;
using namespace sensor;

///////////////////////////////////Strain2DSensor///////////////////////////////////
void Strain2DSensor::initialize(const std::string &path, const Config &config)
{
    direction = config.Read<int>("Direction");
    coordSys = coord::CartCoordSys(2, direction);
    islocalFrame = config.Read<bool>("local frame");
}
void Strain2DSensor::getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate)
{
    auto itf = dynamic_pointer_cast<GetFlowInfoAble>(amatter);
    auto itf_am = dynamic_pointer_cast<GetTransRotAble>(amatter);
    for (unsigned i = 0; i < amatter->amatternum; i++)
    {
        vec3d gu = itf->getGradU(islocalFrame)[i];
        vec3d gv = itf->getGradV(islocalFrame)[i];
        vec3d gw = itf->getGradW(islocalFrame)[i];

        // the state is S11, S12. (S22 = -S11 because of incompressibility)
        if (islocalFrame)
        {
            // if in particle frame, the state will always be Sp2p2, Sp2p0.
            newstate[i][0] = gw(2);
            newstate[i][1] = 0.5 * (gu(2) + gw(0));
        }
        else
        {
            if (direction == 0)
            {
                newstate[i][0] = gu(0);
                newstate[i][1] = 0.5 * (gu(1) + gv(0));
            }
            else if (direction == 1)
            {
                newstate[i][0] = gv(1);
                newstate[i][1] = 0.5 * (gv(2) + gw(1));
            }
            else if (direction == 2)
            {
                newstate[i][0] = gw(2);
                newstate[i][1] = 0.5 * (gu(2) + gw(0));
            }
        }
    }
}
