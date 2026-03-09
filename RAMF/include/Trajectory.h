#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <vector>
#include <fstream>
#include <string>
#include <iomanip>

class Trajectory
{
private:
	std::vector<std::vector<double>> _content; // traj content
	std::string trajHeader;					   // header line in the output file
public:
	// vectors3d pos, p3;

	int size;	// number of variables at each time step
	int maxlen; // maximal length of the trajectory.

	Trajectory(const Trajectory &another) noexcept;
	Trajectory(Trajectory &&another) noexcept;

	Trajectory &operator=(const Trajectory &another);

	Trajectory(int trajsize) : size(trajsize), trajHeader("trajectory") {}
	Trajectory(int trajsize, std::string header, int maxlen) noexcept : size(trajsize), trajHeader(header), _currentPos(0),
																		maxlen(maxlen), _content(maxlen, std::vector<double>(trajsize, 0)) {}
	// Trajectory(int init_capacity) :pos(init_capacity), p3(init_capacity) {
	// }
	// Trajectory(const Trajectory& traj) {
	//	pos = traj.pos;
	//	p3 = traj.p3;
	//	return;
	// }

	inline void Reset()
	{
		// _content.clear();
		_currentPos = 0;
		return;
	};
	// void Reserve(unsigned int len)
	// {
	// 	_content.clear();
	// 	_content.reserve(len);
	// 	// if (trajSize != this->size)
	// 	// {
	// 	// 	throw(std::runtime_error("Trajectory size is not consistent with the presets when reserving space."));
	// 	// }
	// 	for (int i = 0; i < len; i++)
	// 	{
	// 		_content.push_back(std::vector<double>(this->size, 0));
	// 	}
	// }; // reserve size
	// void Record(const double snapshot[], int snapshotSize);
	void Record(const std::vector<double> &snapshot);

	~Trajectory() {};
	void writeTraj(std::string dir);
	void writeTrajBinary(std::string dir);

private:
	int _currentPos; // current recording position
};
#endif