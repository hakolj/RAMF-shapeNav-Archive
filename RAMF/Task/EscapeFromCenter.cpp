#include "EscapeFromCenter.h"
#include "Fop.h"
#include "Random.h"
using namespace std;

void EscapeFromCenter::initialize(const std::string &path, const Config &config)
{
    isCenterSpawningUsingAMatterInfo = config.Read<bool>("CenterUsingAMInfo", false);
    if (isCenterSpawningUsingAMatterInfo)
    {
        std::cout << "EscapeFromCenter: center position will be set using amatter info." << std::endl;
    }
    else
    {
        std::cout << "EscapeFromCenter: center position will be set using config file." << std::endl;
        centerSpawnPosition = Fop::loadEigenVec<double, 3>(config.Read<string>("CenterSpawnPosition"));
        centerSpawnRange = Fop::loadEigenVec<double, 3>(config.Read<string>("CenterSpawnRange"));
    }

    // if one wants no random center spawning, just set the range to zero
    // need to initialize center in reset()

    terminalBox = config.Read<bool>("terminalBox", false);
    if (terminalBox)
    {
        terminalBoxSize = Fop::loadvec1d<double>(config.Read<string>("terminalBoxSize"));
    }
    penalty = config.Read<double>("penalty", 0.0);
    isDumpTaskInfo = config.Read<bool>("dumpTaskInfo", false);
    distRwCoeff = config.Read<double>("distRwCoeff", 1.0);
    rewardExponential = config.Read<double>("rewardExponential", 1.0);
}

void EscapeFromCenter::checkTermination(const SimuManager &simuManager, std::vector<int> &iterminal)
{
    _checkTerminateCnt++;
    int cnt_prev = _cum_terminatedCnt[_checkTerminateCnt - 1];
    if (_cum_terminatedCnt.size() <= _checkTerminateCnt)
    {

        _cum_terminatedCnt.push_back(cnt_prev);
        std::cout << "Wartning: _cum_terminatedCnt size is not enough, extend it to " << _cum_terminatedCnt.size() << std::endl;
    }
    else
        _cum_terminatedCnt[_checkTerminateCnt] = cnt_prev;

    if (terminalBox)
    {
        auto itf = dynamic_pointer_cast<const GetTransDynamicAble>(simuManager.amatter);
        vectors3d pos(itf->getPos());
        for (unsigned i = 0; i < simuManager.amatter->amatternum; i++)
        {
            if (simuManager.ivalid[i] == 0)
            {
                iterminal[i] = 1; // already terminated
                continue;
            }
            else
            {
                // not terminated, do the checking
                vec3d dist = (pos[i] - center).cwiseAbs() * 2;
                if (dist[0] >= terminalBoxSize[0] || dist[1] >= terminalBoxSize[1] || dist[2] >= terminalBoxSize[2])
                {
                    iterminal[i] = 1; // set the terminal condition
                    _cum_terminatedCnt[_checkTerminateCnt]++;
                }
            }
        }
    }
    else
    {
        return; // no termination condition
    }
}

void EscapeFromCenter::getReward(const SimuManager &simuManager, std::vector<double> &reward)
{
    // auto itf = dynamic_cast<const GetTransRotAble*>(amatter);
    auto itf = dynamic_pointer_cast<const GetTransRotAble>(simuManager.amatter);

    // auto itf = dynamic_pointer_cast<GetTransRotAble>(amatter);
    vectors3d pos(itf->getPos());

    for (unsigned i = 0; i < simuManager.amatter->amatternum; i++)
    {
        if (simuManager.iterminated[i] == 1)
        {
            // this amatter is already in a terminal state, check whether now is a terminal transition
            if (simuManager.iterminalTransit[i] == 0)
            {
                reward[i] = 0.0; // no reward for terminal transition
                continue;
            }
            else
            {
                // std::cout << "a terminal transition is reached" << std::endl;
            }
        }

        // otherwise, calculate the reward

        vec3d r_vec = (pos[i] - center);
        vec3d dist = r_vec.cwiseAbs() * 2;
        double bonus = 0;
        if (terminalBox)
        {
            if (dist[0] >= terminalBoxSize[0] || dist[1] >= terminalBoxSize[1] || dist[2] >= terminalBoxSize[2])
            {
                // reach the terminal box, positive reward
                bonus = 1.0;
            }
        }
        double r = r_vec.norm();
        double delta_r_exp = pow(r, rewardExponential);
        double basicRw = (delta_r_exp - _cache[i]) * distRwCoeff; // distance reward, how much the distance has changed
        // double basicRw = 0.0;

        reward[i] = basicRw - penalty + bonus;
        // reward[i] /= rwNormalizer;
        /*NavigationPoint::_recorder[i] += reward[i];*/
        _recorder[i] += basicRw - penalty + bonus;
        _cache[i] = delta_r_exp;

        // std::cout << "r=" << r << std::endl;
        // std::cout << "basicRw=" << basicRw << std::endl;
        // std::cout << "center=" << center << std::endl;
    }

    return;
} // unfinished
std::vector<double> EscapeFromCenter::getTotalReward(const SimuManager &simuManager)
{
    return _recorder;
}

void EscapeFromCenter::reset(const SimuManager &simuManager)
{
    //_reward = std::vector<double>(amatter->amatternum,0.0);
    _cache = std::vector<double>(simuManager.amatter->amatternum, 0.0);
    // _terminal = std::vector<int>(simuManager.amatter->amatternum, 0);
    _recorder = std::vector<double>(simuManager.amatter->amatternum, 0.0);
    auto itf = dynamic_pointer_cast<const GetTransDynamicAble>(simuManager.amatter);
    // auto itf = dynamic_pointer_cast<GetTransRotAble>(amatter);
    vectors3d pos(itf->getPos());

    _cum_terminatedCnt.resize(simuManager.totalstep + 1, 0);
    _checkTerminateCnt = 0;

    vec3d rangeCenter = vec3d::Zero();
    if (isCenterSpawningUsingAMatterInfo)
    {
        // use the average position of all amatter as the center spawning position
        for (unsigned i = 0; i < simuManager.amatter->amatternum; i++)
        {
            rangeCenter += pos[i];
        }
        rangeCenter /= simuManager.amatter->amatternum;
    }
    else
    {
        // spawn new center position
        double xr = rd::randd(-centerSpawnRange[0] / 2, centerSpawnRange[0] / 2);
        double yr = rd::randd(-centerSpawnRange[1] / 2, centerSpawnRange[1] / 2);
        double zr = rd::randd(-centerSpawnRange[2] / 2, centerSpawnRange[2] / 2);
        rangeCenter = centerSpawnPosition + vec3d(xr, yr, zr);
    }

    center = rangeCenter;
    cout << "center=" << center[0] << " " << center[1] << " " << center[2] << endl;

    for (unsigned i = 0; i < simuManager.amatter->amatternum; i++)
    {
        double r = (pos[i] - center).norm();
        _cache[i] = r;
        // _terminal[i] = 0;
    }
}

void EscapeFromCenter::dumpTaskInfo(std::ostream &os)
{
    if (!isDumpTaskInfo || !terminalBox)
        return;
    for (int i = 0; i < _cum_terminatedCnt.size(); i++)
    {
        os << _cum_terminatedCnt[i] << " ";
    }
    os << std::endl;
}
