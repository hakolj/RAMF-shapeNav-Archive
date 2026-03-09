#include "Environment.h"
#include "Fop.h"
#include "Exceptions.h"
#include "SwarmInterface.h"
Environment::Environment(const std::string &cfgContent,
						 const TrainManager &trainManager) : fluid(Fluid::Create(cfgContent)), amatter(ActiveMatter::Create(cfgContent)),
															 task(Task::Create(cfgContent)), sensor(Sensor::Create(cfgContent)), actor(Actor::Create(cfgContent)),
															 smanager(cfgContent), trajs(), smartTrajs(), _reward(), _state(), _action()
{
	std::cout << "Start initialize environment..." << std::endl;
	amatternum = amatter->amatternum;
	amatter->setFluid(fluid);

	smanager.fluid = fluid;
	smanager.amatter = amatter;
	smanager.task = task;
	smanager.actor = actor;
	smanager.sensor = sensor;

	std::istringstream iss(cfgContent);
	Config swarmcfg(iss, "ACTIVEMATTER");
	int swarmSize = swarmcfg.Read<int>("swarmSize", -1);

	// initialization of state, action, reward array
	// initialization of trajectories

	int trajnum;
	int smartTrajnum;
	if (smanager.simType == SimType::individual)
	{
		smartInfoSize = amatternum;
		trajnum = std::min(int(amatter->amatternum), smanager.trajnum);
		smartTrajnum = trajnum;
	}
	else if (smanager.simType == SimType::centralizedSwarm)
	{
		smartInfoSize = amatternum;
		// for centralized swarm, trajnum equals to the config-set trajnum * swarmSize
		// each amatter trajs correspond to each individual
		// smart traj num, however, equals to the config-set trajnum, as each swarm represents one smart traj
		trajnum = std::min(int(amatter->amatternum), smanager.trajnum) * swarmSize;
		trajnum = std::min(trajnum, smanager.maxTrajNum); // enforce hard limit
		smartTrajnum = std::min(int(amatter->amatternum), smanager.trajnum);
	}
	else if (smanager.simType == SimType::decentralizedSwarm)
	{
		// for decentralized swarm, trajnum equals to the config-set trajnum * swarmSize
		// each amatter trajs correspond to each individual
		// smart traj num is the same as trajnum, as each individual has its own state and action
		smartInfoSize = amatternum * swarmSize;
		trajnum = std::min(int(amatter->amatternum), smanager.trajnum) * swarmSize;
		trajnum = std::min(trajnum, smanager.maxTrajNum); // enforce hard limit
		smartTrajnum = trajnum;
	}

	smanager.Init(smartInfoSize, sensor->dim(), actor->dim());

	trajs.reserve(trajnum);
	for (int i = 0; i < trajnum; i++)
	{
		trajs.push_back(Trajectory(amatter->trajSize(), amatter->trajHeader(), trainManager.totalstep + 1));
		// trajs[i].Reserve(trainManager.totalstep + 1);
	}
	smartTrajs.reserve(smartTrajnum);
	for (int i = 0; i < smartTrajnum; i++)
	{
		std::string smarttrajHeader = std::to_string(smanager.sensor->dim()) + " " + std::to_string(smanager.actor->dim());
		smartTrajs.push_back(Trajectory(smanager.sensor->dim() + smanager.actor->dim(), smarttrajHeader, trainManager.totalstep + 1));
		// smartTrajs[i].Reserve(trainManager.totalstep + 1);
	}
	// pre-allocate the state vector
	_state.resize(smartInfoSize);
	for (auto &row : _state)
	{
		row.resize(sensor->dim());
	}
	// pre-allocate the action vector
	_action.resize(smartInfoSize);
	for (auto &row : _action)
	{
		row.resize(actor->dim());
	}
	// allocate the other vectors
	for (int i = 0; i < smartInfoSize; i++)
	{
		//_terminateFlag.push_back(0);

		// _validMask.push_back(1);
		_reward.push_back(0);
	}

	std::cout << "environment initialized." << std::endl;
};

