#pragma once

#include <chrono>
#include <random>
#include <string>

#include <iostream>
#include <time.h>

#ifdef _WIN32
#include <ctime>
#include <iomanip>
#else
#endif

#include "HostDefine.h"

namespace Util
{
	typedef std::chrono::time_point<std::chrono::system_clock> timeclock_type;
	//typedef std::chrono::time_point<std::chrono::steady_clock> timeclock_type;
	typedef std::chrono::steady_clock::time_point clocktimetype;
	typedef std::chrono::milliseconds clock_millisecondtype;

	DHOST_TYPE_GAME_TIME_F GetTime();
	timeclock_type GetNow();

	std::time_t systemtime_now();

	std::string GetLocalTime();
	std::string GetLocalTime(timespec& time);

	DHOST_TYPE_GAME_TIME_F GetElapsedTime(timeclock_type& starttime);
	//DHOST_TYPE_GAME_TIME_F GetElapsedTime(const std::chrono::steady_clock::time_point& starttime);

	clocktimetype GetClocktime();
	clock_millisecondtype getrunTime(clocktimetype& starttime);
	clock_millisecondtype getrunTime(clocktimetype& starttime, clocktimetype& endtime);

	uint32_t GetRandom(const uint32_t min, const uint32_t max);
	int GetRandomSetMax(const int value);
	int GetRandomMinToMax(const int min, const int max);
	uint32_t GetRandomUINT32();
	float GetRandom(const float value);
	float GetRandom(const float value, const int32_t seed);

	const std::string CurrentDateTime();

	std::string CutOnSpecificDecimalPt(string num, int pos);	// 소수점 문자열 특정 자리에서 짜르기
}
