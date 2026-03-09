#pragma once
#ifndef POINTVEHICLE_H
#define POINTVEHICLE_H

#include "ActiveMatter.h"
#include "PointParticle.h"
#include <deque>
class PointVehicle : public PointParticle,
					 public ChangeSwimVelAble,
					 public SharpRotateAble,
					 public ChangeOrientationAble
{
public:
	Eigen::Vector3d eg; // gravity direction

protected:
	// double _Lda;
	// double _rhop, _rhof, _nu, _gravity;

	std::vector<double> vswim;
	// vec3d vsettle;
	// vectors3d mdisp; //mass center displacement
	vectors3d _swimAngVel; // active swimming angular velocity

public:
	PointVehicle(int amatternum);
	virtual void initialize(const std::string &path, const Config &config);
	virtual void update(double timestep, const std::vector<int> &updateMask);
	virtual void reset();
	virtual void getInfo();
	inline virtual void useInfo()
	{
		for (int i = 0; i < amatternum; i++)
			convertFrame(i);
	};
	void dump(const char *path, int step); // dump amatter info

	// inline virtual int trajSize() { return 6; }; // return the size of recorded trajectory
	// virtual void recordTraj(std::vector<Trajectory> trajs); // recording trajecotry

	void setMotility(double vswim);
	// void setVsettle();
	void convertFrame(int idx);

	inline void setSwimVel(const std::vector<double> &newvswim) { vswim = newvswim; }
	inline vectors3d &swimAngVel() { return _swimAngVel; }
	inline std::vector<double> &swimVel() { return vswim; }

public:
	void SharpRotate90(int pn, int axis, int direction); // rotate along which axis, and in which direction
	void SetOrientation(const vec3d &p2, const vec3d &p0, int idx);
};

#endif // !INERTIALESSSWIMMER_H