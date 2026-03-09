#include "PPO2.h"

#include <algorithm>
#include <random>

#include "TorchAPI.h"
using namespace std;
using namespace agent;
PPO2::PPO2(int64_t stateDim, int64_t actionDim,
		   std::shared_ptr<IPPOActorNet> actorNetNew,
		   std::shared_ptr<IPPOActorNet> actorNetOld,
		   std::shared_ptr<IPPOCriticNet> criticNetNew)
	: actionDim(actionDim),
	  stateDim(stateDim),
	  actorNetNew(actorNetNew),
	  actorNetOld(actorNetOld),
	  criticNetNew(criticNetNew),
	  //   actorNetNew(stateDim, nmid, actionDim, init_std_log),
	  //   actorNetOld(stateDim, nmid, actionDim, init_std_log),
	  //   criticNetNew(PPO2CriticNet(stateDim, nmid)),
	  actorOptimizer(actorNetNew->parameters(),
					 torch::optim::AdamOptions().eps(1e-5)),
	  criticOptimizer(criticNetNew->parameters(),
					  torch::optim::AdamOptions().eps(1e-5)),
	  loss_func(torch::nn::MSELoss()),
	  rwStat(1),
	  stateStat(stateDim)
{

	copyParam();
	return;
};

void PPO2::initNetworks(Config config, int64_t dimState,
						int64_t dimAction,
						std::shared_ptr<IPPOActorNet> &actorNetNew,
						std::shared_ptr<IPPOActorNet> &actorNetOld,
						std::shared_ptr<IPPOCriticNet> &criticNetNew)
{
	// in the future here will be a factory method to create different
	// network architectures based on config

	int numMid = config.Read<int>("num mid");
	double init_std_log = config.Read<double>("init std log", -1);

	actorNetNew = std::make_shared<PPO2ActorNetImpl>(dimState, numMid, dimAction, init_std_log);
	actorNetOld = std::make_shared<PPO2ActorNetImpl>(dimState, numMid, dimAction, init_std_log);
	criticNetNew = std::make_shared<PPO2CriticNetImpl>(dimState, numMid);
	std::dynamic_pointer_cast<PPO2ActorNetImpl>(actorNetNew)->initNet(init_std_log);
	std::dynamic_pointer_cast<PPO2CriticNetImpl>(criticNetNew)->initNet();
}

std::shared_ptr<PPO2> PPO2::makeInstance(Config config, int64_t dimState,
										 int64_t dimAction)
{
	std::shared_ptr<IPPOActorNet> actorNetNew;
	std::shared_ptr<IPPOActorNet> actorNetOld;
	std::shared_ptr<IPPOCriticNet> criticNetNew;

	initNetworks(config, dimState, dimAction,
				 actorNetNew, actorNetOld, criticNetNew);

	return std::make_shared<PPO2>(dimState, dimAction, actorNetNew, actorNetOld, criticNetNew);
}

