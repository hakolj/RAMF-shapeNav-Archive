#include "Balloon.h"
#include "Geography.h"
#include "Fop.h"
#include "Random.h"
using namespace std;

Balloon::Balloon(unsigned amatternum) : ActiveMatter(amatternum), pos(vectors3d(amatternum, vec3d::Zero())),
										poslla(vectors3d(amatternum, vec3d::Zero())),
										vp_new(vectors3d(amatternum, vec3d::Zero())), vp_old(vectors3d(amatternum, vec3d::Zero())),
										vppf_new(vectors3d(amatternum, vec3d::Zero())),
										accel_new(vectors3d(amatternum, vec3d::Zero())), accel_old(vectors3d(amatternum, vec3d::Zero())),
										ufpf(vectors3d(amatternum, vec3d::Zero())),
										_istep1trans(false), _istep1rot(false), fluidVelAtPoint(NULL), fluidDensityAtPoint(NULL), envLmin(), envLmax(),
										_initRange(), _dimension(0), _resetEpisode(0), _updateTimeStep(0),
										dragcoeff(10), aproj(1), volume(1), mass(vector<double>(amatternum, 1)), fluidDensity(amatternum),
										massDefault(1), power(amatternum, 0)
{
}

void Balloon::getInfo()
{
	// the obtained fluid vel is in ENU coordinate.
	fluidVelAtPoint->fluidVelAtPoint(poslla, ufpf);
	fluidDensityAtPoint->fluidDensityAtPoint(poslla, fluidDensity);
}

void Balloon::useInfo()
{
	// no need to convert! The velocity data is already stored as ENU.
}

void Balloon::update(double timestep, const std::vector<int> &updateMask)
{
	_updateTimeStep = timestep;

#if DEBUG
#else
#pragma omp parallel for
#endif
	for (int i = 0; i < amatternum; i++)
	{
		if (updateMask[i] == 0)
			continue;
		// vec3d vppf(3, 1);
		vec3d drag(3, 1);	 // drag in ENU frame
		vec3d forcepf(3, 1); // in ENU frame
		vec3d force(3, 1);	 // in ECEF frame, not local ENU frame.

		// calculating force in ENU frame.
		vec3d deltaV = ufpf[i] - vppf_new[i]; //
		drag = 0.5 * fluidDensity[i] * dragcoeff * aproj * deltaV.norm() * deltaV;

		// cout << "deltaV=" << deltaV << endl;
		// cout << "magnitude=" << deltaV.norm() << endl;

		forcepf = drag;
		forcepf(2) = forcepf(2) + (fluidDensity[i] * volume - mass[i]) * geo::G; // vertical direction;
		geo::ENUToECEF(forcepf, poslla[i], force);
		// from now on, all calculation is in ECEF frame.
		accel_new[i] = force / mass[i];
		/*	cout << "accel_new=" << accel_new[i] << endl;*/

		if (_istep1trans)
		{
			accel_old[i] = accel_new[i];
		}
		// adams-bashforth, calculating new velocity
		// vp_new[i] += timestep * (1.5 * accel_new[i] - 0.5 * accel_old[i]);
		// euler
		vp_new[i] += timestep * accel_new[i];
		/*geo::ENUToECEF(vppf, poslla[i], vp_new[i]);*/

		if (_istep1trans)
		{
			vp_old[i] = vp_new[i];
		}
		// pos[i] += timestep * (1.5 * vp_new[i] - 0.5 * vp_old[i]);
		pos[i] += timestep * vp_new[i];

		accel_old[i] = accel_new[i];
		vp_old[i] = vp_new[i];

		// obtain lla for baloon position
		geo::ECEFToLLA(pos[i], poslla[i]);
		// convert new velocity into ENU frame at new pos
		geo::ECEFToENU(vp_new[i], poslla[i], vppf_new[i]);
		if (isnan(pos[0](0)))
		{
			cout << "nan value in baloon swimmers update" << endl;
		}

		// power
		power[i] = min(1.0, power[i] + powerRechargeRate * timestep);
	}
	_istep1rot = false;
	_istep1trans = false;
}

