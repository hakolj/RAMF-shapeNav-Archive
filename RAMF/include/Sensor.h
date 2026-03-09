#pragma once
#ifndef SENSOR_H
#define SENSOR_H

#include <vector>
#include "ActiveMatterInterface.h"
#include "ActiveMatter.h"
#include "vectypedef.h"
#include "Config.h"
#include "Task.h"
#include "Fluid.h"

class Sensor
{
public:
	virtual int dim() const = 0;
	virtual void getState(std::shared_ptr<ActiveMatter> amatter, std::shared_ptr<Fluid> fluid, std::vector<std::vector<double>> &newstate) = 0;
	virtual void initialize(const std::string &cfgContent, const Config &config) = 0;
	virtual void reset(std::shared_ptr<Task> task){}; // reset the sensor before every new episode
	static std::shared_ptr<Sensor> Create(std::string cfgContent);

protected:
	static std::shared_ptr<Sensor> SensorSelector(std::string keyword);
};

#endif // !SENSOR_H
