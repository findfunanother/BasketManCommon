#pragma once

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include<iostream>
#include<vector>

namespace Resource
{
	class CSVLoader
	{
	protected:
		std::vector<std::string> Split(std::string str, std::string delim);
	};
}