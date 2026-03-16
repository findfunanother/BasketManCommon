#pragma once
#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define DECLSPECBALANCE __declspec(dllexport)
#else
#define DECLSPECBALANCE __declspec(dllimport) 
#endif
#define DLLOCAL
#else
#define DECLSPECBALANCE __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif


#include <string>
#include <iostream> // C++ header file for printing
#include <fstream>
#include <map>
#include "AnimationInformation_generated.h"

using namespace std;
template<typename TABLE>
class  CDataController
{
	string strBinName;
	unsigned char* m_data;
	int m_datalen;
private:
	bool ReadFile(std::string filePath);
	string GetCurrentDirectoryStr();
	bool ReadBinary(string namefile);


protected:
	

public:
	unsigned char* GetRawData() { return m_data; }
	int GetDataLength() { return m_datalen; }

	void CreateData(string str);
	CDataController();
	const TABLE* GetData();
	~CDataController();
	
	
};

#include "DataController.hpp"