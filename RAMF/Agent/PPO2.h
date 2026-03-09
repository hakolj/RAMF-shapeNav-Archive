#ifndef PPO2_H
#define PPO2_H
#include "Agent.h"
#include "torch/torch.h"
#include "torch/script.h"
#include "PPOMemory.h"
#include "PPOBase.h"
#include "PPONetworks/PPONetworks.h"

namespace agent
{

	// input: state, ouput: mean action values and variances
	struct PPO2ActorNetImpl : public IPPOActorNet
	{
	public:
		torch::nn::Linear f1;
		// torch::nn::Softmax f2;
		torch::nn::Linear fmid;
		torch::nn::Linear fMu;
		torch::nn::Linear fSigma;
		// torch::nn::BatchNorm1d bn1;
		std::vector<int64_t> nodeNum;
		torch::Tensor std_log;

	public:
		PPO2ActorNetImpl(int64_t nstate, int64_t nmid, int64_t naction, double init_std_log) : f1(register_module("InputLayer", torch::nn::Linear(nstate, nmid))),
																							   fmid(register_module("FullyConnectedLayer", torch::nn::Linear(nmid, nmid))),
																							   fMu(register_module("muOutputLayer", torch::nn::Linear(nmid, naction))),
																							   fSigma(register_module("sigmaOutputLayer", torch::nn::Linear(nmid, naction))),
																							   nodeNum({nstate, nmid, naction})
		{
			std_log = torch::nn::Module::register_parameter("stdlog", init_std_log * torch::ones(naction));
			initNet(init_std_log);
		};
		torch::Tensor forward(torch::Tensor input)
		{
			// std::cout << normalizer << std::endl;
			// std::cout << input << std::endl;
			// std::cout << input << std::endl;
			input = f1(input);
			input = torch::tanh(input);
			input = torch::tanh(fmid(input));

			// torch::Tensor mu = torch::tanh(fMu(input)); // use tanh to limit the range of the mu
			torch::Tensor mu = fMu(input); // not using tanh to limit the range of the mu

			// torch::Tensor sigma = torch::nn::functional::softplus(fSigma(input));
			torch::Tensor sigma = std_log.exp().expand_as(mu);
			// input = f2(input);
			// std::cout << "std_log" << std_log << std::endl;
			return torch::cat({mu, sigma}, 1);
		}

	public:
		void initNet(double init_std_log)
		{
			// torch::nn::init::xavier_normal_(f1->weight);
			torch::nn::init::orthogonal_(f1->weight, sqrt(2));
			torch::nn::init::constant_(f1->bias, 0.0);
			// torch::nn::init::xavier_normal_(fmid->weight);
			torch::nn::init::orthogonal_(fmid->weight, sqrt(2));
			torch::nn::init::constant_(fmid->bias, 0.0);

			// torch::nn::init::xavier_normal_(fMu->weight);
			torch::nn::init::orthogonal_(fMu->weight, 0.01);
			torch::nn::init::constant_(fMu->bias, 0.0);

			// torch::nn::init::xavier_normal_(fSigma->weight);
			torch::nn::init::orthogonal_(fSigma->weight, 0.01);
			torch::nn::init::constant_(fSigma->bias, 0.0);
			std_log.data().fill_(init_std_log); // initialize std_log to the given value
		}
	};
	TORCH_MODULE(PPO2ActorNet);

	struct PPO2CriticNetImpl : public IPPOCriticNet
	{
	public:
		torch::nn::Linear f1;
		torch::nn::Linear fmid;
		torch::nn::Linear fout;
		// torch::nn::BatchNorm1d bn1;
		std::vector<int64_t> nodeNum;

	public:
		PPO2CriticNetImpl(int64_t nin, int64_t nmid) : f1(register_module("InputLayer", torch::nn::Linear(nin, nmid))),
													   fmid(register_module("HiddenLayer", torch::nn::Linear(nmid, nmid))),
													   fout(register_module("OutputLayer", torch::nn::Linear(nmid, 1))),
													   nodeNum({nin, nmid})
		{
			initNet();
		};
		torch::Tensor forward(torch::Tensor input)
		{
			// std::cout << normalizer << std::endl;
			// std::cout << input << std::endl;
			// std::cout << input << std::endl;
			input = f1(input);
			input = torch::tanh(input);
			input = torch::tanh(fmid(input));
			input = fout(input);

			return input;
		}

