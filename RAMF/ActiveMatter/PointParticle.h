#pragma once
#ifndef POINTPARTICLE_H
#define POINTPARTICLE_H

#include "ActiveMatter.h"
#include "CoordSys.h"
#include <unordered_map>
class PointParticle : public ActiveMatter,
					  public GetFlowInfoAble,
					  public GetTransRotAble
{
public:
	vectors3d pos;
	vectors4d ee;
	vectors3d p0, p1, p2;
	vectors3d vp_new, vp_old;
	vectors3d accel;
	vectors4d deedt_new, deedt_old;
	std::string boundaryType;

	vec3d initCoreCenter; // center of initial core position
	vec3d initCoreRange;  // range of initial core position

	// controls the shape of core region
	vec3d initBoxSize;			   // size of initial position box
	vec2d initRingRange;		   // inner and outer radius of initial position ring
	bool _istep1trans, _istep1rot; // if it is the first step for Adam-Bashforth scheme

	enum DumpType
	{
		all,
		slice, // output activematters at a certain slice. (2D:line, 3D: surface. Defined by Ax+By+Cz+D=0)
	};
	enum class PosInitMode
	{
		box,
		ring,
	};
	enum class OrientInitMode
	{
		uniform,
		fixed,
	};

	DumpType dumpType;

protected:
	std::vector<double>
		aspectRatio; // aspect ratio, [0, Inf]
	double a;

	vectors3d uf, ufpf;			   // fluid velocity at amatter position
	vectors3d gradu, gradv, gradw; // fluid gradients
	vectors3d gradupf, gradvpf, gradwpf;

	double _initRange; // initial position in domainsize
	// int _dimension;	   // 2d or 3d
	std::shared_ptr<InfoAtPointAble> fluidInfoAtPoint;
	double envDomain[3] = {0, 0, 0}; // boundary range of fluid
	int _resetEpisode;				 // particles pos and orientation are reset at every N episode
	int _resetCount = 0;
	double _updateTimeStep; // store the time step used at last update
	double rmin, rmax;		// ring aera boundary
	// vectors3d cfginipos;	// initial positions in config
	vec3d fixedInitOrientation; // initial thetas (2d) and theta, phis (3d) in config
	// bool isinirnd;					// if initialise particles randomly=
	std::vector<double> sliceCoeff; // Ax+By+Cz+D = 0
	double sliceWidth;				// the width at which the particle is considered on the slice
	double brownianD_t;				// Brownian diffusion coefficient in translational dynamics
	double brownianD_r;				// Brownian diffusion coefficient in rotational dynamics

	coord::CartCoordSys coordSys;  // coordinate system for 2D/3D
	PosInitMode posInitMode;	   // initial position mode
	OrientInitMode orientInitMode; // initial orientation mode

	// initial position is chosen within a core region
	// center of the core region is uniformly distributed in the domain

public:
	PointParticle(unsigned amatternum);
	virtual void update(double timestep, const std::vector<int> &updateMask) = 0;
	virtual void initialize(const std::string &path, const Config &config) = 0;
	virtual void reset() = 0;
	virtual void getInfo() = 0; // get flow info
	virtual void useInfo() = 0;
	inline virtual void setFluid(std::shared_ptr<Fluid> fluid)
	{
		boundaryType = fluid->boundaryType();
		fluid->getDomainSize(envDomain[0], envDomain[1], envDomain[2]);
		fluidInfoAtPoint = std::dynamic_pointer_cast<InfoAtPointAble>(fluid);
		if (boundaryType[0] == 'R')
		{
			fluid->getDomainSize(rmin, rmax, envDomain[2]);
		}
		return;
	}

	// fully elastic colliding or periodic boundary condition for point-particle.
	virtual void BoundaryCondition(std::shared_ptr<Fluid> fluid);

	inline virtual int trajSize() { return 25; }; // return the size of recorded trajectory
	inline virtual std::string trajHeader()
	{
		return "xpos ypos zpos p21 p22 p23 "
			   "vx vy vz de1 de2 de3 de4 ufx ufy ufz "
			   "dudx dudy dudz dvdx dvdy dvdz dwdx dwdy dwdz";
	}; // return the header of trajectory file
	virtual void recordTraj(std::vector<Trajectory>::iterator begin,
							std::vector<Trajectory>::iterator end); // recording trajecotry

	// static Eigen::Matrix3d ee2R(const vec4d &ee);
	// static Eigen::Matrix3d ee2R(const vec4d &ee, vec3d &p0, vec3d &p1, vec3d &p2);
	// static vec4d R2ee(const Eigen::Matrix3d &R);
	// static vec4d nn2ee(vec3d nn);
	// static vec3d vecpf2vec(const vec3d &vecpf, const vec3d &n1, const vec3d &n2, const vec3d &n3);
	// static vec3d vec2vecpf(const vec3d &vec, const vec3d &n1, const vec3d &n2, const vec3d &n3);

	inline void setInitRange(double range) { _initRange = range; };
	// inline void setDim(int dim) { _dimension = dim; };

	inline const vectors3d &getPos() const { return pos; };
	inline const vectors4d &getEE() const { return ee; };
	inline const vectors3d &getP0() const { return p0; };
	inline const vectors3d &getP1() const { return p1; };
	inline const vectors3d &getP2() const { return p2; };
	inline const vectors3d &getVel() const { return vp_new; }
	inline const vectors3d &getAccel() const { return accel; }
	inline const vectors4d &getdEEdt() const { return deedt_new; }

	inline const vectors3d &getFluidVel(bool ifpf) const { return ifpf ? ufpf : uf; }
	inline const vectors3d &getGradU(bool ifpf) const { return ifpf ? gradupf : gradu; }
	inline const vectors3d &getGradV(bool ifpf) const { return ifpf ? gradvpf : gradv; }
	inline const vectors3d &getGradW(bool ifpf) const { return ifpf ? gradwpf : gradw; }

	inline bool const checkSlice(const double x, const double y, const double z) const
	{
		return abs(sliceCoeff[0] * x + sliceCoeff[1] * y + sliceCoeff[2] * z + sliceCoeff[3]) <= sliceWidth / 2;
	}

	static const std::unordered_map<std::string, PosInitMode> PosInitModeMap;
	static const std::unordered_map<std::string, OrientInitMode> OrientInitModeMap;

	void resetPosition();	 // final interface for resetting position
	void resetOrientation(); // final interface for resetting orientation
	// void initPosBox2D(double xcenter, double ycenter,
	// 				  double xsize, double ysize);
	void initPosBox3D(double xcenter, double ycenter, double zcenter,
					  double xsize, double ysize, double zsize);
	void initPosRing2D(double xcenter, double ycenter, double zcenter,
					   double innerR, double outerR); // a 2d ring in xy, yz or xz plane
	void initPosRing3D(double xcenter, double ycenter, double zcenter,
					   double innerR, double outerR); // a 3d spherical shell

	void initOrient2DUniform();
	void initOrient3DUniform();
	void initOrient2DFixed(double theta);
	void initOrient3DFixed(double theta, double phi);
	void resetFlowInfo();

protected:
	void readDumpType(const Config &config);
	void readInitMode(const Config &config);

	void _updateTrans(int i, const vec3d &vp, double timestep);		// update the translation dynamics via velocity in inertial frame
	void _updateTransPF(int i, const vec3d &vppf, double timestep); // update the translation dynamics via velocity in particle frame
	void _updateRot(int i, const vec3d &angvel, double timestep);

	// void initOrient2D();
	// void initOrient3D();
	// void initPos2D(double xmin, double ymin, double xmax, double ymax);
	// void initPos2DPoint(double x, double y, double r);
	// void initPos2DRing(double rmin, double rmax);
	// void initPos3D(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax);
};

#endif // !POINTPARTICLE_H