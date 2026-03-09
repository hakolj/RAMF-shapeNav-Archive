#ifndef PPOBASE_H
#define PPOBASE_H

#include "Agent.h"
#include "torch/torch.h"
#include "torch/script.h"
#include "PPOMemory.h"
#include "PPONetworks/PPONetworks.h"

namespace agent
{

    class PPOBase
    {
    public:
        virtual int getMemoryNum() const = 0;

        // log prob and entropy of normal function
        inline torch::Tensor log_prob(torch::Tensor action, torch::Tensor mu, torch::Tensor sigma)
        {
            return torch::sum(-((action - mu) * (action - mu)) / (2 * sigma * sigma) -
                                  torch::log(sigma) - log(sqrt(2 * M_PI)),
                              1, true);
        };

        inline torch::Tensor entropy(torch::Tensor sigma)
        {
            return 0.5 + 0.5 * log(2 * M_PI) + torch::log(sigma);
        };

        // log prob when the action is suppressed with tanh
        inline torch::Tensor log_prob_tanhSuppressed(torch::Tensor action, torch::Tensor mu, torch::Tensor sigma)
        {
            // action.fill_((float)1 - (float)1e-3);
            torch::Tensor clamped_action = torch::clamp(action, -1 + 1e-6, 1 - 1e-6); // avoid numerical instability
            // calculate the action before tanh suppression, a_raw = tanh^-1(a)
            torch::Tensor raw_action = 0.5 * (torch::log(1 + clamped_action) - torch::log(1 - clamped_action));

            // torch::Tensor var1 = log_prob(raw_action, mu, sigma);
            // torch::Tensor var2 = torch::log(1 - action * action);
            // torch::Tensor var3 = torch::sum(var2, 1, true);

            // #We use a formula that is more numerically stable, see details in the following link
            // #https: // github.com/tensorflow/probability/blob/master/tensorflow_probability/python/bijectors/tanh.py#L69-L80
            // #Derivation:
            // #log(1 - tanh(x) ^ 2)
            // #= log(sech(x) ^ 2)
            // #= 2 * log(sech(x))
            // #= 2 * log(2e ^ - x /(e ^ - 2x + 1))
            // #= 2 *(log(2) - x - log(e ^ - 2x + 1))
            // #= 2 *(log(2) - x - softplus(- 2x))
            torch::Tensor log_det_jacobian = 2 * (torch::log(torch::tensor(2.0)) - action - torch::nn::functional::softplus(-2 * action));

            // // calculate the log probability of the raw action. log_prob(a) = log_prob(a_raw) - log(1 - a^2)
            torch::Tensor ans = log_prob(raw_action, mu, sigma) - torch::sum(log_det_jacobian, 1, true);
            // torch::Tensor ans = log_prob(raw_action, mu, sigma) - torch::sum(torch::log(1 - action * action + 1e-6), 1, true);

            // if (torch::isnan(ans).any().item<bool>())
            // {
            // 	std::cout << "detected nan in log_prob_tanhSuppressed" << std::endl;
            // 	std::cout << "raw_action: " << raw_action.mean() << std::endl;
            // 	std::cout << "var1: " << var1.mean() << std::endl;
            // 	std::cout << "var2: " << var2.mean() << std::endl;
            // 	std::cout << "var3: " << var3.mean() << std::endl;
            // 	std::cout << "action min, max:" << action.min().item<double>() << ", " << action.max().item<double>() << std::endl;
            // }

            return ans;
        };
        // entropy when the action is suppressed with tanh
        inline torch::Tensor entropy_tanhSuppressed(torch::Tensor sigma, torch::Tensor action)
        {
            // return 0.5 + 0.5 * log(2 * M_PI) + torch::log(sigma) + torch::log(1 - torch::tanh(action) * torch::tanh(action) + 1e-6);
            return 0.5 + 0.5 * log(2 * M_PI) + torch::log(sigma) + 2 * (torch::log(torch::tensor(2.0)) - action - torch::nn::functional::softplus(-2 * action));
        };

        // hyperparameters
        double _init_std_log;

        // some debug parameters
        double episodeMeanALoss;
        double episodeMeanCLoss;
        double episodeMeanValue;
        double clipFraction;
        double kldiv; // kl divergence
        double episodeMeanEntropy;
        double advmean;
        double advstd;
        void dumpTrainInfoPPO(int episode, const std::string &path)
        {
            std::ofstream ofs;
            ofs.open(path + "/PPO2_train.txt", std::ios::app);
            ofs << episode << "    " << episodeMeanALoss << "    " << episodeMeanCLoss
                << "    " << episodeMeanValue << "	" << clipFraction << "	" << kldiv << "  " << episodeMeanEntropy << " "
                << advmean << " " << advstd << std::endl;
            ofs.close();
        };
        void prepareTrainInfoPPO(const std::string &path)
        {
            std::ofstream ofs;
            ofs.open(path + "/PPO2_train.txt", std::ios::out);
            ofs.close();
        };