void Environment::Prepare(const std::string &workinDir, bool isTrain, bool isLoad, int seed)
{
	smanager.workingDir = workinDir;
	// handle the task info
	std::ofstream ofs;
	std::string fname;
	if (isTrain)
	{
		fname = "/taskInfo_train.txt";
		if (!isLoad)
		{
			// when training, only delete old task info when not loading
			std::remove((smanager.workingDir + fname).c_str());
			ofs.open(smanager.workingDir + fname, std::ios::out);
			ofs.close();
		}
	}
	else
	{
		fname = "/taskInfo_run.txt";
		// when not training, always delete old task info
		std::remove((smanager.workingDir + fname).c_str());
		ofs.open(smanager.workingDir + fname, std::ios::out);
		ofs.close();
	}
	fluid->SetRandomEngine(seed + 100);
};
// reset at the begining of every episode
void Environment::Reset(bool isDump)
{
	this->isDump = isDump;
	fluid->reset();
	amatter->reset();
	amatter->getInfo();
	amatter->useInfo();	   // am.convertFrame();
	task->reset(smanager); // must be after ammtter reset, as task may need amatter info
	sensor->reset(task);
	// sensor->getState(amatter.get(), fluid, state); //initial state
	updateCnt = 0;

	// reset terminate and validity
	for (int i = 0; i < smartInfoSize; i++)
	{
		//_terminateFlag[i] = 0;
		// _iterminated_cache[i] = 0;
		// _validMask[i] = 1;
		smanager.iterminalTransit[i] = 0;
		smanager.iterminated[i] = 0; // all amatter are not terminated at the beginning
		smanager.ivalid[i] = 1;		 // all amatter are valid
	}
	ObserveState(smanager.state); // observe the initial state
	if (isDump)
	{
		Dump(smanager.workingDir.c_str(), 0); // dump initial field

		// reset trajectories
		for (auto &traj : trajs)
		{
			traj.Reset();
		}
		for (auto &straj : smartTrajs)
		{
			straj.Reset();
		}
		// record the first step
		RecordTraj();
	}
}

void Environment::Dump(const char *path, int step)
{
	std::string fieldpath = std::string(path) + std::string("/field");
	if (Fop::accessDir(fieldpath))
	{
		amatter->dump(fieldpath.c_str(), step);
		fluid->dump(fieldpath.c_str(), step);
		DumpStateAction(fieldpath.c_str(), step);
		std::cout << "Results dumped at step: " << step << std::endl;
	}
	else
	{
		std::string errmsg("Path not found for Environment::Dump: " + std::string(path));
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
		// exit(-1);
	}
}
void Environment::DumpStateAction(const char *path, int step)
{
	using namespace std;
	ofstream os;

	char stepstr[10];
	cout << "dumping State and Action of step: " << step << endl;
	sprintf(stepstr, "%.7i", step);
	string fullpath = std::string(path) + "/sttactfield" + string(stepstr) + ".txt";
	// string fullpath = _currentWorkingDir + "/field/sttactfield" + string(stepstr) + ".txt";
	os.open(fullpath, ios::out | ios::trunc);
	os.precision(8);
	os << scientific;

	int statedim = smanager.state[0].size();
	int actiondim = smanager.action[0].size();
	// int actionnum = action[0].size();
	os << statedim << " " << actiondim << endl;
	for (unsigned i = 0; i < smanager.state.size(); i++)
	{
		for (unsigned j = 0; j < statedim; j++)
		{
			os << smanager.state[i][j] << " ";
		}
		for (unsigned j = 0; j < actiondim; j++)
		{
			os << smanager.action[i][j] << " ";
		}
		os << endl;
	}
	os.close();
}
void Environment::Act(const std::vector<std::vector<double>> &action, bool inaive)
{
	actor->takeAction(amatter, action, inaive);
}
void Environment::Update()
{
	amatter->update(smanager.timestepsize, smanager.ivalid);
	// amatter->UpdateSenseStepCount();
	amatter->BoundaryCondition(fluid);
	fluid->update(smanager.timestepsize);
	amatter->getInfo();
	amatter->useInfo(); // am.convertFrame();
	task->update(smanager);

	// check termination every step
	task->checkTermination(smanager, smanager.iterminated);
	for (int i = 0; i < smartInfoSize; i++)
	{
		if (smanager.ivalid[i] == 1)
		{
			// this is a valid amatter, check if it is terminated now
			if (smanager.iterminated[i] > 0)
			{
				// just terminated in this step
				smanager.ivalid[i] = 0;
				smanager.iterminalTransit[i] = 1; // this is a terminal transition (until next ObserveTransition)
			}
		}
		// update the cache
		// _iterminated_cache[i] = smanager.iterminated[i];
	}

	// check dumping
	if (isDump)
	{
		RecordTraj(); // record every timestep if the episode needs to be dumped
	}

	updateCnt++;
}

