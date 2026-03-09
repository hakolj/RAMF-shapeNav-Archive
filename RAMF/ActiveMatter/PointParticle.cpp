#include "pch.h"
#include "ActiveMatter.h"
#include "PointParticle.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Random.h"
#include "MatrixOp.h"
#include <algorithm>
#include "Fluid.h"
#include "Fop.h"
#include "Random.h"
#include <unordered_map>

PointParticle::PointParticle(unsigned amatternum) : ActiveMatter(amatternum), pos(vectors3d(amatternum, vec3d::Zero())),
													aspectRatio(amatternum, 1), a(a),
													ee(vectors4d(amatternum, vec4d::Zero())), p0(vectors3d(amatternum, vec3d::Zero())),
													p1(vectors3d(amatternum, vec3d::Zero())), p2(vectors3d(amatternum, vec3d::Zero())),
													vp_new(vectors3d(amatternum, vec3d::Zero())), vp_old(vectors3d(amatternum, vec3d::Zero())),
													accel(vectors3d(amatternum, vec3d::Zero())),
													deedt_new(vectors4d(amatternum, vec4d::Zero())), deedt_old(vectors4d(amatternum, vec4d::Zero())),
													uf(vectors3d(amatternum, vec3d::Zero())), ufpf(vectors3d(amatternum, vec3d::Zero())),
													gradu(vectors3d(amatternum, vec3d::Zero())), gradv(vectors3d(amatternum, vec3d::Zero())), gradw(vectors3d(amatternum, vec3d::Zero())),
													gradupf(vectors3d(amatternum, vec3d::Zero())), gradvpf(vectors3d(amatternum, vec3d::Zero())), gradwpf(vectors3d(amatternum, vec3d::Zero())),
													_istep1trans(false), _istep1rot(false), fluidInfoAtPoint(NULL), envDomain(),
													_initRange(0), _resetEpisode(0), fixedInitOrientation(vec3d::Zero()),
													brownianD_t(0.0), brownianD_r(0.0)
{
}

// void PointParticle::initPosBox2D(double xcenter, double ycenter,
// 								 double xsize, double ysize)
// {
// 	if (coordSys.dir() == 2)
// 	{
// 		initPosBox3D(xcenter, ycenter, 0.0, xsize, ysize, 0.0);
// 	}
// 	else if (coordSys.dir() == 1)
// 	{
// 		initPosBox3D(xcenter, 0.0, ycenter, xsize, 0.0, ysize);
// 	}
// 	else if (coordSys.dir() == 0)
// 	{
// 		initPosBox3D(0.0, xcenter, ycenter, 0.0, xsize, ysize);
// 	}

// 	return;
// }

void PointParticle::initPosBox3D(double xcenter, double ycenter, double zcenter,
								 double xsize, double ysize, double zsize)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		pos[i][0] = rd::randd() * xsize + (xcenter - xsize / 2.0);
		pos[i][1] = rd::randd() * ysize + (ycenter - ysize / 2.0);
		pos[i][2] = rd::randd() * zsize + (zcenter - zsize / 2.0);
		vp_new[i] = vec3d::Zero();
		vp_old[i] = vec3d::Zero();
		_istep1trans = true;
	}

	return;
}

void PointParticle::initOrient2DUniform()
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		double theta = rd::randd() * 2 * M_PI;
		p2[i](0) = std::cos(theta);
		p2[i](1) = std::sin(theta);
		p2[i](2) = 0;
		p0[i](0) = -std::sin(theta);
		p0[i](1) = std::cos(theta);
		p0[i](2) = 0;
		p1[i] = p2[i].cross(p0[i]);

		Eigen::Matrix3d R;
		R.row(0) = p0[i];
		R.row(1) = p1[i];
		R.row(2) = p2[i];
		ee[i] = mat::R2ee(R);

		deedt_new[i] = vec4d::Zero();
		deedt_new[i] = vec4d::Zero();
		_istep1rot = true;
	}
}

