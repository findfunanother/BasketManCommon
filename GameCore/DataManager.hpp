#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include "AnimationInformation_generated.h"

using namespace std;
using namespace flatbuffers;

	template<typename TABLE>
	bool CDataManager<TABLE>::ReadFile(std::string filePath, unsigned char** _data, int* datalen)
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
			*_data = buffer;
			*datalen = length;
			return true;
		}

		return false;
	}

	template<typename TABLE>
	bool CDataManager<TABLE>::ReadBinary()
	{
		unsigned char* _data = NULL;
		int datalen = 0;
		string str = GetCurrentDirectoryStr();
		std::vector<string> candidates;
		candidates.push_back(str + "/game/cservice/Resource/" + strBinName + ".bin");
		candidates.push_back(str + "/Assets/InGame/Res/" + strBinName + ".bin");
		candidates.push_back(str + "/../Assets/InGame/Res/" + strBinName + ".bin");
		candidates.push_back(str + "/../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");
		candidates.push_back(str + "/../../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");
		candidates.push_back(str + "/../../../BasketMan.00.Client/Assets/InGame/Res/" + strBinName + ".bin");

		for (size_t i = 0; i < candidates.size(); ++i)
		{
			if (ReadFile(candidates[i], &_data, &datalen))
			{
				break;
			}
		}
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

	template<typename TABLE>
	string CDataManager<TABLE>::GetCurrentDirectoryStr()
	{
		char buff[FILENAME_MAX]; //create string buffer to hold path
		GetCurrentDir(buff, sizeof(buff));
		std::string currentpath(buff);
		return currentpath;
		//return string();
	}

	template<typename TABLE>
	CDataManager<TABLE>::CDataManager(string str)
	{
		strBinName = str;
		
		//ReadBinary();
		//CreateDictionary();
	}

	template<typename TABLE>
	CDataManager<TABLE>::CDataManager()
	{
	}

	template<typename TABLE>
	void CDataManager<TABLE>::CreateDictionary()
	{

	}

	template<typename TABLE>
	CDataManager<TABLE>::~CDataManager()
	{
		m_mapData->clear();
		delete(m_mapData);
	}

	template<typename TABLE>
	const int CDataManager<TABLE>::GetDataCount()
	{
		return m_mapData->size();
	}
