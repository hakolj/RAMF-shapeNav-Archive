#pragma once
#ifndef MEMORY_H
#define MEMORY_H

// #include <vector>
#include "Random.h"
#include "../Agent/Sumtree.h"
#include "MemoryBase.h"

class Memory : public MemoryBase
{
public:
	int size;
	int rec_position; // the present save position
	// [s_n, a_n, r_n, s_n+1, terminationFlag, weight]
	std::vector<std::vector<double>> pool;

	// Eigen::Matrix<double, 1, 1> pool;
private:
	bool ismemfull;
	int _numState, _numAction;
	int _newestMemNum;				  // number of latest memory
	agent::Sumtree sumtree;			  // used for prioritized experience replay;
	const double _maxPriority = 1.0f; // the maximal priority of an experience;

	double epsilon = 0.01; // small amount to avoid zero priority
	double alpha = 1.0;	   // [0~1] convert the importance of TD error to priority
	double beta = 0.4;	   // importance - sampling, from initial value increasing to 1
	double beta_increment_per_sampling = 0.001;
	double earlyStoreFrac = 0.1; // the fraction of the memory that stores the initial exp
	int earlyStoreNum;
	int minRecNum; // the minimal record number that allow sample.

private:
	Memory(){};

public:
	Memory(int size, int dimState, int dimAction, double earlyFraction, int minRecNum);
	~Memory(){};

	void storeMemory(int num, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action,
					 const std::vector<double> &reward, const std::vector<std::vector<double>> &state_next,
					 const std::vector<int> &iterminated,
					 const std::vector<int> &isStoredMem);

	void randomBatch(int requestNum, std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
					 std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
					 std::vector<int> &iterminated) const;

	void newestBatch(std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
					 std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
					 std::vector<int> &iterminated) const;
	void prioritizedBatch(int requestNum, std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
						  std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
						  std::vector<int> &iterminated, std::vector<double> &ISweight, std::vector<int> &index);
	void setPriority(const std::vector<int> &index, const std::vector<double> &td_error);

	void writeMemory(const std::string &path) const;
	void readMemory(const std::string &path);
	bool allowSample() { return ismemfull || (rec_position > minRecNum); }
};

#endif // MEMORY_H
