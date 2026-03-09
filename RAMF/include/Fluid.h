#pragma once
#ifndef FLUID_H
#define FLUID_H

#include "vectypedef.h"
#include "Config.h"
#include "FluidInterface.h"
#include <memory> // for shared ptr
#include <random>

class Fluid
{
public:
	// get flow information at amatter position
	// virtual void getInfo(const vectors3d& pos, vectors3d& u, vectors3d& gradu, vectors3d& gradv, vectors3d& gradw) = 0;
	virtual void initialize(const std::string &cfgContent, const Config &config) = 0;
	virtual void reset() = 0;
	virtual void update(double dt) = 0;

	// return the boundary type in XYZ.
	// Example: PPP = periodic boundaries in 3 dirs. PWP = periodic boundaries in x and z, wall boundary in y
	// PPN: piriodic in x and y. No z _direction (2D cases).
	virtual std::string boundaryType() = 0;
	virtual void getDomainSize(double &Lx, double &Ly, double &Lz) = 0;
	virtual void getDomainBound(double &xmin, double &xmax, double &ymin, double &ymax, double &zmin, double &zmax) = 0;
	virtual ~Fluid() {};
	virtual void dump(const char *path, int step) {}; // dump fluid info

	// to initialize fluid, return a shared_ptr for fluid
	static std::shared_ptr<Fluid> Create(const std::string &cfgContent);

protected:
	std::default_random_engine reFluid;

public:
	inline void SetRandomEngine(int seed)
	{
		reFluid.seed(seed);
	}
};

#endif // !FLUID_H
