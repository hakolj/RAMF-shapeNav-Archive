#include "pch.h"
#include "InertialessSwimmer.h"
#include "MatrixOp.h"
#include "Fop.h"
#include "Random.h"
#define INFTSM 1e-8

using namespace std;
InertialessSwimmer::InertialessSwimmer(int amatternum, double lda, double a) : PointParticle(amatternum),
																			   vswim(amatternum, 0), gyro(amatternum, 0), vjump(amatternum, 0), tjump(0.0),
																			   vsettle(amatternum, vec3d::Zero()),
																			   mdisp(vectors3d(amatternum, vec3d::Zero())), eg(0.0, -1.0, 0.0),
																			   _Lda(amatternum, (lda * lda - 1) / (lda * lda + 1)), _rhop(0), _rhof(0), _nu(0), _gravity(0),
																			   iFluidInertTorq(true), _Mi(0),
																			   _swimAngVel(vectors3d(amatternum, vec3d::Zero())), tjump_now(amatternum, 0), vjump_now(amatternum, 0),
																			   iReadyJump(amatternum, true)
{
	return;
}

void InertialessSwimmer::initialize(const std::string &path, const Config &config)
{
	// InitSenseStepCount(config);
	double vswim = config.Read<double>("vswim", 0.0);
	double vjump = config.Read<double>("vjump", 0.0);
	tjump = config.Read<double>("tjump", 0);
	double invB = config.Read<double>("invB", 0);
	setMotility(vswim, 1.0 / invB, vjump);
	double a = config.Read("radius", 0.0);
	double lda = config.Read("lambda", 1.0);
	double gravity = config.Read("gravity", 0.0);
	double rho_p = config.Read("rhop", 1000.0);
	double nu = config.Read("nu", 1e-6);
	brownianD_t = config.Read("Dt", 0.0);
	brownianD_r = config.Read("Dr", 0.0);
	if (brownianD_t < 0.0 || brownianD_r < 0.0)
	{
		throw std::runtime_error("InertialessSwimmer: Brownian diffusion coefficient must be non-negative.");
	}
	iFluidInertTorq = config.Read<bool>("fluid inertial torque", true);

	readDumpType(config);

	setSize(a);
	setPhysics(rho_p, 1000, nu, gravity);
	for (int i = 0; i < amatternum; i++)
	{
		setAspectRatio(lda, i);
		setVsettle(i);
		setInertialTorqueConst(i);
	}

	int dim = config.Read("dimension", 3);
	// setDim(dim);
	int direction = 0;
	if (dim != 3)
	{
		direction = config.Read<int>("direction");
		// if (direction != 2 && dim == 2)
		// {
		// 	throw std::runtime_error("InertialessSwimmer: direction must be set to 2 for 2D simulation, but got: " + std::to_string(direction));
		// }
	}
	coordSys = coord::CartCoordSys(dim, direction);

	readInitMode(config);
	// initMode = config.Read<int>("init mode", 0);
	// if ((initMode == 1) || (initMode == 2) || (initMode == 3))
	// {
	// 	_initRange = config.Read("initRange", 2.0);
	// 	std::string strthre = config.Read<std::string>("initCenter");
	// 	std::vector<double> center = Fop::loadvec1d<double>(strthre);
	// 	if (center.size() > 3)
	// 	{
	// 		string errmsg = "PointVehicle::initCenter: center position has a dimension greater than 3.";
	// 		cout << errmsg << endl;
	// 		throw runtime_error(errmsg);
	// 	}
	// 	initCenter[0] = center[0];
	// 	initCenter[1] = center[1];
	// 	initCenter[2] = center[2];

	// 	if (initMode == 3)
	// 	{
	// 		initAngle = config.Read("initAngle", 0.0);
	// 	}
	// }

	// isinirnd = config.Read<bool>("random initial", true);
	// if (!isinirnd)
	// {
	// 	string cfgpos = config.Read<string>("initial position", "0 0 0");
	// 	string cfgori = config.Read<string>("initial orientation", "0 0");
	// 	size_t iPI;
	// 	bool pflag;
	// 	iPI = cfgpos.find("PI");
	// 	pflag = iPI != string::npos;
	// 	if (pflag)
	// 	{
	// 		cfgpos = cfgpos.substr(iPI);
	// 	}
	// 	std::vector<double> inipos = Fop::loadvec1d<double>(cfgpos);
	// 	std::vector<double> iniori = Fop::loadvec1d<double>(cfgori);
	// 	for (int i = 2; i < inipos.size(); i += 3)
	// 	{
	// 		vec3d p;
	// 		p << inipos[i - 2], inipos[i - 1], inipos[i];
	// 		if (pflag)
	// 		{
	// 			p = p * M_PI;
	// 		}
	// 		cfginipos.push_back(p);
	// 	}
	// 	for (int i = 2; i < iniori.size(); i += 3)
	// 	{
	// 		vec3d o;
	// 		o << iniori[i - 2], iniori[i - 1], iniori[i];
	// 		cfginiori.push_back(o);
	// 	}
	// 	if ((cfginipos.size() != cfginiori.size()) || (cfginipos.size() != amatternum))
	// 	{
	// 		string errmsg = "Initial position, orientation and number of active matters dismatch. Check config file.";
	// 		cout << errmsg << endl;
	// 		throw runtime_error(errmsg);
	// 	}
	// }

	_resetEpisode = config.Read<int>("reset episode", 0);
	_resetCount = _resetEpisode; // to ensure always reset for the first episode
	return;
}

