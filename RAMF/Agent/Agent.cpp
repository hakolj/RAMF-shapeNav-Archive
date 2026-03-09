#include "pch.h"
#include "Agent.h"
#include "PPO2.h"
#include "ShapeNavigationAgents/ShapeNavigationAgents.h"
// #include "PPO2Discretized.h"
using namespace std;
using namespace agent;
torch::Device Agent::device = torch::kCPU; // device is set to cpu by default

void Agent::SetDevice(bool isGPU)
{
	if (isGPU && torch::cuda::is_available())
	{ // CUDA is available, set device to GPU
		device = torch::kCUDA;
		std::cout << "CUDA is available. Using GPU." << std::endl;
	}
	else
	{
		// CUDA is not available, set device to CPU
		device = torch::kCPU;
		std::cout << "CUDA is not available. Using CPU." << std::endl;
	}
}
std::shared_ptr<Agent> Agent::Create(const std::string &configContent, const std::shared_ptr<const Sensor> sensor, const std::shared_ptr<const Actor> actor)
{
	std::istringstream cfgiss(configContent);
	Config agentConfig(cfgiss, "AGENT");
	shared_ptr<Agent> pagent;
	string agentname = agentConfig.Read<string>("type", string("_EMPTY"));

	if (agentname == "PPO2")
	{
		pagent = PPO2::makeInstance(agentConfig, sensor->dim(), actor->dim());
	}
	else if (agentname == "ShapeNavigation::TheoreticalStrategy")
	{
		pagent = std::make_shared<shapeNav::TheoreticalStrategy>();
	}
	else if (agentname == "ShapeNavigation::TanhFittedStrategy")
	{
		pagent = std::make_shared<shapeNav::TanhFittedStrategy>();
	}

	else
	{
		throw(std::runtime_error("Undefined Agent name. Check config."));
	}

	pagent->initialize(configContent, agentConfig);
	return pagent;
}
