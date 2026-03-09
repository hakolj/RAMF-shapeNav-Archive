#ifndef GEOGRAPHY_H
#define GEOGRAPHY_H
#include <cmath>
#include "vectypedef.h"

namespace geo {
	const double P0 = 101.325; //  Standard atmospheric pressure, unit = kPa
	const double R0 = 287; // gas constant for air. unit = J/kg * k
	const double G = 9.806; // gravity, unit = m/s^2
	const double E2 = 6.694368786465982e-3; // the square of eccentricity of earth
	const double ERAD = 6378137.0; // long radius of earth

	const double a = 6378137.0;		//Õ÷«Ú≥§∞Î÷·
	const double f_inverse = 298.257223563;			//±‚¬ µπ ˝
	const double b = a - a / f_inverse;
	//const double b = 6356752.314245;			//Õ÷«Ú∂Ã∞Î÷·

	const double e = sqrt(a * a - b * b) / a;


	// calculating height from barometer
	inline double Height(double barometer, double isoBaricHeight, double isoBaric, double T) {
		double h0;
		h0 = isoBaricHeight - R0 / G * T * std::log(barometer / isoBaric);
		return h0;
	}
	
	// calculating barometer from height
	inline double Barometer(double height, double isoBaricHeight, double isoBaric, double T) {
		double b0;
		b0 = isoBaric/std::exp((height- isoBaricHeight) * G / R0 / T);
		return b0;
	}

	extern void LLAToECEF(const vec3d& lla, vec3d& ecef);
	extern void ECEFToLLA(const vec3d& ecef, vec3d& lla);

	// transfer a vector in ECEF to ENU. pLLA: the LLA coordinate of the origin point of ENU frame
	extern void ECEFToENU(const vec3d& vecECEF, const vec3d& pLLA, vec3d& vecENU);
	// transfer a vector in ENU to ECEF. pLLA: the LLA coordinate of the origin point of ENU frame
	extern void ENUToECEF(const vec3d& vecENU, const vec3d& pLLA, vec3d& vecECEF);
}


#endif