void InertialessSwimmer::reset()
{
	_resetCount++;
	bool resetflag = _resetCount >= _resetEpisode;
	if (resetflag)
	{
		_resetCount = 0;
		resetPosition();
		resetOrientation();
	}
	// if (coordSys.dim() == 2)
	// {

	// 	if (resetflag)
	// 	{
	// 		_resetCount = 0;

	// 		if (initMode == 0)
	// 		{
	// 			initOrient2D();
	// 			if (boundaryType[0] == 'R')
	// 			{
	// 				initPos2DRing(rmin, rmax);
	// 			}
	// 			else
	// 			{
	// 				initPos2D(envDomain[0], envDomain[1]);
	// 			}
	// 		}
	// 		else if (initMode == 1)
	// 		{
	// 			initOrient2D();
	// 			initPos3D(initCenter[0] - _initRange, initCenter[1] - _initRange, 0,
	// 					  initCenter[0] + _initRange, initCenter[1] + _initRange, 0);
	// 		}
	// 		else if ((initMode == 2) || (initMode == 3))
	// 		{
	// 			// only for TGV with ludvig's project
	// 			initPos3D(initCenter[0] - _initRange, initCenter[1] - _initRange, 0,
	// 					  initCenter[0] + _initRange, initCenter[1] + _initRange, 0);
	// 			// init orientation based on position

	// 			for (unsigned i = 0; i < amatternum; i++)
	// 			{

	// 				if (isinirnd)
	// 				{
	// 					double theta = atan2(pos[i][1], pos[i][0]);
	// 					if (initMode == 3) // only for ludvig's project
	// 						theta = initAngle;
	// 					p2[i](0) = std::cos(theta);
	// 					p2[i](1) = std::sin(theta);
	// 					p2[i](2) = 0;
	// 					p0[i](0) = -std::sin(theta);
	// 					p0[i](1) = std::cos(theta);
	// 					p0[i](2) = 0;
	// 					p1[i] = p2[i].cross(p0[i]);

	// 					Eigen::Matrix3d R;
	// 					R.row(0) = p0[i];
	// 					R.row(1) = p1[i];
	// 					R.row(2) = p2[i];
	// 					ee[i] = mat::R2ee(R);
	// 				}

	// 				deedt_new[i] = vec4d::Zero();
	// 				deedt_new[i] = vec4d::Zero();
	// 				_istep1rot = true;
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		for (unsigned i = 0; i < amatternum; i++)
	// 		{
	// 			if (boundaryType[0] == 'P')
	// 			{
	// 				pos[i][0] = fmod(pos[i][0], envDomain[0]);
	// 			}
	// 			if (boundaryType[1] == 'P')
	// 			{
	// 				pos[i][1] = fmod(pos[i][1], envDomain[1]);
	// 			}
	// 		}
	// 	}
	// 	// initPos2D(_initRange, _initRange);
	// 	// initPos2DPoint(1.0, 1.0, 2.00);
	// }
	// else if (coordSys.dim() == 3)
	// {

	// 	// initPos3D(2 * M_PI, 2 * M_PI, 2 * M_PI);
	// 	// initPos3D(2  , 2 , 2 );
	// 	if (resetflag)
	// 	{
	// 		_resetCount = 0;
	// 		initOrient3D();
	// 		initPos3D(envDomain[0], envDomain[1], envDomain[2]);
	// 	}
	// 	else
	// 	{
	// 		for (unsigned i = 0; i < amatternum; i++)
	// 		{
	// 			pos[i][0] = fmod(pos[i][0], envDomain[0]);
	// 			pos[i][1] = fmod(pos[i][1], envDomain[1]);
	// 			pos[i][2] = fmod(pos[i][2], envDomain[2]);
	// 		}
	// 	}
	// }

	resetFlowInfo();
	return;
}

