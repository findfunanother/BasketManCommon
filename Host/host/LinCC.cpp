#include "LinCC.h"
#include "Sam.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

/*
"덩크 발동 확률 증가치 = [기존 덩크 발동률] + Passive_DunkCondition_Probability_LinCC
덩크 블록 감소치 = [기존 블록 성공 확률] + Passive_BlockCondition_Probability_LinCC
증가치 = [기존 미들샷 성공률] + Passive_JumpShotProbability_LinCC
증가치 = [기존 3점샷 성공률] + Passive_JumpShotProbability_LinCC

- 게임이 시작되면 Passive_Stamina_LinCC_Max만큼의 체력 게이지를 가진다.
- 체력 게이지는 1초에 Passive_Stamina_LinCC_Reduce만큼 감소한다.
- 대쉬 액션을 할 때마다 Passive_Stamina_LinCC_Dash만큼 감소한다.

- 현재 체력 게이지가 50 % 이상일 때는 덩크 발동 확률이 Passive_DunkCondition_Probability_LinCC만큼 증가하고,
덩크 블록 확률이 Passive_BlockCondition_Probability_LinCC 만큼 감소한다.
- 현재 체력 게이지가 50 % 미만일 때는 점프샷 성공률이 Passive_JumpShotProbability_LinCC 만큼 증가한다."
*/

CLinCC::CLinCC(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveMax) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	m_CharacterPassive->valueCurr = m_pBalanceData->GetValue("VPassive_Stamina_LinCC_Max");

	m_fPlayTimeElapsed = 0.0f;

	m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_Stamina_LinCC_Max");

	m_Passive_Stamina_LinCC_Reduce = m_pBalanceData->GetValue("VPassive_Stamina_LinCC_Reduce");
	m_Passive_Stamina_LinCC_Dash = m_pBalanceData->GetValue("VPassive_Stamina_LinCC_Dash");

	m_Passive_StaminaStandard_LinCC = 0.5f; 
}

CLinCC::~CLinCC()
{
}

void CLinCC::OnPlayerPenentrate(DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_Information->id != characterSN) return;

	//DecreaseStack(m_Passive_Stamina_LinCC_Dash);
}

void CLinCC::OnPlayerDash(DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_Information->id != characterSN) return;

	DecreaseStack(m_Passive_Stamina_LinCC_Dash);
}

DHOST_TYPE_BOOL CLinCC::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CLinCC::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		DHOST_TYPE_FLOAT ratio = m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax;

		DHOST_TYPE_BOOL circleActivate = true;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

		send_data.add_idplayer(m_Information->id);
		if (ratio >= 0.5f)
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
		}
		else
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
		}

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

void CLinCC::OnPlayerShot(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SHOT_TYPE value)
{
	if (m_Information->id != characterSN) return;

	EBURST_ON_FIRE_MODE_STEP step = GetCharacterBurstOnFireModeStep();
	int stepInt = (int)step;

	DHOST_TYPE_BOOL result = false;
	switch (value)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			result = true;
		}
		break;
		default:
		{

		}
		break;
	}

	DHOST_TYPE_FLOAT rate = m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax;

	if ( (result && rate >= 0.5f) || (result && stepInt > 0.0f))
	{
		DHOST_TYPE_BOOL circleActivate = true;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)
		send_data.add_idplayer(m_Information->id);
		if (rate >= 0.5f)
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
		}
		else
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
		}
		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(m_CharacterPassive->valueMax);
		send_data.add_activate(circleActivate);
		send_data.add_effecttrigger(true);
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
}


void CLinCC::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);
	
	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		m_fPlayTimeElapsed += timeDelta;

		if (m_fPlayTimeElapsed > 1.0f)
		{
			m_fPlayTimeElapsed = 0.0f;
			DecreaseStack(m_Passive_Stamina_LinCC_Reduce);
		}
	}
}

// 스택이 쌓임 
void CLinCC::DecreaseStack(float value)
{
	DHOST_TYPE_BOOL circleActivate = true;

	if (m_CharacterPassive->valueCurr == 0.0f) return;

	m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr - value;
	//m_CharacterPassive->valueCurr = CLAMP(m_CharacterPassive->valueCurr, -1000.0f, 1000.0f);


	DHOST_TYPE_FLOAT ratio = m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax;


	if (m_CharacterPassive->valueCurr > 0.0f)
	{
		DHOST_TYPE_FLOAT max = m_CharacterPassive->valueMax;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

		send_data.add_idplayer(m_Information->id);

		if (ratio >= 0.5f)
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
		}
		else
		{
			send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
		}

		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(max);
		send_data.add_activate(circleActivate);
		send_data.add_effecttrigger(false);
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
	else
	{
		// 마지막 한번 보내기 
		m_CharacterPassive->valueCurr = 0.0f;

		DHOST_TYPE_FLOAT max = m_CharacterPassive->valueMax;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

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

		m_CharacterPassive->valueCurr = 0.0f;
	}
}


DHOST_TYPE_FLOAT CLinCC::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	EBURST_ON_FIRE_MODE_STEP step = GetCharacterBurstOnFireModeStep();
	int stepInt = (int)step;

	DHOST_TYPE_FLOAT rate = m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax;

	DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;

	DHOST_TYPE_BOOL circleActivate = true;
	//circleActivate = true;

	if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle || sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
	{
		if (rate < 0.5f || stepInt > 0)
		{
			fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_LinCC");

			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_LinCC_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_LinCC");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				fAddSuccessRate = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_LinCC_Special");
			}
			else
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_LinCC");
			}

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);
			send_data.add_idplayer(m_Information->id);

			if (rate >= 0.5f)
			{
				send_data.add_type(CHARACTER_PASSIVE_TYPE::rimAttack);
			}
			else
			{
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
			}
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



void CLinCC::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{

}