void Balloon::initialize(const std::string &cfgContent, const Config &config)
{

	initMode = config.Read<int>("initMode", 0);
	if (initMode == 1)
	{

		std::string str = config.Read<std::string>("initCenter");
		std::vector<double> center = Fop::loadvec1d<double>(str);

		if (center.size() > 3)
		{
			string errmsg = "Balloon::initCenter: center position has a dimension greater than 3.";
			cout << errmsg << endl;
			throw runtime_error(errmsg);
		}
		initCenter[0] = center[0];
		initCenter[1] = center[1];
		initCenter[2] = center[2];

		str = config.Read<std::string>("initRange");
		std::vector<double> range = Fop::loadvec1d<double>(str);
		_initRange[0] = range[0];
		_initRange[1] = range[1];
		_initRange[2] = range[2];
	}
	aproj = config.Read<double>("projection area");
	volume = config.Read<double>("volume");
	massDefault = config.Read<double>("defult mass");
	dragcoeff = config.Read<double>("drag coefficient");
	for (int i = 0; i < amatternum; i++)
	{
		mass[i] = massDefault;
	}

	powerRechargeRate = config.Read<double>("power recharge rate", 0);
	powerUsageRate = config.Read<double>("power usage rate", 0);

	return;
}

void Balloon::reset()
{

	_resetCount = 0;
	if (initMode == 0)
		initPos3D(envLmin[0], envLmin[1], envLmin[2], envLmax[0], envLmax[1], envLmax[2]);
	else if (initMode == 1)
	{
		initPos3D(initCenter[0] - _initRange[0], initCenter[1] - _initRange[1], initCenter[2] - _initRange[2],
				  initCenter[0] + _initRange[0], initCenter[1] + _initRange[1], initCenter[2] + _initRange[2]);
	}
	resetFlowInfo();
	for (int i = 0; i < amatternum; i++)
	{
		vppf_new[i] = ufpf[i];
		geo::ENUToECEF(vppf_new[i], poslla[i], vp_new[i]);

		power[i] = 1; // reset power
	}

	// update with very small time step to reach a stable initial condition
	vector<int> mask(amatternum, 1);
	for (int j = 0; j < 10; j++)
	{
		update(0.5, mask);
		getInfo();
		useInfo();
	}

	return;
}

void Balloon::resetFlowInfo()
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		fluidVelAtPoint->fluidVelAtPoint(poslla, ufpf);
		fluidDensityAtPoint->fluidDensityAtPoint(poslla, fluidDensity);
		// ufpf[i] = vec3d::Zero();
		// fluidDensity[i] = 0;
	}
}

void Balloon::BoundaryCondition(std::shared_ptr<Fluid> fluid)
{
	std::string bc = fluid->boundaryType();
	// nothing to do for periodic bc
	// double L[3];
	// fluid->getDomainSize(L[0], L[1], L[2]);

	double Lmin[3], Lmax[3];
	fluid->getDomainBound(Lmin[0], Lmax[0], Lmin[1], Lmax[1], Lmin[2], Lmax[2]);

	// wall bc
	for (int i = 0; i < amatternum; i++)
	{
		// loop for three dims
		for (int dim = 0; dim < 3; dim++)
		{
			if (bc[dim] == 'W')
			{
				if (poslla[i](dim) < Lmin[dim])
				{
					poslla[i](dim) = Lmin[dim] + Lmin[dim] - poslla[i](dim);
					// pos[i](dim) = 2 * this->a - pos[i](dim);
					vp_new[i](dim) *= -1;
				}
				else if (poslla[i](dim) > Lmax[dim])
				{
					poslla[i](dim) = Lmax[dim] - (poslla[i](dim) - Lmax[dim]);
					// pos[i](dim) = 2 * (L[dim] - this->a) - pos[i](dim);
					vp_new[i](dim) *= -1;
				}
			}
		}
	}
}