void PointParticle::initOrient3DUniform()
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		vec3d n0;
		double phi = 0;
		double theta = 0;
		double h = rd::randd();

		theta = h * 2.0 * M_PI;
		h = rd::randd();
		phi = asin(h * 2.0 - 1.0);
		n0(0) = cos(theta) * cos(phi);
		n0(1) = sin(theta) * cos(phi);
		n0(2) = sin(phi);

		ee[i] = mat::nn2ee(n0);
		mat::ee2R(ee[i], p0[i], p1[i], p2[i]);
		deedt_new[i] = vec4d::Zero();
		deedt_new[i] = vec4d::Zero();
		_istep1rot = true;
	}
}
void PointParticle::initOrient2DFixed(double theta)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		vec3d n0;
		n0(coordSys[0]) = cos(theta);
		n0(coordSys[1]) = sin(theta);
		n0(coordSys[2]) = 0;
		ee[i] = mat::nn2ee(n0);

		// old implementation which only works for xy plane
		// double al = theta;
		// Eigen::Matrix3d R;
		// R(0, 0) = -std::sin(al);
		// R(0, 1) = std::cos(al);
		// R(0, 2) = 0;
		// R(1, 0) = 0;
		// R(1, 1) = 0;
		// R(1, 2) = 1;
		// R(2, 0) = std::cos(al);
		// R(2, 1) = std::sin(al);
		// R(2, 2) = 0;
		// ee[i] = mat::R2ee(R);
		mat::ee2R(ee[i], p0[i], p1[i], p2[i]);

		deedt_new[i] = vec4d::Zero();
		deedt_new[i] = vec4d::Zero();
		_istep1rot = true;
	}
}
void PointParticle::initOrient3DFixed(double theta, double phi)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		vec3d n0;
		n0(0) = cos(theta) * cos(phi);
		n0(1) = sin(theta) * cos(phi);
		n0(2) = sin(phi);

		ee[i] = mat::nn2ee(n0);
		mat::ee2R(ee[i], p0[i], p1[i], p2[i]);
		deedt_new[i] = vec4d::Zero();
		deedt_new[i] = vec4d::Zero();
		_istep1rot = true;
	}
}

void PointParticle::initPosRing2D(double xcenter, double ycenter, double zcenter,
								  double innerR, double outerR)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		double r, theta;
		r = rd::randd(innerR, outerR);
		theta = rd::randd() * 2 * M_PI;
		double pos1 = r * cos(theta);
		double pos2 = r * sin(theta);

		if (coordSys.dir() == 2)
		{
			pos[i][0] = xcenter + pos1;
			pos[i][1] = ycenter + pos2;
			pos[i][2] = zcenter;
		}
		else if (coordSys.dir() == 1)
		{
			pos[i][0] = xcenter + pos2;
			pos[i][1] = ycenter;
			pos[i][2] = zcenter + pos1;
		}
		else if (coordSys.dir() == 0)
		{
			pos[i][0] = xcenter;
			pos[i][1] = ycenter + pos1;
			pos[i][2] = zcenter + pos2;
		}

		vp_new[i] = vec3d::Zero();
		vp_old[i] = vec3d::Zero();
		_istep1trans = true;
	}

	return;
}

void PointParticle::initPosRing3D(double xcenter, double ycenter, double zcenter,
								  double innerR, double outerR)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		vec3d n0;
		double phi = 0;
		double theta = 0;
		theta = rd::randd() * 2.0 * M_PI;
		phi = asin(rd::randd() * 2.0 - 1.0);
		n0(0) = cos(theta) * cos(phi);
		n0(1) = sin(theta) * cos(phi);
		n0(2) = sin(phi);

		double r = rd::randd(innerR, outerR);
		pos[i][0] = r * n0(0) + xcenter;
		pos[i][1] = r * n0(1) + ycenter;
		pos[i][2] = r * n0(2) + zcenter;

		vp_new[i] = vec3d::Zero();
		vp_old[i] = vec3d::Zero();
		_istep1trans = true;
	}

	return;
}

void PointParticle::resetFlowInfo()
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		uf[i] = vec3d::Zero();
		ufpf[i] = vec3d::Zero();
		gradu[i] = vec3d::Zero();
		gradv[i] = vec3d::Zero();
		gradw[i] = vec3d::Zero();
		gradupf[i] = vec3d::Zero();
		gradvpf[i] = vec3d::Zero();
		gradwpf[i] = vec3d::Zero();
	}
}

