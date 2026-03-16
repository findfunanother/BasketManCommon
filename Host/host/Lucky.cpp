#include "Lucky.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

CLucky::CLucky(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT fDeltaJumpshot2Value, DHOST_TYPE_FLOAT fDeltaJumpshot3Value) 
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	DHOST_TYPE_FLOAT a = kFLOAT_INIT;
	DHOST_TYPE_FLOAT b = kFLOAT_INIT;
	DHOST_TYPE_FLOAT c = kFLOAT_INIT;

	if (m_Information->specialcharacterlevel == 1)
	{
		a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_Max_Special");
		b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_Max");
		c = m_pBalanceData->GetValue("VPassive_Special_2");
		float fJumpShotProbability_Lucky_Max = b + (a - b) * c;
		m_CharacterPassive->valueMax = fJumpShotProbability_Lucky_Max; // 럭키의 패시브 보너스 맥스 값 

		a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_2_Special");
		b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_2");
		c = m_pBalanceData->GetValue("VPassive_Special_2");
		float fDeltaJumpshot2Value = b + (a - b) * c;
		m_fDeltaJumpshot2Value = fDeltaJumpshot2Value;

		a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_3_Special");
		b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_3");
		c = m_pBalanceData->GetValue("VPassive_Special_2");
		float fDeltaJumpshot3Value = b + (a - b) * c;
		m_fDeltaJumpshot3Value = fDeltaJumpshot3Value;
	}
	else
	if (m_Information->specialcharacterlevel == 2)
	{
		m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_Max_Special");
		m_fDeltaJumpshot2Value = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_2_Special");
		m_fDeltaJumpshot3Value = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_3_Special");
	}
	else
	{
		m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_Max");
		m_fDeltaJumpshot2Value = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_2");
		m_fDeltaJumpshot3Value = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_3");
	}

	m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;
	m_bCheckPassiveValidity = false;
}

CLucky::~CLucky()
{
}


DHOST_TYPE_BOOL CLucky::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	/*string position_log = "[HACK_CHECK] Mina SetPacketCharacterPassive";
	m_pHost->ToLog(position_log.c_str());*/

	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		DHOST_TYPE_BOOL circleActivate = false;

		if (m_CharacterPassive->valueCurr > 0.0f)
		{
			circleActivate = true;
		}
		else
		{
			m_CharacterPassive->valueCurr = 0.0f;
		}

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

	return false;
}

DHOST_TYPE_BOOL CLucky::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity;
}


DHOST_TYPE_FLOAT CLucky::GetBonusShotPassiveCurrent() 
{ 
	return m_CharacterPassive->valueCurr;
}  

DHOST_TYPE_FLOAT CLucky::GetBonusShotPassiveMax()
{ 
	return m_CharacterPassive->valueMax;
}  

void CLucky::EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType)
{
	m_bCheckPassiveValidity = false;

	if (shotType == F4PACKET::SHOT_TYPE::shotType_middle)
	{
		float stackMax = m_CharacterPassive->valueMax;

		//stackMax = stackMax <= 0 ? FLT_EPSILON : stackMax; // 1.192092896e-07F
		stackMax = stackMax <= 0 ? 1.192092896e-07F : stackMax; // 1.192092896e-07F

		float stackNew = m_CharacterPassive->valueCurr + m_fDeltaJumpshot2Value;

		stackNew = stackNew < 1e-7f ? 0 : stackNew;                 // 음수 및 부동소수점 오차 보정
		stackNew = stackNew > stackMax ? stackMax : stackNew;

		m_CharacterPassive->valueCurr = stackNew;

	}
	else
	if (shotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
	{
		float stackMax = m_CharacterPassive->valueMax;

		//stackMax = stackMax <= 0 ? FLT_EPSILON : stackMax;
		stackMax = stackMax <= 0 ? 1.192092896e-07F : stackMax; // 1.192092896e-07F

		float stackNew = m_CharacterPassive->valueCurr + m_fDeltaJumpshot3Value;

		stackNew = stackNew < 1e-7f ? 0 : stackNew;                 // 음수 및 부동소수점 오차 보정
		stackNew = stackNew > stackMax ? stackMax : stackNew;

		m_CharacterPassive->valueCurr = stackNew;
	}


	DHOST_TYPE_BOOL circleActivate = false;

	if (m_CharacterPassive->valueCurr > 0.0f)
	{
		circleActivate = true;
	}
	else
	{
		m_CharacterPassive->valueCurr = 0.0f;
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(m_CharacterPassive->valueMax);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(false);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}


DHOST_TYPE_BOOL  CLucky::IsValidatePassiveValue(DHOST_TYPE_FLOAT valueCurr)
{
	if( m_CharacterPassive->valueCurr <= 0.0f ) // 패시브를 모두 소진해도 인정
	{ 
		return true;
	}

	if (m_CharacterPassive->valueCurr < valueCurr)
	{
		return false;  // 핵킹 의심 
	}

	return m_bCheckPassiveValidity;
}


