#include "pch.h"
#include "LoadedFlow.h"
#include "Random.h"
#include <fstream>
#include "Timer.h"

using namespace std;

namespace fluid
{

	LoadedFlow::LoadedFlow(const Mesh &ms) : Nx(ms.Nx), Ny(ms.Ny), Nz(ms.Nz), Lx(ms.Lx), Ly(ms.Ly), Lz(ms.Lz), ms(ms),
											 u(nullptr), v(nullptr), w(nullptr),
											 dudx(make_shared<Scalar>(ms)), dudy(make_shared<Scalar>(ms)), dudz(make_shared<Scalar>(ms)),
											 dvdx(make_shared<Scalar>(ms)), dvdy(make_shared<Scalar>(ms)), dvdz(make_shared<Scalar>(ms)),
											 dwdx(make_shared<Scalar>(ms)), dwdy(make_shared<Scalar>(ms)), dwdz(make_shared<Scalar>(ms)),
											 indexlist(0, 0), interpolater(), datapool(),
											 unext(nullptr), vnext(nullptr), wnext(nullptr),
											 dudxnext(make_shared<Scalar>(ms)), dudynext(make_shared<Scalar>(ms)), dudznext(make_shared<Scalar>(ms)),
											 dvdxnext(make_shared<Scalar>(ms)), dvdynext(make_shared<Scalar>(ms)), dvdznext(make_shared<Scalar>(ms)),
											 dwdxnext(make_shared<Scalar>(ms)), dwdynext(make_shared<Scalar>(ms)), dwdznext(make_shared<Scalar>(ms))

	{
		// interpolater.interpCoef(ms.Lx / (ms.Nx - 1), ms.Ly / (ms.Ny - 1), ms.Lz / (ms.Nz - 1));
		// u = Scalar(ms); v = Scalar(ms); w = Scalar(ms);
		// dudx = Scalar(ms); dudy = Scalar(ms); dudz = Scalar(ms);
		// dvdx = Scalar(ms); dvdy = Scalar(ms); dvdz = Scalar(ms);
		// dwdx = Scalar(ms); dwdy = Scalar(ms); dwdz = Scalar(ms);

		return;
	}

