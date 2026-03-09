#include "pch.h"
#include "Memory.h"
#include "Fop.h"
#include "torch/torch.h"
#include "torch/script.h"

using namespace std;

Memory::Memory(int size, int dimState, int dimAction, double earlyFraction, int minRecNum) : size(size), rec_position(0), pool(size, std::vector<double>(dimState * 2 + dimAction + 3, 0)),
																							 ismemfull(false), _numState(dimState), _numAction(dimAction), sumtree(size), _newestMemNum(0), earlyStoreFrac(earlyFraction), minRecNum(minRecNum)
{
	if (earlyStoreFrac >= 1.0)
		throw("the fraction of early memory must be less than 1!");
	earlyStoreNum = (int)(size * earlyStoreFrac);
};

void Memory::storeMemory(int num, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action,
						 const std::vector<double> &reward, const std::vector<std::vector<double>> &state_next,
						 const std::vector<int> &iterminated,
						 const std::vector<int> &isStoredMem)
{
	unsigned numState = state[0].size();
	unsigned numAction = action[0].size();
	_newestMemNum = 0;
	for (int i = 0; i < num; i++)
	{
		vector<double> *rec = &(pool[rec_position]);
		if (isStoredMem[i] > 0)
		{
			for (unsigned j = 0; j < numState; j++)
			{
				// pool[rec_position][j] = state[i][j]; //sn
				// pool[rec_position][j + numState + 2] = state_next[i][j]; //sn_next

				rec->at(j) = state[i][j];								   // sn
				rec->at(j + numState + _numAction + 1) = state_next[i][j]; // sn_next
			}

			// pool[rec_position][numState] = action[i];
			// pool[rec_position][numState + 1] = reward[i];
			// pool[rec_position][numState * 2 + 2] = iterminated[i];
			for (unsigned j = 0; j < numAction; j++)
			{
				rec->at(numState + j) = action[i][j];
			}
			rec->at(numState + _numAction) = reward[i];
			rec->at(numState * 2 + _numAction + 1) = iterminated[i];

			// pool(rec_position, numState) = action[i];
			// pool(rec_position, numState + 1) = reward[i];
			// pool(rec_position][numState * 2 + 2] = iterminated[i];

			sumtree.update(rec_position + sumtree.capacity() - 1, _maxPriority); // set max priority for new exp
			rec->at(numState * 2 + _numAction + 2) = _maxPriority;
			rec_position++;
			if (!ismemfull)
			{
				ismemfull = (rec_position == size);
			}
			if (rec_position >= size)
			{
				// rec_position rewind to the beginning of NOT early experience.
				rec_position = earlyStoreNum;
				// rec_position %= size;
			}
			_newestMemNum++;
		}
	}
	return;
}

void Memory::randomBatch(int requestNum, std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
						 std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
						 std::vector<int> &iterminated) const
{
	std::vector<int> selected = ismemfull ? rd::randomChoice(min(requestNum, size), 0, size) : rd::randomChoice(min(requestNum, rec_position), 0, rec_position);
	int num = selected.size();
	state = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	state_next = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	action = std::vector<std::vector<double>>(num, std::vector<double>(_numAction));
	reward = std::vector<double>(num);
	iterminated = std::vector<int>(num);

	for (int i = 0; i < num; i++)
	{
		const vector<double> *rec = &(pool[selected[i]]);
		for (unsigned j = 0; j < _numState; j++)
		{
			// state[i][j] = pool[selected[i]][j];//sn
			// state_next[i][j] = pool[selected[i]][j + _numState + 2]; //sn_next

			state[i][j] = rec->at(j);									// sn
			state_next[i][j] = rec->at(j + _numState + _numAction + 1); // sn_next
		}
		// action[i] = pool[selected[i]][_numState];
		// reward[i] = pool[selected[i]][_numState + 1];
		// iterminated[i] = pool[selected[i]][_numState * 2 + 2];
		for (unsigned j = 0; j < _numAction; j++)
		{
			action[i][j] = rec->at(_numState + j);
		}
		reward[i] = rec->at(_numState + _numAction);
		iterminated[i] = rec->at(_numState * 2 + _numAction + 1);
	}
	return;
}

void Memory::newestBatch(std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
						 std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
						 std::vector<int> &iterminated) const
{
	unsigned int choicestart = 0;
	unsigned int num = 0;
	int requestNum = _newestMemNum;
	if (ismemfull)
	{
		choicestart = (rec_position - requestNum) % size + size;
		num = requestNum;
	}
	else
	{
		choicestart = max(rec_position - requestNum, 0);
		num = rec_position - choicestart;
	}

	state = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	state_next = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	action = std::vector<std::vector<double>>(num, std::vector<double>(_numAction));
	reward = std::vector<double>(num);
	iterminated = std::vector<int>(num);

	for (unsigned int i = 0; i < num; i++)
	{
		unsigned int selected = (choicestart + i) % size;
		const vector<double> *rec = &(pool[selected]);
		for (unsigned j = 0; j < _numState; j++)
		{
			// state[i][j] = pool[selected][j];//sn
			// state_next[i][j] = pool[selected][j + _numState + 2]; //sn_next

			state[i][j] = rec->at(j);									// sn
			state_next[i][j] = rec->at(j + _numState + _numAction + 1); // sn_next
		}

		for (unsigned j = 0; j < _numAction; j++)
		{
			action[i][j] = rec->at(_numState + j);
		}
		reward[i] = rec->at(_numState + _numAction);
		iterminated[i] = rec->at(_numState * 2 + _numAction + 1);

		// action[i] = rec->at(_numState);
		// reward[i] = rec->at(_numState + 1);
		// iterminated[i] = rec->at(_numState * 2 + 2);
	}
}