void PointParticle::_updateTrans(int i, const vec3d &vp, double timestep)
{
	vp_new[i] = vp;
	if (_istep1trans)
	{
		vp_old[i] = vp_new[i];
	}
	vec3d brownianDist = vec3d::Zero();
	for (int i = 0; i < coordSys.dim(); i++)
	{
		// only add Brownian noise in the directions contained in the coordinate system
		brownianDist[coordSys[i]] = rd::Normal() * sqrt(2.0 * timestep * brownianD_t);
	}

	pos[i] += timestep * (1.5 * vp_new[i] - 0.5 * vp_old[i]) + brownianDist;
	accel[i] = (vp_new[i] - vp_old[i]) / timestep;
	vp_old[i] = vp_new[i];
	// double x = pos[i][0];
	// double y = pos[i][1];
	// std::cout << vp_new[i][2] << std::endl;
	return;
};

void PointParticle::_updateTransPF(int i, const vec3d &vppf, double timestep)
{
	vp_new[i] = mat::vecpf2vec(vppf, p0[i], p1[i], p2[i]);
	if (_istep1trans)
	{
		vp_old[i] = vp_new[i];
	}
	vec3d brownianDist = vec3d::Zero();
	for (int i = 0; i < coordSys.dim(); i++)
	{
		int partAx = (2 + i) % 3; // the major axis is 2, minor is 0, and the third is 1
		brownianDist[partAx] = rd::Normal() * sqrt(2.0 * timestep * brownianD_t);
	}

	pos[i] += timestep * (1.5 * vp_new[i] - 0.5 * vp_old[i]) + brownianDist;
	accel[i] = (vp_new[i] - vp_old[i]) / timestep;
	vp_old[i] = vp_new[i];
	// double x = pos[i][0];
	// double y = pos[i][1];
	// std::cout << vp_new[i][2] << std::endl;
	return;
};

void PointParticle::_updateRot(int i, const vec3d &angvel, double timestep)
{
	// calculate brownian noise effective angular velocity

	vec3d brownianAngvel = vec3d::Zero();
	if (coordSys.dim() == 3)
	{
		for (int j = 0; j < coordSys.dim(); j++)
		{
			brownianAngvel[j] = rd::Normal() * sqrt(2.0 * timestep * brownianD_r) / timestep;
		}
	}
	else if (coordSys.dim() == 2)
	{
		// in 2D, the out-of-plane axis is 1.
		brownianAngvel[1] = rd::Normal() * sqrt(2.0 * timestep * brownianD_r) / timestep;
	}

	vec3d angvel_new = angvel + brownianAngvel; // add the Brownian noise to the angular velocity

	deedt_new[i][0] = -0.5 * (ee[i][1] * angvel_new[0] + ee[i][2] * angvel_new[1] + ee[i][3] * angvel_new[2]);
	deedt_new[i][1] = 0.5 * (ee[i][0] * angvel_new[0] - ee[i][3] * angvel_new[1] + ee[i][2] * angvel_new[2]);
	deedt_new[i][2] = 0.5 * (ee[i][3] * angvel_new[0] + ee[i][0] * angvel_new[1] - ee[i][1] * angvel_new[2]);
	deedt_new[i][3] = 0.5 * (-ee[i][2] * angvel_new[0] + ee[i][1] * angvel_new[1] + ee[i][0] * angvel_new[2]);
	if (_istep1rot)
	{
		deedt_old[i] = deedt_new[i];
	}
	ee[i] += timestep * (1.5 * deedt_new[i] - 0.5 * deedt_old[i]);
	deedt_old[i] = deedt_new[i];
	double nee = ee[i].norm();
	ee[i] /= nee;
	mat::ee2R(ee[i], p0[i], p1[i], p2[i]); // update particle axis direction

	return;
}

