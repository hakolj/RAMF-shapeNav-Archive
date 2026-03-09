#pragma once
#ifndef FLOWFIELDDATAPOOL_H
#define FLOWFIELDDATAPOOL_H

#include "Scalar.h"

class FlowFieldDataPool
{

public:
	FlowFieldDataPool();

	std::vector<std::shared_ptr<fluid::Scalar>> upool;
	std::vector<std::shared_ptr<fluid::Scalar>> vpool;
	std::vector<std::shared_ptr<fluid::Scalar>> wpool;

	void LoadData(const fluid::Mesh &ms, const std::string &datapath, const std::vector<int> &idxlist);
	void LoadData(const fluid::Mesh &ms, std::string uName, std::string vName, std::string wName, const std::string &datapath, const std::vector<int> &idxlist);
	void LoadData2D(const fluid::Mesh &ms, std::string uName, std::string vName, const std::string &datapath, const std::vector<int> &idxlist);
	void LoadData2D(const fluid::Mesh &ms, const std::string &datapath, const std::vector<int> &idxlist);
	inline int Size() const { return upool.size(); };
	~FlowFieldDataPool();
	void AllocateNew(int size, const fluid::Mesh &ms);

private:
	std::string _datapath;
};

#endif