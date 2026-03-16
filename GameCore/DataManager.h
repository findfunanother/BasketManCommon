#pragma once
#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define DECLSPECT __declspec(dllexport)
#else
#define DECLSPECT __declspec(dllimport) 
#endif
#define DLLOCAL
#else
#define DECLSPECT __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif
#include <string>
#include <map>
#include <iostream>

using namespace std;
namespace flatbuffers
{
	class Table;
}

template<typename KEY,typename TABLE>
class CDataManager
{

	string strBinName;
private:
	bool ReadFile(std::string filePath, unsigned char** _data, int* datalen)
	{
		std::ifstream is(filePath, std::ifstream::binary);
		if (is) {
			// seekg를 이용한 파일 크기 추출
			is.seekg(0, is.end);
			int length = (int)is.tellg();
			is.seekg(0, is.beg);

			// malloc으로 메모리 할당
			unsigned char* buffer = (unsigned char*)malloc(length);

			// read data as a block:
			is.read((char*)buffer, length);
			is.close();
			*_data = buffer;
			*datalen = length;
			return true;
		}
	}
	string GetCurrentDirectoryStr()
	{
		char buff[FILENAME_MAX]; //create string buffer to hold path
		GetCurrentDir(buff, sizeof(buff));
		std::string currentpath(buff);
		return currentpath;
		//return string();
	}


	bool ReadBinary()
	{
		{
			unsigned char* _data = NULL;
			int datalen = 0;
			string str = GetCurrentDirectoryStr();
			str.append("/game/cservice/Resource/");
			str.append(strBinName);
			str.append(".bin");
			ReadFile(str, &_data, &datalen);
			if (!_data)
				return false;

			m_data = flatbuffers::GetRoot<TABLE>(_data);
			if (m_data)
			{
				m_mapData = new MAPDATA();
				CreateDictionary();
				return true;
			}
			else
			{
				m_mapData = NULL;
				return false;
			}


		}
	}

protected:
	typedef map<KEY, const flatbuffers::Table*> MAPDATA;
	const TABLE* m_data;

	MAPDATA* m_mapData;

public:
	bool HasData()
	{
		return m_mapData != NULL && m_mapData->size() > 0;
	}
	// { return m_mapData != NULL; };
	virtual void CreateDictionary() = 0;


	CDataManager(string str);
	
	virtual ~CDataManager();
	const int GetDataCount();
};

#include "DataManager.hpp"