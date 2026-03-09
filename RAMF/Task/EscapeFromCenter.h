#pragma once
#ifndef ESCAPEFROMCENTER_H
#define ESCAPEFROMCENTER_H

// #include "pch.h"
#include "Task.h"
#include "Fluid.h"
#include "Fop.h"
#include "TaskInterface.h"
class EscapeFromCenter : public Task, public GetTargetable
{
public:
    vec3d center;
    std::vector<double> _cache;
    // std::vector<int> _terminal;

protected:
    std::vector<double> _recorder;       // to record the initial position. used for calculating total _reward
                                         // std::vector<double> _reward;
    bool terminalBox = false;            // whether the task is terminated by reaching the terminal box
    std::vector<double> terminalBoxSize; // center of the terminal box
    double penalty = 0.0;                // penalty for not reaching the terminal box
    std::vector<int> _cum_terminatedCnt; // cumulative count of terminated amatter
    int _checkTerminateCnt;              // count how many times the termination is checked
    bool isDumpTaskInfo = false;         // whether to dump task info
    double distRwCoeff;                  // coefficient for distance (increment) reward

    double rewardExponential; // rw = d^alpha_{t} - d^alpha_{t-1} - penalty + bonus, alpha= rewardExponential

    bool isCenterSpawningUsingAMatterInfo = false; // whether to use amatter info to spawn center
    vec3d centerSpawnPosition;                     // center of center spawning range (box center)
    vec3d centerSpawnRange;                        // range of center spawning (box size)

public:
    EscapeFromCenter() : _cache(1, 0) {};
    void initialize(const std::string &path, const Config &config);
    virtual void update(const SimuManager &simuManager) {};
    void checkTermination(const SimuManager &simuManager, std::vector<int> &iterminal);

    void getReward(const SimuManager &simuManager, std::vector<double> &reward);
    std::vector<double> getTotalReward(const SimuManager &simuManager);
    void reset(const SimuManager &simuManager);

    void dumpTaskInfo(std::ostream &os) override;

    virtual vec3d getTarget() const override
    {
        return center;
    }
};

#endif // !ESCAPEFROMCENTER_H