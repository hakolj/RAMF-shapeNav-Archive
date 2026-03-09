#include "pch.h"
#include "Interpolation.h"
// #include "LagrangianInterp.h"

namespace fluid
{
	void Linear3D::interp3d(const vec3d &pos,
							const Scalar &sclx,
							const Scalar &scly,
							const Scalar &sclz,
							vec3d &info,
							const FieldStoreType storeType) const
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

		// locate particle index
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

		// Lag2Bases(pos(0), sclx.ms.xc(ic - 1), sclx.ms.xc(ic), sclx.ms.xc(ic + 1), basex);
		// Lag2Bases(pos(1), sclx.ms.yc(jc - 1), sclx.ms.yc(jc), sclx.ms.yc(jc + 1), basey);
		// Lag2Bases(pos(2), sclx.ms.zc(kc - 1), sclx.ms.zc(kc), sclx.ms.zc(kc + 1), basez);
		////double Qx = 0.0;
		////double Qy = 0.0;
		////double Qz = 0.0;

		// info = vec3d::Zero();

		// if (storeType == FieldStoreType::CCC) {
		//	for (int i = 0; i < 3; ++i) {
		//		for (int j = 0; j < 3; ++j) {
		//			for (int k = 0; k < 3; ++k) {
		//				//double mult1 = basex[i] * basey[j] * basez[k];
		//				//mult1 = c1 * c2 * c3;

		//				int idx = sclx.ms.idx(id[i], jd[j], kd[k]);
		//				double temp = scly[idx];
		//				info[0] += sclx[idx] * basex[i] * basey[j] * basez[k];
		//				info[1] += scly[idx] * basex[i] * basey[j] * basez[k];
		//				info[2] += sclz[idx] * basex[i] * basey[j] * basez[k];

		//			}
		//		}
		//	}
		//}
		// else if (storeType == FieldStoreType::CYC) {
		//	double baseyy[3]{}; // used for interp of v
		//	int m = (jc < Ny / 2.0) ? jc + 1 : jc; // middle index of yc
		//	int jdym[3]{ sclx.ms.jma(m), m, sclx.ms.jpa(m) };
		//	Lag2Bases(pos(1), scly.ms.y(m - 1), scly.ms.y(m), scly.ms.y(m + 1), baseyy); // used only for interp of v

		//	for (int i = 0; i < 3; ++i) {
		//		for (int j = 0; j < 3; ++j) {
		//			for (int k = 0; k < 3; ++k) {
		//				info[0] += sclx(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];
		//				//info[1] += scly(id[i], jd[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
		//				info[1] += scly(id[i], jdym[j], kd[k]) * basex[i] * baseyy[j] * basez[k];
		//				info[2] += sclz(id[i], jd[j], kd[k]) * basex[i] * basey[j] * basez[k];

		//			}
		//		}
		//	}
		//}

#if DEBUG
		if (isnan(info[0]) || isnan(info[1]) || isnan(info[2]))
		{
			std::cout << "nan value in interp" << std::endl;
		}
#endif
	}

}