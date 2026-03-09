#pragma once
#ifndef ACTIVEMATTER_H
#define ACTIVEMATTER_H
#include "vectypedef.h"
#include "ActiveMatterInterface.h"
#include "Fluid.h"
#include "Config.h"
#include "FluidInterface.h"
#include "Trajectory.h"
#include <memory> // for shared ptr

class ActiveMatter
{
public:
	unsigned amatternum;
	ActiveMatter(unsigned amatternum);
	virtual void update(double timestep, const std::vector<int> &updateMask) = 0;
	virtual void initialize(const std::string &path, const Config &config) = 0; // initialize parameters using config file
	virtual void reset() = 0;													// reset amatter. Used in: episode initialization
	virtual void getInfo() = 0;													// get flow info
	virtual void useInfo() = 0;													// preprocess flow info before update
	virtual void setFluid(std::shared_ptr<Fluid> fluid) = 0;
	virtual void dump(const char *path, int step) = 0; // dump amatter info
	virtual void BoundaryCondition(std::shared_ptr<Fluid> fluid) {};
	virtual int trajSize() = 0;			  // return the size of recorded trajectory
	virtual std::string trajHeader() = 0; // return the header of trajectory file
	// virtual void recordTraj(std::vector<Trajectory> &trajs) = 0; // recording trajecotry
	virtual void recordTraj(std::vector<Trajectory>::iterator begin,
							std::vector<Trajectory>::iterator end) = 0; // recording trajecotry
	// virtual bool ReadyToSense(int idx);
	// void UpdateSenseStepCount() { for (int i = 0; i < amatternum; i++) (++senseStepCount[i]) %= senseStep; }

	static std::shared_ptr<ActiveMatter> Create(const std::string &cfgContent);
	static std::shared_ptr<ActiveMatter> ActiveMatterSelector(const std::string &name, int amatternum);

protected:
	// std::vector<int> senseStepCount;
	// int senseStep = 0;
	// void InitSenseStepCount(const Config& config);
};

// extern std::shared_ptr<ActiveMatter> Create(const std::string& path);

#endif
