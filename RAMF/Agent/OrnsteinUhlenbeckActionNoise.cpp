#include "pch.h"
#include "OrnsteinUhlenbeckActionNoise.h"

#include "Random.h"
namespace agent
{
	void OrnsteinUhlenbeckActionNoise::Sample(std::vector<double> &noise, double sigma)
	{
		if (!_isReset)
			_Reset(noise.size());
		if (noise.size() != num)
		{
			std::cout << "Wrong size of sample vector in OrnsteinUhlenbeckActionNoise!" << std::endl;
			throw;
		}
		for (int i = 0; i < num; i++)
		{
			noise[i] = noise_prev[i] + theta * (mu - noise_prev[i]) * dt;
			noise[i] += sigma * sqrt(dt) * rd::Normal();
			noise_prev[i] = noise[i];
		}
	}

	void OrnsteinUhlenbeckActionNoise::_Reset(double num)
	{
		if (this->num != num)
		{
			this->num = num;
			noise_prev = std::vector<double>(num, 0.0);
		}
		{
			for (int i = 0; i < num; i++)
				noise_prev[i] = 0.0;
		}
		_isReset = true;
	}
	void OrnsteinUhlenbeckActionNoise::Reset()
	{
		for (int i = 0; i < num; i++)
			noise_prev[i] = 0.0;
	}
}