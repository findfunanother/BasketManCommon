#include "CSVLoader.h"

namespace Resource
{
	std::vector<std::string> CSVLoader::Split(std::string str, std::string delim)
	{
		std::vector<std::string> retValue;

		auto start = 0U;
		auto end = str.find(delim);
		while (end != std::string::npos)
		{
			retValue.push_back(str.substr(start, end - start));
			start = static_cast<unsigned int>(end + delim.length());
			end = str.find(delim, start);
		}

		return retValue;
	}
}