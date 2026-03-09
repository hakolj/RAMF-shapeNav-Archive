#include "pch.h"
#include "TrainManager.h"
#include "Config.h"
#include "Fop.h"
#include "Random.h"
#include <torch/torch.h>

using namespace std;

TrainManager::TrainManager(const std::string &configpath,
						   int subCaseIDlo, int subCaseIDhi) : episode_record(std::vector<int>(0)),
															   reward_record(std::vector<double>(0)),
															   loss_record(std::vector<double>(0)),
															   qvalue_record(std::vector<double>(0)),
															   subCaseIDlo(subCaseIDlo), subCaseIDhi(subCaseIDhi)
{
	_configPath = configpath; // save config path
	// load all config content into the string
	std::ifstream configFile(_configPath.c_str());
	_configContent = std::string(std::istreambuf_iterator<char>(configFile), std::istreambuf_iterator<char>());
	Config config(configpath, "GLOBAL");
	caseDir = config.Read<string>("caseDir"); // read case directory

	// set up all random seed
	randomSeed = config.Read<unsigned>("random seed");
	if (randomSeed < 0)
		randomSeed = (unsigned)std::time(nullptr);
	// int uns_rs = randomSeed >= 0 ? randomSeed : (unsigned)time(NULL);
	// set real random seed as randomSeed + subCaseID in TrainManager::Prepare()
	// srand(uns_rs);
	// rd::setRandomSeed(uns_rs);
	// torch::manual_seed(uns_rs);

	sensestep = config.Read<int>("sense step");
	learnstep = config.Read<int>("learn step");
	totalstep = config.Read<int>("total step");
	// taskCheckStep = config.Read<int>("task check step", sensestep);
	// timestepsize = config.Read<double>("time step");
	dumpstep = config.Read<int>("dump step", totalstep);
	episodenum = config.Read<int>("episode num");
	saveinterval = config.Read<int>("save interval");	// episode interval of saving training
	evalinterval = config.Read<int>("evalue interval"); // episode interval of evaluating strategy
	ilearn = config.Read<bool>("ilearn");
	iload = config.Read<bool>("iload");
	iloadBest = config.Read<bool>("iloadBest"); // whether loading the best model
	iTorch_cuda = config.Read<bool>("icuda");	// whether using cuda for pytorch

	inaive = config.Read<bool>("inaive", false);
	dumpItv = config.Read<int>("dump interval");
	memoryMode = config.Read<int>("memory mode");
	// trajnum = config.Read<int>("export traj num", 10);

	_currentSubCase = subCaseIDlo;

	return;
}

void TrainManager::recording(int episode, double reward, double loss, double qvalue)
{
	episode_record.push_back(episode);
	reward_record.push_back(reward);
	loss_record.push_back(loss);
	qvalue_record.push_back(qvalue);
	return;
}
void TrainManager::writeRecord(const std::string &path)
{
	std::ofstream outfile;

	if (ilearn)
		outfile.open(path + "/trainrec.txt", ios::out);
	else
		outfile.open(path + "/runrec.txt", ios::out);
	outfile << "episode\t"
			<< "reward\t";
	if (ilearn)
		outfile << "zplossos\t"
				<< "qvalue\t";
	outfile << endl;
	// outfile << std::showpoint << std::left << std::fixed << std::setprecision(8);
	outfile.setf(ios::showpoint | ios::left | ios::scientific);
	outfile.precision(8);
	int start = ilearn ? 0 : startEpisode;
	for (unsigned i = start; i < episode_record.size(); i++)
	{
		outfile << std::setw(8) << episode_record[i] << " ";
		outfile << std::setw(16) << reward_record[i] << " ";
		if (ilearn)
		{
			outfile << std::setw(16) << loss_record[i] << " ";
			outfile << std::setw(16) << qvalue_record[i] << " ";
		}
		outfile << endl;
	}
	outfile.close();
}

void TrainManager::readRecord(const std::string &path)
{
	episode_record.clear();
	reward_record.clear();
	loss_record.clear();
	qvalue_record.clear();
	std::ifstream infile;
	infile.open(path + "/trainrec.txt", ios::in);
	std::string line;
	getline(infile, line); // skip header
	while (infile.peek() != EOF)
	{
		getline(infile, line);
		stringstream ss;
		ss << line;
		int episode;
		double reward;
		double loss;
		double qvalue;
		ss >> episode >> reward >> loss >> qvalue;
		episode_record.push_back(int(episode));
		reward_record.push_back(reward);
		loss_record.push_back(loss);
		qvalue_record.push_back(qvalue);

		// cout << episode << " " << reward << " " << loss << " " << qvalue << endl;
	}
}

