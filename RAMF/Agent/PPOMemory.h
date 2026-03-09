#ifndef PPOMEMORY_H
#define PPOMEMORY_H
#include <vector>
#include <iostream>
#include <string.h>
#include "Memory.h"

// only supports single active matter trajecotry
class PPOMemory : public MemoryBase
{
public:
	int size;
	std::vector<int> rec_position; // the present save position
	// [s_n, a_n,action_prop, r_n, s_n+1, terminationFlag, weight]
	std::vector<std::vector<double>> pool;
	std::vector<std::vector<double>> state;
	std::vector<std::vector<double>> action;
	// std::vector<double> actionProp;
	std::vector<std::vector<double>> stateNext;
	// std::vector<double> value;
	std::vector<double> reward;
	std::vector<int> iterminated;

	int expLen;					// the length of each trajectory
	int expNum;					// number of trajectory stored in the memory
	std::vector<int> expOffset; // the begin position of each trajectory

	// Eigen::Matrix<double, 1, 1> pool;
private:
	bool ismemfull;
	int _dimState, _dimAction;

	// Sumtree sumtree; // used for prioritized experience replay;
	// const double _maxPriority = 1.0f; // the maximal priority of an experience;
	// double epsilon = 0.01;  // small amount to avoid zero priority
	// double alpha = 1.0;  // [0~1] convert the importance of TD error to priority
	// double beta = 0.4;  // importance - sampling, from initial value increasing to 1
	// double beta_increment_per_sampling = 0.001;

private:
	PPOMemory();

public:
	PPOMemory(int expNum, int expLen, int dimState, int dimAction);
	~PPOMemory(){};
	void storeMemory(int num, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action,
					 const std::vector<std::vector<double>> &stateNext,
					 const std::vector<double> &reward,
					 const std::vector<int> &iterminated,
					 const std::vector<int> &isStoredMem);
	// sample a single period of experience.
	void Sample(std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
				std::vector<std::vector<double>> &stateNext,
				std::vector<double> &reward, std::vector<int> &iterminated,
				std::vector<int> &expLen);
	// void getGAE(std::vector<double> &gae, double gaeDiscount, double gamma) const;
	void ClearMemory();

	inline int getMemoryLength() const
	{
		int l = 0;
		for (int i = 0; i < expNum; i++)
		{
			l += rec_position[i];
		}
		return l;
	};

	const std::vector<double> &getState(int i) const { return state[i]; };
	const std::vector<double> &getStateNext(int i) const { return stateNext[i]; };
	double getReward(int i) const { return reward[i]; };

	void writeMemory(const std::string &path) const;
	void readMemory(const std::string &path);
};

#endif