void PPO2::train(MemoryBase *memory)
{
	PPOMemory *ppomem = dynamic_cast<PPOMemory *>(memory);
	double aloss = 0, closs = 0, clipFraction_ = 0; // debug information
	double entropy_stat = 0;						// record the entropy of action distribution
	double kldiv_ = 0;
	// int epochs = 1; // epochs per train
	int memoryLen;

	memoryLen = ppomem->getMemoryLength();
	if (memoryLen <= 0)
	{
		cout << "Warning: PPO tries to sample an empty memory" << endl;
		return;
	}
	vector<vector<double>> state(memoryLen, std::vector<double>(stateDim, 0));
	vector<vector<double>> stateNext(memoryLen, std::vector<double>(stateDim, 0));
	vector<vector<double>> action(memoryLen, std::vector<double>(actionDim, 0));
	// vector<double> _value(memoryLen, 0);
	vector<double> reward(memoryLen, 0);
	std::vector<int> iterminated(memoryLen, 0);
	std::vector<int> expLen(ppomem->expNum, 0);

	ppomem->Sample(state, action, stateNext, reward, iterminated, expLen);

	// vector<double> rwtemp(1,0);
	// for (int i = 0; i < memoryLen; i++) {
	//	// reward norm
	//	rewardRt +=  GAMMA * reward[i];
	//	rwtemp[0] = rewardRt;
	//	rwStat.update(rwtemp);
	//	reward[i] /= rwStat.getStd()[0];

	//	// state norm
	//	stateStat.update(state[i]);
	//	for (int j = 0; j < stateDim; j++) {
	//		state[i][j] = (state[i][j] - stateStat.getMean()[j]) /
	// stateStat.getStd()[j]; 		stateNext[i][j] = (stateNext[i][j] -
	// stateStat.getMean()[j]) / stateStat.getStd()[j];
	//	}
	//	if (iterminated[i] != 0) {
	//		rwStat.reset();
	//		rewardRt = 0;
	//		//stateStat.reset();
	//	}
	//}

	torch::Tensor s = TorchAPI::from_vector(state);
	torch::Tensor s_ = TorchAPI::from_vector(stateNext);

	// calculate old action prob
	torch::Tensor dist_old = actorNetOld->forward(s).detach();
	torch::Tensor mu_old = dist_old.index_select(
		1, torch::linspace(0, actionDim - 1, actionDim, torch::kLong));
	torch::Tensor sigma_old = dist_old.index_select(
		1,
		torch::linspace(actionDim, 2 * actionDim - 1, actionDim, torch::kLong));

	torch::Tensor actionTensor = TorchAPI::from_vector(action);

	// torch::Tensor old_probs_all =
	// 	log_prob(actionTensor, mu_old, sigma_old).detach();
	torch::Tensor old_probs_all =
		log_prob_tanhSuppressed(actionTensor, mu_old, sigma_old).detach();

	if (torch::isnan(old_probs_all).any().item<bool>())
	{
		cout << "detcted nan in old_probs_all" << endl;
		cout << "mu_old: " << mu_old.mean() << endl;
		cout << "sigma_old: " << sigma_old.mean() << endl;
		cout << "actionTensor: " << actionTensor.mean() << endl;
	}

	// torch::Tensor val = criticNetOld(s).detach();
	// torch::Tensor val_ = criticNetOld(s_).detach();
	torch::Tensor val = criticNetNew->forward(s).detach();
	torch::Tensor val_ = criticNetNew->forward(s_).detach();
	auto vs = val.accessor<float, 2>();
	auto vs_ = val_.accessor<float, 2>();

	// cout << val.sizes() << endl;

	vector<double> returnsVec(
		memoryLen, 0); // the returns used by critci loss. Here we use GAE.
	vector<double> adv(memoryLen, 0);
	int offset = 0; // offset in the array that stores several trajectories
	for (int ntraj = 0; ntraj < ppomem->expNum; ntraj++)
	{
		// gae advantage
		double gae = 0;		  // generalized advantage estimation
		double lambda = 0.99; // a GAE hyper parameter;
		// double advmean = 0;
		// double advstd = 0;

		for (int i = offset + expLen[ntraj] - 1; i >= offset; i--)
		{
			double delta =
				reward[i] + GAMMA * (1 - iterminated[i]) * vs_[i][0] - vs[i][0];
			// if terminated, then gae is recalculated; otherwise accumulate.
			gae = delta + GAMMA * lambda * gae * (1 - iterminated[i]);
			// returnsVec[i] = gae;

			// adv[i] = gae - vs[i][0];
			adv[i] = gae;
			returnsVec[i] = gae + vs[i][0];
		}
		offset += expLen[ntraj];
	}

	// normalize adv
	advmean = 0;
	advstd = 0;
	for (int i = 0; i < memoryLen; i++)
	{
		advmean += adv[i];
	}
	advmean /= memoryLen;
	for (int i = 0; i < memoryLen; i++)
	{
		advstd += (adv[i] - advmean) * (adv[i] - advmean);
	}
	advstd = sqrt(advstd / memoryLen) + 1e-6;

	torch::Tensor advTensor = (TorchAPI::from_vector(adv).reshape({memoryLen}) - advmean) / advstd; // using whole batch normalization

	// torch::Tensor advTensor = (TorchAPI::from_vector(adv).reshape({memoryLen})); // not using whole batch normalization

	// torch::Tensor advTensor2 = (TorchAPI::from_vector(adv).reshape({memoryLen}) - advmean) / advstd; // using whole batch normalization
	// std::cout << "adv mean=" << advmean << ", std=" << advstd << std::endl;
	// std::cout << "advTensor2 mean=" << advTensor2.mean().item().toDouble()
	// 		  << ", std=" << advTensor2.std().item().toDouble() << std::endl;

	torch::Tensor returnsTensor = TorchAPI::from_vector(returnsVec);

	int batchNum = memoryLen / _batchSize;
	for (int ep = 0; ep < epochs; ep++)
	{
		// start mini batch gradient descent
		// shuffle the memory squence

		// vector<int> indices(memoryLen);
		// for (int i = 0; i < memoryLen; i++)
		// {
		// 	indices[i] = i;
		// }
		// // std::mt19937 rng(std::random_device());
		// // std::shuffle(indices.begin(), indices.end(), rng);
		// std::shuffle(indices.begin(), indices.end(), rd::re);

		// // divided into several batches...

		// torch::Tensor batchidx = TorchAPI::from_vector<int>(indices)
		// 							 .toType(torch::kLong)
		// 							 .reshape({memoryLen});

		// get shuffled indices using torch api
		torch::Tensor batchidx = torch::randperm(memoryLen, torch::dtype(torch::kLong));

		// torch::Tensor old_probTensor = TorchAPI::from_vector(_actionProb);
		// torch::Tensor old_probTensor = actorNetOld(s);

		for (int i = 0; i < batchNum; i++)
		{
			// torch::Tensor lspace = torch::linspace(_batchSize * i, _batchSize * (i
			// + 1), 1, torch::dtype(torch::kInt16));
			torch::Tensor lspace =
				torch::linspace(_batchSize * i, _batchSize * (i + 1) - 1, _batchSize,
								torch::dtype(torch::kLong));
			torch::Tensor tempidx = batchidx.index_select(0, lspace);

			// ///////////////////////////
			// MiniBatchTrainInfo mbInfo;
			MiniBatchTrainInfo mbInfo = trainMiniBatch(
				tempidx, s, actionTensor, old_probs_all, returnsTensor, advTensor,
				actorNetNew, criticNetNew,
				actionDim, clip, entropy_loss_coeff);
			// accumulate stats
			clipFraction_ += mbInfo.clip_fraction;
			kldiv_ += mbInfo.kl_divergence;
			aloss += mbInfo.actor_loss_bmean;
			closs += mbInfo.critic_loss_bmean;
			entropy_stat += mbInfo.entropy_bmean;
			// update networks
			actorOptimizer.zero_grad();
			mbInfo.actor_loss.backward();
			double totalNorm_actorgrad = torch::nn::utils::clip_grad_norm_(actorNetNew->parameters(), 10);
			actorOptimizer.step();

			criticOptimizer.zero_grad();
			mbInfo.critic_loss.backward();
			double totalNorm_criticgrad = torch::nn::utils::clip_grad_norm_(criticNetNew->parameters(), 10);
			criticOptimizer.step();
			// //////////////////////////////////

			// torch::Tensor states = s.index_select(0, tempidx);
			// torch::Tensor actions = actionTensor.index_select(0, tempidx);
			// // torch::Tensor old_probs = old_probTensor.index_select(0, tempidx);

			// // calculate old action prob
			// torch::Tensor old_probs = old_probs_all.index_select(0, tempidx);

			// // calculate new action prob
			// torch::Tensor dist = actorNetNew(states);
			// torch::Tensor mu = dist.index_select(
			// 	1, torch::linspace(0, actionDim - 1, actionDim, torch::kLong));
			// torch::Tensor sigma =
			// 	dist.index_select(1, torch::linspace(actionDim, 2 * actionDim - 1,
			// 										 actionDim, torch::kLong));

			// // torch::Tensor new_probs = log_prob(actions, mu, sigma);
			// torch::Tensor new_probs = log_prob_tanhSuppressed(actions, mu, sigma); // tanh suppressed

			// torch::Tensor probRatio = (new_probs - old_probs).exp();

			// clipFraction_ += torch::mean(torch::greater(torch::abs(probRatio - 1.0), clip).to(torch::kFloat)).item().toDouble();
			// // std::cout << "epoch: " << ep << ", batch: " << i
			// // 		  << ", clipFraction_=" << clipFraction_ << std::endl;

			// kldiv_ += -probRatio.log().mean().item().toDouble();
			// // critic net
			// torch::Tensor criticVal = criticNetNew(states);

			// // minibatch adv
			// torch::Tensor returns = returnsTensor.index_select(0, tempidx).detach();
			// torch::Tensor advmbTensor = advTensor.index_select(0, tempidx);
			// advmbTensor = (advmbTensor - advmbTensor.mean()) / (advmbTensor.std() + 1e-8); // minibatch normalize

			// // actor loss
			// torch::Tensor weighted_probs = advmbTensor * probRatio.squeeze();
			// // cout << "dim1=" << advTensor.index_select(0, tempidx) << endl;

			// // clipped weighted probablity
			// torch::Tensor clipped_weighted_probs = advmbTensor * torch::clamp(probRatio.squeeze(), 1 - clip, 1 + clip);

			// // cout << "probRatio=" << probRatio.squeeze() << endl;
			// // cout << "advmbTensor=" << advmbTensor.squeeze() << endl;
			// // cout << "loss before mean=" << torch::min(weighted_probs, clipped_weighted_probs) << endl;

			// // torch::Tensor policy_entropy = torch::mean(
			// // 	entropy(sigma)); // negative of entropy: to encourage exploaration
			// torch::Tensor policy_entropy = torch::mean(
			// 	entropy_tanhSuppressed(sigma, actions)); // negative of entropy: to encourage exploaration

			// torch::Tensor actor_loss =
			// 	-torch::mean(torch::min(weighted_probs, clipped_weighted_probs)) -
			// 	entropy_loss_coeff * policy_entropy;

			// // cout << "sigma=" << sigma << endl;
			// // cout << "entropy=" << policy_entropy << endl;

			// // critic loss
			// // torch::Tensor returns = (advTensor.index_select(0, tempidx) +
			// // val.index_select(0, tempidx)).detach(); use td target as target for
			// // critic net

			// torch::Tensor critic_loss =
			// 	torch::mean((returns - criticVal) * (returns - criticVal));
			// // torch::Tensor critic_loss = (returns - criticVal).square().mean();

			// // cout << "actor_loss = " << endl << actor_loss << endl;
			// // cout << "critic_loss = " << endl << critic_loss << endl;

			// aloss += actor_loss.item().toDouble();
			// closs += critic_loss.item().toDouble();
			// entropy_stat += policy_entropy.item().toDouble();

			// actorOptimizer.zero_grad();
			// actor_loss.backward();
			// double totalNorm_actorgrad = torch::nn::utils::clip_grad_norm_(actorNetNew->parameters(), 10);
			// actorOptimizer.step();

			// criticOptimizer.zero_grad();
			// critic_loss.backward();
			// double totalNorm_criticgrad = torch::nn::utils::clip_grad_norm_(criticNetNew->parameters(), 10);
			// criticOptimizer.step();

			if (isnan(aloss) || isnan(closs) || abs(aloss) > 1e4)
			{
				cout << "detected nan in loss function" << endl;
				// cout << "old_probs=" << old_probs.mean() << endl;
				// cout << "new_probs=" << new_probs.mean() << endl;
				// cout << "advTensor=" << advTensor.mean() << endl;
				// // cout << "advTensor" << advTensor << endl;
				cout << "probRatio=" << mbInfo.probRatio_bmean << endl;
				// // cout << "new_probs" << new_probs << endl << "old_probs" << old_probs
				// << endl;
				// cout << "clamped ratio="
				// 	 << torch::clamp(probRatio.squeeze(), 1 - clip, 1 + clip).mean() << endl;
				// cout << "clipped_weighted_probs=" << clipped_weighted_probs.mean() << endl;
				// cout << "weighted_probs=" << weighted_probs.mean() << endl;
				// cout << "min=" << torch::min(weighted_probs, clipped_weighted_probs).mean()
				// 	 << endl;
				// exit(-1);
			}

			if (i == 0 && ep == 0)
			{
				// normal info
				std::cout
					<< "actor_loss= " << mbInfo.actor_loss_bmean
					<< ", critic_loss= " << mbInfo.critic_loss_bmean << std::endl;
				// // full debug info
				// std::cout << "actor grad norm: " << totalNorm_actorgrad
				// 		  << ", critic grad norm: " << totalNorm_criticgrad << std::endl;

				// std::cout << "policy entropy=" << mbInfo.entropy_bmean
				// 		  << ", actor_loss= " << mbInfo.actor_loss_bmean
				// 		  << ", critic_loss= " << mbInfo.critic_loss_bmean
				// 		  << ", clipFraction_= " << mbInfo.clip_fraction << std::endl;
			}
		}
	}
	aloss /= batchNum * epochs;
	closs /= batchNum * epochs;
	clipFraction_ /= batchNum * epochs;
	kldiv_ /= batchNum * epochs;
	entropy_stat /= batchNum * epochs;

	episodeMeanALoss =
		(episodeMeanALoss * _episodeTrainCnt + aloss) / (_episodeTrainCnt + 1);
	episodeMeanCLoss =
		(episodeMeanCLoss * _episodeTrainCnt + closs) / (_episodeTrainCnt + 1);
	episodeMeanValue =
		(episodeMeanValue * _episodeTrainCnt + val.mean().item().toDouble()) / (_episodeTrainCnt + 1);
	clipFraction =
		(clipFraction * _episodeTrainCnt + clipFraction_) / (_episodeTrainCnt + 1);
	kldiv =
		(kldiv * _episodeTrainCnt + kldiv_) / (_episodeTrainCnt + 1);

	episodeMeanEntropy =
		(episodeMeanEntropy * _episodeTrainCnt + entropy_stat) / (_episodeTrainCnt + 1);

	_episodeTrainCnt++;

	copyParam(); // hard copy

	ppomem->ClearMemory();

	// deltas = r + self.gamma * (1.0 - dw) * vs_ - vs
	// for delta, d in zip(reversed(deltas.flatten().numpy()),
	// reversed(done.flatten().numpy())) :
	//	gae = delta + self.gamma * self.lamda * gae * (1.0 - d)
	//	adv.insert(0, gae)
	//	adv = torch.tensor(adv, dtype = torch.float).view(-1, 1)
	//	v_target = adv + vs
	//	if self.use_adv_norm : # Trick 1:advantage normalization
	//		adv = ((adv - adv.mean()) / (adv.std() + 1e-5))
};

