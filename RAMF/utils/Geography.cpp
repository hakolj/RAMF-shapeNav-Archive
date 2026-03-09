#include "pch.h"
#include "Geography.h"

using namespace std;
namespace geo {
	void LLAToECEF(const vec3d& lla, vec3d& ecef) {



		double coslon = cos(lla[0] / 180 * M_PI);
		double sinlon = sin(lla[0] / 180 * M_PI);
		double coslat = cos(lla[1] / 180 * M_PI);
		double sinlat = sin(lla[1] / 180 * M_PI);

		double N = a / sqrt(1 - E2 * sinlat * sinlat);

		ecef[0] = (N + lla[2]) * coslat * coslon;
		ecef[1] = (N + lla[2]) * coslat * sinlon;
		ecef[2] = (N * (1 - E2) + lla[2]) * sinlat;

	}

	void ECEFToLLA(const vec3d& ecef, vec3d& lla) {

		bool converged = false;
		double lon = atan2(ecef[1], ecef[0]) * 180 / M_PI; // can be directly calculated
		double alt=0;
		double p = sqrt(pow(ecef[0], 2) + pow(ecef[1], 2));
		//double lat = atan(ecef[2] / p / (1 - E2 * ERAD / (p * p + ecef[2] * ecef[2]))); // initial guess
		//double lat = atan2(ecef[2], p);
		double lat = 0; // initial guess
		double N; 
		double cache = lat;
		int cnt=0;
		while (!converged && (cnt++ < 10)) {
			N = ERAD / sqrt(1 - E2 * pow(sin(lat), 2)); 

			lat = atan((ecef[2] + N * E2 * sin(lat)) / p);
			//alt = p / cos(lat) - N;
			//lat = atan(ecef[2] / p / (1 - E2 * N / (N + alt)));
			double err = abs(lat - cache);
			converged = err < 1e-5;
			cache = lat;

			//cout << cnt << endl;
			//cout << "lat=" << lat<< endl;
		}
		alt = ecef[2] / sin(lat) - N * (1 - E2);
		lla[0] = lon;
		lla[1] = lat * 180 / M_PI;
		lla[2] = alt;
	}
	// transfer a vector in ECEF to ENU. pLLA: the LLA coordinate of the origin point of ENU frame
	void ECEFToENU(const vec3d& vecECEF, const vec3d& pLLA, vec3d& vecENU) {
		Eigen::Matrix3d S(Eigen::Matrix3d::Zero());
		double slon = sin(pLLA[0] / 180 * M_PI);
		double clon = cos(pLLA[0] / 180 * M_PI);
		double slat = sin(pLLA[1] / 180 * M_PI);
		double clat = cos(pLLA[1] / 180 * M_PI);
		S(0, 0) = -slon;
		S(0, 1) = clon;
		S(0, 2) = 0;
		S(1, 0) = -slat * clon;
		S(1, 1) = -slat * slon;
		S(1, 2) = clat;
		S(2, 0) = clat * clon;
		S(2, 1) = clat * slon;
		S(2, 2) = slat;
		vecENU = S * vecECEF;
	}

	// transfer a vector in ENU to ECEF pLLA: the LLA coordinate of the origin point of ENU frame
	void ENUToECEF(const vec3d& vecENU, const vec3d& pLLA, vec3d& vecECEF) {
		Eigen::Matrix3d S(Eigen::Matrix3d::Zero());
		double slon = sin(pLLA[0] / 180 * M_PI);
		double clon = cos(pLLA[0] / 180 * M_PI);
		double slat = sin(pLLA[1] / 180 * M_PI);
		double clat = cos(pLLA[1] / 180 * M_PI);
		S(0, 0) = -slon;
		S(0, 1) = clon;
		S(0, 2) = 0;
		S(1, 0) = -slat * clon;
		S(1, 1) = -slat * slon;
		S(1, 2) = clat;
		S(2, 0) = clat * clon;
		S(2, 1) = clat * slon;
		S(2, 2) = slat;
		vecECEF = S.transpose() * vecENU;
	}
}