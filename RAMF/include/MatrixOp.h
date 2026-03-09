#pragma once
#ifndef MATRIXOP_H
#define MATRIXOP_H

// #include <Eigen/Dense>
#include "vectypedef.h"
#include <iostream>
#include <unsupported/Eigen/MatrixFunctions>
namespace mat
{
	template <class T1, class T2>
	Eigen::Matrix<double, T1::RowsAtCompileTime, T2::ColsAtCompileTime> matmul(const T1 &A, const T2 &B)
	{
		Eigen::Matrix<double, T1::RowsAtCompileTime, T2::ColsAtCompileTime> C(T1::RowsAtCompileTime, T2::ColsAtCompileTime);
		C.setZero();
		for (int i = 0; i < T1::RowsAtCompileTime; i++)
		{
			for (int j = 0; j < T2::ColsAtCompileTime; j++)
			{
				for (int k = 0; k < T1::ColsAtCompileTime; k++)
				{
					C(i, j) += A(i, k) * B(k, j);
				}
			}
		}
		return C;
	}

	// inline void MatExp3D(const Eigen::Matrix3d &mat, double tau, Eigen::Matrix3cd &out)
	// {
	// 	Eigen::EigenSolver<Eigen::Matrix3d> eigensolver(mat); // eigen solver
	// 	if (eigensolver.info() != Eigen::ComputationInfo::Success)
	// 	{
	// 		std::cout << "Eigen solver failed" << std::endl;
	// 		std::cout << "mat=" << mat << std::endl;
	// 	}

	// 	Eigen::Vector3cd evals = eigensolver.eigenvalues();	 // eigen values
	// 	Eigen::Matrix3cd evecs = eigensolver.eigenvectors(); // eigen vectors (a matrix)

	// 	Eigen::Matrix3cd result;
	// 	// diagonalization
	// 	result = Eigen::Matrix3cd::Zero();
	// 	result(0, 0) = exp(tau * evals(0));
	// 	result(1, 1) = exp(tau * evals(1));
	// 	result(2, 2) = exp(tau * evals(2));

	// 	out = evecs * result * evecs.inverse(); // restore the matrix
	// }

	// this is a wrapper of Eigen's matrix exponential function
	inline Eigen::MatrixXd MatrixExp(const Eigen::MatrixXd &mat)
	{
		// 1. Check if the matrix is square
		if (mat.rows() != mat.cols())
		{
			throw std::invalid_argument("Matrix must be square to calculate exponential.");
		}

		// 2. Compute and return the matrix exponential
		// Eigen uses the scaling and squaring method with Pade approximation
		return mat.exp();
	}

	// inline Eigen::Matrix3cd MatExp3D(const Eigen::Matrix3d &mat, double tau)
	// {
	// 	Eigen::EigenSolver<Eigen::Matrix3d> eigensolver(mat); // eigen solver
	// 	if (eigensolver.info() != Eigen::ComputationInfo::Success)
	// 	{
	// 		std::cout << "Eigen solver failed" << std::endl;
	// 		std::cout << "mat=" << mat << std::endl;
	// 	}

	// 	Eigen::Vector3cd evals = eigensolver.eigenvalues();	 // eigen values
	// 	Eigen::Matrix3cd evecs = eigensolver.eigenvectors(); // eigen vectors (a matrix)

	// 	Eigen::Matrix3cd result;
	// 	// diagonalization
	// 	result = Eigen::Matrix3cd::Zero();
	// 	result(0, 0) = exp(tau * evals(0));
	// 	result(1, 1) = exp(tau * evals(1));
	// 	result(2, 2) = exp(tau * evals(2));

	// 	// if (std::isnan(result(0, 0).real()))
	// 	// {
	// 	// 	std::cout << result(0, 0).real() + result(1, 1).real() << std::endl;
	// 	// 	std::cout << "NAN value in surfersensor" << std::endl;
	// 	// }
	// 	// if (std::isnan(evecs(0, 0).real()))
	// 	// {
	// 	// 	std::cout << evecs(0, 0).real() + evecs(1, 1).real() << std::endl;
	// 	// 	std::cout << "NAN value in surfersensor" << std::endl;
	// 	// }
	// 	// Eigen::Matrix3cd rs = evecs * result * evecs.inverse();
	// 	// Eigen::Matrix3cd evec_inverse = evecs.inverse();
	// 	// if (std::isnan(rs(0, 0).real()))
	// 	// {
	// 	// 	std::cout << rs(0, 0).real() + rs(1, 1).real() << std::endl;
	// 	// 	std::cout << evec_inverse(0, 0).real() + evec_inverse(1, 1).real() << std::endl;
	// 	// 	std::cout << evecs << std::endl;
	// 	// 	std::cout << mat << std::endl;
	// 	// 	std::cout << "NAN value in surfersensor" << std::endl;
	// 	// }
	// 	return evecs * result * evecs.inverse(); // restore the matrix
	// }