void InertialessSwimmer::setMotility(double vswim, double B, double vjump)
{
	for (unsigned i = 0; i < amatternum; i++)
	{
		const vec3d defalut_mdisp(0.0, 0.0, -1.0);
		this->vswim[i] = vswim;
		this->gyro[i] = 0.5 / B;
		this->mdisp[i] = defalut_mdisp;
		this->vjump[i] = vjump;
		iReadyJump[i] = true;
	}
}

void InertialessSwimmer::getInfo()
{
	fluidInfoAtPoint->infoAtPoint(pos, uf, gradu, gradv, gradw);
}

// convert fluid vels & gradients to amatter frame
// calculate p0, p1, p2 at the same time
void InertialessSwimmer::convertFrame(int id)
{
	using namespace mat;

	Eigen::Matrix3d Reul;
	Reul.row(0) = p0[id];
	Reul.row(1) = p1[id];
	Reul.row(2) = p2[id];
	// ee2R(ee[id], p0[id], p1[id], p2[id]);
	Eigen::Matrix3d gradmat(3, 3);
	gradmat.col(0) = gradu[id];
	gradmat.col(1) = gradv[id];
	gradmat.col(2) = gradw[id];
	Eigen::Matrix3d gradmatpf(3, 3);
	gradmatpf = matmul(matmul(Reul, gradmat), Reul.transpose());
	gradupf[id] = gradmatpf.col(0);
	gradvpf[id] = gradmatpf.col(1);
	gradwpf[id] = gradmatpf.col(2);
	ufpf[id] = matmul(Reul, uf[id]);

	return;
}

