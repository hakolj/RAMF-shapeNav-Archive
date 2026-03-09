#include "Surfer.h"
#include "Fop.h"
#include <string>
using namespace agent::navigation;

void MatrixExponential(const Eigen::Matrix3d &mat, Eigen::Matrix3cd &result)
{

	Eigen::EigenSolver<Eigen::Matrix3d> eigensolver(mat); // eigen solver

	if (eigensolver.info() != Eigen::ComputationInfo::Success)
	{
		std::cout << "Eigen solver failed" << std::endl;
		std::cout << "mat=" << mat << std::endl;
	}

	Eigen::Vector3cd evals = eigensolver.eigenvalues();	 // eigen values
	Eigen::Matrix3cd evecs = eigensolver.eigenvectors(); // eigen vectors (a matrix)

	// diagonalization
	result = Eigen::Matrix3cd::Zero();
	result(0, 0) = exp(evals(0));
	result(1, 1) = exp(evals(1));
	result(2, 2) = exp(evals(2));

	result = evecs * result * evecs.inverse(); // restore the matrix
}

void OptimalSurfingAgent::decideAction(const std::vector<std::vector<double>> &state,
									   std::vector<std::vector<double>> &action,
									   bool iexplore)
{

	Eigen::Vector3d targetDir;
	targetDir(0) = direction[0];
	targetDir(1) = direction[1];
	targetDir(2) = direction[2];
	Eigen::Matrix3d matExpTranspose;

	Eigen::Vector3d optimalDir;
	for (int i = 0; i < state.size(); i++)
	{

		int n = 0;
		for (int r = 0; r < 3; r++)
			for (int c = 0; c < 3; c++)
			{
				matExpTranspose(c, r) = state[i][n++] * _stateNormalizer; // transpose this matrix
			}
		optimalDir = matExpTranspose * targetDir;
		action[i][0] = optimalDir[0];
		action[i][1] = optimalDir[1];
		action[i][2] = optimalDir[2];
	}
}
void OptimalSurfingAgent::initialize(const std::string &cfgContent, const Config &config)
{
	direction = Fop::loadvec1d<double>(config.Read<std::string>("direction"));
	_stateNormalizer = config.Read<double>("state normalizer");
}