	// inline Eigen::Matrix3cd MatExp3D(const Eigen::Matrix3d &mat)
	// {
	// 	return MatExp3D(mat, 1.0);
	// }

	inline vec4d nn2ee(vec3d nn)
	{
		double ex = -nn(1);
		double ey = nn(0);
		double ez = 0;
		double norme = sqrt(ex * ex + ey * ey + ez * ez);
		ex /= norme;
		ey /= norme;
		double dphi = acos(nn(2)) / 2.0;
		vec4d ee;
		ee(0) = cos(dphi);
		ee(1) = ex * sin(dphi);
		ee(2) = ey * sin(dphi);
		ee(3) = ez * sin(dphi);
		return ee;
	}

	inline Eigen::Matrix3d ee2R(const vec4d &ee)
	{
		Eigen::Matrix3d R(3, 3);
		R(0, 0) = ee(0) * ee(0) + ee(1) * ee(1) - ee(2) * ee(2) - ee(3) * ee(3);
		R(0, 1) = 2.0 * (ee(1) * ee(2) + ee(0) * ee(3));
		R(0, 2) = 2.0 * (ee(1) * ee(3) - ee(0) * ee(2));
		R(1, 0) = 2.0 * (ee(1) * ee(2) - ee(0) * ee(3));
		R(1, 1) = ee(0) * ee(0) - ee(1) * ee(1) + ee(2) * ee(2) - ee(3) * ee(3);
		R(1, 2) = 2.0 * (ee(2) * ee(3) + ee(0) * ee(1));
		R(2, 0) = 2.0 * (ee(1) * ee(3) + ee(0) * ee(2));
		R(2, 1) = 2.0 * (ee(2) * ee(3) - ee(0) * ee(1));
		R(2, 2) = ee(0) * ee(0) - ee(1) * ee(1) - ee(2) * ee(2) + ee(3) * ee(3);
		return R;
	}

	inline Eigen::Matrix3d ee2R(const vec4d &ee, vec3d &p0, vec3d &p1, vec3d &p2)
	{
		Eigen::Matrix3d R = ee2R(ee);
		p0 = R.row(0);
		p1 = R.row(1);
		p2 = R.row(2);
		return R;
	}

	inline vec4d R2ee(const Eigen::Matrix3d &R)
	{
		vec4d ee(vec4d::Zero());

		ee(0) = 0.5 * sqrt(std::max(1.0 + R(0, 0) + R(1, 1) + R(2, 2), 0.0));
		ee(1) = 0.5 * sqrt(std::max(1.0 + R(0, 0) - R(1, 1) - R(2, 2), 0.0));
		ee(2) = 0.5 * sqrt(std::max(1.0 - R(0, 0) + R(1, 1) - R(2, 2), 0.0));
		ee(3) = 0.5 * sqrt(std::max(1.0 - R(0, 0) - R(1, 1) + R(2, 2), 0.0));

		ee(1) = (R(1, 2) - R(2, 1) > 0) ? ee(1) : -ee(1);
		ee(2) = (R(2, 0) - R(0, 2) > 0) ? ee(2) : -ee(2);
		ee(3) = (R(0, 1) - R(1, 0) > 0) ? ee(3) : -ee(3);

		ee.normalize();

		return ee;
	}

	inline vec3d vecpf2vec(const vec3d &vecpf, const vec3d &n1, const vec3d &n2, const vec3d &n3)
	{
		Eigen::Matrix3d temp(3, 3);
		temp.col(0) = n1;
		temp.col(1) = n2;
		temp.col(2) = n3;
		return mat::matmul(temp, vecpf);
	}
	inline vec3d vec2vecpf(const vec3d &vec, const vec3d &n1, const vec3d &n2, const vec3d &n3)
	{
		Eigen::Matrix3d temp(3, 3);
		temp.row(0) = n1;
		temp.row(1) = n2;
		temp.row(2) = n3;
		return mat::matmul(temp, vec);
	}
}

// vec3d cross(const vec3d& a, const vec3d& b) {
//	vec3d c(3, 1);
//	c.setZero();
//
// }

#endif // !MATRIXOP_H