void InertialessSwimmer::update(double timestep, const std::vector<int> &updateMask)
{

	_updateTimeStep = timestep;
#pragma omp parallel for
	for (int i = 0; i < amatternum; i++)
	{
		if (updateMask[i] == 0)
			continue; // skip the dead ones
		vec3d vppf(3, 1), egpf(3, 1);
		vec3d t_gyro(3, 1), t_rotate(3, 1), t_deform(3, 1), angvel(3, 1);
		vec3d t_finert(3, 1);
		vec3d vslippf(3, 1);

		egpf = mat::vec2vecpf(eg, p0[i], p1[i], p2[i]);
		// translation
		vec3d setvel = vsettle[i].array() * egpf.array();
		vppf = ufpf[i] + setvel;

		/*cout << setvel << endl;*/

		// vppf[2] += vswim[i] + _vjump;  // adding jumping velocity
		vppf[2] += vswim[i] + vjump_now[i]; // adding jumping velocity

		// jumping velocity dreceasing, according to Ardeshiri et al. 2016
		//  when jump start, tjump_now = 1. Then tjump_now decrease to 0. This corresponds to _vjump = vjump*exp(0) to vjump* exp(-log(100))
		//  jump time decrease rate = timestep / (tjump * log(100.))
		// double _vjump = 0;
		if (vjump_now[i] < vjump[i] * 1e-2)
		{
			// only when current jump vel < 1e-2 * maxvjump can a copepod start a new jump
			iReadyJump[i] = true;
		}
		else
			iReadyJump[i] = false;

		vjump_now[i] *= exp(-timestep / tjump); // exponontial decrease

		// if ((tjump > 0) && (tjump_now[i] > 0)) {
		//	//_vjump = vjump[i] * exp((tjump_now[i] - 1) * log(100.0)); // final vjump  = 1/100 vjump max
		//	//cout << vjump[i] * exp((tjump_now[i] - 1) * log(100.0))<<" ";
		//	vjump_now[i] *= exp(-timestep / tjump); //exponontial decrease
		//	tjump_now[i] -= timestep / (tjump * log(100.0));

		//}
		// else {
		//	vjump_now[i] = 0.0; // not jump or jump ends
		//}

		_updateTransPF(i, vppf, timestep);

		// rotation (only in particle frame)
		t_gyro = mdisp[i].cross(egpf);
		t_gyro *= gyro[i];

		t_rotate[0] = 0.5 * (gradwpf[i][1] - gradvpf[i][2]);
		t_rotate[1] = 0.5 * (gradupf[i][2] - gradwpf[i][0]);
		t_rotate[2] = 0.5 * (gradvpf[i][0] - gradupf[i][1]);

		t_deform[0] = -_Lda[i] * 0.5 * (gradvpf[i][2] + gradwpf[i][1]);
		t_deform[1] = _Lda[i] * 0.5 * (gradupf[i][2] + gradwpf[i][0]);
		t_deform[2] = 0.0;

		vslippf = vppf - ufpf[i];
		t_finert[0] = -_Mi / _nu * vslippf[1] * vslippf[2];
		t_finert[1] = _Mi / _nu * vslippf[0] * vslippf[2];
		t_finert[2] = 0.0;

		// t_deform = vec3d::Zero();
		// t_rotate = vec3d::Zero();
		angvel = t_gyro + t_rotate + t_deform + t_finert + _swimAngVel[i];
		// angvel = vec3d::Zero();
		_updateRot(i, angvel, timestep);
		// cout << p2[i][0] <<" "<< p2[i][1]<<" "<< p2[i][2] << endl;
		if (isnan(pos[0](0)))
		{
			cout << "nan value in inertialess swimmers update" << endl;
		}
	}
	_istep1rot = false;
	_istep1trans = false;
}

