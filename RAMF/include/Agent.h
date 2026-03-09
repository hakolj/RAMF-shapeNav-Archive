#pragma once
#ifndef AGENT_H
#define AGENT_H
// #include "Memory.h"
#include "MemoryBase.h"
#include "Config.h"
#include "Sensor.h"
#include "Actor.h"
#include <torch/torch.h>

class Agent
{
public:
	// virtual std::vector<int> decideAction(const std::vector<std::vector<double>>& state, double epsilon) = 0;
	// virtual std::vector<double> decideAction(const std::vector<std::vector<double>>& state, bool iexplore) = 0;
	virtual void decideAction(const std::vector<std::vector<double>> &state,
							  std::vector<std::vector<double>> &action,
							  bool iexplore) = 0;
	virtual void train(MemoryBase *memory) = 0;
	virtual void initialize(const std::string &cfgContent, const Config &config) = 0;
	virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
	virtual void model_saver(const std::string &path, const std::string &suffix) = 0;
	// virtual void model_saver(const std::string& path) { model_saver(path, ""); };

	virtual void model_loader(const std::string &path, const std::string &suffix) = 0;
	virtual void recParam(const char *path) = 0;
	virtual void dumpTrainInfo(int episode, const std::string &path) {};
	virtual void prepareTrainInfo(const std::string &path) {};
	static std::shared_ptr<Agent> Create(const std::string &configContent, const std::shared_ptr<const Sensor> psensor, const std::shared_ptr<const Actor> pactor);
	static torch::Device device; // device for libtorch, can be cpu or gpu
	static void SetDevice(bool isGPU);
};

#endif // AGENT_H