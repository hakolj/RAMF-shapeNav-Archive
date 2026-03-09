#ifndef OrnsteinUhlenbeckActionNoise_H
#define OrnsteinUhlenbeckActionNoise_H

#include <vector>
namespace agent
{
	class OrnsteinUhlenbeckActionNoise
	{
	public:
		double theta, mu, dt;
		double num;
		std::vector<double> noise_prev;
		OrnsteinUhlenbeckActionNoise() : OrnsteinUhlenbeckActionNoise(0, 0, 0){};
		OrnsteinUhlenbeckActionNoise(double mu, double theta, double dt) : theta(theta), mu(mu), dt(dt), num(0), _isReset(false)
		{
		}
		void Sample(std::vector<double> &noise, double sigma);
		void Reset();

	private:
		bool _isReset;
		void _Reset(double num);
	};
}
#endif // !OrnsteinUhlenbeckActionNoise_H
