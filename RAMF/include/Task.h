#pragma once
#ifndef TASK_H
#define TASK_H

#include <vector>
#include "ActiveMatter.h"
#include "Config.h"
#include "TaskInterface.h"
#include "SimuManager.h"
class Task
{

public:
	static std::shared_ptr<Task> Create(const std::string &cfgContent);
	// to update any task info during time roop
	virtual void update(const SimuManager &simuManager) = 0;
	virtual void getReward(const SimuManager &simuManager, std::vector<double> &reward) = 0;
	// virtual void getReward(const Agent* const amatter, std::vector<double>& reward) = 0;
	virtual std::vector<double> getTotalReward(const SimuManager &simuManager) = 0;
	virtual void initialize(const std::string &cfgContent, const Config &config) = 0;
	virtual void reset(const SimuManager &simuManager) = 0;
	virtual void checkTermination(const SimuManager &simuManager, std::vector<int> &iterminal)
	{
		for (int i = 0; i < iterminal.size(); i++)
		{
			iterminal[i] = 0; // all not terminated
		}
	}

	virtual void dumpTaskInfo(std::ostream &os) {};
};

#endif // !TASK_H