void TrainManager::writeRunRec(const std::string &path, int startepisode)
{
	std::ofstream outfile;
	outfile.open(path + "/runrec.txt", ios::out);
	outfile << "episode\t"
			<< "reward\t" << endl;
	// outfile << std::showpoint << std::left << std::fixed << std::setprecision(8);
	outfile.setf(ios::showpoint | ios::left | ios::scientific);
	outfile.precision(8);
	for (unsigned i = startepisode; i < episode_record.size(); i++)
	{
		outfile << std::setw(8) << episode_record[i] << " ";
		outfile << std::setw(16) << reward_record[i] << endl;
	}
	outfile.close();
}

// void TrainManager::dumpStateAction(std::string path, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action, int step)
// {
// 	ofstream os;

// 	char stepstr[10];
// 	cout << "dumping State and Action of step: " << step << endl;
// 	sprintf(stepstr, "%.7i", step);
// 	string fullpath = path + "/sttactfield" + string(stepstr) + ".txt";
// 	// string fullpath = _currentWorkingDir + "/field/sttactfield" + string(stepstr) + ".txt";
// 	os.open(fullpath, ios::out | ios::trunc);
// 	os.precision(8);
// 	os << scientific;
// 	int statedim = state[0].size();
// 	int actiondim = action[0].size();
// 	// int actionnum = action[0].size();
// 	os << statedim << " " << actiondim << endl;
// 	for (unsigned i = 0; i < state.size(); i++)
// 	{
// 		for (unsigned j = 0; j < statedim; j++)
// 		{
// 			os << state[i][j] << " ";
// 		}
// 		for (unsigned j = 0; j < actiondim; j++)
// 		{
// 			os << action[i][j] << " ";
// 		}
// 		os << endl;
// 	}
// 	os.close();
// }

// void TrainManager::dumpStateAction(const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action, int step)
// {
// 	dumpStateAction(_currentWorkingDir + "/field/", state, action, step);
// }

void TrainManager::setupNewSubCase(int subCaseID)
{
	this->_currentSubCase = subCaseID;
	setupDirectory();
	episode_record.clear();
	reward_record.clear();
	loss_record.clear();
	qvalue_record.clear();
	real_rs = randomSeed + subCaseID;
	std::cout << "seed=" << real_rs << std::endl;
	srand(real_rs);
	rd::setRandomSeed(real_rs);
	torch::manual_seed(real_rs);

	std::cout << "test random number:" << rd::randd() << std::endl;

	std::ofstream rdseedFile(_currentWorkingDir + "/randomSeed.txt");
	if (rdseedFile.is_open())
	{
		rdseedFile << real_rs;
		rdseedFile.close();
	}
}

void TrainManager::setupDirectory()
{
	Fop::makeDir(caseDir + "/");
	_currentWorkingDir = caseDir + "/" + std::to_string(_currentSubCase) + "/";
	Fop::makeDir(_currentWorkingDir);

	// write config content to the file
	std::ofstream cfgOutput(_currentWorkingDir + "/config.cfg");
	if (cfgOutput.is_open())
	{
		cfgOutput << _configContent;
		cfgOutput.close();
	}
	else
	{
		std::string errmsg = "Working direcotry is unavailable. Check config.cfg";
		std::cout << errmsg << std::endl;
		throw std::runtime_error(errmsg);
	}
	Fop::makeDir(_currentWorkingDir + "/field/");
	Fop::makeDir(_currentWorkingDir + "/his/");
	cout << "Working directory is set at " + _currentWorkingDir << endl;

	if (!iload)
	{
		// remove history files if not loading
#ifdef __linux__
		string cmd = "rm -f " + _currentWorkingDir + "/his/*";
#else
		string cmd = "del /F /Q " + _currentWorkingDir + "\\his\\*.*";
#endif
		system(cmd.c_str());
		logs.close();
		logs.open(_currentWorkingDir + "/log.txt");
	}
	else
	{
		logs.close();
		logs.open(_currentWorkingDir + "/log.txt", ofstream::out | ofstream::ate);
	}
}