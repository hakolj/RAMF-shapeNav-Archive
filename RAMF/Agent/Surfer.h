#ifndef SURFER_H
#define SURFER_H

#include "Agent.h"
namespace agent::navigation
{

	class OptimalSurfingAgent : public Agent
	{
	public:
		OptimalSurfingAgent(){};
		virtual void decideAction(const std::vector<std::vector<double>> &state,
								  std::vector<std::vector<double>> &action,
								  bool iexplore);
		virtual void train(MemoryBase *memory) {};
		virtual void initialize(const std::string &cfgContent, const Config &config);
		virtual void setNewEp(int ep) {}; // do sth for a new episode/epoch
		virtual void model_saver(const std::string &path, const std::string &suffix) {};
		// virtual void model_saver(const std::string& path) { model_saver(path, ""); };

		virtual void model_loader(const std::string &path, const std::string &suffix) {};
		virtual void recParam(const char *path) {}
		virtual void dumpTrainInfo(int episode, const std::string &path) {};
		virtual void prepareTrainInfo(const std::string &path) {};

	public:
		std::vector<double> direction;
		double _stateNormalizer; // to rescale the modified state
	};
}
#endif // !1