void PPO2::decideAction(const std::vector<std::vector<double>> &state,
						std::vector<std::vector<double>> &action,
						bool iexplore)
{
	// vector<double> action(state.size());
	//  check the size of action
	if (action.size() != state.size())
	{
		cout << "In PPO2::decideAction: size of action and state vector mismatch."
			 << endl;
		exit(-1);
	}

	torch::NoGradGuard no_grad_guard;
	torch::Tensor stateTensor = TorchAPI::from_vector(state);
	// torch::Tensor stateMean =
	// 	TorchAPI::from_vector(stateStat.getMean()).transpose(0, 1);
	// torch::Tensor stateStd =
	// 	TorchAPI::from_vector(stateStat.getStd()).transpose(0, 1);

	// cout << "stateTensor" << stateTensor << endl;
	// cout << "stateMean" << stateMean << endl << "stateStd" << stateStd << endl;

	// stateTensor = (stateTensor - stateMean) / stateStd;
	// cout << "stateTensor" << stateTensor << endl;

	// output distribution of action
	torch::Tensor dist =
		actorNetOld->forward(stateTensor); // always use old net to decide action
										   // get value from critic
										   // torch::Tensor valueTensor = criticNetNew->forward(stateTensor);

	torch::Tensor tempidx = torch::linspace(0, actionDim - 1, actionDim, torch::kLong);
	torch::Tensor mu = dist.index_select(1, tempidx);
	// torch::Tensor mu = torch::index_select(dist, 1, tempidx);

	torch::Tensor tempidx2 = torch::linspace(actionDim, 2 * actionDim - 1, actionDim, torch::kLong);
	torch::Tensor sigma = dist.index_select(1, tempidx2);

	torch::Tensor actionTensor = torch::zeros_like(mu);

	// sample action
	actionTensor = iexplore ? at::normal(mu, sigma) : mu;
	actionTensor = torch::tanh(actionTensor); // tanh to limit the range of action
	// actionTensor = torch::clamp(actionTensor, -1 + 1e-3, 1 - 1e-3); // avoid numerical instability

	// cout << mu << endl;
	// cout << mu.sizes() << endl;

	// cout << sigma << endl;
	// cout << sigma.sizes() << endl;

	auto ac = actionTensor.accessor<float, 2>();
	// auto logprobac = logprobTensor.accessor<float, 2>();

	// if (actionProb.size() != action.size())
	// {
	// 	actionProb.resize(action.size());
	// }

	for (int i = 0; i < action.size(); i++)
	{
		// actionProb[i] = 0; // not used now
		for (int j = 0; j < actionDim; j++)
		{

			action[i][j] = ac[i][j]; // asign action value

			if (isnan(action[i][j]))
			{
				cout << "isnan in ppo2" << endl;
			}
		}
	}
}

