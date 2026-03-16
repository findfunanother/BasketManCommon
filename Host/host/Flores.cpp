#include "Flores.h"
#include "Sam.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"


CFlores::CFlores(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	if (passiveDuration <= 0.0f) passiveDuration = 10.0f;

	m_fPassiveDuration = passiveDuration;

	m_CharacterPassive->valueCurr = 0.0f;

	m_fPlayTimeElapsed = 0.0f;

	m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_Flores_Level2");

	m_PlayedCeremony = false;
}

CFlores::~CFlores()
{
}


DHOST_TYPE_BOOL CFlores::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CFlores::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		return false;
	}

	return false;
}

void CFlores::OnPlayerShotHindrance(DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_Information->id != characterSN) return;

	DHOST_TYPE_BOOL circleActivate = false;
	DHOST_TYPE_BOOL effectTrigger = false;

	if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level2"))
	{
		circleActivate = true;
		effectTrigger = true;
	}
	else
	if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level1"))
	{
		circleActivate = true;
		effectTrigger = true;
	}
	else
	{
		circleActivate = false;
		effectTrigger = false;
	}

	if (circleActivate)
	{
		DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Flores_Level2");

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

		send_data.add_idplayer(m_Information->id);
		send_data.add_type(CHARACTER_PASSIVE_TYPE::defensePerimeter);
		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(max);
		send_data.add_activate(circleActivate);
		send_data.add_effecttrigger(effectTrigger);
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
}

void CFlores::OnPlayerBlock(DHOST_TYPE_CHARACTER_SN blockCharacterSN, DHOST_TYPE_CHARACTER_SN blockedCharacterSN, DHOST_TYPE_BOOL bSuccess)
{
	if (m_Information->id != blockCharacterSN) return;

	if (bSuccess)
	{
		m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Flores_Stack1");

		if (m_CharacterPassive->valueCurr > m_CharacterPassive->valueMax)
		{
			m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;
		}

		ResultBallShotHindrance();
	}
}

void CFlores::OnGoalFailed(std::vector<DHOST_TYPE_CHARACTER_SN> vBallShotHindrance)
{
	// 내가 슛 방해를 했다면 
	for (int i = 0; i < vBallShotHindrance.size(); ++i)
	{
		if (vBallShotHindrance[i] == m_Information->id)
		{
			m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Flores_Stack1");

			if (m_CharacterPassive->valueCurr > m_CharacterPassive->valueMax)
			{
				m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;
			}

			ResultBallShotHindrance();

			break;
		}
	}
}

void CFlores::OnTurnOver(DHOST_TYPE_CHARACTER_SN turnoverCauser, DHOST_TYPE_CHARACTER_SN turnoverTarget)
{
	if (m_Information->id != turnoverCauser) return;

	m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Flores_Stack2");

	if (m_CharacterPassive->valueCurr > m_CharacterPassive->valueMax)
	{
		m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;
	}

	ResultBallShotHindrance();
}

// 스택이 쌓임 
void CFlores::ResultBallShotHindrance()
{
	DHOST_TYPE_BOOL circleActivate = false;

	if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level2"))
	{
		circleActivate = true;
	}
	else
	if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level1"))
	{
		circleActivate = true;
	}
	else
	{
		circleActivate = false;
	}

	DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Flores_Level2");

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::defensePerimeter);
	send_data.add_valuecurr(m_CharacterPassive->valueCurr);
	send_data.add_valuemax(max);
	send_data.add_activate(circleActivate);
	send_data.add_effecttrigger(false);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CFlores::OnCharacterPlayerCoupleCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerCoupleCeremony_data>*)pData;
	auto* data = pPacket->GetData();

	DHOST_TYPE_CHARACTER_SN target = 0;
	DHOST_TYPE_CHARACTER_SN id1 = data->id1();
	DHOST_TYPE_CHARACTER_SN id2 = data->id2();

	if (id1 != m_Information->id && id2 != m_Information->id) return;

	if (id1 == m_Information->id)
	{
		target = id2;
	}

	if (id2 == m_Information->id)
	{
		target = id1;
	}

	if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level1"))
	{
		m_CharacterPassive->valueCurr = m_CharacterPassive->valueCurr - m_pBalanceData->GetValue("VPassive_Flores_Level1");


		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data)

		send_data.add_idplayer(m_Information->id);
		send_data.add_type(CHARACTER_PASSIVE_TYPE::defensePerimeter);
		send_data.add_valuecurr(m_CharacterPassive->valueCurr);
		send_data.add_valuemax(m_CharacterPassive->valueMax);

				
		if (m_CharacterPassive->valueCurr >= m_pBalanceData->GetValue("VPassive_Flores_Level1"))
		{
			send_data.add_activate(true);
		}
		else
		{
			send_data.add_activate(false);
		}

		send_data.add_effecttrigger(true); // 이펙트는 터짐 
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		m_PlayedCeremony = true;

		ResultTeamShotRate(target);
	}
	
}

// 해당 캐릭터 슛 확률 증가 
void CFlores::ResultTeamShotRate(DHOST_TYPE_CHARACTER_SN buffCharacterSN)
{
	CCharacter*  pCharacter = m_pHost->GetCharacterManager()->GetCharacter(buffCharacterSN);
	if (pCharacter)
	{
		DHOST_TYPE_FLOAT buffShotRate = 0.0f;

		if (m_Information->specialcharacterlevel == 1)
		{
			DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_Flores_Buff_Special");
			DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_Flores_Buff");
			DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
			buffShotRate = b + (a - b) * c;

		}
		else
		if (m_Information->specialcharacterlevel == 2)
		{
			buffShotRate = m_pBalanceData->GetValue("VPassive_Flores_Buff_Special");
		}
		else
		{
			buffShotRate = m_pBalanceData->GetValue("VPassive_Flores_Buff");
		}

		DHOST_TYPE_FLOAT duration = m_pBalanceData->GetValue("VPassive_Flores_Buff_Time");

		SCharacterBuff buff(m_Information->id, pCharacter->GetCharacterInformation()->id, // 버프가 되는 캐릭터 
			F4PACKET::CHARACTER_BUFF_TYPE::Flores_Passive, // 누적이되려면  캐릭터 별로 타입을 넣어야 함 
			F4PACKET::ACTION_TYPE::action_shot,
			DHOST_TYPE_BOOL(true),
			duration,
			buffShotRate, m_Information->specialcharacterlevel);

		pCharacter->GenerateBuff(buff);
	}
}

void CFlores::OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime)
{
	if (recordType == ECHARACTER_RECORD_TYPE::TRY_3POINT || recordType == ECHARACTER_RECORD_TYPE::TRY_2POINT)
	{
		m_PlayedCeremony = false;
	}
}


void CFlores::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{

}
