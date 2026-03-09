#include "PPOMemory.h"
#include <iostream>
#include <string>
#include <iostream>
#include "Fop.h"
using namespace std;

PPOMemory::PPOMemory(int expNum, int expLen, int dimState, int dimAction) : expNum(expNum), expLen(expLen),
																			size(expNum * expLen), rec_position(expNum, 0),
																			ismemfull(false), _dimState(dimState), _dimAction(dimAction),
																			state(size, std::vector<double>(dimState, 0)),
																			stateNext(size, std::vector<double>(dimState, 0)),
																			action(size, std::vector<double>(dimAction, 0)),
																			reward(size, 0),
																			iterminated(size, 0),
																			expOffset(expNum)
{
	for (int i = 0; i < expNum; i++)
	{
		expOffset[i] = i * expLen;
	}
};

void PPOMemory::storeMemory(int num, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action,
							const std::vector<std::vector<double>> &stateNext,
							const std::vector<double> &reward,
							const std::vector<int> &iterminated,
							const std::vector<int> &isStoredMem)
{
	// if (num != state.size())
	// {
	// 	throw(std::runtime_error("PPOMemory: number of memory is not consistent with the data."));
	// }
	// if (num != 1)
	// 	cout << "Warning: PPOMemory only supports single active matter trajectory!" << endl;

	unsigned numState = state[0].size();
	unsigned numAction = action[0].size();
	if (state.size() < num)
	{
		throw(std::runtime_error("the required memory number is larger than the input data. Check config file"));
	}
	for (int i = 0; i < num; i++)
	{
		if (isStoredMem[i] > 0) // this need to be checked in the future
		{
			int abs_rec_pos = rec_position[i] + expOffset[i]; // the real position of each traj

			for (int j = 0; j < numState; j++)
			{
				this->state[abs_rec_pos][j] = state[i][j];
				this->stateNext[abs_rec_pos][j] = stateNext[i][j];
			}
			for (int j = 0; j < numAction; j++)
			{
				this->action[abs_rec_pos][j] = action[i][j];
			}
			// this->actionProp[abs_rec_pos] = actionProp[i];
			// this->value[rec_position] = value[i];

			this->reward[abs_rec_pos] = reward[i];
			this->iterminated[abs_rec_pos] = iterminated[i];

			rec_position[i]++;
			// ismemfull = (rec_position == expLen);
			// if (!ismemfull)
			// {
			// 	ismemfull = (rec_position == expLen);
			// }
			// else
			// {
			// 	cout << "Warning: PPO Memory is full! Make sure the memory is large enough to store a whole episode" << endl;
			// }
			// if (rec_position >= expLen)
			// {
			// 	// rec_position rewind to the beginning
			// 	rec_position = 0;
			// 	// rec_position %= size;
			// }
		}
	}
	return;
}

void PPOMemory::Sample(std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
					   std::vector<std::vector<double>> &stateNext,
					   std::vector<double> &reward, std::vector<int> &iterminated,
					   std::vector<int> &expLen)
{
	int writePos = 0; // position to write the memory
	// load the whole memory
	for (int ntraj = 0; ntraj < expNum; ntraj++)
	{
		expLen[ntraj] = rec_position[ntraj];
		for (int i = 0; i < expLen[ntraj]; i++)
		{
			int readPos = this->expOffset[ntraj] + i;

			for (int j = 0; j < _dimState; j++)
			{
				state[writePos][j] = this->state[readPos][j];
				stateNext[writePos][j] = this->stateNext[readPos][j];
			}
			for (int j = 0; j < _dimAction; j++)
			{
				action[writePos][j] = this->action[readPos][j];
				// actionProp[writePos] = this->actionProp[readPos];
			}
			// value[i] = this->value[i];
			reward[writePos] = this->reward[readPos];
			iterminated[writePos] = this->iterminated[readPos];
			writePos++;
		}
		// rec_position[ntraj] = 0; // clear memory len
	}
}

// void PPOMemory::getGAE(std::vector<double> &gae, double gaeDiscount, double gamma) const
// {
// }

void PPOMemory::ClearMemory()
{
	for (int i = 0; i < expNum; i++)
		rec_position[i] = 0;
}

void PPOMemory::writeMemory(const std::string &path) const
{
	Fop::writevec2d<double>(this->state, path + "/mem_state.txt");
	Fop::writevec2d<double>(this->stateNext, path + "/mem_stateNext.txt");
	Fop::writevec2d<double>(this->action, path + "/mem_action.txt");
	Fop::writevec1d(this->reward, path + "/mem_reward.txt");
	// Fop::writevec1d<double>(this->actionProp, path + "/mem_actionProb.txt");
	// Fop::writevec1d<double>(this->value, path+"/mem_value.txt");
}
void PPOMemory::readMemory(const std::string &path)
{
	this->state = Fop::loadvec2d<double>(path + "/mem_state.txt");
	this->stateNext = Fop::loadvec2d<double>(path + "/mem_stateNext.txt");
	this->action = Fop::loadvec2d<double>(path + "/mem_action.txt");
	this->reward = Fop::loadvec1d<double>(path + "/mem_reward.txt", 0);
	// this->actionProp = Fop::loadvec1d<double>(path + "/mem_actionProb.txt", 0);
	// this->value = Fop::loadvec1d<double>(path + "/mem_value.txt");
}