#include "RAMF_main.h"
#include "TorchAPI.h"
#include "../Agent/PPOMemory.h"
#include "../Agent/PPO2.h"
#include "SwarmInterface.h"

using namespace Eigen;
using namespace std;
using namespace rd;

int main(int argc, char *argv[])
{
	bool itest = false;
	bool iinferNet = false;
	int trainMode = 0; // 0: train during play (off-policy), 1: train after play (on-policy)
	// int memoryMode = 0; // 0: classical memory, 1: PPO memory

	int subCaselo = 0; // variables to determine subcase range
	int subCasehi = 0;



	int n_proc;
	std::string configPath;
	ArgHandler(argc, argv, n_proc, configPath, subCaselo, subCasehi);

	// set omp threads to 4 (if n_proc not given) or n_proc
	omp_set_num_threads(n_proc < 0 ? 4 : n_proc);

	// set default configPath. May be changed by input args
	if (configPath == "")
	{
#ifdef __linux__
		configPath = "/mnt/disk3/lbc/RAMFcases/config.cfg";
#else
		configPath = "C:/Studies/RAMF_cases/config.cfg";
#endif
	}

	cout << "initializing basic components ..." << endl;
	TrainManager tm = TrainManager(configPath, subCaselo, subCasehi);
	// create env
	Environment env(tm.getCfgContent(), tm);
	cout << "basic components initialization done." << endl;

	for (int subCaseID = subCaselo; subCaseID <= subCasehi; subCaseID++)
	{
		tm.setupNewSubCase(subCaseID);
		// re-create RL agent in every subcase
		Agent::SetDevice(tm.iTorch_cuda);
		shared_ptr<Agent> agent = Agent::Create(tm.getCfgContent(), env.sensor, env.actor);
		auto ppo2 = dynamic_pointer_cast<agent::PPOBase>(agent); // for ppo only
		env.Prepare(tm.getWorkingDir(), tm.ilearn, tm.iload, tm.real_rs);

		// declaring arrays for RL
		std::vector<int> numAction = env.actor->num();
		vector<double> zero(env.sensor->dim(), 0.0);
		vector<int> isterminated(env.smartInfoSize, 0);
		vector<int> isStoredMem(env.smartInfoSize, 1); // store memory only when it is >0
		int numTerminated = 0;

		Timer timer;
		MemoryBase *memory;
		// int PPOMemoryNum = 1;
		if (tm.memoryMode == 0)
		{
			memory = new Memory(5000, env.sensor->dim(), env.actor->dim(), 0.0, pow(2, 10));
		}
		else if (tm.memoryMode == 1)
		{

			memory = new PPOMemory(ppo2->getMemoryNum(), tm.learnstep, env.sensor->dim(), env.actor->dim());
		}
		// MemoryBase* memory = &mem;

		int startepisode = 0;

		int ppoTrainStep = 100;

		// need to be optimized
		if (tm.iload)
		{
			if (tm.ilearn)
			{
				// load latest model to train.
				agent->model_loader(tm.getWorkingDir(), ""); // load latest model, no prefix
				memory->readMemory(tm.getWorkingDir());
				// mem.readMemory(tm.workingDir);
				tm.readRecord(tm.getWorkingDir());
				startepisode = tm.episode_record.back() + 1;
			}
			else
			{
				if (tm.iloadBest)
					agent->model_loader(tm.getWorkingDir(), "_best"); // load best model
				else
					agent->model_loader(tm.getWorkingDir(), ""); // load latest model, no prefix

				startepisode = 0;
			}
		}
		else
		{
			if (tm.ilearn)
				agent->prepareTrainInfo(tm.getWorkingDir());
		}
		tm.startEpisode = startepisode;

		//----------------------- episode loop ---------------------------------------//
		for (int episode = startepisode; episode < tm.episodenum + startepisode; episode++)
		{
			bool isEval = false;
			// checking whether it is time to evalue the strategy
			if (tm.evalinterval > 0)
			{
				if ((episode % tm.evalinterval == 0) || (episode == tm.episodenum + startepisode - 1))
				{
					if (episode != 0)
						isEval = true;
				}
			}
			bool isExplore = tm.ilearn && !isEval; // exploration only at learning and not evaluating.

			agent->setNewEp(episode);
			bool isDump = (episode % tm.dumpItv == 0) || (episode == tm.episodenum + startepisode - 1);
			env.Reset(isDump); // including observing initial state to env.smamager.state
			// get initial action
			agent->decideAction(env.smanager.state, env.smanager.action, isExplore);
			env.Act(env.smanager.action, tm.inaive);

			// reset store memory flag
			for (int i = 0; i < isStoredMem.size(); i++)
			{
				isStoredMem[i] = 1;
			}

			timer.Tic();

			bool _stopEpisode = false;

			// -------------time step loop------------------ //
			int statestep = 0;
			for (int i = 0; i < tm.totalstep; i++)
			{
				// if (i % tm.sensestep == 0)
				// {
				// 	agent->decideAction(env.smanager.state, env.smanager.action, isExplore);
				// 	// for (int j = 0; j < action.size(); j++) {
				// 	//	action[j] = 1;
				// 	// }
				// 	env.Act(env.smanager.action, tm.inaive);
				// }

				// if ((episode % tm.dumpItv == 0) || (episode == tm.episodenum + startepisode - 1))
				// {
				// 	// dump amatter info
				// 	if (i % tm.dumpstep == 0)
				// 	{
				// 		tm.dumpStateAction(env.smanager.state, env.smanager.action, i);
				// 	}
				// }

				env.Update();

				// if ((i + 1) % tm.taskCheckStep == 0) task->checkTermination(tm, isterminated);
				if ((i + 1) % tm.sensestep == 0)
				{
					// env.ObserveState(state_new);
					// env.ObserveReward(reward);
					env.ObserveTransition(env.smanager.state_new, env.smanager.reward, isterminated, isStoredMem);
					statestep++;
					if (tm.ilearn && !isEval)
					{
						// only store memory in training mode
						if (tm.memoryMode == 0)
						{
							Memory *m = dynamic_cast<Memory *>(memory);
							m->storeMemory(env.smanager.state.size(),
										   env.smanager.state,
										   env.smanager.action,
										   env.smanager.reward,
										   env.smanager.state_new,
										   isterminated, isStoredMem);
						}
						else if (tm.memoryMode == 1)
						{
							PPOMemory *m = dynamic_cast<PPOMemory *>(memory);
							m->storeMemory(ppo2->getMemoryNum(),
										   env.smanager.state,
										   env.smanager.action,
										   env.smanager.state_new,
										   env.smanager.reward,
										   isterminated, isStoredMem);
						}

						// if (trainMode == 0) {
						//	// off-policy training
						//	//if (mem.allowSample()) agent->train(&mem);
						// }
						bool trainNow = false;
						if (tm.memoryMode == 1)
						{
							// ppo training
							trainNow = ((statestep) % tm.learnstep == 0) && (statestep != 0);
							if (isStoredMem[0] == 0)
								if (isterminated[0] == 1)
									trainNow = true; // train when the episode is about to stop. PPOMemory number must be 1 for terminatable task.
						}
						else
						{
							trainNow = dynamic_cast<Memory *>(memory)->allowSample();
						}
						if (trainNow)
							agent->train(memory);
					}

					agent->decideAction(env.smanager.state_new, env.smanager.action, isExplore);
					env.Act(env.smanager.action, tm.inaive);

					// state = state_new;
					env.smanager.state.swap(env.smanager.state_new);

					// managing the indices of state and action to be stored in memory
					_stopEpisode = true;
					for (int i = 0; i < isStoredMem.size(); i++)
					{

						if (isStoredMem[i] > 0)
						{
							_stopEpisode = false;
							// isStoredMem[i] = isterminated[i] == 0 ? 1 : 0;
						}
					}
				}

				if (isDump && ((i + 1) % env.smanager.dumpstep == 0))
				{
					env.Dump(env.smanager.workingDir.c_str(), i + 1);
				}

				if (_stopEpisode)
					break; // stop an episode when all amatter are terminated and no memory will be stored;
			}

			// end episode -------------------------//

			if (trainMode == 1)
			{
				// on-policy training
				// tbd
			}

			double meantotalreward;
			// summarize the current episode. also write task info
			env.EpisodeSummarize(meantotalreward, tm.ilearn);

			timer.Toc();

			tm.logs << "Episode " << episode << ": total reward = " << meantotalreward << ". ";
			tm.logs << "time cost: " << timer.elaspe() << std::endl;
			std::cout << "Episode " << episode << ": total reward = " << meantotalreward << ". ";
			std::cout << "time cost: " << timer.elaspe() << std::endl;

			tm.recording(episode, meantotalreward, 0, 0);
			if (tm.ilearn)
				agent->dumpTrainInfo(episode, tm.getWorkingDir()); // dump training information

			// below is the saving of models
			// saving best policy.
			vector<double>::iterator maxval = std::max_element(tm.reward_record.begin(), tm.reward_record.end());
			if (tm.ilearn)
			{
				if (meantotalreward >= *maxval)
					agent->model_saver(tm.getWorkingDir(), "_best"); // reaching new mean reward, record this 'best' policy
			}

			// backup ML agent and memory ...
			if ((episode % tm.saveinterval == 0) || (episode == tm.episodenum + startepisode - 1))
			{

				tm.writeRecord(tm.getWorkingDir());

				if (tm.ilearn)
				{
					memory->writeMemory(tm.getWorkingDir());
					agent->model_saver(tm.getWorkingDir(), ""); // normal saving, no suffix
					agent->recParam(tm.getWorkingDir().c_str());
					char idx[9];
					sprintf(idx, "%.7i", episode);
					string idxstr = string(idx);

					agent->model_saver(tm.getWorkingDir() + "/his/", idxstr);
				}
			}
		}

		delete memory;
	}
	return 0;
}
