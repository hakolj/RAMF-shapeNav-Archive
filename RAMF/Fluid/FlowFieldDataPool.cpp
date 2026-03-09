#include "pch.h"
#include "FlowFieldDataPool.h"

using namespace std;
FlowFieldDataPool::FlowFieldDataPool() : upool(), vpool(), wpool(), _datapath()
{
}

void FlowFieldDataPool::AllocateNew(int size, const fluid::Mesh &ms)
{
	// delete existing data
	upool.clear();
	vpool.clear();
	wpool.clear();

	// allocate new data
	upool.resize(size);
	vpool.resize(size);
	wpool.resize(size);

	for (int i = 0; i < size; i++)
	{
		upool[i] = std::make_shared<fluid::Scalar>(ms);
		vpool[i] = std::make_shared<fluid::Scalar>(ms);
		wpool[i] = std::make_shared<fluid::Scalar>(ms);
	}
}

void FlowFieldDataPool::LoadData(const fluid::Mesh &ms, const std::string &datapath, const std::vector<int> &idxlist)
{
	LoadData(ms, "u", "v", "w", datapath, idxlist);
	return;
}

void FlowFieldDataPool::LoadData(const fluid::Mesh &ms, std::string uName, std::string vName, std::string wName, const std::string &datapath, const std::vector<int> &idxlist)
{
	_datapath = datapath;

	// delete existing data
	upool.clear();
	vpool.clear();
	wpool.clear();
	upool.resize(idxlist.size());
	vpool.resize(idxlist.size());
	wpool.resize(idxlist.size());

	// note: when load hit data of Nx,Ny,Nz grid points,
	// the scalar has a data of size Nx+1, Ny+1, Nz+1. example: U
	// Because the domain is periodic, U(0,:,:) = U(Nx+1,:,:). U(:,0,:) = U(:,Ny+1,:) ...
	// BUT IN THE CURRENT DATA U(0,:,:) IS SET TO ZERO BECAUSE IT IS NEVER USED.

	// for HIT flow data, all the velocities and gradients are stored at the center of a grid in 3 directions
	for (int i = 0; i < idxlist.size(); i++)
	{
		char idx[256];
		string fname;
		sprintf(idx, "%.7d", idxlist[i]);
		string idxstr = string(idx);
		// fname = uName + "-" + idxstr;
		// upool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');
		// fname = vName + "-" + idxstr;
		// vpool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');
		// fname = wName + "-" + idxstr;
		// wpool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');

		fname = uName + "-" + idxstr;
		upool[i] = std::make_shared<fluid::Scalar>(ms);
		upool[i]->FileIO(_datapath.c_str(), fname.c_str(), 'r');
		fname = vName + "-" + idxstr;
		vpool[i] = std::make_shared<fluid::Scalar>(ms);
		vpool[i]->FileIO(_datapath.c_str(), fname.c_str(), 'r');
		fname = wName + "-" + idxstr;
		wpool[i] = std::make_shared<fluid::Scalar>(ms);
		wpool[i]->FileIO(_datapath.c_str(), fname.c_str(), 'r');
	}
	// cout << upool[0](1, 2, 1) << endl;
	// cout << upool[1](1, 2, 1) << endl;
	return;
}

void FlowFieldDataPool::LoadData2D(const fluid::Mesh &ms, std::string uName, std::string vName, const std::string &datapath, const std::vector<int> &idxlist)
{
	using namespace fluid;

	_datapath = datapath;

	// delete existing data
	upool.clear();
	vpool.clear();
	wpool.clear();
	upool.resize(idxlist.size());
	vpool.resize(idxlist.size());
	wpool.resize(idxlist.size());

	// note: when load hit data of Nx,Ny,Nz grid points,
	// the scalar has a data of size Nx+1, Ny+1, Nz+1. example: U
	// Because the domain is periodic, U(0,:,:) = U(Nx+1,:,:). U(:,0,:) = U(:,Ny+1,:) ...
	// BUT IN THE CURRENT DATA U(0,:,:) IS SET TO ZERO BECAUSE IT IS NEVER USED.

	// for HIT flow data, all the velocities and gradients are stored at the center of a grid in 3 directions
	for (int i = 0; i < idxlist.size(); i++)
	{
		char idx[256];
		string fname;
		sprintf(idx, "%.7d", idxlist[i]);
		string idxstr = string(idx);
		// fname = uName + "-" + idxstr;
		// upool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');
		// fname = vName + "-" + idxstr;
		// vpool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');
		// fname = wName + "-" + idxstr;
		// wpool[i].FileIO(_datapath.c_str(), fname.c_str(), 'r');
		fname = uName + "-" + idxstr;
		upool[i] = std::make_shared<fluid::Scalar>(ms);
		upool[i]->FileIO(_datapath.c_str(), fname.c_str(), 'r');
		fname = vName + "-" + idxstr;
		vpool[i] = std::make_shared<fluid::Scalar>(ms);
		vpool[i]->FileIO(_datapath.c_str(), fname.c_str(), 'r');
	}

	// now set wpool to zero
	wpool[0] = std::make_shared<fluid::Scalar>(ms);
	wpool[0]->Set(0.0);
	// replicate for all other indices
	for (int i = 1; i < idxlist.size(); i++)
	{
		wpool[i] = wpool[0];
	}

	return;
}

void FlowFieldDataPool::LoadData2D(const fluid::Mesh &ms, const std::string &datapath, const std::vector<int> &idxlist)
{
	LoadData2D(ms, "u", "v", datapath, idxlist);
	return;
}

FlowFieldDataPool::~FlowFieldDataPool()
{
	upool.clear();
	vpool.clear();
	wpool.clear();
}
