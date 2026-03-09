#ifndef TRAINMANAGER_H
#define TRAINMANAGER_H
#include "vectypedef.h"
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "ActiveMatterInterface.h"
// a class to handle things about training
class TrainManager
{
public:
	std::vector<int> episode_record;
	std::vector<double> reward_record;
	std::vector<double> loss_record;
	std::vector<double> qvalue_record;
	std::string caseDir; // working directory of the whole case
	TrainManager() = delete;

	TrainManager(const std::string &configpath, int subCaseIDlo, int subCaseIDhi);

	void recording(int episode, double reward, double loss, double qvalue);
	void writeRecord(const std::string &path);
	void readRecord(const std::string &path);

	// used for writing record of running (without training)
	void writeRunRec(const std::string &path, int startepisode);
	// void saveTraining(const std::string& path);
	// void loadTraining(const std::string& path);
	int randomSeed;
	int sensestep;
	int learnstep;
	int totalstep;
	int dumpstep;
	// double timestepsize;
	int startEpisode;
	int episodenum;
	int evalinterval; // episode interval of evaluing policy
	int saveinterval; // episode interval of saving training
	int dumpItv;	  // episode interval of dumping fields
	bool ilearn;
	bool iload;
	bool inaive; // naive actor will always give the same action
	// int trajnum; // number of recorded trajectory
	// int taskCheckStep; // number of update time steps that check task is terminated
	bool iloadBest;	  // whether loading the best model
	bool iTorch_cuda; // whether using cuda for pytorch
	int memoryMode;	  // 0: classic memory pool; 1: ppo memory

	int real_rs; // the acutal random seed used in the simulation

	std::ofstream logs; // fstream for output logs

	void setupDirectory();
	void setupNewSubCase(int subCaseID);

	inline void setSubCaseID(int subCaseID) { _currentSubCase = subCaseID; }
	const std::string &getWorkingDir() const { return _currentWorkingDir; }
	const std::string &getCfgContent() const { return _configContent; }

	// void dumpStateAction(std::string path, const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action, int step);
	// void dumpStateAction(const std::vector<std::vector<double>> &state, const std::vector<std::vector<double>> &action, int step);
	// inline void nextSubCase() { _currentSubCase += 1; }

private:
	std::string _currentWorkingDir; // current working directory of the subcase, i.e. "caseDir/subCaseID/"
	int _currentSubCase;
	std::string _configPath;
	std::string _configContent; // the content of config file, stored as string
	int subCaseIDlo;			// lower bound of subCase ID range
	int subCaseIDhi;			// upper bound of subCase ID range
};

#endif