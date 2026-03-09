#include "pch.h"
#include "CompositeSensor.h"
#include "Fop.h"

using namespace std;
using namespace sensor;
void CompositeSensor::initialize(const std::string &cfgContent, const Config &config)
{
	_numSensors = config.Read<int>("sensor number");
	sensors = std::vector<std::shared_ptr<Sensor>>(_numSensors);
	_dim = 0;
	for (int i = 0; i < _numSensors; i++)
	{
		std::stringstream ss;
		ss.str("");
		ss << i;
		string sensorname = config.Read<string>("sensor" + ss.str());

		sensors[i] = SensorSelector(sensorname);
		sensors[i]->initialize(cfgContent, config);

		_dim += sensors[i]->dim();
	}

	// config signal mask
	_maskedSignalCnt = 0;
	string signalMaskStr = config.Read<string>("signal mask", "");
	if (signalMaskStr == "")
	{
		signalMask = std::vector<int>(_dim, 1); // by default, all signals are used
	}
	else
	{
		signalMask = Fop::loadvec1d<int>(signalMaskStr);
		for (int i = 0; i < signalMask.size(); i++)
		{
			if (signalMask[i] == 0)
			{
				_maskedSignalCnt++;
			}
		}
	}

	cout << "signalMask size: " << signalMask.size() << endl;
	cout << "dim: " << _dim << endl;
	cout << "maskedSignalCnt: " << _maskedSignalCnt << endl;

	string nmlstr = config.Read<string>("normalizer");
	normalizer = Fop::loadvec1d<double>(nmlstr);
	nmlstr = config.Read<string>("offset");
	offset = Fop::loadvec1d<double>(nmlstr);
	if (normalizer.size() != _dim)
	{
		cout << "The size of normalizer, offset or signalMask is not equal to the dimension of State!" << endl;
		throw("The size of normalizer, offset or signalMask is not equal to the dimension of State!");
	}

	signalIdxOffset = std::vector<int>(_dim, 0);
	for (int i = 1; i < _dim; i++)
	{
		signalIdxOffset[i] = signalMask[i - 1] + signalIdxOffset[i - 1]; // the offset of the i-th signal in the whole state
	}
}

void CompositeSensor::reset(std::shared_ptr<Task> task)
{
	for (int i = 0; i < _numSensors; i++)
	{
		sensors[i]->reset(task);
	}
}

void CompositeSensor::getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate)
{
	int startidx = 0; // the idx of newstate;

	for (int i = 0; i < _numSensors; i++)
	{
		// create  vector to store substate
		vector<vector<double>> substate(amatter->amatternum, vector<double>(sensors[i]->dim()));
		sensors[i]->getState(amatter, fluid, substate);
		for (int j = 0; j < amatter->amatternum; j++)
		{
			for (int stateidx = 0; stateidx < sensors[i]->dim(); stateidx++)
			{
				// check mask here
				int idx = startidx + stateidx;
				int the_offset = signalIdxOffset[idx];
				if (signalMask[idx] == 0)
				{
					continue;
				}
				else
				{
					newstate[j][the_offset] = (substate[j][stateidx] + offset[idx]) / normalizer[idx];
					// std::cout << "offset =" << the_offset << std::endl;
				}
			}
		}
		startidx += sensors[i]->dim();
	}
}
