#pragma once
#ifndef INERTIALESSSWIMMER_H
#define INERTIALESSSWIMMER_H

#include "ActiveMatter.h"
#include "PointParticle.h"
#include <deque>
class InertialessSwimmer : public PointParticle,
						   public ChangeMassCenterAble,
						   public ChangeSwimVelAble,
						   public ChangeJumpVelAble,
						   public GetActiveMotionAble,
						   public SharpRotateAble,
						   public ChangeShapeAble
{
public:
	Eigen::Vector3d eg; // gravity direction

protected:
	std::vector<double> _Lda;

	double _rhop, _rhof, _nu, _gravity;

	bool iFluidInertTorq;
	// fluid inertial torque const
	double _Mi;
	// aspect ratio & radius

	std::vector<double> vswim, gyro;
	double tjump; // maximal jumping velocity, jump duration time
	std::vector<double> vjump;
	std::vector<double> tjump_now;
	std::deque<bool> iReadyJump;
	std::vector<double> vjump_now; // to record the current jumping velocity foe each amatter
	vectors3d vsettle;			   // settling velocity of each partile
	vectors3d mdisp;			   // mass center displacement
	vectors3d _swimAngVel;		   // active swimming angular velocity
	int initMode = 0;			   // 0-domain uniform, 1-in a box at a specified center
	vec3d initCenter;
	double initAngle; // initial orientation, only used for 2D now

public:
	InertialessSwimmer(int amatternum, double lda = 1.0, double a = 0.0);
	virtual void initialize(const std::string &cfgContent, const Config &config);
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

	inline void setPhysics(double rhop, double rhof, double nu, double gravity)
	{
		_rhop = rhop;
		_rhof = rhof;
		_nu = nu;
		_gravity = gravity;
		return;
	}
	inline void setSize(double a)
	{
		this->a = a;
	}
	inline void setAspectRatio(double aspectRatio, int id)
	{
		this->aspectRatio[id] = aspectRatio;
		_Lda[id] = (aspectRatio * aspectRatio - 1) / (aspectRatio * aspectRatio + 1);
		return;
	};
	void setMotility(double vswim, double B, double vjump0 = 0.0);
	void setVsettle(int id);
	void convertFrame(int id);

	inline void setMassCenter(const vectors3d &newMassCenter)
	{
		mdisp = newMassCenter;
		return;
	}

	inline vectors3d getMassCenter() const { return mdisp; }
	inline void setSwimVel(const std::vector<double> &newvswim) { vswim = newvswim; }
	inline vectors3d &swimAngVel() { return _swimAngVel; }
	inline std::vector<double> &swimVel() { return vswim; }
	inline virtual std::vector<double> &jumpTime() { return tjump_now; };
	inline virtual std::vector<double> &jumpVel() { return vjump; };
	virtual void TriggerJump(int idx, double jumpVel);
	inline virtual bool Jumpable(int idx) { return iReadyJump[idx]; };
	inline const std::vector<double> &getVswim() const { return vswim; };

	void setShape(double aspectRatio, int id);

public:
	// return the F_{\beta}, used for calculating fluid inertia constant
	double _Fbeta(double lda);
	double _solveMi(double lda);
	// setting fluid inertial torque constant Mi
	void setInertialTorqueConst(int id);

	void SharpRotate90(int pn, int axis, int direction); // rotate along which axis, and in which direction
};

#endif // !INERTIALESSSWIMMER_H