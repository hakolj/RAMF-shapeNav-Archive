#pragma once
#ifndef SIMUMANAGER_H
#define SIMUMANAGER_H

#include "vectypedef.h"
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "ActiveMatterInterface.h"
#include "Trajectory.h"
// #include "ActiveMatter.h"
// #include "Fluid.h"
// #include "Task.h"
// #include "Actor.h"
// #include "Sensor.h"
// using std::endl;

class Fluid;
class ActiveMatter;
class Task;
class Actor;
class Sensor;

enum SimType
{
	individual = 0,
	centralizedSwarm = 1,
	decentralizedSwarm = 2,
};

// a class to handle things about env simulation
class SimuManager
{
public:
	std::string workingDir;

	std::shared_ptr<Fluid> fluid;
	std::shared_ptr<ActiveMatter> amatter;
	std::shared_ptr<Task> task;
	std::shared_ptr<Actor> actor;
	std::shared_ptr<Sensor> sensor;

	SimuManager() = delete;
	SimuManager(const std::string &cfgContent);

	void Init(int amatternum, int dimState, int dimAction);
	std::vector<std::vector<double>> state;
	std::vector<std::vector<double>> state_new;
	std::vector<double> reward;
	std::vector<std::vector<double>> action;
	std::vector<int> iterminalTransit; // an vector storing whether this is a terminal transition
	std::vector<int> iterminated;	   // an vector storing whether an amatter is terminated
	std::vector<int> ivalid;		   // an vector storing whether the amatter is valid in this step
	int sensestep;
	int totalstep;
	int dumpstep;
	double timestepsize;
	bool inaive;	   // naive actor will always give the same action
	int trajnum;	   // number of recorded trajectory
	int maxTrajNum;	   // maximal number of trajectories to be recorded (hard limit)
	int taskCheckStep; // number of update time steps that check task is terminated

	std::ofstream logs; // fstream for output logs
	SimType simType;	// simulation type
};

#endif // !SIMUMANAGER_H