// dump particle field
void InertialessSwimmer::dump(const char *path, int step)
{
	ofstream os;

	char stepstr[10];
	sprintf(stepstr, "%.7i", step);
	string fullpath = string(path) + "/partfield" + string(stepstr) + ".txt";
	os.open(fullpath, ios::out | ios::trunc);
	os.precision(8);
	os << scientific;
	for (unsigned i = 0; i < amatternum; i++)
	{

		bool flag = true;
		if (dumpType == DumpType::slice)
		{
			// conditional output
			flag = checkSlice(pos[i][0], pos[i][1], pos[i][2]);
		}
		if (flag)
		{
			os << i << " "; // dump the particle index
			os << pos[i][0] << " " << pos[i][1] << " " << pos[i][2] << " " << p2[i][0] << " " << p2[i][1] << " " << p2[i][2] << " " << vp_new[i][0] << " " << vp_new[i][1] << " " << vp_new[i][2] << " " << uf[i][0] << " " << uf[i][1] << " " << uf[i][2] << " " << gradu[i][0] << " " << gradu[i][1] << " " << gradu[i][2] << " " << gradv[i][0] << " " << gradv[i][1] << " " << gradv[i][2] << " " << gradw[i][0] << " " << gradw[i][1] << " " << gradw[i][2];
			os << endl;
		}
	}
	os.close();

	// FILE *fp;
	// fullpath = string(path) + "/partfield" + string(stepstr) + ".bin";
	// fp = fopen(fullpath.c_str(), "wb");
	// for (unsigned i = 0; i < amatternum; i++)
	// {
	// 	fwrite(&pos[i][0], sizeof(double), 3, fp);
	// 	fwrite(&p2[i][0], sizeof(double), 3, fp);
	// 	fwrite(&vp_new[i][0], sizeof(double), 3, fp);
	// 	fwrite(&uf[i][0], sizeof(double), 3, fp);
	// 	fwrite(&gradu[i][0], sizeof(double), 3, fp);
	// 	fwrite(&gradv[i][0], sizeof(double), 3, fp);
	// 	fwrite(&gradw[i][0], sizeof(double), 3, fp);
	// }
	// fclose(fp);
}

void InertialessSwimmer::setVsettle(int id)
{
	double mu = _nu * _rhof;
	double pi = M_PI;
	double kapa, x0, a0, b0, r0;

	double lda = aspectRatio[id];
	if (fabs(a) < INFTSM)
	{
		vsettle[id] = vec3d::Zero();
		return;
	}
	if (fabs(lda - 1.0) < INFTSM)
	{
		double vs = 2.0 / 9.0 * (_rhop - _rhof) * pow(a, 2) / mu * _gravity;
		vsettle[id] = vec3d(vs, vs, vs);
		// cout << vsettle[id] << endl;
		return;
	}
	else if (lda > 1.0)
	{
		kapa = log((lda - sqrt(pow(lda, 2) - 1)) / (lda + sqrt(pow(lda, 2) - 1)));
		x0 = -pow(a, 2) * lda / sqrt(pow(lda, 2) - 1) * kapa;
		a0 = pow(lda, 2) / (pow(lda, 2) - 1) + lda / (2.0 * pow((pow(lda, 2) - 1), 1.5)) * kapa;
		b0 = a0;
		r0 = -2.0 / (pow(lda, 2) - 1) - lda / pow((pow(lda, 2) - 1.0), 1.5) * kapa;
	}
	else if (lda < 1.0)
	{
		kapa = 2.0 * atan(lda / sqrt(1.0 - pow(lda, 2)));
		x0 = pow(a, 2) * lda / sqrt(1.0 - pow(lda, 2)) * (pi - kapa);
		a0 = -0.5 * lda / pow(sqrt(1.0 - pow(lda, 2)), 3) * (kapa - pi + 2.0 * lda * sqrt(1.0 - pow(lda, 2)));
		b0 = a0;
		r0 = 1.0 / pow(sqrt(1.0 - pow(lda, 2)), 3) * (lda * kapa - pi * lda + 2.0 * sqrt(1.0 - pow(lda, 2)));
	}

	double b = a;
	double c = lda * a;
	vec3d k = vec3d::Zero();
	k[0] = 16.0 * pi * a * b * c * 1.0 / (x0 + pow(a, 2) * a0);
	k[1] = 16.0 * pi * a * b * c * 1.0 / (x0 + pow(b, 2) * b0);
	k[2] = 16.0 * pi * a * b * c * 1.0 / (x0 + pow(c, 2) * r0);
	vsettle[id][0] = 6.0 * pi * a * lda * (1.0 / ((k[2] - k[0]) * pow(cos(pi / 2.0), 2) + k[0])) * 2.0 / 9.0 * (_rhop - _rhof) * pow(a, 2) / mu * _gravity;
	vsettle[id][1] = vsettle[id][0];
	vsettle[id][2] = 6.0 * pi * a * lda * (1.0 / ((k[2] - k[0]) * pow(cos(0.0), 2) + k[0])) * 2.0 / 9.0 * (_rhop - _rhof) * pow(a, 2) / mu * _gravity;
	return;
}

