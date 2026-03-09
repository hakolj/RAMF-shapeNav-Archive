#ifndef AIRSHIP_H
#define AIRSHIP_H
#include "InertialessSwimmer.h"
#include "Geography.h"
#include "vectypedef.h"

class Airship : public InertialessSwimmer,
				public GetTransinLLAAble
{
public:
	vectors3d poslla, lastposlla;
	std::shared_ptr<FluidVelAtPointAble> fluidVelAtPoint;
	// vec3d centerlla; // removed bc initial position is now controlled by PointParticle
	// double lonrange, latrange;
public:
	Airship(unsigned amatternum) : InertialessSwimmer(amatternum, 1.0, 0.0), lastposlla(vectors3d(amatternum, vec3d::Zero())), poslla(vectors3d(amatternum, vec3d::Zero())) {};

	virtual void initialize(const std::string &cfgContent, const Config &config);
	virtual void update(double timestep, const std::vector<int> &updateMask);
	virtual void getInfo();
	virtual void dump(const char *path, int step);
	virtual void reset();
	virtual void ENU2LLA(vec3d &poslla, vec3d &lastposlla, const vec3d &pos);
	virtual void initPos3D();
	inline const vectors3d &getPoslla() const { return poslla; };
	inline const vectors3d &getPos() const { return pos; };
	inline void setFluid(std::shared_ptr<Fluid> fluid)
	{
		boundaryType = fluid->boundaryType();
		fluid->getDomainSize(envDomain[0], envDomain[1], envDomain[2]);
		fluidVelAtPoint = std::dynamic_pointer_cast<FluidVelAtPointAble>(fluid);
		return;
	}
	void BoundaryCondition(std::shared_ptr<Fluid> fluid) {};
};
#endif // AIRSHIP_H