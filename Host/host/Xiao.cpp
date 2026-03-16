#include "Xiao.h"

#include "Thompson.h"

#include "Lucky.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

CXiao::CXiao(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	value1 = 0.0f;
	value2 = 0.0f;
	value3 = 0.0f;

	DHOST_TYPE_FLOAT passiveValue = 0.0f;
	if (m_Information->specialcharacterlevel == 1)
	{
		float a = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Xiao_Special");
		float b = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Xiao");
		float c = m_pBalanceData->GetValue("VPassive_Special_2");
		passiveValue = b + (a - b) * c;
	}
	else
	if (m_Information->specialcharacterlevel == 2)
	{
		passiveValue = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Xiao_Special");
	}
	else
	{
		passiveValue = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Xiao");
	}

	value1 = passiveValue;
}

CXiao::~CXiao()
{
}

DHOST_TYPE_BOOL CXiao::InitialPassiveDataSet(const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo)
{
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassiveDataSet, message, send_data);

	send_data.add_idplayer(m_Information->id);
	send_data.add_value1(value1);
	send_data.add_value2(value2);
	send_data.add_value3(value3);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}





