#pragma once

#include <iostream> // C++ header file for printing
#include <fstream>
#include <string>
//#include <windows.h>
#include <map>

#define FILE_MAX_PATH 256

#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport) 
#endif

#define DLLOCAL
#else
#define DECLSPEC __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif

using namespace std;

extern "C"
{
	namespace TB
	{
		struct F4Balance;
		struct F4BalanceTable;
	}
	using namespace TB;
	class DECLSPEC CBalanceData
	{
	private:
		std::map<string, float> * m_BalanceValues;
		std::string GetCurrentDirectoryStr();

		bool ReadFile(std::string filePath, unsigned char** _data, int* datalen);
	public:
		CBalanceData();
		~CBalanceData();
		bool ReadBinary();
		float GetValue(string str);
		int GetDataCount();
		
	};
}
