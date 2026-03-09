#include "pch.h"
#include "PointVehicle.h"
#include "MatrixOp.h"
#include "Fop.h"
#include "Random.h"

using namespace std;
PointVehicle::PointVehicle(int amatternum) : PointParticle(amatternum),
											 vswim(amatternum, 0), eg(0.0, -1.0, 0.0),
											 _swimAngVel(vectors3d(amatternum, vec3d::Zero()))
{
	return;
}

void PointVehicle::initialize(const std::string &path, const Config &config)
{
	// InitSenseStepCount(config);
	double vswim = config.Read<double>("vswim", 0.0);

	setMotility(vswim);
	int dim = config.Read("dimension", 3);
	// setDim(dim);
	int direction = 0;
	if (dim != 3)
	{
		direction = config.Read<int>("direction");
		if (direction != 2 && dim == 2)
		{
			throw std::runtime_error("PointVehicle: direction must be set to 2 for 2D simulation, but got: " + std::to_string(direction));
		}
	}
	coordSys = coord::CartCoordSys(dim, direction);

	_resetEpisode = config.Read<int>("reset episode", 0);
	_resetCount = _resetEpisode; // to ensure always reset for the first episode

	readInitMode(config);
	return;
}

void PointVehicle::reset()
{
	_resetCount++;
	bool resetflag = _resetCount >= _resetEpisode;

	if (resetflag)
	{
		_resetCount = 0;
		resetPosition();
		resetOrientation();
	}

	resetFlowInfo();
	return;
}

void PointVehicle::setMotility(double vswim)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		// const vec3d defalut_mdisp(0.0, 0.0, -1.0);
		this->vswim[i] = vswim;
	}
}

void PointVehicle::getInfo()
{
	fluidInfoAtPoint->infoAtPoint(pos, uf, gradu, gradv, gradw);
}

void PointVehicle::convertFrame(int id)
{
	using namespace mat;

	Eigen::Matrix3d Reul;
	Reul.row(0) = p0[id];
	Reul.row(1) = p1[id];
	Reul.row(2) = p2[id];
	// ee2R(ee[id], p0[id], p1[id], p2[id]);
	Eigen::Matrix3d gradmat(3, 3);
	gradmat.col(0) = gradu[id];
	gradmat.col(1) = gradv[id];
	gradmat.col(2) = gradw[id];
	Eigen::Matrix3d gradmatpf(3, 3);
	gradmatpf = matmul(matmul(Reul, gradmat), Reul.transpose());
	gradupf[id] = gradmatpf.col(0);
	gradvpf[id] = gradmatpf.col(1);
	gradwpf[id] = gradmatpf.col(2);
	ufpf[id] = matmul(Reul, uf[id]);
	return;
}

void PointVehicle::update(double timestep, const std::vector<int> &updateMask)
{

	_updateTimeStep = timestep;
	// vec3d vppf(3, 1), egpf(3, 1);
	// vec3d t_gyro(3, 1), t_rotate(3, 1), t_deform(3, 1), angvel(3, 1);
	// vec3d t_finert(3, 1);
	// vec3d vslippf(3, 1);

#pragma omp parallel for
	for (unsigned i = 0; i < amatternum; i++)
	{
		if (updateMask[i] == 0)
			continue; // skip the dead ones

		vec3d vp(3, 1);
		vp = uf[i] + vswim[i] * p2[i]; // use velocity in inertial frame
		_updateTrans(i, vp, timestep);
	}
	_istep1rot = false;
	_istep1trans = false;
}

// dump particle field
void PointVehicle::dump(const char *path, int step)
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
		os << i << " "; // dump the particle index
		os << pos[i][0] << " " << pos[i][1] << " " << pos[i][2] << " " << p2[i][0] << " " << p2[i][1] << " " << p2[i][2] << " " << vp_new[i][0] << " " << vp_new[i][1] << " " << vp_new[i][2] << " " << uf[i][0] << " " << uf[i][1] << " " << uf[i][2] << " " << gradu[i][0] << " " << gradu[i][1] << " " << gradu[i][2] << " " << gradv[i][0] << " " << gradv[i][1] << " " << gradv[i][2] << " " << gradw[i][0] << " " << gradw[i][1] << " " << gradw[i][2];
		os << endl;
	}
	os.close();

	FILE *fp;
	fullpath = string(path) + "/partfield" + string(stepstr) + ".bin";
	fp = fopen(fullpath.c_str(), "wb");
	for (unsigned i = 0; i < amatternum; i++)
	{
		fwrite(&pos[i][0], sizeof(double), 3, fp);
		fwrite(&p2[i][0], sizeof(double), 3, fp);
		fwrite(&vp_new[i][0], sizeof(double), 3, fp);
		fwrite(&uf[i][0], sizeof(double), 3, fp);
		fwrite(&gradu[i][0], sizeof(double), 3, fp);
		fwrite(&gradv[i][0], sizeof(double), 3, fp);
		fwrite(&gradw[i][0], sizeof(double), 3, fp);
	}
	fclose(fp);
}

void PointVehicle::SharpRotate90(int pn, int axis, int direction)
{
	if (axis == 0)
	{
		vec3d tempvec = p2[pn];
		if (direction > 0)
		{
			p2[pn] = -p1[pn];
			p1[pn] = tempvec;
		}
		else
		{
			p2[pn] = p1[pn];
			p1[pn] = -tempvec;
		}
	}
	else if (axis == 1)
	{
		vec3d tempvec = p0[pn];
		if (direction > 0)
		{
			p0[pn] = -p2[pn];
			p2[pn] = tempvec;
		}
		else
		{
			p0[pn] = p2[pn];
			p2[pn] = -tempvec;
		}
	}

	if (axis == 2)
	{
		vec3d tempvec = p1[pn];
		if (direction > 0)
		{
			p1[pn] = -p0[pn];
			p0[pn] = tempvec;
		}
		else
		{
			p1[pn] = p0[pn];
			p0[pn] = -tempvec;
		}
	}

	Eigen::Matrix3d R(Eigen::Matrix3d::Zero());
	R.row(0) = p0[pn];
	R.row(1) = p1[pn];
	R.row(2) = p2[pn];
	ee[pn] = mat::R2ee(R);
	convertFrame(pn); // important: update info int particle frame since oriantation is changed
}

void PointVehicle::SetOrientation(const vec3d &p2, const vec3d &p0, int idx)
{

	this->p0[idx] = p0;
	this->p2[idx] = p2;
	this->p1[idx] = p2.cross(p0);
	Eigen::Matrix3d R(Eigen::Matrix3d::Zero());
	R.row(0) = this->p0[idx];
	R.row(1) = this->p1[idx];
	R.row(2) = this->p2[idx];
	ee[idx] = mat::R2ee(R);
	convertFrame(idx); // important: update info int particle frame since oriantation is changed
}
