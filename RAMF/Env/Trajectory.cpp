
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include "Trajectory.h"
#include <iostream>

// copy constructor
Trajectory::Trajectory(const Trajectory &other) noexcept : trajHeader(other.trajHeader),
														   size(other.size),
														   maxlen(other.maxlen),
														   _currentPos(other._currentPos),
														   _content(other.maxlen, std::vector<double>(other.size, 0))
{

	for (int i = 0; i < other._currentPos; i++)
	{
		// copy other traj
		for (int j = 0; j < size; j++)
		{
			_content[i][j] = other._content[i][j];
		}
		// _content.push_back(cont);
	}
}

// move constructor
Trajectory::Trajectory(Trajectory &&other) noexcept : trajHeader(std::move(other.trajHeader)), _content(std::move(other._content)),
													  size(other.size), maxlen(other.maxlen), _currentPos(other._currentPos)
{
	// std::cout << "Trajectory moved" << std::endl;
}

// operator =, overide copy constructre
Trajectory &Trajectory::operator=(const Trajectory &other)
{
	if (this != &other)
	{
		trajHeader = other.trajHeader;
		size = other.size;
		maxlen = other.maxlen;
		_currentPos = other._currentPos;
		_content.resize(maxlen, std::vector<double>(size));
		for (int i = 0; i < other._currentPos; i++)
		{
			for (int j = 0; j < size; j++)
			{
				_content[i][j] = other._content[i][j];
			}
		}
	}
	return *this;
}

void Trajectory::Record(const std::vector<double> &snapshot)
{
	if (snapshot.size() != size)
	{
		throw("wrong traj size in Trajectory::Record");
	}

	for (int i = 0; i < size; i++)
	{
		_content[_currentPos][i] = snapshot[i];
	}
	_currentPos++;
}

void Trajectory::writeTraj(std::string dir)
{
	std::ofstream outfile;
	outfile.open(dir);
	outfile << trajHeader << std::endl;
	// outfile << "xpos\t" << "ypos\t" << "zpos\t" << "p31\t" << "p32\t" << "p33\t" << std::endl;
	outfile << std::showpoint << std::setprecision(8) << std::left;

	for (unsigned i = 0; i < _currentPos; i++)
	{
		for (unsigned j = 0; j < size; j++)
		{
			outfile << std::setw(16) << _content[i][j] << "\t";
		}
		outfile << std::endl;
	}

	/*	for (unsigned i = 0; i < pos.size(); i++) {
			outfile << std::setw(16) << pos[i][0] << "\t" << pos[i][1] << "\t" << pos[i][2] << "\t"
				<< p3[i][0] << "\t" << p3[i][1] << "\t" << p3[i][2] << "\t" << std::endl;
		}*/
	outfile.close();
}

void Trajectory::writeTrajBinary(std::string dir)
{
	std::ofstream outfile(dir, std::ios::binary);
	if (!outfile)
	{
		std::cerr << "Error opening file: " << dir << std::endl;
		return;
	}

	// 写入头部信息（如果 `trajHeader` 是 std::string）
	size_t headerSize = trajHeader.size();
	outfile.write(reinterpret_cast<const char *>(&headerSize), sizeof(size_t)); // 先写入长度
	outfile.write(trajHeader.data(), headerSize);								// 再写入实际字符串

	// 写入数据大小信息
	outfile.write(reinterpret_cast<const char *>(&_currentPos), sizeof(unsigned));
	outfile.write(reinterpret_cast<const char *>(&size), sizeof(unsigned));

	// 写入数据内容
	for (unsigned i = 0; i < _currentPos; i++)
	{
		outfile.write(reinterpret_cast<const char *>(_content[i].data()), size * sizeof(double));
	}

	outfile.close();
}