void PointParticle::BoundaryCondition(std::shared_ptr<Fluid> fluid)
{
	std::string bc = fluid->boundaryType();
	// nothing to do for periodic bc
	double Lmin[3], Lmax[3];
	fluid->getDomainBound(Lmin[0], Lmax[0], Lmin[1], Lmax[1], Lmin[2], Lmax[2]);
	double L[3] = {Lmax[0] - Lmin[0], Lmax[1] - Lmin[1], Lmax[2] - Lmin[2]};
	for (int i = 0; i < amatternum; i++)
	{
		// loop for three dims
		for (int dim = 0; dim < 3; dim++)
		{
			// wall bc
			if (bc[dim] == 'W')
			{
				if (pos[i](dim) < Lmin[dim] + this->a)
				{
					pos[i](dim) = 2 * (Lmin[dim] + this->a) - pos[i](dim);
					vp_new[i](dim) *= -1;
					// reflective rotation
					vec3d newP2 = p2[i];
					newP2(dim) *= -1;
					vec3d newP0 = p0[i];
					newP0(dim) *= -1;
					vec3d newP1;
					newP1(0) = newP2(1) * newP0(2) - newP2(2) * newP0(1);
					newP1(1) = newP2(2) * newP0(0) - newP2(0) * newP0(2);
					newP1(2) = newP2(0) * newP0(1) - newP2(1) * newP0(0);
					Eigen::Matrix3d R(Eigen::Matrix3d::Zero());
					R.row(0) = newP0;
					R.row(1) = newP1;
					R.row(2) = newP2;
					ee[i] = mat::R2ee(R);
				}
				else if (pos[i](dim) > Lmax[dim] - this->a)
				{
					pos[i](dim) = 2 * (Lmax[dim] - this->a) - pos[i](dim);
					vp_new[i](dim) *= -1;
				}
			}
			else if (bc[dim] == 'P')
			{
				// periodic boundary
				// pos[i](dim) = std::fmod(pos[i](dim) + L[dim] - Lmin[dim], L[dim]);
			}
		}
	}
}

void PointParticle::recordTraj(std::vector<Trajectory>::iterator begin,
							   std::vector<Trajectory>::iterator end)
{
	std::vector<double> snapShot(trajSize(), 0);
	// return "xpos ypos zpos p21 p22 p23 \ 6
	// 	vx vy vz de1 de2 de3 de4 ufx ufy ufz \ 16
	// 	dudx dudy dudz dvdx dvdy dvdz dwdx dwdy dwdz";

	int n = 0;
	for (auto it = begin; it != end; ++it)
	{
		for (int j = 0; j < 3; j++)
		{
			snapShot[j] = pos[n][j];
			snapShot[j + 3] = p2[n][j];
			snapShot[j + 6] = vp_new[n][j];
			snapShot[j + 13] = uf[n][j];
			snapShot[j + 16] = gradu[n][j];
			snapShot[j + 19] = gradv[n][j];
			snapShot[j + 22] = gradw[n][j];
		}
		snapShot[9] = ee[n][0];
		snapShot[10] = ee[n][1];
		snapShot[11] = ee[n][2];
		snapShot[12] = ee[n][3];

		(*it).Record(snapShot);
		n++;
	}
}

void PointParticle::readDumpType(const Config &config)
{
	std::string str = config.Read<std::string>("DumpType", "");
	if (str == "all")
	{
		dumpType = DumpType::all;
	}
	else if (str == "slice")
	{
		dumpType = DumpType::slice;
	}
	else
	{
		std::cout << "Unrecognized DumpType: '" << str << "'. Use 'all' by defalut!" << std::endl;
		dumpType = DumpType::all;
	}
	if (dumpType == DumpType::slice)
	{
		sliceCoeff = Fop::loadvec1d<double>(config.Read<std::string>("slice.func"));
		if (sliceCoeff.size() != 4)
		{
			throw std::runtime_error("In PointParticle::readDumpType: slice function must be a vector of 4. (Ax+By+Cz+D=0)");
		}
		sliceWidth = config.Read<double>("slice.width");
	}
}

