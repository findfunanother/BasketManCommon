#include "XorShift.h"

namespace XorShifts
{
	XorShift::XorShift(uint32_t w, uint32_t x, uint32_t y, uint32_t z)
	{
		x = x != NULL ? x : w << 13;
		y = y != NULL ? y : (w >> 9) ^ (x << 6);
		z = z != NULL ? z : y >> 7;
		seeds = { x,y,z,w };
		this->w = w; this->x = x; this->y = y; this->z = z;
	}

	uint32_t XorShift::rand()
	{
		randCount++;
		uint32_t t = x ^ (x << 11);
		x = y;
		y = z;
		z = w;
		return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
	}

	int XorShift::randInt(int min, int max)
	{
		return rand() % (max - min + 1) + min;
	}

	float XorShift::randFloat(float min, float max)
	{
		return (float)(rand() % 0xFFFF) / 0xFFFF * (max - min) + min;
	}
}