	public:
		void initNet()
		{
			// torch::nn::init::xavier_normal_(f1->weight);
			torch::nn::init::orthogonal_(f1->weight);
			torch::nn::init::constant_(f1->bias, 0.0);
			// torch::nn::init::xavier_normal_(fmid->weight);
			torch::nn::init::orthogonal_(fmid->weight);
			torch::nn::init::constant_(fmid->bias, 0.0);
			// torch::nn::init::xavier_normal_(fout->weight);
			torch::nn::init::orthogonal_(fout->weight);
			torch::nn::init::constant_(fout->bias, 0.0);
		}
	};
	TORCH_MODULE(PPO2CriticNet);

	class RunningStat
	{
	private:
		std::vector<double> mean, std, S; // mean standard deviation, a temp value;
		int n;
		int dim;

	public:
		RunningStat(int dim) : dim(dim), mean(dim, 0), std(dim, 1), S(dim, 0), n(0) {};
		const std::vector<double> &getMean() const { return mean; }
		const std::vector<double> &getStd() const { return std; }
		void reset()
		{
			n = 0;
			for (int i = 0; i < dim; i++)
			{
				mean[i] = 0;
				std[i] = 0;
				S[i] = 0;
			}
		}
		void update(const std::vector<double> &x)
		{
			n += 1;
			if (n == 1)
			{
				for (int i = 0; i < dim; i++)
				{
					mean[i] = x[i];
					std[i] = x[i];
				}
			}
			else
			{
				for (int i = 0; i < dim; i++)
				{
					double old_mean = mean[i];
					mean[i] = old_mean + (x[i] - old_mean) / n;
					S[i] = S[i] + (x[i] - old_mean) * (x[i] - mean[i]);
					std[i] = sqrt(S[i] / n);
				}
			}
		}
	};

	class PPO2 : public Agent,
				 public PPOBase
	{
	public:
		int actionDim, stateDim;
		std::shared_ptr<IPPOActorNet> actorNetNew;
		std::shared_ptr<IPPOActorNet> actorNetOld;
		std::shared_ptr<IPPOCriticNet> criticNetNew;
		// PPO2CriticNet criticNetOld;

		torch::nn::MSELoss loss_func;
		torch::optim::Adam actorOptimizer;
		torch::optim::Adam criticOptimizer;

		std::vector<double> value; // the value of last action
		// std::vector<double> actionProb; // the probability of last action (useless)

		double GAMMA = 0.99;

		// int copyStep;
		int ppoMemoryNum; // vectorized environment

		PPO2(int64_t stateDim, int64_t actionDim, std::shared_ptr<IPPOActorNet> actorNetNew,
			 std::shared_ptr<IPPOActorNet> actorNetOld,
			 std::shared_ptr<IPPOCriticNet> criticNetNew);
		void copyParam();
		static void initNetworks(Config config, int64_t dimState,
								 int64_t dimAction,
								 std::shared_ptr<IPPOActorNet> &actorNetNew,
								 std::shared_ptr<IPPOActorNet> &actorNetOld,
								 std::shared_ptr<IPPOCriticNet> &criticNetNew);
		static std::shared_ptr<PPO2> makeInstance(Config config, int64_t dimState, int64_t dimAction);

		virtual void initialize(const std::string &cfgContent, const Config &config);
		// virtual std::vector<double> decideAction(const std::vector<std::vector<double>>& state, bool iexplore);
		virtual void decideAction(const std::vector<std::vector<double>> &state,
								  std::vector<std::vector<double>> &action,
								  bool iexplore);
		virtual void train(MemoryBase *memory);
		// virtual void model_saver(const std::string& path);
		virtual void model_saver(const std::string &path, const std::string &suffix);
		virtual void model_loader(const std::string &path, const std::string &suffix);
		virtual void setNewEp(int ep); // do sth for a new episode/epoch

		// to record parameters to file
		void recParam(const char *path);
		void clearRec(const char *path);
		virtual void dumpTrainInfo(int episode, const std::string &path) override { dumpTrainInfoPPO(episode, path); };
		virtual void prepareTrainInfo(const std::string &path) override { prepareTrainInfoPPO(path); };

		inline virtual int getMemoryNum() const { return ppoMemoryNum; }

	private:
		int _countTrain;
		int _episodeTrainCnt;
		double _lr_initial;
		int _lr_zero_episode;
		int _batchSize;
		double clip = 0.2; // how much the weighted prob is cliped

		bool _lr_decay;
		double _lr_critic_ratio; // the ratio between lr of actor and critic (<1 for smaller lr of critic)
		int epochs;				 // number of epochs per train
		double entropy_loss_coeff;

		RunningStat rwStat;
		RunningStat stateStat;
		double rewardRt;
	};
}
#endif // !PPO2DISCRETIZED_H
