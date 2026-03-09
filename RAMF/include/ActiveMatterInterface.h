#pragma once
#ifndef ACTIVEMATTERITF_H
#define ACTIVEMATTERITF_H

#include "vectypedef.h"
class GetFlowInfoAble
{
public:
	// virtual void getFlowInfo(vectors3d& ufpf, vectors3d& gradupf, vectors3d& gradvpf, vectors3d& gradwpf) = 0;
	virtual const vectors3d &getFluidVel(bool ifpf) const = 0;
	virtual const vectors3d &getGradU(bool ifpf) const = 0;
	virtual const vectors3d &getGradV(bool ifpf) const = 0;
	virtual const vectors3d &getGradW(bool ifpf) const = 0;
};

// an interface for translational dynamic
class GetTransDynamicAble
{
public:
	virtual const vectors3d &getPos() const = 0;
	virtual const vectors3d &getVel() const = 0;
	virtual const vectors3d &getAccel() const = 0; // translational acceleration.
												   // virtual const std::vector<double> &getVswim() const;
};

class GetActiveMotionAble
{
public:
	virtual const std::vector<double> &getVswim() const = 0;
};

class GetTransinLLAAble
{
public:
	virtual const vectors3d &getPoslla() const = 0;
};

class GetTransRotAble : public GetTransDynamicAble
{
public:
	// virtual const vectors3d& getPos() const = 0;
	virtual const vectors4d &getEE() const = 0;
	virtual const vectors3d &getP0() const = 0;
	virtual const vectors3d &getP1() const = 0;
	virtual const vectors3d &getP2() const = 0;
	// virtual const vectors3d& getVel() const = 0;
	// virtual const vectors3d& getAccel() const = 0;
	virtual const vectors4d &getdEEdt() const = 0;
};

class ChangeMassCenterAble
{
public:
	virtual void setMassCenter(const vectors3d &newMassCenter) = 0;
	virtual vectors3d getMassCenter() const = 0;
};
class ChangeSwimVelAble
{
public:
	virtual void setSwimVel(const std::vector<double> &newvswim) = 0;
	virtual vectors3d &swimAngVel() = 0;
	virtual std::vector<double> &swimVel() = 0;
};

class ChangeJumpVelAble
{
public:
	virtual std::vector<double> &jumpTime() = 0;
	virtual std::vector<double> &jumpVel() = 0;
	virtual void TriggerJump(int idx, double jumpVel) = 0;
	virtual bool Jumpable(int idx) = 0;
};

class ChangeShapeAble
{
public:
	virtual void setShape(double shapeFactor, int id) = 0;
};

class SharpRotateAble
{
public:
	virtual void SharpRotate90(int pn, int axis, int direction) = 0;
};

class ChangeOrientationAble
{
public:
	// p3 is usually the symmetry axis/ swimming, jumping direction ...
	virtual void SetOrientation(const vec3d &p2, const vec3d &p0, int idx) = 0;
};

#endif // !ACTIVEMATTERITF_H
