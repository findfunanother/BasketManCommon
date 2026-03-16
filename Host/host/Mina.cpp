#include "Mina.h"
#include "Host.h"
#include "DataManagerBalance.h"
#include "Character.h"

CMina::CMina(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax) :
	CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	/*
	m_CharacterPassive->valueMax = shotBonusMax; //

	if (shotBonusMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 0.20f;
	}
	*/
}

CMina::~CMina()
{
}

DHOST_TYPE_BOOL CMina::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	//string position_log = "[HACK_CHECK] Mina SetPacketCharacterPassive";
	//m_pHost->ToLog(position_log.c_str());

	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Mina_Max");
		DHOST_TYPE_FLOAT minaStack1 = m_pBalanceData->GetValue("VPassive_Mina_Stack1");
		DHOST_TYPE_FLOAT minaStack2 = m_pBalanceData->GetValue("VPassive_Mina_Stack2");
		DHOST_TYPE_BOOL circleActivate = false;

		if (max <= m_CharacterPassive->valueCurr) // 이미 꽉 찾으면 
		{
			return false;
		}


		m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr + 1.0f;

		if (minaStack1 <= m_CharacterPassive->valueCurr)
		{
			circleActivate = true;
		}


		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

		send_data.add_idplayer(m_Information->id);
		send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(max);
		send_data.add_activate(circleActivate);
		send_data.add_effecttrigger(false);
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return false;
}

DHOST_TYPE_BOOL CMina::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{

	//m_CharacterPassive->valueCurr = passiveCurrent;
	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity; // 패킷을 보르드 캐스팅을 안함 
}

void CMina::EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType)
{

}

DHOST_TYPE_FLOAT CMina::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CMina::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}

void CMina::OnTurnOver(DHOST_TYPE_CHARACTER_SN turnoverCauser, DHOST_TYPE_CHARACTER_SN turnoverTarget)
{
	if (turnoverTarget != m_Information->id) return;

	float stackDelta = m_pBalanceData->GetValue("VPassive_Mina_Penalty");

	m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr + stackDelta;

	if (m_CharacterPassive->valueCurr <= 0.0f)
	{
		m_CharacterPassive->valueCurr = 0.0f;
	}

	DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Mina_Max");

	DHOST_TYPE_FLOAT minaStack1 = m_pBalanceData->GetValue("VPassive_Mina_Stack1");

	DHOST_TYPE_BOOL circleActivate = false;

	if (m_CharacterPassive->valueCurr >= minaStack1)
	{
		circleActivate = true;
	}


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(max);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(false);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}


// 1. 아래 함수가 이 함수보다 SetPacketCharacterPassive  먼저 호출됨
// 2. 슛 성공율은 스택이 모두 찬 이후부터 적용. 슛 쏜 다음 부터 
DHOST_TYPE_FLOAT CMina::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	//string position_log = "[HACK_CHECK] Mina GetPassiveBonus";
	//m_pHost->ToLog(position_log.c_str());


	//m_CharacterPassive->valueCurr = 6.0f;

	DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;
	DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Mina_Max");
	DHOST_TYPE_FLOAT minaStack1 = m_pBalanceData->GetValue("VPassive_Mina_Stack1");
	DHOST_TYPE_FLOAT minaStack2 = m_pBalanceData->GetValue("VPassive_Mina_Stack2");

	if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle || sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
	{
		if (m_CharacterPassive->valueCurr >= minaStack2)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina2_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina2");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				fAddSuccessRate = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina2_Special");
			}
			else
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina2");
			}

		}
		else
		if (m_CharacterPassive->valueCurr >= minaStack1)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a  = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina1_Special");
				DHOST_TYPE_FLOAT b  = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina1");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				fAddSuccessRate = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina1_Special");
			}
			else
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Mina1");
			}
		}
	}

	DHOST_TYPE_BOOL circleActivate = false;

	if (m_CharacterPassive->valueCurr >= minaStack1)
	{
		circleActivate = true;
	}


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);
	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(max);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(circleActivate);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return fAddSuccessRate;

}