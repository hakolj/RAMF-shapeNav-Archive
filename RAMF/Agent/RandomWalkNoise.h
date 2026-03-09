#ifndef RANDOMWALKNOISE_H
#define RANDOMWALKNOISE_H
#include "Random.h"
#include <iostream>
#include <math.h>
class RandomWalkNoise {
public:
	int num;
private:
	// the last sampled values, the range, the strength of noise (sigma) of each value
	std::vector<double> sample_old, range, sigma;

public:
	RandomWalkNoise(int num) :num(num), 
		sample_old(std::vector<double>(num)), range(std::vector<double>(num)),
		sigma(std::vector<double>(num)){};
	void Reset(const std::vector<double>& sigma) {
		for (int i = 0; i < num; i++) {
			this->sigma[i] = sigma[i];
			sample_old[i] = rd::randd(-range[i], range[i]);
		}
	}
	void Sample(std::vector<double> sample) {
		if (sample.size() != num) {
			std::cout << "Wrong number of sample vector in RandomWalkNoise!" << std::endl;
			throw;
		}
		else {
			for (int i = 0; i < num; i++) {
				sample[i] = sample_old[i] + rd::Normal() * sigma[i] * range[i];
				//sample[i] = std::max(std::min(sample[i], range[i]), -range[i]);
				sample_old[i] = sample[i];
			}
		}
	}

};


#endif 