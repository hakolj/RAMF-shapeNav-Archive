#include "Airship.h"
#include "Fop.h"
#include "Random.h"

using namespace std;

void Airship::initialize(const std::string &path, const Config &config)
{
	// string inict = config.Read<string>("initial center", "0 0 0");
	// vector<double> clla = Fop::loadvec1d<double>(inict);
	// centerlla << clla[0], clla[1], clla[2];
	// lonrange = config.Read<double>("initial lon range", 0);
	// latrange = config.Read<double>("initial lat range", 0);
	InertialessSwimmer::initialize(path, config);
	return;
}

void Airship::update(double timestep, const std::vector<int> &updateMask)
{
	for (auto &elem : pos)
	{
		elem.setZero();
	}
	InertialessSwimmer::update(timestep, updateMask);
	for (int i = 0; i < amatternum; i++)
	{
		if (updateMask[i] == 0)
			continue;
		ENU2LLA(poslla[i], lastposlla[i], pos[i]);
	}
	lastposlla = poslla;
}

void Airship::getInfo()
{
	for (int i = 0; i < amatternum; i++)
	{
		gradu[i] = vec3d::Zero();
		gradv[i] = vec3d::Zero();
		gradw[i] = vec3d::Zero();
	}
	fluidVelAtPoint->fluidVelAtPoint(poslla, uf);
}

void Airship::ENU2LLA(vec3d &poslla, vec3d &lastposlla, const vec3d &pos)
{
	vec3d relaposecef, lastposecef;
	geo::ENUToECEF(pos, lastposlla, relaposecef);
	geo::LLAToECEF(lastposlla, lastposecef);
	geo::ECEFToLLA(relaposecef + lastposecef, poslla);
	poslla[2] = lastposlla[2];
}

void Airship::reset()
{
	_resetCount++;
	bool resetflag = _resetCount >= _resetEpisode;
	if (resetflag)
	{
		_resetCount = 0;
		// resetPosition(); // cannot use resetPosition bc Airship has its own pos init
		initPos3D();
		resetOrientation();
	}
	resetFlowInfo();
	return;
}

void Airship::initPos3D()
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		poslla[i][0] = rd::randd(-1.0, 1.0) * initBoxSize[0] + initCoreCenter[0];
		poslla[i][1] = rd::randd(-1.0, 1.0) * initBoxSize[1] + initCoreCenter[1];
		poslla[i][2] = rd::randd(-1.0, 1.0) * initBoxSize[2] + initCoreCenter[2];
		vp_new[i] = vec3d::Zero();
		vp_old[i] = vec3d::Zero();
		_istep1trans = true;
	}

	for (auto &elem : pos)
	{
		elem.setZero();
	}
	lastposlla = poslla;
	return;
}

void Airship::dump(const char *path, int step)
{
	ofstream os;
	char stepstr[10];
	sprintf(stepstr, "%.7i", step);
	string fullpath = string(path) + "/partfield" + string(stepstr) + ".txt";
	os.open(fullpath, ios::out | ios::trunc);
	os.precision(8);
	os << scientific;
	for (unsigned i = 0; i < amatternum; i++)
	{
		os << poslla[i][0] << " " << poslla[i][1] << " " << poslla[i][2] << " " << p2[i][0] << " " << p2[i][1] << " " << p2[i][2] << " " << vp_new[i][0] << " " << vp_new[i][1] << " " << vp_new[i][2] << " " << uf[i][0] << " " << uf[i][1] << " " << uf[i][2] << " " << gradu[i][0] << " " << gradu[i][1] << " " << gradu[i][2] << " " << gradv[i][0] << " " << gradv[i][1] << " " << gradv[i][2] << " " << gradw[i][0] << " " << gradw[i][1] << " " << gradw[i][2];
		os << endl;
	}
	os.close();

	FILE *fp;
	fullpath = string(path) + "/partfield" + string(stepstr) + ".bin";
	fp = fopen(fullpath.c_str(), "wb");
	for (unsigned i = 0; i < amatternum; i++)
	{
		fwrite(&poslla[i][0], sizeof(double), 3, fp);
		fwrite(&p2[i][0], sizeof(double), 3, fp);
		fwrite(&vp_new[i][0], sizeof(double), 3, fp);
		fwrite(&uf[i][0], sizeof(double), 3, fp);
		fwrite(&gradu[i][0], sizeof(double), 3, fp);
		fwrite(&gradv[i][0], sizeof(double), 3, fp);
		fwrite(&gradw[i][0], sizeof(double), 3, fp);
	}
	fclose(fp);
}