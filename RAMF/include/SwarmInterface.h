
class SwarmActiveMatter
{
protected:
	unsigned swarmSize; // number of individuals of each swarm
	std::vector<std::shared_ptr<ActiveMatter>> swarms;
	std::vector<std::vector<int>> swarmValidMask; // validmask for each swarm

public:
	SwarmActiveMatter(int amatternum) : swarmSize(0), swarms(amatternum, nullptr) {}
	const std::vector<std::shared_ptr<ActiveMatter>> &swarm() { return swarms; }
	unsigned getSwarmSize() const { return swarmSize; }
};