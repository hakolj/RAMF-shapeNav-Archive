#include "pch.h"
#include "SimuManager.h"
#include "Config.h"
#include "Fop.h"

using namespace std;

SimuManager::SimuManager(const std::string &cfgContent) : trajnum(0)
{
	std::istringstream iss(cfgContent);
	Config config(iss, "GLOBAL");
	sensestep = config.Read<int>("sense step");
	totalstep = config.Read<int>("total step");
	// taskCheckStep = config.Read<int>("task check step", sensestep);
	timestepsize = config.Read<double>("time step");
	dumpstep = config.Read<int>("dump step", totalstep);
	inaive = config.Read<bool>("inaive", false);
	trajnum = config.Read<int>("export traj num", 10);
	maxTrajNum = config.Read<int>("max traj num", trajnum);

	simType = (SimType)config.Read<int>("simulation type");
}
void SimuManager::Init(int amatternum, int dimState, int dimAction)
{
	state.resize(amatternum);
	state_new.resize(amatternum);
	reward.resize(amatternum);
	action.resize(amatternum);
	iterminalTransit.resize(amatternum, 0);
	iterminated.resize(amatternum, 0);
	ivalid.resize(amatternum, 1);

	for (int i = 0; i < amatternum; i++)
	{
		state[i].resize(dimState);
		state_new[i].resize(dimState);
		action[i].resize(dimAction);
	}
}