void PPO2::initialize(const std::string &cfgContent, const Config &config)
{
	// copyStep = 100; // every copyStep replace the parameters of target net
	// copyStep = config.Read<int>("copy step", 100);
	GAMMA = config.Read<double>("GAMMA", 0.99);
	_lr_initial = config.Read<double>("lr");
	_lr_zero_episode = config.Read<double>("lr zero episode", -1);
	_lr_decay = (_lr_zero_episode > 0);
	_lr_critic_ratio = config.Read<double>("lr critic ratio", 1.0);

	_batchSize = config.Read<int>("batch size");
	_countTrain = 0;
	rewardRt = 0; // used for reward normalization

	epochs = config.Read<int>("epochs");
	entropy_loss_coeff = config.Read<double>("entropy loss coeff", 0.01);
	ppoMemoryNum = config.Read<int>("memory number", 1);
	clip = config.Read<double>("clip");
}

void PPO2::model_saver(const std::string &path, const std::string &suffix)
{
	ofstream testf;
	testf.open(path + "/actor_old_net" + suffix + ".pt", ios::out);
	if (!testf)
	{
		cout << "Directory inexistent when saving model. Model not saved." << endl;
		return;
	}
	torch::save(actorNetOld, path + "/actor_old_net" + suffix + ".pt");
	torch::save(actorNetNew, path + "/actor_new_net" + suffix + ".pt");
	// torch::save(criticNetOld, path + "/critic_old_net" + suffix + ".pt");
	torch::save(criticNetNew, path + "/critic_new_net" + suffix + ".pt");
	return;
}