void Environment::ObserveState(std::vector<std::vector<double>> &state)
{
#ifdef DEBUG
	int requireStateSize;
	if (smanager.simType == SimType::individual)
		requireStateSize = amatternum;
	else if (smanager.simType == SimType::centralizedSwarm)
	{
		requireStateSize = amatternum;
	}
	else if (smanager.simType == SimType::decentralizedSwarm)
	{
		auto swarm = std::dynamic_pointer_cast<SwarmActiveMatter>(amatter);
		requireStateSize = amatternum * swarm->getSwarmSize();
	}

	if (state.size() != requireStateSize)
	{
		throwException<std::runtime_error>("In Environment::ObserveState: The size of vector is not identical to amatternum!");
	}
	if (state[0].size() != sensor->dim())
	{
		throwException<std::runtime_error>("In Environment::ObserveState: The size of vector is not identical to state dimension!");
	}
#endif
	sensor->getState(amatter, fluid, state);
};

void Environment::ObserveTransition(std::vector<std::vector<double>> &state,
									std::vector<double> &reward,
									std::vector<int> &terminateState,
									std::vector<int> &memoryValidMask)
{
#ifdef DEBUG
	int requireStateSize;
	if (smanager.simType == SimType::individual)
		requireStateSize = amatternum;
	else if (smanager.simType == SimType::centralizedSwarm)
	{
		requireStateSize = amatternum;
	}
	else if (smanager.simType == SimType::decentralizedSwarm)
	{
		auto swarm = std::dynamic_pointer_cast<SwarmActiveMatter>(amatter);
		requireStateSize = amatternum * swarm->getSwarmSize();
	}

	if (state.size() != requireStateSize)
	{
		throwException<std::runtime_error>("In Environment::ObserveTransition: The size of state is not identical to amatternum!");
	}
	if (reward.size() != requireStateSize)
	{
		throwException<std::runtime_error>("In Environment::ObserveTransition: The size of reward is not identical to amatternum!");
	}
	if (terminateState.size() != requireStateSize)
	{
		throwException<std::runtime_error>("In Environment::ObserveTransition: The size of terminateState is not identical to amatternum!");
	}
	if (validMask.size() != requireStateSize)
	{
		throwException<std::runtime_error>("In Environment::ObserveTransition: The size of validMask is not identical to amatternum!");
	}
	if (state[0].size() != sensor->dim())
	{
		throwException<std::runtime_error>("In Environment::ObserveTransition: The size of state is not identical to state dimension!");
	}

#endif
	sensor->getState(amatter, fluid, state);
	task->getReward(smanager, reward);

	for (int i = 0; i < amatternum; i++)
	{
		terminateState[i] = smanager.iterminalTransit[i];

		if (smanager.iterminalTransit[i] > 0)
		{
			// special terminal state, should store this transition
			memoryValidMask[i] = 1;
			smanager.iterminalTransit[i] = 0; // reset the terminal transition flag
		}
		else
		{
			memoryValidMask[i] = smanager.ivalid[i]; // otherwise just depends on whether the amatter is valid
		}
	}
};

