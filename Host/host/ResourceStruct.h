#pragma once

#include <stdint.h>
#include <array>
#include <sstream>
#include <string>
#include <map>


namespace Resource
{
	typedef struct _InGameBalanceData
	{
		std::map<std::string, float> BalanceData;
	} InGameBalanceData;

}  // namespace Resource
