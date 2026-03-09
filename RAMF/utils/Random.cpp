#include "pch.h"
#include <random>
#include "Random.h"

namespace rd
{
	using namespace std;
	// static std::default_random_engine re(time(0));
	static std::uniform_real_distribution<double> default_real_distr(0.0, 1.0);
	static std::normal_distribution<double> standard_normal_distr(0, 1.0); // ��׼��̬�ֲ�����ֵ0, ���� 1

	void setRandomSeed(unsigned randomSeed)
	{
		re = std::default_random_engine(randomSeed);
		std::cout << "set random seed and test rand: " << rd::randd() << std::endl;
	}

	int randi(int low, int high)
	{
		if (low > high - 1)
		{
			std::string errmsg = "Error: low > high -1 in rd::randi.";
			std::cout << errmsg << std::endl;
			throw std::invalid_argument(errmsg);
		}
		std::uniform_int_distribution<int> intdistr(low, high - 1);
		return intdistr(re);
		// return (rand() % (high - low)) + low;
	}
	std::vector<int> randi(int num, int low, int high)
	{
		if (low > high - 1)
		{
			std::string errmsg = "Error: low > high -1 in rd::randi.";
			std::cout << errmsg << std::endl;
			throw std::invalid_argument(errmsg);
		}
		std::uniform_int_distribution<int> intdistr(low, high - 1);
		std::vector<int> out(num);
		for (int i = 0; i < num; i++)
		{
			// out[i] = (rand() % (high - low)) + low;
			out[i] = intdistr(re);
		}
		return out;
	}

	double randd()
	{
		return default_real_distr(re);
	}

	double randd(double lo, double hi)
	{
		std::uniform_real_distribution<double> custom_real_distr(lo, hi);
		return custom_real_distr(re);
	}

	std::vector<double> randd(int num, double lo, double hi)
	{
		std::uniform_real_distribution<double> custom_real_distr(lo, hi);
		std::vector<double> vec(num);

		for (unsigned i = 0; i < vec.size(); i++)
		{
			vec[i] = custom_real_distr(re);
		}
		return vec;
	}

	vector<int> randomChoice(int num, int lo, int hi)
	{
		int total = hi - lo;
		if (lo > hi - 1)
		{
			std::string errmsg = "Error: low > high -1 in rd::randomChoice.";
			std::cout << errmsg << std::endl;
			throw std::invalid_argument(errmsg);
		}
		else if (num > total)
		{
			std::string errmsg = "Error: low > high -1 in rd::randomChoice.";
			std::cout << errmsg << std::endl;
			throw std::invalid_argument(errmsg);
		}
		int remain = total;
		int select = num;
		vector<int> vec(num, 0);
		int idx = 0;

		for (int i = 0; i < total; i++)
		{
			if (randi(0, remain) < select)
			{
				vec[idx++] = i + lo;
				select--;
			}
			remain--;
			if (select <= 0)
				break;
		}
		return vec;
	}

	double Normal()
	{
		return Normal(re);
	}

	double Normal(std::default_random_engine &rengine)
	{
		return standard_normal_distr(rengine);
	}
	std::vector<double> Normal(int size)
	{
		std::vector<double> result(size);
		for (int i = 0; i < result.size(); i++)
			result[i] = standard_normal_distr(re);
		return result;
	}

	double Normal_log_prop(double x, double mu, double sigma)
	{
		return -(x - mu) * (x - mu) / (2 * sigma * sigma) - log(sigma) - 0.5 * log(2 * M_PI);
	}

}
