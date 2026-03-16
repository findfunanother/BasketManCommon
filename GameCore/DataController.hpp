#pragma once
#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


#include "DataController.h"

#include <vector>

template<typename TABLE>
CDataController<TABLE>::CDataController()
{
	m_data = NULL;
	m_datalen = 0;
}

template<typename TABLE>
inline const TABLE* CDataController<TABLE>::GetData()
{
	if (!m_data)
		return NULL;
	return flatbuffers::GetRoot<TABLE>(m_data);
}

template<typename TABLE>
CDataController<TABLE>::~CDataController()
{
	if (m_data)
	{
		free(m_data);
		m_data = NULL;
	}
}

template<typename TABLE>
bool CDataController<TABLE>::ReadFile(std::string filePath)
{
	std::ifstream is(filePath, std::ifstream::binary);
	if (is) {
		// seekg�� �̿��� ���� ũ�� ����
		is.seekg(0, is.end);
		int length = (int)is.tellg();
		is.seekg(0, is.beg);

		// malloc���� �޸� �Ҵ�
		unsigned char* buffer = (unsigned char*)malloc(length);

		// read data as a block:
		is.read((char*)buffer, length);
		is.close();
		m_data = buffer;
		m_datalen = length;
		return true;
	}
	return false;
}

template<typename TABLE>
string CDataController<TABLE>::GetCurrentDirectoryStr()
{
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, sizeof(buff));
	std::string currentpath(buff);
	return currentpath;
	//return string();
}

template<typename TABLE>
bool CDataController<TABLE>::ReadBinary(string namefile)
{
	strBinName = namefile;
	string cwd = GetCurrentDirectoryStr();

	std::vector<string> candidates;
	candidates.push_back(cwd + "/game/cservice/Resource/" + strBinName + ".bin");
	candidates.push_back(cwd + "/Assets/InGame/Res/" + strBinName + ".bin");
	candidates.push_back(cwd + "/../Assets/InGame/Res/" + strBinName + ".bin");
	candidates.push_back(cwd + "/../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");
	candidates.push_back(cwd + "/../../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");
	candidates.push_back(cwd + "/../../../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");

	for (size_t i = 0; i < candidates.size(); ++i)
	{
		if (ReadFile(candidates[i]))
		{
			return true;
		}
	}

	return false;
}

template<typename TABLE>
inline void CDataController<TABLE>::CreateData(string str)
{
	strBinName = str;
	if (ReadBinary(strBinName))
	{
	}
}

