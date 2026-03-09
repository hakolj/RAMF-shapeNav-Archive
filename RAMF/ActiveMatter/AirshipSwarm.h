#include "HomogeneousSwarm.h"
#include "Airship.h"
class AirshipSwarm : public HomogeneousSwarm {
public:
	double meandist;
	int resetCount, resetEpisode;
	AirshipSwarm(unsigned amatternum) : HomogeneousSwarm(amatternum) {};
	virtual void initialize(const std::string& cfgContent, const Config& config);
	virtual void reset();
	virtual void initPos(std::shared_ptr<Airship> amatter);
};