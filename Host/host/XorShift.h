#pragma once
#include "HostDefine.h"

#include <fstream>
#include <iostream>
#include <cstdarg>
#include <vector>
#include <ctime>

using namespace std;

namespace XorShifts
{
	class XorShift
	{
	private:
		struct dictionary { uint32_t x; uint32_t y; uint32_t z; uint32_t w; };
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t w;
		uint32_t t;

	public:
		uint32_t randCount = 0;
		struct dictionary seeds;

		XorShift(uint32_t w = time(nullptr), uint32_t x = NULL, uint32_t y = NULL, uint32_t z = NULL);

		uint32_t rand();

		int randInt(int min = 0, int max = 0x7FFFFFFF);

		float randFloat(float min = 0, float max = 1);
	};
}
