#include "pch.h"
#include "Interpolation.h"
// #include "LagrangianInterp.h"
namespace fluid
{
	void Lag2nd3D::getIndeces(const vec3d &pos, const Mesh &ms, int &ic, int &jc, int &kc) const
	{
		ic = 0;
		jc = 0;
		kc = 0;

		double xp2, yp2, zp2;
		int Nx = ms.Nx;
		int Ny = ms.Ny;
		int Nz = ms.Nz;

		while (pos(0) >= ms.x(++ic))
			;
		ic--;
		while (pos(1) >= ms.y(++jc))
			;
		jc--;
		while (pos(2) >= ms.z(++kc))
			;
		kc--;
	}

	void Lag2nd3D::interp3d(const vec3d &pos, const Scalar &sclx, const Scalar &scly, const Scalar &sclz, vec3d &info, const FieldStoreType storeType) const
	{
		int ic = 0, jc = 0, kc = 0;

		double xp2, yp2, zp2;
		int Nx = sclx.ms.Nx;
		int Ny = sclx.ms.Ny;
		int Nz = sclx.ms.Nz;

		// the index of the grid where the particle is
		// Nx = 7
		// 0    1    2    3    4    5    6    7
		// :----|----|----|----|----|----|----|----:
		//   0     1    2   3    4    5     6   7
		// if periodic: 0=6, 1=7
		// actual grid range: 1 to 6

		while (pos(0) >= sclx.ms.x(++ic))
			;
		ic--;
		while (pos(1) >= sclx.ms.y(++jc))
			;
		jc--;
		while (pos(2) >= sclx.ms.z(++kc))
			;
		kc--;

		// store i+-1,k+-1,j+-1
		int id[3]{sclx.ms.ima(ic), ic, sclx.ms.ipa(ic)};
		int jd[3]{sclx.ms.jma(jc), jc, sclx.ms.jpa(jc)};
		int kd[3]{sclx.ms.kma(kc), kc, sclx.ms.kpa(kc)};

		double basex[3]{}, basey[3]{}, basez[3]{};
		// Lag2Bases(pos(0), xcenter[0], xcenter[1], xcenter[2], basex);
		// Lag2Bases(pos(1), ycenter[0], ycenter[1], ycenter[2], basey);
		// Lag2Bases(pos(2), zcenter[0], zcenter[1], zcenter[2], basez);

		Lag2Bases(pos(0), sclx.ms.xc(ic - 1), sclx.ms.xc(ic), sclx.ms.xc(ic + 1), basex);
		Lag2Bases(pos(1), sclx.ms.yc(jc - 1), sclx.ms.yc(jc), sclx.ms.yc(jc + 1), basey);
		Lag2Bases(pos(2), sclx.ms.zc(kc - 1), sclx.ms.zc(kc), sclx.ms.zc(kc + 1), basez);
		// double Qx = 0.0;
		// double Qy = 0.0;
		// double Qz = 0.0;

		info = vec3d::Zero();

		if (storeType == FieldStoreType::CCC)
		{
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					for (int k = 0; k < 3; ++k)
					{
						// double mult1 = basex[i] * basey[j] * basez[k];
						// mult1 = c1 * c2 * c3;

						int idx = sclx.ms.idx(id[i], jd[j], kd[k]);
						double temp = scly[idx];
						info[0] += sclx[idx] * basex[i] * basey[j] * basez[k];
						info[1] += scly[idx] * basex[i] * basey[j] * basez[k];
						info[2] += sclz[idx] * basex[i] * basey[j] * basez[k];

						// std::cout << sclx[idx] << std::endl;
					}
				}
			}
		}
		else if (storeType == FieldStoreType::CYC)
		{
			double baseyy[3]{};					   // used for interp of v
			int m = (jc < Ny / 2.0) ? jc + 1 : jc; // middle index of yc
			int jdym[3]{sclx.ms.jma(m), m, sclx.ms.jpa(m)};
			Lag2Bases(pos(1), scly.ms.y(m - 1), scly.ms.y(m), scly.ms.y(m + 1), baseyy); // used only for interp of v

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					for (int k = 0; k < 3; ++k)
					{
						info[0] += sclx(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];
						// info[1] += scly(id[i], jd[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
						info[1] += scly(id[i], jdym[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
						info[2] += sclz(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];
					}
				}
			}
		}
	}

	void Lag2nd3D::interp3dCCC(const int id[3], const int jd[3], const int kd[3], const double basex[3], const double basey[3], const double basez[3],
							   const Scalar &sclx, const Scalar &scly, const Scalar &sclz, vec3d &info) const
	{
		info = vec3d::Zero();
		// only support store type of CCC
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 3; ++k)
				{
					// double mult1 = basex[i] * basey[j] * basez[k];
					// mult1 = c1 * c2 * c3;

					int idx = sclx.ms.idx(id[i], jd[j], kd[k]);
					info[0] += sclx[idx] * basex[i] * basey[j] * basez[k];
					info[1] += scly[idx] * basex[i] * basey[j] * basez[k];
					info[2] += sclz[idx] * basex[i] * basey[j] * basez[k];
				}
			}
		}
	}

	void Lag2nd3D::interp3dMultiFields(const vec3d &pos, int fieldnum, const Scalar *sclx[], const Scalar *scly[], const Scalar *sclz[],
									   vectors3d &info, const FieldStoreType storeType) const
	{
		if (fieldnum != info.size())
		{
			throw(std::runtime_error("[Lag2nd3D::interp3dMultiFields] The number of fields does not match the size of the vector"));
		}
		const Mesh &ms = sclx[0]->ms; // reference to mesh
		int ic = 0, jc = 0, kc = 0;

		double xp2, yp2, zp2;
		int Nx = ms.Nx;
		int Ny = ms.Ny;
		int Nz = ms.Nz;

		// the index of the grid where the particle is
		// Nx = 7
		// 0    1    2    3    4    5    6    7
		// :----|----|----|----|----|----|----|----:
		//   0     1    2   3    4    5     6   7
		// if periodic: 0=6, 1=7
		// actual grid range: 1 to 6

		while (pos(0) >= ms.x(++ic))
			;
		ic--;
		while (pos(1) >= ms.y(++jc))
			;
		jc--;
		while (pos(2) >= ms.z(++kc))
			;
		kc--;

		// store i+-1,k+-1,j+-1
		int id[3]{ms.ima(ic), ic, ms.ipa(ic)};
		int jd[3]{ms.jma(jc), jc, ms.jpa(jc)};
		int kd[3]{ms.kma(kc), kc, ms.kpa(kc)};

		double basex[3]{}, basey[3]{}, basez[3]{};

		Lag2Bases(pos(0), ms.xc(ic - 1), ms.xc(ic), ms.xc(ic + 1), basex);
		Lag2Bases(pos(1), ms.yc(jc - 1), ms.yc(jc), ms.yc(jc + 1), basey);
		Lag2Bases(pos(2), ms.zc(kc - 1), ms.zc(kc), ms.zc(kc + 1), basez);

		for (int n = 0; n < fieldnum; n++)
			info[n] = vec3d::Zero();

		if (storeType == FieldStoreType::CCC)
		{
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					for (int k = 0; k < 3; ++k)
					{
						int idx = ms.idx(id[i], jd[j], kd[k]);
						double c = basex[i] * basey[j] * basez[k];
						for (int n = 0; n < fieldnum; n++)
						{
							info[n][0] += (*sclx[n])[idx] * c;
							info[n][1] += (*scly[n])[idx] * c;
							info[n][2] += (*sclz[n])[idx] * c;
						}
					}
		}
		else if (storeType == FieldStoreType::CYC)
		{
			double baseyy[3]{};					   // used for interp of v
			int m = (jc < Ny / 2.0) ? jc + 1 : jc; // middle index of yc
			int jdym[3]{ms.jma(m), m, ms.jpa(m)};
			Lag2Bases(pos(1), scly[0]->ms.y(m - 1), scly[0]->ms.y(m), scly[0]->ms.y(m + 1), baseyy); // used only for interp of v

			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					for (int k = 0; k < 3; ++k)
					{
						for (int n = 0; n < fieldnum; n++)
						{
							info[n][0] += (*sclx[n])(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];
							// info[1] += scly(id[i], jd[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
							info[n][1] += (*scly[n])(id[i], jdym[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
							info[n][2] += (*sclz[n])(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];
						}
					}
		}
	}

	double Lag2nd3D::Lag2Base(const int &iflag, const double xp, const double x0, const double x1, const double x2) const
	{
		if (iflag == 0)
		{
			return (xp - x1) * (xp - x2) / (x0 - x1) / (x0 - x2);
		}
		else if (iflag == 1)
		{
			return (xp - x0) * (xp - x2) / (x1 - x0) / (x1 - x2);
		}
		else if (iflag == 2)
		{
			return (xp - x0) * (xp - x1) / (x2 - x0) / (x2 - x1);
		}
		else
		{
			std::cout << "[error]at [Lag2Base()]:check the input iflag" << std::endl;
			return 0;
		}
	}

	void Lag2nd3D::Lag2Bases(const double xp, const double x0, const double x1, const double x2, double coeff[3]) const
	{
		double xpx0 = xp - x0;
		double xpx1 = xp - x1;
		double xpx2 = xp - x2;
		double x0x1 = x0 - x1;
		double x1x2 = x1 - x2;
		double x0x2 = x0 - x2;

		coeff[0] = xpx1 * xpx2 / (x0x1) / (x0x2);
		coeff[1] = xpx0 * (xpx2) / (-x0x1) / (x1x2);
		coeff[2] = (xpx0) * (xpx1) / (-x0x2) / (-x1x2);
	}

	void Lag2nd3D::Lag2Bases(const float xp, const float x0, const float x1, const float x2, float coeff[3]) const
	{
		float xpx0 = xp - x0;
		float xpx1 = xp - x1;
		float xpx2 = xp - x2;
		float x0x1 = x0 - x1;
		float x1x2 = x1 - x2;
		float x0x2 = x0 - x2;

		coeff[0] = xpx1 * xpx2 / (x0x1) / (x0x2);
		coeff[1] = xpx0 * (xpx2) / (-x0x1) / (x1x2);
		coeff[2] = (xpx0) * (xpx1) / (-x0x2) / (-x1x2);
	}

	// void Lag2nd3D::interpCoef(double dx, double dy, double dz) {
	// 	this->dx = dx;
	// 	this->dy = dy;
	// 	this->dz = dz;

	// 	for (int m = -1; m <= 1; m++) {
	// 		A(m + 1, 0) = 1.0 / pow(dx, 2);
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) A(m + 1, 0) /= (1.0 * (m - m1));
	// 		}

	// 		A(m + 1, 1) = 0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			for (int m2 = -1; m2 <= 1; m2++) {
	// 				if ((m1 != m) & (m1 != m2) & (m2 != m)) {
	// 					A(m + 1, 1) += -1.0 * m2 / (1.0 * (m - m1) * (m - m2));
	// 				}
	// 			}
	// 		}
	// 		A(m + 1, 1) /= dx;

	// 		A(m + 1, 2) = 1.0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) A(m + 1, 2) *= m1 / (1.0 * (m - m1));
	// 		}
	// 	}

	// 	for (int m = -1; m <= 1; m++) {
	// 		B(m + 1, 0) = 1.0 / pow(dx, 2);
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) B(m + 1, 0) /= (1.0 * (m - m1));
	// 		}

	// 		B(m + 1, 1) = 0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			for (int m2 = -1; m2 <= 1; m2++) {
	// 				if ((m1 != m) & (m1 != m2) & (m2 != m)) {
	// 					B(m + 1, 1) += -1.0 * m2 / (1.0 * (m - m1) * (m - m2));
	// 				}
	// 			}
	// 		}
	// 		B(m + 1, 1) /= dx;

	// 		B(m + 1, 2) = 1.0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) B(m + 1, 2) *= m1 / (1.0 * (m - m1));
	// 		}
	// 	}

	// 	for (int m = -1; m <= 1; m++) {
	// 		C(m + 1, 0) = 1.0 / pow(dx, 2);
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) C(m + 1, 0) /= (1.0 * (m - m1));
	// 		}

	// 		C(m + 1, 1) = 0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			for (int m2 = -1; m2 <= 1; m2++) {
	// 				if ((m1 != m) & (m1 != m2) & (m2 != m)) {
	// 					C(m + 1, 1) += -1.0 * m2 / (1.0 * (m - m1) * (m - m2));
	// 				}
	// 			}
	// 		}
	// 		C(m + 1, 1) /= dx;

	// 		C(m + 1, 2) = 1.0;
	// 		for (int m1 = -1; m1 <= 1; m1++) {
	// 			if (m1 != m) C(m + 1, 2) *= m1 / (1.0 * (m - m1));
	// 		}
	// 	}
	// 	//std::cout << A << std::endl;
	// 	//std::cout << B << std::endl;
	// 	//std::cout << C << std::endl;
	// 	return;
	// }

}