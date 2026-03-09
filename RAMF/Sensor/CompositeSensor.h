#ifndef COMPOSITSENSOR_H
#define COMPOSITSENSOR_H

#include "Sensor.h"
namespace sensor
{
	class CompositeSensor : public Sensor
	{

	public:
		std::vector<std::shared_ptr<Sensor>> sensors;

		CompositeSensor() : _numSensors(0), _dim(0) {};

		inline virtual int dim() const { return _dim - _maskedSignalCnt; };
		virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate);
		virtual void initialize(const std::string &cfgContent, const Config &config);
		virtual void reset(std::shared_ptr<Task> task); // reset the sensor before every new episode
	private:
		int _numSensors;
		int _dim;
		std::vector<double> normalizer; // s_ = s / normalizer + offset
		std::vector<double> offset;		// s_ = s / normalizer + offset
		std::vector<int> signalMask;	// allows to mask out signals. 0: neglected, 1: used
		int _maskedSignalCnt;

		std::vector<int> signalIdxOffset; // offset of the signal index in the composite state (used for masking)
	};
}
#endif // !COMPOSITSENSOR_H
