#include "DataController.h"
#include "DataManagerBalance.h"
#include "AnimationInformation_generated.h"
using namespace TB;
using namespace std;


CDataManagerBalance::CDataManagerBalance()
{
	m_mapBalanceData.clear();
	m_dataController = new CDataController<TB::F4BalanceTable>();
	m_dataController->CreateData("BalanceData");
	CreateDictionary();
}

CDataManagerBalance::~CDataManagerBalance()
{
	delete(m_dataController);
	m_mapData.clear();
}

const unsigned char* CDataManagerBalance::GetBalanceRawData() const
{
	return m_dataController ? m_dataController->GetRawData() : nullptr;
}

int CDataManagerBalance::GetBalanceRawDataLen() const
{
	return m_dataController ? m_dataController->GetDataLength() : 0;
}

void CDataManagerBalance::CreateDictionary()
{
	m_mapData.clear();
	m_mapBalanceData.clear();

	if (!m_dataController)
		return;

	auto list = m_dataController->GetData();
	if (!list)
		return;
	auto datas = list->datas();
	if (!datas)
		return;

	int idx = 0;
	for (flatbuffers::uoffset_t i = 0; i < datas->size(); i++)
	{
		auto data = datas->Get(i);
		if (!data || !data->key())
			continue;

		string str("V");// = 
		str.append(data->key()->str());
		auto datapair = std::make_pair(str, (const TB::F4Balance*)data);
		m_mapData.insert(datapair);

		string key_data = "";
		key_data.append(data->key()->c_str());
		m_mapBalanceData.insert(std::pair<int, string>(idx, key_data));
		++idx;
	}
}

bool CDataManagerBalance::HasData()
{
	return m_mapData.size()>0;
}

float CDataManagerBalance::GetValue(std::string str)
{
	auto data = m_mapData.find(str);
	if (data != m_mapData.end())
	{
		auto value = (const TB::F4Balance*)data->second;
		if (!value)
			return 0.f;
		return value->value();
	}
	return 0.f;
}

void CDataManagerBalance::GetIndexName(int idx, string& str)
{
	auto data = m_mapBalanceData.find(idx);
	if (data != m_mapBalanceData.end())
	{
		str = data->second;
	}
}