void PointParticle::readInitMode(const Config &config)
{
	std::string posmode = config.Read<std::string>("PosInitMode");
	std::string orientmode = config.Read<std::string>("OrientInitMode");

	// read mode
	if (PosInitModeMap.find(posmode) != PosInitModeMap.end())
	{
		posInitMode = PosInitModeMap.at(posmode);
	}
	else
	{
		throw std::runtime_error("In PointParticle::readInitMode: unrecognized PosInitMode: " + posmode);
	}

	if (OrientInitModeMap.find(orientmode) != OrientInitModeMap.end())
	{
		orientInitMode = OrientInitModeMap.at(orientmode);
	}
	else
	{
		throw std::runtime_error("In PointParticle::readInitMode: unrecognized OrientInitMode: " + orientmode);
	}

	// handel logic
	// need to read initial core parameter
	initCoreCenter = Fop::loadEigenVec<double, 3>(config.Read<std::string>("PosInitCoreCenter"));
	initCoreRange = Fop::loadEigenVec<double, 3>(config.Read<std::string>("PosInitCoreRange"));

	if (posInitMode == PosInitMode::box)
	{
		// read box parameter
		// auto initBoxSize_vec = Fop::loadvec1d<double>(config.Read<std::string>("PosInitBoxSize"));
		initBoxSize = Fop::loadEigenVec<double, 3>(config.Read<std::string>("PosInitBoxSize"));
	}
	else if (posInitMode == PosInitMode::ring)
	{
		// need to read ring parameter
		initRingRange = Fop::loadEigenVec<double, 2>(config.Read<std::string>("PosInitRingRange"));
	}

	if (orientInitMode == OrientInitMode::fixed)
	{
		fixedInitOrientation = vec3d::Zero();
		// need to read fixed orientation
		if (coordSys.dim() == 2)
		{
			fixedInitOrientation(0) = config.Read<double>("OrientInitTheta");
		}
		else if (coordSys.dim() == 3)
		{
			fixedInitOrientation(0) = config.Read<double>("OrientInitTheta");
			fixedInitOrientation(1) = config.Read<double>("OrientInitPhi");
		}
	}
}

const std::unordered_map<std::string, PointParticle::PosInitMode> PointParticle::PosInitModeMap = {
	{"box", PosInitMode::box},
	{"ring", PosInitMode::ring},
};
const std::unordered_map<std::string, PointParticle::OrientInitMode> PointParticle::OrientInitModeMap = {
	{"uniform", OrientInitMode::uniform},
	{"fixed", OrientInitMode::fixed},
};

void PointParticle::resetPosition()
{
	vec3d coreCenter = vec3d::Zero();
	for (int i = 0; i < coordSys.dim(); i++)
	{
		coreCenter[i] = initCoreCenter[i] + rd::randd(-initCoreRange[i] / 2.0, initCoreRange[i] / 2.0);
	}
	std::cout << "coreCenter=" << coreCenter[0] << " " << coreCenter[1] << " " << coreCenter[2] << std::endl;
	if (posInitMode == PosInitMode::box)
	{
		initPosBox3D(coreCenter(0), coreCenter(1), coreCenter(2),
					 initBoxSize(0), initBoxSize(1), initBoxSize(2));
	}
	else if (posInitMode == PosInitMode::ring)
	{
		if (coordSys.dim() == 2)
		{
			initPosRing2D(coreCenter(0), coreCenter(1), coreCenter(2),
						  initRingRange(0), initRingRange(1));
		}
		else if (coordSys.dim() == 3)
		{
			initPosRing3D(coreCenter(0), coreCenter(1), coreCenter(2),
						  initRingRange(0), initRingRange(1));
		}
	}
}

void PointParticle::resetOrientation()
{
	if (orientInitMode == OrientInitMode::uniform)
	{
		if (coordSys.dim() == 2)
		{
			initOrient2DUniform();
		}
		else if (coordSys.dim() == 3)
		{
			initOrient3DUniform();
		}
	}
	else if (orientInitMode == OrientInitMode::fixed)
	{
		// re-use the initial configuration read from the input file
		if (coordSys.dim() == 2)
		{
			initOrient2DFixed(fixedInitOrientation[0]);
		}
		else if (coordSys.dim() == 3)
		{
			initOrient3DFixed(fixedInitOrientation[0], fixedInitOrientation[1]);
		}
	}
}