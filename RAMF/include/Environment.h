#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

// a class to control the simulation environment including active matter, fluid, task, sensor, actor
#include "Fluid.h"
#include "ActiveMatter.h"
#include "Task.h"
#include "Sensor.h"
#include "Actor.h"
#include "pch.h"
#include "SimuManager.h"
#include "TrainManager.h"
class Environment
{
public:
	std::shared_ptr<Fluid> fluid;
	std::shared_ptr<ActiveMatter> amatter;
	std::shared_ptr<Task> task;
	std::shared_ptr<Sensor> sensor;
	std::shared_ptr<Actor> actor;

	SimuManager smanager;

	int amatternum;
	int smartInfoSize; // the size of state, action, reward, ...

protected:
	int updateCnt;
	bool isDump; // whether dumping the simulation info.
	std::vector<Trajectory> trajs;
	std::vector<Trajectory> smartTrajs; // trajectories of state and action

	// std::vector<int> _terminateFlag; // store whether the active matter is a terminated at this time step
	// std::vector<int> _iterminated_cache; // store whether terminated
	// std::vector<int> _validMask;		 // whether the state/reward is meaningful.

	std::vector<double> _reward;			  // cache for receive python numpy arr
	std::vector<std::vector<double>> _state;  // cache for receive python numpy arr
	std::vector<std::vector<double>> _action; // cache for receive python numpy arr

public:
	// create simulation env.
	Environment(const std::string &cfgContent, const TrainManager &trainManager);

	// prepare enviroment data before start of simulation
	void Prepare(const std::string &workingDir, bool isTrain, bool isLoad, int seed);
	// reset at the begining of every episode
	void Reset(bool isDump);
	// dumping simulation result
	void Dump(const char *path, int step);
	void DumpStateAction(const char *path, int step);

	// taking action
	void Act(const std::vector<std::vector<double>> &action, bool inaive);
	// update env
	void Update();
	// receive state from env
	void ObserveState(std::vector<std::vector<double>> &state);
	// receive reward from env
	void ObserveReward(std::vector<double> &reward);

	// receive transition from env
	void ObserveTransition(std::vector<std::vector<double>> &state,
						   std::vector<double> &reward,
						   std::vector<int> &terminateState,
						   std::vector<int> &memoryValidMask); // memoryValidMask: whether the transition is valid and to be stored in memory

	// summarize an episode, receive mean (total) reward
	void EpisodeSummarize(double &meanRw, bool isTrain);

protected:
	void RecordTraj();
};

#endif // ! ENVIRONMENT_H
