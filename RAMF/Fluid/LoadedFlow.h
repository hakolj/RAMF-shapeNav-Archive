#pragma once
#ifndef LOADEDFLOW_H
#define LOADEDFLOW_H

#include "Fluid.h"
#include "Interpolation.h"
#include "Scalar.h"
#include "FlowFieldDataPool.h"

namespace fluid
{
	class LoadedFlow : public Fluid,
					   public InfoAtPointAble,
					   public FluidVelGradAtPointAble,
					   public FluidVelAtPointAble
	{

	public:
		LoadedFlow(const Mesh &ms);
		~LoadedFlow() {}

	public:
		int Nx, Ny, Nz;
		double Lx, Ly, Lz; // domain size

		Mesh ms;
		// Scalar u, v, w;
		std::shared_ptr<Scalar> dudx, dudy, dudz;
		std::shared_ptr<Scalar> dvdx, dvdy, dvdz;
		std::shared_ptr<Scalar> dwdx, dwdy, dwdz;

		FieldStoreType fieldstore = FieldStoreType::UDF;

		std::vector<int> indexlist; // list of index of flow data

		// std::array<int,3> steprange; // [step0, step1, interval]
		Lag2nd3D interpolater;
		FlowFieldDataPool datapool;

	protected:
		bool ifrozen = false;
		int _flowIndexCount = 0;
		int _updateStepCount = 0;
		int _nextFieldCount = 1;
		std::string _boundaryType = "NNN";
		std::string _fieldStoreStr = "";

		std::string flowfieldpath;
		std::shared_ptr<Scalar> u, v, w;
		std::shared_ptr<Scalar> unext, vnext, wnext;
		std::shared_ptr<Scalar> dudxnext, dudynext, dudznext;
		std::shared_ptr<Scalar> dvdxnext, dvdynext, dvdznext;
		std::shared_ptr<Scalar> dwdxnext, dwdynext, dwdznext;
		// Scalar ustep, vstep, wstep;
		// Scalar dudxstep, dudystep, dudzstep;
		// Scalar dvdxstep, dvdystep, dvdzstep;
		// Scalar dwdxstep, dwdystep, dwdzstep;

		double _timeInterpFactor;
		double _uscale; // a scaling factor for velocity, used in interpolation

	public:
		virtual void initialize(const std::string &path, const Config &config);
		virtual void reset();
		virtual void infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const;
		virtual void fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const;
		virtual void fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const;
		virtual void update(double dt);
		virtual std::string boundaryType() { return _boundaryType; }
		inline virtual void getDomainSize(double &Lx, double &Ly, double &Lz)
		{
			Lx = this->Lx;
			Ly = this->Ly;
			Lz = this->Lz;
		}
		inline virtual void getDomainBound(double &xmin, double &xmax, double &ymin, double &ymax, double &zmin, double &zmax)
		{
			xmin = 0;
			xmax = Lx;
			ymin = 0;
			ymax = Ly;
			zmin = 0;
			zmax = Lz;
		}
		static std::shared_ptr<LoadedFlow> makeInstance(const Config &config);

		// HomoIsoTurb(const Config& config);

		void LoadFlowData(int loadstep);
		// void makeGradient();
		void makeGradientBoundary(Scalar &dudx, Scalar &dudy, Scalar &dudz,
								  Scalar &dvdx, Scalar &dvdy, Scalar &dvdz,
								  Scalar &dwdx, Scalar &dwdy, Scalar &dwdz);

	protected:
		void loadConfig(const Config &config);
	};
}
#endif // !LOADEDFLOW_H
