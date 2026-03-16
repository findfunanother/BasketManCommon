#include <sstream>
#include "Util.h"

namespace Util
{
	DHOST_TYPE_GAME_TIME_F GetTime()
	{
		static std::chrono::time_point<std::chrono::system_clock> startChronoTime = std::chrono::system_clock::now();
		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<float> dur = end - startChronoTime;
		return dur.count();
	}
/*
	timeclock_type GetNow()
	{
		return std::chrono::steady_clock::now();
	}

*/
	timeclock_type GetNow()
	{
		return std::chrono::system_clock::now();
	}


	// epoch time from 2019/10/01
	uint64_t GetSserverBaseTime()
	{
		std::tm tm_value = std::tm();
		tm_value.tm_year = 119;  // localtime(&tNow)->tm_year;	 from 1900 + 119
		tm_value.tm_mon = 9;	 // localtime(&tNow)->tm_mon;
		tm_value.tm_mday = 1;	// localtime(&tNow)->tm_mday;
		tm_value.tm_hour = 0;
		tm_value.tm_min = 0;
		tm_value.tm_sec = 0;

		std::time_t the_epoch_point_day_zero = std::mktime(&tm_value);
		std::chrono::system_clock::time_point tp_day_zero =
			std::chrono::system_clock::from_time_t(the_epoch_point_day_zero);
		std::chrono::system_clock::time_point tp_now = std::chrono::system_clock::now();

		uint64_t mill_from_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp_now - tp_day_zero).count();

		return mill_from_epoch;
	}

	
	
	DHOST_TYPE_GAME_TIME_F GetElapsedTime(timeclock_type& starttime)
	{
		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed_seconds = end - starttime;
		return elapsed_seconds.count();
	}
	
	/*
	DHOST_TYPE_GAME_TIME_F GetElapsedTime(timeclock_type& starttime)
	{
		std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed_seconds = end - starttime;
		return elapsed_seconds.count();
	}
	*/

	std::tm localtime(const std::time_t& time)
	{
		std::tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&tm_snapshot, &time);
#else
		localtime_r(&time, &tm_snapshot);  // POSIX
#endif
		return tm_snapshot;
	}

	std::string put_time(const std::tm* date_time, const char* c_time_format)
	{
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		std::ostringstream oss;

		// BOGUS hack done for VS2012: C++11 non-conformant since it SHOULD take a "const struct tm*  "
		// ref. C++11 standard: ISO/IEC 14882:2011, ¡× 27.7.1,
		oss << std::put_time(const_cast<std::tm*>(date_time), c_time_format);
		return oss.str();

#else  // LINUX
		const size_t size = 1024;
		char buffer[size];
		auto success = std::strftime(buffer, size, c_time_format, date_time);

		if (0 == success)
			return c_time_format;

		return buffer;
#endif
	}

	std::time_t systemtime_now()
	{
		std::chrono::time_point<std::chrono::system_clock> system_now = std::chrono::system_clock::now();
		return std::chrono::system_clock::to_time_t(system_now);
	}



	std::string GetLocalTime()
	{
		std::tm lt = localtime(systemtime_now());
		return std::move(put_time(&lt, "%F %T"));
	}

	std::string GetLocalTime(timespec& time)
	{
		char timebuffer[32] = { 0 };
		char timestring[32];
		time_t seconds = time.tv_sec;

		std::tm lt = localtime(seconds);

		strftime(timebuffer, 32, "%Y-%m-%d %H:%M:%S", &lt);
		snprintf(timestring, 32, "%s.%ld", timebuffer, time.tv_nsec);

		return timestring;
	}

	clocktimetype GetClocktime()
	{
		return std::chrono::steady_clock::now();
	}

	clock_millisecondtype getrunTime(clocktimetype& starttime)
	{
		clocktimetype endChronoTime = GetClocktime();
		return getrunTime(starttime, endChronoTime);
	}

	clock_millisecondtype getrunTime(clocktimetype& starttime, clocktimetype& endtime)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(endtime - starttime);
	}

	uint32_t GetRandom(const uint32_t min, const uint32_t max)
	{
		// set seed.
		std::random_device rd;
		std::mt19937_64 rnd(rd());

		// set distribute.
		std::uniform_int_distribution<int> range(min, max);
		int rv = range(rnd);

		return static_cast<uint32_t>(rv);
	}

	int GetRandomSetMax(const int value)
	{
		std::random_device rd;
		std::mt19937_64 rnd(rd());
		std::uniform_int_distribution<int> range(1, value);
		int rv = range(rnd);

		return rv;
	}

	int GetRandomMinToMax(const int min, const int max)
	{
		std::random_device rd;
		std::mt19937_64 rnd(rd());
		std::uniform_int_distribution<int> range(min, max);
		int rv = range(rnd);

		return rv;
	}

	uint32_t GetRandomUINT32()
	{
		std::random_device rd;
		std::mt19937_64 rnd(rd());
		std::uniform_int_distribution<uint32_t> range(1, 4294967295);
		uint32_t rv = range(rnd);

		return rv;
	}

	float GetRandom(const float value)
	{
		std::random_device rd;
		std::mt19937_64 rnd(rd());
		std::uniform_real_distribution<> range(0.0f, value);
		float rv = range(rnd);

		return static_cast<float>(rv);
	}

	float GetRandom(const float value, const int32_t seed)
	{
		std::random_device rd;
		std::mt19937_64 rnd(seed);
		std::uniform_real_distribution<> range(0.0f, value);
		float rv = range(rnd);

		return static_cast<float>(rv);
	}

	const std::string CurrentDateTime()
	{
		time_t     now = time(NULL);
		struct tm  tstruct;
		char       buf[80];

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
		localtime_s(&tstruct, &now);
#else
		localtime_r(&now, &tstruct);
#endif
		strftime(buf, sizeof(buf), "[%Y-%m-%d %X]", &tstruct);
		return buf;
	}

	std::string CutOnSpecificDecimalPt(string num, int pos)
	{
		return num.substr(0, num.find('.') + pos + 1);
	}

}