// return the F_{\beta}, used for calculating fluid inertia constant
double InertialessSwimmer::_Fbeta(double lda)
{
	const double eps = 1e-15;
	double pi = M_PI;
	double ksi, e, t1up, t2up, t3up, tdown;
	if (abs(lda - 1.0) < eps)
		return 0;
	if (lda > 1.0)
	{
		ksi = lda / sqrt(pow(lda, 2) - 1);
		e = 1.0 / ksi;
		t1up = -pi * pow(e, 2) * (420 * e + 2240 * pow(e, 3) + 4249 * pow(e, 5) - 2152 * pow(e, 7));
		tdown = 315 * pow((pow(e, 2) + 1) * atanh(e) - e, 2) * ((1 - 3 * pow(e, 2)) * atanh(e) - e);
		t2up = +pi * pow(e, 2) * (420 + 3360 * pow(e, 2) + 1890 * pow(e, 4) - 1470 * pow(e, 6)) * atanh(e);
		t3up = -pi * pow(e, 2) * (1260 * e - 1995 * pow(e, 3) + 2730 * pow(e, 5) - 1995 * pow(e, 7)) * pow(atanh(e), 2);
		return (t1up + t2up + t3up) / tdown;
	}
	if (lda < 1.0)
	{
		e = sqrt(1 - pow(lda, 2));
		tdown = 315 * sqrt(1 - pow(e, 2)) * (-e * sqrt(1 - pow(e, 2)) + (1 + 2 * pow(e, 2)) * asin(e)) * pow(e * sqrt(1 - pow(e, 2)) + (2 * pow(e, 2) - 1) * asin(e), 2);
		t1up = pi * pow(e, 3) * sqrt(1 - pow(e, 2)) * (-420 + 3500 * pow(e, 2) - 9989 * pow(e, 4) + 4757 * pow(e, 6));
		t2up = 210 * pi * pow(e, 2) * (2 - 24 * pow(e, 2) + 69 * pow(e, 4) - 67 * pow(e, 6) + 20 * pow(e, 8)) * asin(e);
		t3up = 105 * pi * pow(e, 3) * (12 - 17 * pow(e, 2) + 24 * pow(e, 4)) * pow(asin(e), 2) * sqrt(1 - pow(e, 2));
		return (t1up + t2up + t3up) / tdown;
	}
}
// setting fluid inertial torque constant Mi
double InertialessSwimmer::_solveMi(double lda)
{
	const double eps = 1e-15;
	if (abs(lda - 1.0) < eps)
	{
		return 0;
	}
	if (lda < 1.0)
	{
		cout << "Warning: Mi for oblate particles is not defined!" << endl;
	}
	double a = 1.0;
	double apar = lda * a;
	double aorg = a;
	double F = _Fbeta(lda);
	double Aprime = 5.0 / (6.0 * M_PI) * F / (pow(lda, 2) + 1) * pow(max(lda, 1.0), 3);
	// double beta = log(lda + sqrt(pow(lda, 2) - 1)) / (lda * sqrt(pow(lda, 2) - 1));
	// double Corg = 8.0 * apar * aorg * (pow(lda, 4) - 1) / (9 * pow(lda, 2) * ((2 * pow(lda, 2) - 1) * beta - 1));
	double Corg = 0;
	double Iorg = (1 + pow(lda, 2)) / 5.0 * pow(aorg, 2);
	if (lda > 1.0)
	{
		double e = sqrt(1 - pow(lda, -2));
		double L = log((1 + e) / (1 - e));
		double YC = 4.0 / 3.0 * pow(e, 3) * (2 - pow(e, 2)) * pow(-2 * e + (1 + pow(e, 2)) * L, -1);
		Corg = 4.0 / 3.0 * pow(a, 2) * pow(lda, 3) * YC;
		// double XC = 4.0 / 3.0 * pow(e, 3) * (1 - pow(e, 2)) * pow(2 * e - (1 - pow(e, 2)) * L, -1);
		// Cpara = 4.0 / 3.0 * pow(a, 2) * pow(lda, 3) * XC;
	}
	if (lda < 1.0)
	{
		double e = sqrt(1 - pow(lda, 2));
		double C = M_PI_2 - atan(sqrt(1 - pow(e, 2)) / e);
		double YC = 2.0 / 3.0 * pow(e, 3) * (2 - pow(e, 2)) * pow(e * sqrt(1 - pow(e, 2)) - (1 - 2 * pow(e, 2)) * C, -1);
		Corg = 4.0 / 3.0 * pow(a, 2) * YC;
	}

	double Mi = Aprime * Iorg / Corg;
	return Mi;
}

