#ifndef BALOON_H
#define BALOON_H
#include "ActiveMatter.h"
class Balloon : public ActiveMatter, public GetTransDynamicAble
{
public:
	vectors3d pos;					// position in ecef coordinate
	vectors3d poslla;				// lla coordinate of the baloon;
	vectors3d vp_new, vp_old;		// velocity of balloon
	vectors3d vppf_new;				// velocity of balloon in ENU frame
	vectors3d accel_old, accel_new; // acceleration of baloon
	std::string boundaryType;

	void AdjustMass(double adjustMass, int index);

protected:
	vectors3d ufpf; // fluid velocity at amatter position in ENU frame

	bool _istep1trans, _istep1rot; // if it is the first step for Adam-Bashforth scheme
	double _initRange[3];		   // range of the initial position in LLA coordinate
	int _dimension;				   // 2d or 3d
	std::shared_ptr<FluidVelAtPointAble> fluidVelAtPoint;
	std::shared_ptr<FluidDensityAtPointAble> fluidDensityAtPoint;
	double envLmin[3] = {0, 0, 0}; // min boundary range of fluid
	double envLmax[3] = {0, 0, 0}; // min boundary range of fluid
	int _resetEpisode;			   // particles pos and orientation are reset at every N episode
	int _resetCount = 0;
	double _updateTimeStep; // store the time step used at last update

	int initMode = 0; // 0-domain uniform, 1-in a box at a specified center
	vec3d initCenter;

	double massDefault;				  // defalut mass
	std::vector<double> mass;		  // the mass of each balloon
	double dragcoeff;				  // drag coefficient in ENU frame;
	std::vector<double> fluidDensity; // fluid density at baloon position
	double aproj;					  // projection area of the balloon;
	double volume;					  // the volume of a balloon

	std::vector<double> power; // power ~ [0,1], recharge over time
	double powerRechargeRate;  // how much persecond
	double powerUsageRate;	   // how much per mass increase;

public:
	Balloon(unsigned amatternum);
	virtual void update(double timestep, const std::vector<int> &updateMask);
	virtual void initialize(const std::string &cfgContent, const Config &config);
	virtual void reset();
	virtual void getInfo(); // get flow info
	virtual void useInfo();
	inline virtual void setFluid(std::shared_ptr<Fluid> fluid)
	{
		boundaryType = fluid->boundaryType();
		fluid->getDomainBound(envLmin[0], envLmax[0], envLmin[1], envLmax[1], envLmin[2], envLmax[2]);
		// fluid->getDomainSize(envDomain[0], envDomain[1], envDomain[2]);
		fluidVelAtPoint = std::dynamic_pointer_cast<FluidVelAtPointAble>(fluid);
		fluidDensityAtPoint = std::dynamic_pointer_cast<FluidDensityAtPointAble>(fluid);
		return;
	}
	virtual void dump(const char *path, int step); // dump amatter info

	// fully elastic colliding or periodic boundary condition for point-particle.
	virtual void BoundaryCondition(std::shared_ptr<Fluid> fluid);

	inline virtual int trajSize() { return 15; }; // return the size of recorded trajectory
	inline virtual std::string trajHeader()
	{
		return "xpos\typos\tzpos\tlon\tlat\talt\tvpE\tvpN\tvpU\tapx\tapy\tapz\tufE\tufN\tufU";
	}; // return the header of trajectory file
	virtual void recordTraj(std::vector<Trajectory>::iterator begin,
							std::vector<Trajectory>::iterator end); // recording trajecotry

	// static vec3d vecpf2vec(const vec3d& vecpf, const vec3d& n1, const vec3d& n2, const vec3d& n3);
	// static vec3d vec2vecpf(const vec3d& vec, const vec3d& n1, const vec3d& n2, const vec3d& n3);

	// void initOrient3D();
	// void initPos3D(double xmax, double ymax, double zmax);
	void initPos3D(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax);
	void resetFlowInfo();
	// void _updateTransPF(int i, const vec3d& vppf, double timestep);
	// void _updateRot(int i, const vec3d& angvel, double timestep);

	// inline void setInitRange(double range) { _initRange = range; };
	inline void setDim(int dim) { _dimension = dim; };

	inline const vectors3d &getPos() const { return poslla; };
	inline const vectors3d &getVel() const { return vppf_new; }
	inline const vectors3d &getAccel() const { return accel_new; }

	inline const vectors3d &getFluidVel(bool ifpf) const
	{
		if (!ifpf)
			std::cout << "Warning: fluid vel is only available in particle frame for Balloon" << std::endl;
		return ufpf;
	}

	inline const std::vector<double> &getPower() const { return power; }
};
#endif