        // a struct to collect training info for each minibatch
        struct MiniBatchTrainInfo
        {
            torch::Tensor actor_loss;
            torch::Tensor critic_loss;
            torch::Tensor policy_entropy;

            // batch mean stats
            double clip_fraction;
            double kl_divergence;
            double actor_loss_bmean;
            double critic_loss_bmean;
            double entropy_bmean;
            double probRatio_bmean;
        };

        // template <typename ActorNetType, typename CriticNetType>
        MiniBatchTrainInfo trainMiniBatch(const torch::Tensor &selectedIdx,
                                          const torch::Tensor &s, const torch::Tensor &actionTensor,
                                          const torch::Tensor &old_probs_all,
                                          const torch::Tensor &returnsTensor, const torch::Tensor &advTensor,
                                          std::shared_ptr<IPPOActorNet> &actorNetNew, std::shared_ptr<IPPOCriticNet> &criticNetNew,
                                          int actionDim, double clip, double entropy_loss_coeff)
        {
            MiniBatchTrainInfo mbInfo;

            torch::Tensor states = s.index_select(0, selectedIdx);
            torch::Tensor actions = actionTensor.index_select(0, selectedIdx);
            // torch::Tensor old_probs = old_probTensor.index_select(0, tempidx);

            // calculate old action prob
            torch::Tensor old_probs = old_probs_all.index_select(0, selectedIdx);

            // calculate new action prob
            torch::Tensor dist = actorNetNew->forward(states);
            torch::Tensor mu = dist.index_select(
                1, torch::linspace(0, actionDim - 1, actionDim, torch::kLong));
            torch::Tensor sigma =
                dist.index_select(1, torch::linspace(actionDim, 2 * actionDim - 1,
                                                     actionDim, torch::kLong));

            // torch::Tensor new_probs = log_prob(actions, mu, sigma);
            torch::Tensor new_probs = log_prob_tanhSuppressed(actions, mu, sigma); // tanh suppressed

            torch::Tensor probRatio = (new_probs - old_probs).exp();

            // critic net
            torch::Tensor criticVal = criticNetNew->forward(states);

            // minibatch adv
            torch::Tensor returns = returnsTensor.index_select(0, selectedIdx).detach();
            torch::Tensor advmbTensor = advTensor.index_select(0, selectedIdx);
            advmbTensor = (advmbTensor - advmbTensor.mean()) / (advmbTensor.std() + 1e-8); // minibatch normalize

            // actor loss
            torch::Tensor weighted_probs = advmbTensor * probRatio.squeeze();

            // clipped weighted probablity
            torch::Tensor clipped_weighted_probs = advmbTensor * torch::clamp(probRatio.squeeze(), 1 - clip, 1 + clip);

            // torch::Tensor policy_entropy = torch::mean(
            // 	entropy(sigma)); // negative of entropy: to encourage exploaration
            mbInfo.policy_entropy = torch::mean(
                entropy_tanhSuppressed(sigma, actions)); // negative of entropy: to encourage exploaration

            mbInfo.actor_loss =
                -torch::mean(torch::min(weighted_probs, clipped_weighted_probs)) -
                entropy_loss_coeff * mbInfo.policy_entropy;

            // cout << "sigma=" << sigma << endl;
            // cout << "entropy=" << policy_entropy << endl;

            // critic loss
            // torch::Tensor returns = (advTensor.index_select(0, tempidx) +
            // val.index_select(0, tempidx)).detach(); use td target as target for
            // critic net

            mbInfo.critic_loss =
                torch::mean((returns - criticVal) * (returns - criticVal));
            // torch::Tensor critic_loss = (returns - criticVal).square().mean();

            // cout << "actor_loss = " << endl << actor_loss << endl;
            // cout << "critic_loss = " << endl << critic_loss << endl;

            mbInfo.clip_fraction = torch::mean(torch::greater(torch::abs(probRatio - 1.0), clip).to(torch::kFloat)).item().toDouble();
            mbInfo.kl_divergence = -probRatio.log().mean().item().toDouble();
            mbInfo.actor_loss_bmean = mbInfo.actor_loss.item().toDouble();
            mbInfo.critic_loss_bmean = mbInfo.critic_loss.item().toDouble();
            mbInfo.entropy_bmean = mbInfo.policy_entropy.item().toDouble();
            mbInfo.probRatio_bmean = probRatio.mean().item().toDouble();

            return mbInfo;
        }
    };

} // namespace agent

#endif // !PPOBASE_H