void InertialessSwimmer::setInertialTorqueConst(int id)
{
	this->_Mi = iFluidInertTorq ? _solveMi(this->aspectRatio[id]) : 0.0;
	return;
}

void InertialessSwimmer::TriggerJump(int idx, double jumpVel)
{
	if ((tjump > 0) && (iReadyJump[idx]))
	{
		vjump[idx] = jumpVel;
		vjump_now[idx] = vjump[idx]; // set current jumping velocity as max jumping velocity
		iReadyJump[idx] = false;	 // can not trigger jump for a time period.
	}
	else
		return; // cannot trigger a new jump
				// if ((tjump > 0) && (tjump_now[idx] <= 0)) // if a particle can jump and is not jumping
	//{
	//	//tjump_now[idx] = 1.0; // set jumping timer
	//	vjump_now[idx] = vjump[idx]; //set current jumping velocity as max jumping velocity
	// }
	// else {
	//	return; // cannot trigger a new jump
	// }
}

void InertialessSwimmer::SharpRotate90(int pn, int axis, int direction)
{
	if (axis == 0)
	{
		vec3d tempvec = p2[pn];
		if (direction > 0)
		{
			p2[pn] = -p1[pn];
			p1[pn] = tempvec;
		}
		else
		{
			p2[pn] = p1[pn];
			p1[pn] = -tempvec;
		}
	}
	else if (axis == 1)
	{
		vec3d tempvec = p0[pn];
		if (direction > 0)
		{
			p0[pn] = -p2[pn];
			p2[pn] = tempvec;
		}
		else
		{
			p0[pn] = p2[pn];
			p2[pn] = -tempvec;
		}
	}

	if (axis == 2)
	{
		vec3d tempvec = p1[pn];
		if (direction > 0)
		{
			p1[pn] = -p0[pn];
			p0[pn] = tempvec;
		}
		else
		{
			p1[pn] = p0[pn];
			p0[pn] = -tempvec;
		}
	}

	Eigen::Matrix3d R(Eigen::Matrix3d::Zero());
	R.row(0) = p0[pn];
	R.row(1) = p1[pn];
	R.row(2) = p2[pn];
	ee[pn] = mat::R2ee(R);
	convertFrame(pn); // important: update info int particle frame since oriantation is changed
}

void InertialessSwimmer::setShape(double aspectRatio, int id)
{
	if (aspectRatio == this->aspectRatio[id])
		return; // new shape = old shape. do nothing

	setAspectRatio(aspectRatio, id);
	// recalculate settling speed & inertial torque constant
	setVsettle(id);
	setInertialTorqueConst(id);
}