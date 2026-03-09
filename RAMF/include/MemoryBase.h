#ifndef MEMORYBASE_H
#define MEMORYBASE_H
#include <iostream>
#include <string.h>

class MemoryBase
{
public:
	virtual void writeMemory(const std::string &path) const = 0;
	virtual void readMemory(const std::string &path) = 0;
	virtual ~MemoryBase(){};
};

#endif