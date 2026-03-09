#ifndef HOMOGENEOUSSWARM_H
#define HOMOGENEOUSSWARM_H

#include "ActiveMatter.h"
#include "SwarmInterface.h"

class HomogeneousSwarm : public ActiveMatter, public SwarmActiveMatter
{
public:
	HomogeneousSwarm(unsigned amatternum);
	virtual void update(double timestep, const std::vector<int> &updateMask);
	virtual void initialize(const std::string &cfgContent, const Config &config); // initialize parameters using config file
	virtual void reset();														  // reset amatter. Used in: episode initialization
	virtual void getInfo();														  // get flow info
	virtual void useInfo();														  // preprocess flow info before update
	virtual void setFluid(std::shared_ptr<Fluid> fluid);
	virtual void dump(const char *path, int step); // dump amatter info
	virtual void BoundaryCondition(std::shared_ptr<Fluid> fluid);
	virtual int trajSize();			  // return the size of recorded trajectory
	virtual std::string trajHeader(); // return the header of trajectory file
	virtual void recordTraj(std::vector<Trajectory>::iterator begin,
							std::vector<Trajectory>::iterator end); // recording trajecotry
																	// virtual bool ReadyToSense(int idx);
																	// void UpdateSenseStepCount() { for (int i = 0; i < amatternum; i++) (++senseStepCount[i]) %= senseStep; }

private:
};

#endif