void PPO2::model_loader(const std::string &path, const std::string &suffix)
{
	torch::load(actorNetOld, path + "/actor_old_net" + suffix + ".pt");
	torch::load(actorNetNew, path + "/actor_new_net" + suffix + ".pt");
	// torch::load(criticNetOld, path + "/critic_old_net" + suffix + ".pt");
	torch::load(criticNetNew, path + "/critic_new_net" + suffix + ".pt");
	return;
}

void PPO2::recParam(const char *path)
{
	vector<vector<double>> whis =
		TorchAPI::as2Dvector<double>(std::dynamic_pointer_cast<PPO2ActorNetImpl>(actorNetNew)->f1->weight.detach());
	ofstream outfile;
	string dir(path);

	outfile.open(dir + "/whis.txt", ios_base::out | ios_base::app);
	for (int i = 0; i < whis.size(); i++)
	{
		for (int j = 0; j < whis[0].size(); j++)
		{
			outfile << whis[i][j] << " ";
		}
	}
	outfile << endl;
	outfile.close();

	vector<vector<double>> his =
		TorchAPI::as2Dvector<double>(std::dynamic_pointer_cast<PPO2ActorNetImpl>(actorNetNew)->f1->weight.detach());

	outfile.open(dir + "/bhis.txt", ios_base::out | ios_base::app);
	for (int i = 0; i < his.size(); i++)
	{
		for (int j = 0; j < his[0].size(); j++)
		{
			outfile << his[i][j] << " ";
		}
	}
	outfile << endl;
	outfile.close();
	return;
}