void Environment::ObserveReward(std::vector<double> &reward)
{
	if (reward.size() != amatternum)
	{
		throwException<std::runtime_error>("In Environment::ObserveReward: The size of vector is not identical to amatternum!");
	}
	task->getReward(smanager, reward);
};
void Environment::EpisodeSummarize(double &meanRw, bool isTrain)
{

	std::vector<double> totalRw = task->getTotalReward(smanager);
	meanRw = 0;
	for (double trwd : totalRw)
	{
		meanRw += trwd;
	}
	// meantotalreward /= (smanager.totalstep * smanager.timestepsize);
	meanRw /= totalRw.size();

	// output the task info
	std::ofstream ofs;
	std::string fname = isTrain ? "/taskInfo_train.txt" : "/taskInfo_run.txt";
	ofs.open(smanager.workingDir + fname, std::ios::app);
	task->dumpTaskInfo(ofs);
	ofs.close();

	// dump simulation information
	if (isDump)
	{
		std::string fieldpath = std::string(smanager.workingDir) + std::string("/field");
		if (Fop::accessDir(fieldpath))
		{
#pragma omp parallel for
			for (int i = 0; i < trajs.size(); i++)
			{
				// trajs[i].writeTraj(fieldpath + "/" + std::to_string(i) + ".txt");
				trajs[i].writeTrajBinary(fieldpath + "/" + std::to_string(i) + ".bin");
			}
#pragma omp parallel for
			for (int i = 0; i < smartTrajs.size(); i++)
			{
				// smartTrajs[i].writeTraj(fieldpath + "/smartTraj" + std::to_string(i) + ".txt");
				smartTrajs[i].writeTrajBinary(fieldpath + "/smartTraj" + std::to_string(i) + ".bin");
			}
		}
		else
		{
			std::string errmsg("Path not found for Environment::Dump: " + fieldpath);
			std::cout << errmsg << std::endl;
			throw std::runtime_error(errmsg);
		}

		// #pragma omp parallel for
		// 		for (int i = 0; i < trajs.size(); i++)
		// 		{
		// 			// std::string fieldpath = std::string(smanager.workingDir) + std::string("/field");
		// 			if (Fop::accessDir(fieldpath))
		// 			{
		// 				// amatter->dump(fieldpath.c_str(), step);
		// 				trajs[i].writeTraj(fieldpath + "/" + std::to_string(i) + ".txt");
		// 				smartTrajs[i].writeTraj(fieldpath + "/smartTraj" + std::to_string(i) + ".txt");
		// 			}
		// 			else
		// 			{
		// 				std::string errmsg("Path not found for Environment::Dump: " + fieldpath);
		// 				std::cout << errmsg << std::endl;
		// 				throw std::runtime_error(errmsg);
		// 			}
		// 		}
	}
};

void Environment::RecordTraj()
{

	amatter->recordTraj(trajs.begin(), trajs.end());

	// double *snapShot = new double[smartTrajs[0].size];
	std::vector<double> snapShot(smartTrajs[0].size, 0);

	for (unsigned j = 0; j < smartTrajs.size(); j++)
	{
		int n = 0;
		for (unsigned ns = 0; ns < smanager.sensor->dim(); ns++)
		{
			snapShot[n++] = smanager.state[j][ns];
		}
		for (unsigned na = 0; na < smanager.actor->dim(); na++)
		{
			snapShot[n++] = smanager.action[j][na];
		}

		smartTrajs[j].Record(snapShot);
	}
	// delete[] snapShot;
}