// record trajectory
void Balloon::recordTraj(std::vector<Trajectory>::iterator begin,
						 std::vector<Trajectory>::iterator end)
{
	std::vector<double> snapShot(trajSize(), 0);
	// // delete[] snapShot;
	// // double *snapShot = new double[trajSize()];
	// for (int i = 0; i < trajs.size(); i++)
	// {
	// 	for (int j = 0; j < 3; j++)
	// 	{
	// 		snapShot[j] = pos[i][j];
	// 		snapShot[j + 3] = poslla[i][j];
	// 		snapShot[j + 6] = vppf_new[i][j];
	// 		snapShot[j + 9] = accel_new[i][j];
	// 		snapShot[j + 12] = ufpf[i][j];
	// 	}

	// 	trajs[i].Record(snapShot);
	// }
	// // delete[] snapShot;

	int n = 0;
	for (auto it = begin; it != end; ++it)
	{
		for (int j = 0; j < 3; j++)
		{
			snapShot[j] = pos[n][j];
			snapShot[j + 3] = poslla[n][j];
			snapShot[j + 6] = vppf_new[n][j];
			snapShot[j + 9] = accel_new[n][j];
			snapShot[j + 12] = ufpf[n][j];
		}
		(*it).Record(snapShot);
		n++;
	}
}

void Balloon::initPos3D(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		poslla[i][0] = rd::randd() * (xmax - xmin) + xmin;
		poslla[i][1] = rd::randd() * (ymax - ymin) + ymin;
		poslla[i][2] = rd::randd() * (zmax - zmin) + zmin;

		geo::LLAToECEF(poslla[i], pos[i]);
		vp_new[i] = vec3d::Zero();
		vp_old[i] = vec3d::Zero();
		_istep1trans = true;
	}
	return;
}

void Balloon::dump(const char *path, int step)
{
	ofstream os;

	char stepstr[10];
	cout << "dumping active matter field at step: " << step << endl;
	sprintf(stepstr, "%.7i", step);
	string fullpath = string(path) + "/partfield" + string(stepstr) + ".txt";
	os.open(fullpath, ios::out | ios::trunc);
	os.precision(8);
	os << scientific;
	for (unsigned i = 0; i < amatternum; i++)
	{
		os << pos[i][0] << " " << pos[i][1] << " " << pos[i][2] << " " << poslla[i][0] << " " << poslla[i][1] << " " << poslla[i][2] << " " << vp_new[i][0] << " " << vp_new[i][1] << " " << vp_new[i][2] << " " << accel_new[i][0] << " " << accel_new[i][1] << " " << accel_new[i][2] << " " << ufpf[i][0] << " " << ufpf[i][1] << " " << ufpf[i][2] << " ";
		os << endl;
	}
	os.close();

	FILE *fp;
	fullpath = string(path) + "/partfield" + string(stepstr) + ".bin";
	fp = fopen(fullpath.c_str(), "wb");
	for (unsigned i = 0; i < amatternum; i++)
	{
		fwrite(&pos[i][0], sizeof(double), 3, fp);
		fwrite(&poslla[i][0], sizeof(double), 3, fp);
		fwrite(&vp_new[i][0], sizeof(double), 3, fp);
		fwrite(&accel_new[i][0], sizeof(double), 3, fp);
		fwrite(&ufpf[i][0], sizeof(double), 3, fp);
	}
	fclose(fp);
}

void Balloon::AdjustMass(double adjustMass, int index)
{
	double deltaMass = massDefault + adjustMass - mass[index];
	if (deltaMass > 0)
	{
		// pumped gas in, consumes power
		double powerReq = powerUsageRate * deltaMass;
		// if no enough power, use all remaining power
		mass[index] += min(deltaMass, power[index] / (powerUsageRate + 1e-8));
		power[index] = max(0.0, power[index] - powerReq);
	}
	else
	{
		mass[index] += deltaMass;
	}
};