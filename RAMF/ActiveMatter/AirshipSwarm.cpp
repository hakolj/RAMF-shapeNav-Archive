#include "AirshipSwarm.h"
#include "Random.h"
#include "Geography.h"
#include "MatrixOp.h"
void AirshipSwarm::initialize(const std::string &cfgContent, const Config &config)
{
	std::istringstream iss(cfgContent);
	Config Cfg(iss, "TASK");
	meandist = Cfg.Read<double>("Distance");
	Config indCfg(iss, "INDIVIDUAL");
	resetEpisode = indCfg.Read<int>("reset episode", 0);
	resetCount = resetEpisode; // to ensure always reset for the first episode
	HomogeneousSwarm::initialize(cfgContent, config);
}
void AirshipSwarm::reset()
{
	resetCount++;
	bool resetflag = resetCount >= resetEpisode;
	if (resetflag)
	{
		resetCount = 0;
		for (int i = 0; i < amatternum; i++)
		{
			auto airships = std::dynamic_pointer_cast<Airship>(swarms[i]);
			for (auto &v : swarmValidMask[i])
				v = 1;

			double c = std::sin(M_PI_4);
			double phi = 0.0;
			phi = rd::randd() * M_PI;
			for (unsigned j = 0; j < swarmSize; j++)
			{
				airships->ee[j](0) = c * std::cos(phi);
				airships->ee[j](1) = c * std::cos(phi);
				airships->ee[j](2) = c * std::sin(phi);
				airships->ee[j](3) = c * std::sin(phi);
				mat::ee2R(airships->ee[j], airships->p0[j], airships->p1[j], airships->p2[j]);
			}

			initPos(airships);
			airships->resetFlowInfo();
		}
	}
}
void AirshipSwarm::initPos(std::shared_ptr<Airship> amatter)
{
	amatter->poslla[0][0] = rd::randd(-1.0, 1.0) * amatter->initBoxSize[0] + amatter->initCoreCenter[0];
	amatter->poslla[0][1] = rd::randd(-1.0, 1.0) * amatter->initBoxSize[1] + amatter->initCoreCenter[1];
	amatter->poslla[0][2] = rd::randd(-1.0, 1.0) * amatter->initBoxSize[2] + amatter->initCoreCenter[2];
	vec3d ecef0, posenu, posecef;
	double theta;
	geo::LLAToECEF(amatter->poslla[0], ecef0);

	for (unsigned i = 1; i < amatter->amatternum; i++)
	{
		theta = rd::randd(-1.0, 1.0) * M_PI;
		posenu[0] = meandist * cos(theta);
		posenu[1] = meandist * sin(theta);
		posenu[2] = 0;
		geo::ENUToECEF(posenu, amatter->poslla[0], posecef);
		geo::ECEFToLLA(posecef + ecef0, amatter->poslla[i]);
		amatter->poslla[i][2] = amatter->poslla[0][2]; // keep the same altitude
	}
	for (unsigned i = 0; i < amatter->amatternum; i++)
	{
		amatter->vp_new[i] = vec3d::Zero();
		amatter->vp_old[i] = vec3d::Zero();
		amatter->_istep1trans = true;
	}
	for (auto &elem : amatter->pos)
	{
		elem.setZero();
	}
	amatter->lastposlla = amatter->poslla;
	return;
}