#pragma once
#ifndef FLUIDITF_H
#define FLUIDITF_H

#include "vectypedef.h"

class InfoAtPointAble
{
public:
	virtual void infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const = 0;
};

// an interface that only returns the fluid gradient
// grad: the gradient of u,v,or w. velcomponent = 0,1,2: u,v,w
class FluidVelGradAtPointAble
{
public:
	virtual void fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const = 0;
};

// an interface that only returns the fluid vel
class FluidVelAtPointAble
{
public:
	virtual void fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const = 0;
};

class ShearRateGradientAtPointAble
{
public:
	virtual void ShearRateAtPoint(const vectors3d &pos, vectors3d &dSr) const = 0;
};

class SijGradAtPointAble
{
public:
	// pos: position to be inteporlated. sij: output. i, j: the index of sij. i,j = 0,1,2
	virtual void SijGradAtPoint(const vectors3d &pos, vectors3d &dsij, int i, int j) = 0;
};

class FluidDensityAtPointAble
{
public:
	virtual void fluidDensityAtPoint(const vectors3d &pos, std::vector<double> &densityf) const = 0;
};

class FluidPressureGradientAtPointAble {
public:
	virtual void fluidPressGradAtPoint(const vectors3d& pos, vectors3d& prsgdt) const = 0;
};
#endif // !FLUIDITF_H
