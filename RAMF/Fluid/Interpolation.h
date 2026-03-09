#pragma once
#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "pch.h"
#include "Scalar.h"

// a base class of Interpolator
namespace fluid
{
	class Interpolator
	{
	public:
		virtual void interp3d(const vec3d &pos,
							  const Scalar &sclx,
							  const Scalar &scly,
							  const Scalar &sclz,
							  vec3d &info,
							  const FieldStoreType storeType) const = 0;
	};

	class Lag2nd3D : public Interpolator
	{
	public:
		// Eigen::Matrix<double, 3, 3> A;
		// Eigen::Matrix<double, 3, 3> B;
		// Eigen::Matrix<double, 3, 3> C;
		// private:
		// 	double dx, dy, dz;
	public:
		Lag2nd3D() {};

		void getIndeces(const vec3d &pos, const Mesh &ms, int &ic, int &jc, int &kc) const;

		void interp3d(const vec3d &pos, const Scalar &sclx, const Scalar &scly, const Scalar &sclz, vec3d &info, const FieldStoreType storeType) const;
		void interp3dCCC(const int id[3], const int jd[3], const int kd[3], const double basex[3], const double basey[3], const double basez[3],
						 const Scalar &sclx, const Scalar &scly, const Scalar &sclz, vec3d &info) const;

		// interpolation for the same point but at different fields. (not recommended)
		void interp3dMultiFields(const vec3d &pos, int fieldnum, const Scalar *sclx[], const Scalar *scly[], const Scalar *sclz[],
								 vectors3d &info, const FieldStoreType storeType) const;

		// void interp3d(const vectors3d& pos, const Scalar& sclx, const Scalar& scly, const Scalar& sclz, vectors3d& info, const FieldStoreType storeType);

		// calculate Lagrangian interpolation coefficient using local grid information
		double Lag2Base(const int &iflag, const double xp, const double x0, const double x1, const double x2) const;

		// calculate Lagrangian interpolation coefficients for three interp points using local grid information
		// out: coeff
		void Lag2Bases(const double xp, const double x0, const double x1, const double x2, double coeff[3]) const;

		void Lag2Bases(const float xp, const float x0, const float x1, const float x2, float coeff[3]) const;
	};

	class Linear3D : Interpolator
	{
	public:
		void interp3d(const vec3d &pos, const Scalar &sclx, const Scalar &scly, const Scalar &sclz, vec3d &info, const FieldStoreType storeType) const;
	};
}
#endif // !INTERPOLATION_H