	void LoadedFlow::infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const
	{
#pragma omp parallel for
		for (int pn = 0; pn < pos.size(); pn++)
		{
			vec3d temppos;
			// temppos[0] = fmod(pos[pn][0], ms.Lx);
			// temppos[1] = fmod(pos[pn][1], ms.Ly);
			// temppos[2] = fmod(pos[pn][2], ms.Lz);
			////cout << temppos << endl;
			// temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
			// temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
			// temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);

			temppos[0] = fmod(pos[pn][0], ms.Lx);
			temppos[1] = fmod(pos[pn][1], ms.Ly);
			temppos[2] = fmod(pos[pn][2], ms.Lz);
			// cout << temppos << endl;
			temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
			temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
			temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);
			// cout << temppos << endl;
			// interpolater.interp3d(temppos, *u, *v, *w, uf[pn]);
			// interpolater.interp3d(temppos, dudx, dudy, dudz, gradu[pn]);
			// interpolater.interp3d(temppos, dvdx, dvdy, dvdz, gradv[pn]);
			// interpolater.interp3d(temppos, dwdx, dwdy, dwdz, gradw[pn]);

			vec3d uf1, uf2;
			vec3d gradu1, gradu2, gradv1, gradv2, gradw1, gradw2;
			// interpolater.interp3d(temppos, *u, *v, *w, uf[pn], fieldstore);

			interpolater.interp3d(temppos, *u, *v, *w, uf1, fieldstore);

			interpolater.interp3d(temppos, *dudx, *dudy, *dudz, gradu1, FieldStoreType::CCC);
			interpolater.interp3d(temppos, *dvdx, *dvdy, *dvdz, gradv1, FieldStoreType::CCC);
			interpolater.interp3d(temppos, *dwdx, *dwdy, *dwdz, gradw1, FieldStoreType::CCC);

			interpolater.interp3d(temppos, *unext, *vnext, *wnext, uf2, fieldstore);
			interpolater.interp3d(temppos, *dudxnext, *dudynext, *dudznext, gradu2, FieldStoreType::CCC);
			interpolater.interp3d(temppos, *dvdxnext, *dvdynext, *dvdznext, gradv2, FieldStoreType::CCC);
			interpolater.interp3d(temppos, *dwdxnext, *dwdynext, *dwdznext, gradw2, FieldStoreType::CCC);

			uf[pn] = uf1 * (1 - _timeInterpFactor) + uf2 * _timeInterpFactor; // temporal interpolation

			gradu[pn] = gradu1 * (1 - _timeInterpFactor) + gradu2 * _timeInterpFactor;
			gradv[pn] = gradv1 * (1 - _timeInterpFactor) + gradv2 * _timeInterpFactor;
			gradw[pn] = gradw1 * (1 - _timeInterpFactor) + gradw2 * _timeInterpFactor;

			// interpolater.interp3d_old(temppos, *u, *v, *w, uf[pn]);
			// interpolater.interp3d_old(temppos, dudx, dudy, dudz, gradu[pn]);
			// interpolater.interp3d_old(temppos, dvdx, dvdy, dvdz, gradv[pn]);
			// interpolater.interp3d_old(temppos, dwdx, dwdy, dwdz, gradw[pn]);
		}
	}

	void LoadedFlow::fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const
	{
		std::shared_ptr<const Scalar> ddx, ddy, ddz;
		std::shared_ptr<const Scalar> ddx2, ddy2, ddz2;
		if (velcomponent == 0)
		{
			ddx = dudx;
			ddy = dudy;
			ddz = dudz;
			ddx2 = dudxnext;
			ddy2 = dudynext;
			ddz2 = dudznext;
		}
		else if (velcomponent == 1)
		{
			ddx = dvdx;
			ddy = dvdy;
			ddz = dvdz;
			ddx2 = dvdxnext;
			ddy2 = dvdynext;
			ddz2 = dvdznext;
		}
		else if (velcomponent == 2)
		{
			ddx = dwdx;
			ddy = dwdy;
			ddz = dwdz;
			ddx2 = dwdxnext;
			ddy2 = dwdynext;
			ddz2 = dwdznext;
		}
		else
		{
			throw(std::runtime_error(
				"fluidVelGradAtPoint: velcomponent should be [0, 1, 2], but encounter " + std::to_string(velcomponent)));
		}
#pragma omp parallel for
		for (int pn = 0; pn < pos.size(); pn++)
		{
			vec3d temppos;
			temppos[0] = fmod(pos[pn][0], ms.Lx);
			temppos[1] = fmod(pos[pn][1], ms.Ly);
			temppos[2] = fmod(pos[pn][2], ms.Lz);
			// cout << temppos << endl;
			temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
			temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
			temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);

			vec3d tempgrad1, tempgrad2;
			interpolater.interp3d(temppos, *ddx, *ddy, *ddz, tempgrad1, FieldStoreType::CCC);
			interpolater.interp3d(temppos, *ddx2, *ddy2, *ddz2, tempgrad2, FieldStoreType::CCC);

			// temporal interpolation
			grad[pn] = tempgrad1 * (1 - _timeInterpFactor) + tempgrad2 * _timeInterpFactor;
		}
	}

	void LoadedFlow::fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const
	{
#pragma omp parallel for
		for (int pn = 0; pn < pos.size(); pn++)
		{
			vec3d temppos;
			temppos[0] = fmod(pos[pn][0], ms.Lx);
			temppos[1] = fmod(pos[pn][1], ms.Ly);
			temppos[2] = fmod(pos[pn][2], ms.Lz);
			// cout << temppos << endl;
			temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
			temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
			temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);
			// cout << temppos << endl;

			vec3d uf1, uf2;
			// interpolater.interp3d(temppos, *u, *v, *w, uf[pn], fieldstore);
			interpolater.interp3d(temppos, *u, *v, *w, uf1, fieldstore);
			interpolater.interp3d(temppos, *unext, *vnext, *wnext, uf2, fieldstore);

			uf[pn] = uf1 * (1 - _timeInterpFactor) + uf2 * _timeInterpFactor; // temporal interpolation
		}
	}

	void LoadedFlow::loadConfig(const Config &config)
	{
		string stpstr = config.Read<string>("step range", "NULL");
		if (stpstr == "NULL")
		{
			std::cout << "Wrong flow field index list in HomoIsoTurb" << std::endl;
			// step = config.Read<int>("step");
			// indexlist = vector<int>(1, step);
		}
		else
		{
			stringstream ss;
			int start, end, interv;
			ss << stpstr;
			ss >> start >> end >> interv;
			int curridx = start;
			if (interv == 0)
			{
				cout << "Error: flow field index has zero interv." << endl;
			}
			else
			{
				while (curridx < end + interv)
				{
					indexlist.push_back(curridx);
					curridx += interv;
				}
			}
		}
		ifrozen = config.Read<bool>("random frozen", false);
		_nextFieldCount = config.Read<int>("steps for next field", 1);

		_boundaryType = config.Read<string>("Boundary Type");

		_fieldStoreStr = config.Read<string>("Field Store Type", "UDF");
		if (_fieldStoreStr == "UDF")
		{
			fieldstore = FieldStoreType::UDF; // undefined
		}
		else if (_fieldStoreStr == "CCC")
		{
			fieldstore = FieldStoreType::CCC;
		}
		else if (_fieldStoreStr == "CYC")
		{
			fieldstore = FieldStoreType::CYC;
		}

		if (fieldstore == FieldStoreType::UDF)
		{
			cout << "Error: the field store type is not assigned explicitly in config.txt" << endl;
			return;
		}

		flowfieldpath = config.Read<string>("data path");
		flowfieldpath = flowfieldpath + "/";

		_uscale = config.Read<double>("uscale", 1.0); // a scaling factor for velocity
		if (_uscale != 1.0)
		{
			cout << "LoadedFlow: velocity is scaled by " << _uscale << endl;
		}
		return;
	}

	void LoadedFlow::initialize(const std::string &path, const Config &config)
	{
		// read configuration
		loadConfig(config);

		// load flow data files
		datapool.LoadData(ms, flowfieldpath, indexlist); // load flow data to datapool
		for (int i = 0; i < datapool.upool.size(); i++)
		{
			datapool.upool[i]->Multiply(_uscale); // scale the velocity by a factor
			datapool.vpool[i]->Multiply(_uscale);
			datapool.wpool[i]->Multiply(_uscale);
		}

		LoadFlowData(0);
		return;
	}

	void LoadedFlow::reset()
	{

		_flowIndexCount = 0;
		_updateStepCount = 0;
		_timeInterpFactor = 0; // reset temporal interp factor
		int chosen = ifrozen ? rd::randi(0, indexlist.size()) : 0;
		// LoadFlowData(flowfieldpath.c_str(), indexlist[chosen]);
		cout << "chosen flow idx = " << chosen << endl;
		LoadFlowData(chosen);
		// makeGradient();
	}

	// to make an instance of the class from the config.
	// cannot be done in constructor because member "mesh" don't have default constructor
	std::shared_ptr<LoadedFlow> LoadedFlow::makeInstance(const Config &config)
	{
		string str = config.Read<string>("Mesh Number");
		stringstream ss;
		int Nx, Ny, Nz;
		double Lx, Ly, Lz;
		ss << str;
		ss >> Nx >> Ny >> Nz;

		size_t iPI;
		bool pflag;

		str = config.Read<string>("DomainX");
		iPI = str.find("PI");
		pflag = iPI != string::npos;
		if (pflag)
		{
			str = str.substr(0, iPI);
		}
		ss.str("");
		ss.clear();
		ss << str;
		ss >> Lx;
		if (pflag)
			Lx *= M_PI;

		str = config.Read<string>("DomainY");
		iPI = str.find("PI");
		pflag = iPI != string::npos;
		if (pflag)
		{
			str = str.substr(0, iPI);
		}
		ss.str("");
		ss.clear();
		ss << str;
		ss >> Ly;
		if (pflag)
			Ly *= M_PI;

		str = config.Read<string>("DomainZ");
		iPI = str.find("PI");
		pflag = iPI != string::npos;
		if (pflag)
		{
			str = str.substr(0, iPI);
		}
		ss.str("");
		ss.clear();
		ss << str;
		ss >> Lz;
		if (pflag)
			Lz *= M_PI;

		string boundType = config.Read<string>("Boundary Type");

		Geometry *geo = NULL;

		if (boundType == "PPP")
		{

			geo = new Geometry_prdXYZ(Nx, Ny, Nz, Lx, Ly, Lz);
		}
		else if (boundType == "PWP")
		{
			geo = new Geometry_prdXZ(Nx, Ny, Nz, Lx, Ly, Lz);
		}
		else
		{
			string errmsg = "fluid boundary type " + boundType + " is undefined";
			cout << errmsg << endl;
			throw runtime_error(errmsg);
		}
		string meshYPath = config.Read<string>("Y Mesh Path", "");
		if (meshYPath.length() == 0)
			geo->InitMeshEdge(); // uniform mesh
		else
			geo->InitMeshEdge(true, meshYPath.c_str()); // y mesh from file

		geo->InitMesh();
		geo->InitIndices();
		geo->InitInterval();
		geo->InitWaveNumber();
		Mesh mesh(*geo);

		delete geo;

		return make_shared<LoadedFlow>(mesh);
	}

	void LoadedFlow::LoadFlowData(int loadstep)
	{
		// std::cout << "Loading flow data (LoadFlowData) at step " << loadstep << std::endl;
		// u = &(datapool.upool[loadstep]);
		// v = &(datapool.vpool[loadstep]);
		// w = &(datapool.wpool[loadstep]);
		u = (datapool.upool[loadstep]);
		v = (datapool.vpool[loadstep]);
		w = (datapool.wpool[loadstep]);

		// u.Set(datapool.upool[loadstep]);
		// v.Set(datapool.vpool[loadstep]);
		// w.Set(datapool.wpool[loadstep]);

		u->GradientAtCenter(*dudx, *dudy, *dudz, _fieldStoreStr[0]);
		v->GradientAtCenter(*dvdx, *dvdy, *dvdz, _fieldStoreStr[1]);
		w->GradientAtCenter(*dwdx, *dwdy, *dwdz, _fieldStoreStr[2]);

		makeGradientBoundary(
			*dudx, *dudy, *dudz,
			*dvdx, *dvdy, *dvdz,
			*dwdx, *dwdy, *dwdz); // set gradients at boundary. Used by interpolation

		// std::cout << "u(3,4,5)=" << u->operator()(3, 4, 5) << std::endl;
		// std::cout << "v(3,4,5)=" << v->operator()(3, 4, 5) << std::endl;
		// std::cout << "w(3,4,5)=" << w->operator()(3, 4, 5) << std::endl;
		// std::cout << "dudxyz(3,4,5)=" << dudx(3, 4, 5) << " " << dudy(3, 4, 5) << " " << dudz(3, 4, 5) << std::endl;
		// std::cout << "dvdxyz(3,4,5)=" << dvdx(3, 4, 5) << " " << dvdy(3, 4, 5) << " " << dvdz(3, 4, 5) << std::endl;
		// std::cout << "dwdxyz(3,4,5)=" << dwdx(3, 4, 5) << " " << dwdy(3, 4, 5) << " " << dwdz(3, 4, 5) << std::endl;

		// at final step, next step = loadstep (no interp), otherwise next step = loadstep+1;
		int nextStep = (loadstep == datapool.upool.size() - 1) ? loadstep : (loadstep + 1);
		if (ifrozen)
			nextStep = loadstep; // for frozen flow, next step = this step.

		// unext = &(datapool.upool[nextStep]);
		// vnext = &(datapool.vpool[nextStep]);
		// wnext = &(datapool.wpool[nextStep]);
		unext = (datapool.upool[nextStep]);
		vnext = (datapool.vpool[nextStep]);
		wnext = (datapool.wpool[nextStep]);

		unext->GradientAtCenter(*dudxnext, *dudynext, *dudznext, _fieldStoreStr[0]);
		vnext->GradientAtCenter(*dvdxnext, *dvdynext, *dvdznext, _fieldStoreStr[1]);
		wnext->GradientAtCenter(*dwdxnext, *dwdynext, *dwdznext, _fieldStoreStr[2]);
		makeGradientBoundary(
			*dudxnext, *dudynext, *dudznext,
			*dvdxnext, *dvdynext, *dvdznext,
			*dwdxnext, *dwdynext, *dwdznext); // set gradients at boundary. Used by interpolation

		// std::cout << "div=" << dudx(3, 3, 3) + dvdy(3, 3, 3) + dwdz(3, 3, 3) << std::endl;

		// if (u->ms.Nx != 96) {
		//	cout << "debug 96"<< loadstep << endl;
		// }

		return;
	}

	// void LoadedFlow::makeGradient() {
	//
	//	u.GradientAtCenter(dudx, dudy, dudz, _fieldStoreStr[0]);
	//	v.GradientAtCenter(dvdx, dvdy, dvdz, _fieldStoreStr[1]);
	//	w.GradientAtCenter(dwdx, dwdy, dwdz, _fieldStoreStr[2]);
	//	makeGradientBoundary(
	//		dudx, dudy, dudz,
	//		dvdx, dvdy, dvdz,
	//		dwdx, dwdy, dwdz); // set gradients at boundary. Used by interpolation
	//	return;
	// }

	void LoadedFlow::update(double dt)
	{
		if (ifrozen)
			return;
		++_updateStepCount;

		_timeInterpFactor += 1.0 / _nextFieldCount;

		if (_updateStepCount % _nextFieldCount == 0)
		{
			_updateStepCount = 0;
			_timeInterpFactor = 0; // reset temporal interp factor
			_flowIndexCount++;
			if (_flowIndexCount >= indexlist.size())
			{
				cout << "Error: indexcount is out of the bound of indexlist while updating fluid" << endl;
				return;
			}

			LoadFlowData(_flowIndexCount);
			// makeGradient();
			// if (_flowIndexCount % 10 == 0)	cout << indexlist[_flowIndexCount] << " ";
		}
		else
		{
			//// linear time interp
			// Timer timer;

			// timer.Tic();
			// u.Plus(ustep);
			// v.Plus(vstep);
			// w.Plus(wstep);

			// dudx.Plus(dudxstep);
			// dudy.Plus(dudystep);
			// dudz.Plus(dudzstep);
			// dvdx.Plus(dvdxstep);
			// dvdy.Plus(dvdystep);
			// dvdz.Plus(dvdzstep);
			// dwdx.Plus(dwdxstep);
			// dwdy.Plus(dwdystep);
			// dwdz.Plus(dwdzstep);
			// timer.Toc();
			// cout << "time cost at update = " << timer.elaspe() << endl;
		}

		// if (++_updateStepCount % _nextFieldCount != 0) return;

		return;
	}

	void LoadedFlow::makeGradientBoundary(Scalar &dudx, Scalar &dudy, Scalar &dudz,
										  Scalar &dvdx, Scalar &dvdy, Scalar &dvdz,
										  Scalar &dwdx, Scalar &dwdy, Scalar &dwdz)
	{
		// only make boundary for velocity grads.
		// Boundary of velocity field is ensured by the loaded data
		// velocity must be stored as CCC or CYC

		// x direction
		if (_boundaryType[0] == 'W')
		{
			// to be done
		}
		else if (_boundaryType[0] == 'P')
		{
			// no need to deal with boundary because the index system will not refer to boundary points
		}

		// y direction

		if (_boundaryType[1] == 'W')
		{
#pragma omp parallel for
			for (int k = 1; k <= Nz - 1; k++)
			{
				for (int i = 1; i <= Nx - 1; i++)
				{

					// simple linear extrapolation
					dudy(i, 0, k) = dudy(i, 1, k) - (dudy(i, 2, k) - dudy(i, 1, k)) / dudy.ms.hy(2) * dudy.ms.hy(1);
					dvdy(i, 0, k) = 0.0;
					dwdy(i, 0, k) = dwdy(i, 1, k) - (dwdy(i, 2, k) - dwdy(i, 1, k)) / dwdy.ms.hy(2) * dwdy.ms.hy(1);

					dudy(i, Ny, k) = dudy(i, Ny - 1, k) + (dudy(i, Ny - 1, k) - dudy(i, Ny - 2, k)) / dudy.ms.hy(Ny - 1) * dudy.ms.hy(Ny);
					dvdy(i, Ny, k) = 0.0;
					dwdy(i, Ny, k) = dwdy(i, Ny - 1, k) + (dwdy(i, Ny - 1, k) - dwdy(i, Ny - 2, k)) / dwdy.ms.hy(Ny - 1) * dwdy.ms.hy(Ny);

					// dudy(i, 0, k) = (*u)(i, 1, k) / (u->ms.dy(1) / 2.0);
					// dwdy(i, 0, k) = (*w)(i, 1, k) / (w->ms.dy(1) / 2.0);

					// dudy(i, Ny, k) = -(*u)(i, Ny - 1, k) / (u->ms.dy(Ny - 1) / 2.0);
					// dwdy(i, Ny, k) = -(*w)(i, Ny - 1, k) / (w->ms.dy(Ny - 1) / 2.0);
				}
			}
		}
		else if (_boundaryType[1] == 'P')
		{
			// no need to deal with boundary because the index system will not refer to boundary points
		}

		// z direction
		if (_boundaryType[2] == 'W')
		{
			// to be done
		}
		else if (_boundaryType[2] == 'P')
		{
			// no need to deal with boundary because the index system will not refer to boundary points
		}
	}
}