// to clear record file
void PPO2::clearRec(const char *path)
{
	string dir(path);
	ofstream outfile;
	outfile.open(dir + "/whis.txt", ios_base::out);
	outfile.close();
	ofstream outfile1;
	outfile1.open(dir + "/bhis.txt", ios_base::out);
	outfile1.close();
	return;
}

void PPO2::copyParam()
{
	torch::autograd::GradMode::set_enabled(false);
	// copy parameter from actor_new to old
	auto actorOld_params = actorNetOld->named_parameters(true);
	auto actorNew_params = actorNetNew->named_parameters(true);

	for (auto &val : actorNew_params)
	{
		auto name = val.key();
		auto *t = actorOld_params.find(name);
		if (t != nullptr)
		{
			// t->copy_(val.value());
			t->copy_(val.value().detach()); // hard copy
		}
		else
		{
			std::cout << "Key " << val.key() << " is not found" << std::endl;
		}
	}

	// auto criticOld_params = criticNetOld->named_parameters(true);
	// auto criticNew_params = criticNetNew->named_parameters(true);
	// for (auto& val : criticNew_params) {
	//	auto name = val.key();
	//	auto* t = criticOld_params.find(name);
	//	if (t != nullptr) {
	//		//t->copy_(val.value());
	//		t->copy_(val.value().detach()); // hard copy
	//	}
	//	else {
	//		std::cout << "Key " << val.key() << " is not found" <<
	// std::endl;
	//	}
	// }

	torch::autograd::GradMode::set_enabled(true);
	// std::cout << "copy parameter" << endl;
}

void PPO2::setNewEp(int ep)
{
	double lr = _lr_initial;
	if (_lr_decay)
	{
		lr = max(0.0, _lr_initial * (1.0 - double(ep) / double(_lr_zero_episode)));
	}
	cout << "lr=" << lr << endl;
	TorchAPI::SetLR<torch::optim::Adam, torch::optim::AdamOptions>(actorOptimizer,
																   lr);
	TorchAPI::SetLR<torch::optim::Adam, torch::optim::AdamOptions>(
		criticOptimizer, lr * _lr_critic_ratio);
	episodeMeanALoss = 0;
	episodeMeanCLoss = 0;
	episodeMeanValue = 0;
	episodeMeanEntropy = 0;
	clipFraction = 0;
	kldiv = 0;
	_episodeTrainCnt = 0;
}
