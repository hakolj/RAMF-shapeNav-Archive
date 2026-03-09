#pragma once
#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace rd
{
	static std::default_random_engine re;
	// return random int from [low, hi)
	int randi(int low, int high);

	// return 'num' ints as std::vector, range from [low, hi)
	std::vector<int> randi(int num, int low, int high);

	// return random double from [0, 1)
	double randd();
	// return random double range from [low, hi)
	double randd(double lo, double hi);

	// return 'num' doubles as std::vector, range from [low, hi)
	std::vector<double> randd(int num, double lo, double hi);

	// return random selected numbers from [low, hi) as std::vector. num: length of vector
	std::vector<int> randomChoice(int num, int low, int high);

	void setRandomSeed(unsigned randomSeed);

	double Normal();
	double Normal(std::default_random_engine &rengine);
	std::vector<double> Normal(int size);
	double Normal_log_prop(double x, double mu, double sigma);

	// Ornstein–Uhlenbeck noise, return the step size, not the new value
	inline double ounoise(double x, double theta, double sigma, double mu, double dt, std::default_random_engine &rengine)
	{
		return theta * (mu - x) * dt + sigma * sqrt(dt) * Normal(rengine);
	}

}
#endif
