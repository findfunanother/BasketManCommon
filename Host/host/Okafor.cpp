#include "Okafor.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"



COkafor::COkafor(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveMax) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	m_CharacterPassive->valueCurr = 0.0f;

	m_fPlayTimeElapsed = 0.0f;

	m_Passive_Stack_FieldGoalCountMax = m_pBalanceData->GetValue("VPassive_Okafor_BuffOn");
	m_Passive_Stack_FieldGoalCount = 0;
	m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_Okafor_Stack_Max");
	m_Passive_Stack_TryRimAttack = m_pBalanceData->GetValue("VPassive_Okafor_Stack1");
	m_Passive_Stack_SucessRimAttack = m_pBalanceData->GetValue("VPassive_Okafor_Stack2");
	m_Passive_Stack_SucessJumpShot = m_pBalanceData->GetValue("VPassive_Okafor_Stack3");

}

COkafor::~COkafor()
{
}


DHOST_TYPE_BOOL COkafor::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL COkafor::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		DHOST_TYPE_FLOAT ratio = m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax;

		DHOST_TYPE_BOOL circleActivate = true;

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

		return false;
	}

	return false;
}



void COkafor::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);
}

void COkafor::IncreaseStack(float value)
{
	DHOST_TYPE_BOOL circleActivate = false;
	DHOST_TYPE_BOOL effectActivate = true;

	if (m_CharacterPassive->valueMax <= m_CharacterPassive->valueCurr) return;

	m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr + value;
	m_CharacterPassive->valueCurr = CLAMP(m_CharacterPassive->valueCurr, 0.0f, m_CharacterPassive->valueMax);

	if (m_CharacterPassive->valueCurr >= m_CharacterPassive->valueMax)
	{
		circleActivate = true;
	}


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(m_CharacterPassive->valueMax);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(effectActivate); // 이펙트 보내기 
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

}

// 볼샷 호출 
DHOST_TYPE_FLOAT COkafor::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	EBURST_ON_FIRE_MODE_STEP step = GetCharacterBurstOnFireModeStep();
	int stepInt = (int)step;

	DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;
	DHOST_TYPE_BOOL circleActivate = true;

	if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle) // 온파이어모드 &&  맥스가 됐으면 미드레인지 점프샷 증가 
	{
		if (stepInt > 0 && m_CharacterPassive->valueMax <= m_CharacterPassive->valueCurr)
		{
			fAddSuccessRate = m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Okafor");

			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Okafor_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Okafor");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				fAddSuccessRate = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Okafor_Special");
			}
			else
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Okafor");
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
	}
	return fAddSuccessRate;

}


void COkafor::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
}

void COkafor::ClearStack()
{
	EBURST_ON_FIRE_MODE_STEP step = GetCharacterBurstOnFireModeStep();
	int stepInt = (int)step;

	if (m_Passive_Stack_FieldGoalCount >= m_Passive_Stack_FieldGoalCountMax)
	{
		m_Passive_Stack_FieldGoalCount = 0;
		m_CharacterPassive->valueCurr = 0;

		DHOST_TYPE_BOOL circleActivate = false;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)
		send_data.add_idplayer(m_Information->id);
		send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(m_CharacterPassive->valueMax);
		send_data.add_activate(circleActivate);
		send_data.add_effecttrigger(circleActivate);
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		return;
	}
}

void COkafor::OnGoalResult(DHOST_TYPE_CHARACTER_SN CharacterSN, F4PACKET::SHOT_TYPE value, DHOST_TYPE_BOOL result)
{
	if (m_Information->id != CharacterSN) return;

	if (result)
	{
		if (value == F4PACKET::SHOT_TYPE::shotType_middle || value == F4PACKET::SHOT_TYPE::shotType_threePoint)
		{
			IncreaseStack(m_Passive_Stack_SucessJumpShot);
		}
		else
		if (m_pHost->CheckShotTypeRimAttack(value))
		{
			IncreaseStack(m_Passive_Stack_TryRimAttack);
		}
	}

	ClearStack();
}

// 볼샷 보다 먼저 호출, 제일 먼저 호출됨 
void COkafor::OnPlayerShot(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SHOT_TYPE value)
{
	if (m_Information->id != characterSN) return;

	if (m_CharacterPassive->valueMax <= m_CharacterPassive->valueCurr)
	{
		m_Passive_Stack_FieldGoalCount++;
	}

	if (m_pHost->CheckShotTypeRimAttack(value))
	{
		IncreaseStack(m_Passive_Stack_TryRimAttack);
	}
}

