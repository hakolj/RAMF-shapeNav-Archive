#pragma once
#ifndef ACTOR_H
#define ACTOR_H

#include "ActiveMatter.h"
#include "ActiveMatterInterface.h"
#include <array>
#include "Config.h"
class Actor
{
public:
	virtual void takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive) = 0;
	virtual void initialize(const std::string &cfgContent, const Config &config) = 0;
	virtual int dim() const = 0; // return the dimension of action

	// return the number of possible action of each dimension. return zero if the action is continueous.
	virtual std::vector<int> num() const = 0;
	static std::shared_ptr<Actor> Create(const std::string &path);

	static std::vector<double> ActionDecoder(int id, const std::vector<int> &actionNum)
	{
		std::vector<double> action(actionNum.size());
		for (int i = 0; i < actionNum.size(); i++)
		{
			action[i] = id % actionNum[i];
			id = (id - action[i]) / actionNum[i];
		}
		return action;
	}
	static int ActionEncoder(std::vector<double> action, const std::vector<int> actionNum)
	{
		int id = 0;
		int mult = 1;
		for (int i = 0; i < actionNum.size(); i++)
		{
			id += action[i] * mult;
			mult *= actionNum[i];
		}
		return id;
	}

protected:
	static std::shared_ptr<Actor> ActorSelector(std::string actorName);
	// to rescale actions from the output of agents. example: action = actionScale * ouput + actionOffset
	// purpose: to adapt to DRL agents, which only output within range [-1, 1] or [0, 1]
	std::vector<double> actionScale;  // re-scaling actions factor
	std::vector<double> actionOffset; // adding an offset to actions
	void SetActionRescalingFactor(const Config &config);

	// std::vector<std::vector<double>> action; // last action
	//// to backup the action taken last time
	// void BackupAction(const std::vector<std::vector<double>> action) {
	//	if (action.size() != this->action.size()) {
	//		this->action.resize(action.size());
	//	}
	//	for (int i = 0; i < action.size(); i++) {
	//		this->action[i].resize(dim());
	//		for (int j = 0; j < dim(); j++) {
	//			this->action[j] = action[j];
	//		}
	//	}
	// }
};

// a actor that do nothing
class NullActor : public Actor
{
	virtual void takeAction(std::shared_ptr<ActiveMatter> am, const std::vector<std::vector<double>> &action, bool inaive) {};
	virtual void initialize(const std::string &cfgContent, const Config &config) {}
	virtual int dim() const { return 1; }

	// return the number of possible action of each dimension. return zero if the action is continueous.
	virtual std::vector<int> num() const { return std::vector<int>(1, 1); };
	static std::shared_ptr<Actor> Create(const std::string &cfgContent);
};

#endif // !ACTOR_H
