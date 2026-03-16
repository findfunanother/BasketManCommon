#include "Kent.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"



CKent::CKent(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax)
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{

	m_CharacterPassive->valueMax = passiveValueMax; //
	m_CharacterPassive->valueCurr = 0.0f;

	if (passiveValueMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 50;
	}

	m_fPlayTimeElapsed = 0.0f;
}


CKent::~CKent()
{

}


DHOST_TYPE_BOOL  CKent::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pInfo;
	auto* data = pPacket->GetData();

	DHOST_TYPE_BOOL circleActivate = true;

	if (m_Information->specialcharacterlevel == 1)
	{
		DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Special");
		DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent");
		DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
		float successRateDelta = b + (a - b) * c;


		DHOST_TYPE_FLOAT e = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Max_Special");
		DHOST_TYPE_FLOAT f = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Max");
		DHOST_TYPE_FLOAT g = m_pBalanceData->GetValue("VPassive_Special_2");
		float successRateMax = f + (e - f) * g;

		float newSuccessRate = successRateMax < m_CharacterPassive->valueCurr + successRateDelta ? successRateMax : m_CharacterPassive->valueCurr + successRateDelta;
		m_CharacterPassive->valueCurr = newSuccessRate;
		m_CharacterPassive->valueMax = successRateMax;
	}
	else
	if (m_Information->specialcharacterlevel == 2)
	{
		float successRateDelta = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Special");
		float successRateMax = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Max_Special");
		float newSuccessRate = successRateMax < m_CharacterPassive->valueCurr + successRateDelta ? successRateMax : m_CharacterPassive->valueCurr + successRateDelta;

		m_CharacterPassive->valueCurr = newSuccessRate;
		m_CharacterPassive->valueMax = successRateMax;
	}
	else
	{
		float successRateDelta = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent");
		float successRateMax = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Max");
		float newSuccessRate = successRateMax < m_CharacterPassive->valueCurr + successRateDelta ? successRateMax : m_CharacterPassive->valueCurr + successRateDelta;

		m_CharacterPassive->valueCurr = newSuccessRate;
		m_CharacterPassive->valueMax = successRateMax;
	}

	// 클라이언트에서 보내주기는 하지만, 서버에서 다시 계산한 값을 보내주자 

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::defensePost);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(m_CharacterPassive->valueMax);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(circleActivate);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return false; // 외부에서 브로드 캐스팅 안함 

}

void CKent::OnCharacterPlayerCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerCeremony_data>*)pData;
	auto* data = pPacket->GetData();
}

void CKent::OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime)
{
	if (CharacterSN == m_Information->id)
	{
		if (recordType == ECHARACTER_RECORD_TYPE::BLOCK)
		{
			DHOST_TYPE_BOOL circleActivate = false;
			circleActivate = false;

			float rateDelta = 0.0f;

			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Penalty_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Penalty");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				rateDelta = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				rateDelta = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Penalty_Special");
			}
			else
			{
				rateDelta = m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Penalty");
			}

			m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr - rateDelta;
			CLAMP(m_CharacterPassive->valueCurr, 0.0f, 1000.0f);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

			send_data.add_idplayer(m_Information->id);
			send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
			send_data.add_valuecurr(m_CharacterPassive->valueCurr);
			send_data.add_valuemax(m_CharacterPassive->valueMax);
			send_data.add_activate(circleActivate);
			send_data.add_effecttrigger(circleActivate);
			send_data.add_starttime(0.0f);
			send_data.add_hostpermit(true);
			STORE_FBPACKET(builder, message, send_data)
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}
	}
}

DHOST_TYPE_FLOAT CKent::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;
	return fAddSuccessRate;
}


void CKent::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	// 할게 없음 
}


void CKent::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);
}
