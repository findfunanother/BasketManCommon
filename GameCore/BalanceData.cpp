#include "pch.h"
#include "BalanceData.h"
#include "AnimationInformation_generated.h"

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


std::string CBalanceData::GetCurrentDirectoryStr()
{
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, sizeof(buff));
	std::string currentpath(buff);
	return currentpath;
}

bool CBalanceData::ReadFile(std::string filePath, unsigned char** _data, int* datalen)
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
bool CBalanceData::ReadBinary()
{
	unsigned char* _data = NULL;
	int datalen = 0;
	string str = GetCurrentDirectoryStr();
	std::vector<string> candidates;
	candidates.push_back(str + "/game/cservice/Resource/BalanceData.bin");
	candidates.push_back(str + "/Assets/InGame/Res/BalanceData.bin");
	candidates.push_back(str + "/../Assets/InGame/Res/BalanceData.bin");
	candidates.push_back(str + "/../BasketMan.00.Client/Assets/InGame/Res/BalanceData.bin");
	candidates.push_back(str + "/../../BasketMan.00.Client/Assets/InGame/Res/BalanceData.bin");
	candidates.push_back(str + "/../../../BasketMan.00.Client/Assets/InGame/Res/BalanceData.bin");

	for (size_t i = 0; i < candidates.size(); ++i)
	{
		if (ReadFile(candidates[i], &_data, &datalen))
		{
			break;
		}
	}
	if (!_data)
		return false;
	auto monster = flatbuffers::GetRoot<F4BalanceTable>(_data);
	auto datas = monster->datas();
	if (m_BalanceValues)
		delete(m_BalanceValues);
	m_BalanceValues = new std::map<string, float>();
	for (size_t i = 0; i < datas->size(); i++)
	{
		auto data = datas->Get(i);
		m_BalanceValues->insert(pair<string, float>("V" + data->key()->str(), data->value()));
	}
	return true;
}

CBalanceData::CBalanceData()
{
	m_BalanceValues = NULL;
	ReadBinary();
}

CBalanceData::~CBalanceData()
{
}

float CBalanceData::GetValue(string str)
{
	auto iter = m_BalanceValues->find(str);
	if (iter != m_BalanceValues->end())
		return iter->second;
	return -1.f;
}

int CBalanceData::GetDataCount()
{
	return m_BalanceValues->size();
}