void Memory::prioritizedBatch(int requestNum, std::vector<std::vector<double>> &state, std::vector<std::vector<double>> &action,
							  std::vector<double> &reward, std::vector<std::vector<double>> &state_next,
							  std::vector<int> &iterminated, std::vector<double> &ISweight, std::vector<int> &index)
{
	if (requestNum > size)
	{
		cout << "the number of requested memory batch exceeds the size of memory" << endl;
		exit(-1);
	}

	int num = ismemfull ? requestNum : min(requestNum, rec_position);
	state = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	state_next = std::vector<std::vector<double>>(num, std::vector<double>(_numState));
	action = std::vector<std::vector<double>>(num, std::vector<double>(_numAction));
	reward = std::vector<double>(num);
	iterminated = std::vector<int>(num);
	ISweight = std::vector<double>(num);
	index = std::vector<int>(num);

	int pri_seg = sumtree.getTotalP() / num;			 // priority segment
	beta = min(1.0, beta + beta_increment_per_sampling); // max = 1
	// double min_prob = sumtree.getMinP() / sumtree.getTotalP();
	double min_prob = 1;

	for (int i = 0; i < num; i++)
	{
		int a = pri_seg * i;
		int b = pri_seg * (i + 1);
		double v = rd::randd((double)a, (double)b);
		int idx;
		double p;
		sumtree.getLeaf(v, idx, p); // determin sample idx
		index[i] = idx;
		double prop = p / sumtree.getTotalP();
		if (min_prob > prop)
			min_prob = prop;
		ISweight[i] = prop; // calculate weight 1. normalize it later.

		const vector<double> *rec = &(pool[idx]);
		for (unsigned j = 0; j < _numState; j++)
		{
			// state[i][j] = pool[selected[i]][j];//sn
			// state_next[i][j] = pool[selected[i]][j + _numState + 2]; //sn_next

			state[i][j] = rec->at(j);									// sn
			state_next[i][j] = rec->at(j + _numState + _numAction + 1); // sn_next
		}
		for (unsigned j = 0; j < _numAction; j++)
		{
			action[i][j] = rec->at(_numState + j);
		}
		reward[i] = rec->at(_numState + _numAction);
		iterminated[i] = rec->at(_numState * 2 + _numAction + 1);

		// action[i] = rec->at(_numState);
		// reward[i] = rec->at(_numState + 1);
		// iterminated[i] = rec->at(_numState * 2 + 2);
	}

	for (double &w : ISweight)
	{
		w = pow(w / min_prob, -beta);
	}

	return;
}

void Memory::writeMemory(const std::string &path) const
{
	std::ofstream of;
	of.open(path + "/mem.txt", ios::out);

	// outfile << std::showpoint << std::left << std::fixed << std::setprecision(8);
	// of.setf(ios::showpoint | ios::left | ios::scientific);
	// of.precision(8);
	of << this->size << " " << this->_numState << " " << this->_numAction
	   << " " << this->ismemfull << " " << this->rec_position << endl;
	Fop::writevec2d<double>(this->pool, of);
	of.close();

	return;
}

void Memory::readMemory(const std::string &path)
{
	std::ifstream infile;
	infile.open(path + "/mem.txt", ios::in);
	infile >> this->size >> this->_numState >> this->_numAction >> this->ismemfull >> this->rec_position;
	infile.close();
	this->pool = Fop::loadvec2d<double>(path + "/mem.txt", 1);

	// check
	if (pool[0].size() != _numState * 2 + _numAction + 3)
	{
		cout << "pool size error";
		exit(-1);
	}
	for (int i = 0; i < size; i++)
	{
		// load the sum tree
		sumtree.update(i + sumtree.capacity() - 1, pool[i][_numState * 2 + _numAction + 2]);
	}
	return;
}

void Memory::setPriority(const std::vector<int> &index, const std::vector<double> &td_error)
{
	for (int i = 0; i < index.size(); i++)
	{

		double value = min(abs(td_error[i]) + epsilon, _maxPriority);
		value = pow(value, alpha);

		// if (isnan(value2)) {
		//	cout << "bug" << endl;
		// }
		sumtree.update(index[i] + sumtree.capacity() - 1, value);
		pool[index[i]][_numState * 2 + _numAction + 2] = value;
	}
}