#include "HomogeneousSwarm.h"
#include "Fop.h"
using namespace std;
HomogeneousSwarm::HomogeneousSwarm(unsigned amatternum) : ActiveMatter(amatternum), SwarmActiveMatter(amatternum)
{
}

void HomogeneousSwarm::update(double timestep, const std::vector<int> &updateMask)
{
	for (int i = 0; i < amatternum; i++)
	{
		if (updateMask[i] == 0)
			continue; // skip the dead ones
		swarms[i]->update(timestep, swarmValidMask[i]);
	}
}
void HomogeneousSwarm::initialize(const std::string &cfgContent, const Config &config)
{
	// initialize parameters using config file
	swarmSize = config.Read<int>("swarmSize");
	// must create swarm individual based on a seperated config block.
	std::istringstream iss(cfgContent);
	Config configIndiv(iss, "INDIVIDUAL");
	string indivname = configIndiv.Read("type", string("_EMPTY"));
	for (int i = 0; i < amatternum; i++)
	{
		// create each swarm
		swarms[i] = ActiveMatterSelector(indivname, swarmSize);
		swarms[i]->initialize(cfgContent, configIndiv);
		// initialize valid mask
		swarmValidMask.push_back(vector<int>(swarmSize, 1));
	}
}
void HomogeneousSwarm::reset()
{
	for (int i = 0; i < amatternum; i++)
	{
		swarms[i]->reset();

		for (auto &v : swarmValidMask[i])
			v = 1;
	}
}
void HomogeneousSwarm::getInfo()
{ // get flow info
	for (int i = 0; i < amatternum; i++)
	{
		swarms[i]->getInfo();
	}
}

void HomogeneousSwarm::useInfo()
{
	// preprocess flow info before update
	for (int i = 0; i < amatternum; i++)
	{
		swarms[i]->useInfo();
	}
}
void HomogeneousSwarm::setFluid(std::shared_ptr<Fluid> fluid)
{
	for (int i = 0; i < amatternum; i++)
	{
		swarms[i]->setFluid(fluid);
	}
}
void HomogeneousSwarm::dump(const char *path, int step)
{
	// dump amatter info
	for (int i = 0; i < amatternum; i++)
	{
		string fname = to_string(i);
		string subpath = string(path) + string("/") + fname;
		Fop::makeDir(subpath, false);
		swarms[i]->dump(subpath.c_str(), step);
	}
}
void HomogeneousSwarm::BoundaryCondition(std::shared_ptr<Fluid> fluid)
{
	for (int i = 0; i < amatternum; i++)
	{
		swarms[i]->BoundaryCondition(fluid);
	}
}
int HomogeneousSwarm::trajSize()
{
	// return the size of recorded trajectory
	return swarms[0]->trajSize();
}
std::string HomogeneousSwarm::trajHeader()
{
	// return the header of trajectory file
	return swarms[0]->trajHeader();
}
void HomogeneousSwarm::recordTraj(std::vector<Trajectory>::iterator begin,
								  std::vector<Trajectory>::iterator end)
{
	unsigned int trajnum = end - begin;

	// recording trajecotry of each individual (start from the first swarm)
	int recordPos = 0;
	while (trajnum > 0)
	{
		unsigned int recordNum = std::min(trajnum, swarmSize);
		for (int i = 0; i < amatternum; i++)
		{

			swarms[i]->recordTraj(begin + recordPos, begin + recordPos + recordNum);
			recordPos += recordNum;
			trajnum -= recordNum;
			if (trajnum <= 0)
				break;
		}
	}
}