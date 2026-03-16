#include "Lia.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"


CLia::CLia(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	if (passiveDuration <= 0.0f) passiveDuration = 10.0f;

	m_fPassiveDuration = passiveDuration;

	if (shotBonusMax <= 0.0f) shotBonusMax = 0.1f;

	if (m_Information->specialcharacterlevel > 0)
	{
		m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lia_Special");

		if (m_CharacterPassive->valueMax <= 0.0f)
		{
			m_CharacterPassive->valueMax = 0.12f;
		}
	}
	else
	{
		m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lia");

		if (m_CharacterPassive->valueMax <= 0.0f)
		{
			m_CharacterPassive->valueMax = 0.1f;
		}
	}


	m_CharacterPassive->valueCurr = 0.0f;
	
	m_fPlayTimeElapsed = 0.0f;
}

CLia::~CLia()
{
}


DHOST_TYPE_BOOL CLia::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	// 3. 리아는 적용패시브가 최대값과 동일 
	m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;

	m_bCheckPassiveValidity = true;

	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CLia::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		return true;
	}

	return false;
}

DHOST_TYPE_FLOAT CLia::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	return m_CharacterPassive->valueCurr;	
}

DHOST_TYPE_FLOAT CLia::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CLia::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}


void CLia::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pData;
	auto* data = pPacket->GetData();

	std::vector<CCharacter*> vecTeamsCharacter;
	vecTeamsCharacter.clear();
	m_pHost->GetCharacterManager()->GetMyTeamsCharacterVector(vecTeamsCharacter, m_Information->team);

	m_fPlayTimeElapsed = 0.0f;

	for (int i = 0; i < vecTeamsCharacter.size(); i++)
	{
		CCharacter* pCharacter = vecTeamsCharacter[i];

		if (pCharacter->GetCharacterInformation()->id == m_Information->id) continue; // 나는 제외 

		DHOST_TYPE_FLOAT duration = m_pBalanceData->GetValue("VPassive_Lia_Time");

		SCharacterBuff buff(m_Information->id, pCharacter->GetCharacterInformation()->id, // 버프가 되는 캐릭터 
							 F4PACKET::CHARACTER_BUFF_TYPE::Lia_Passive,
							 F4PACKET::ACTION_TYPE::action_shot, 
			                 DHOST_TYPE_BOOL(true), 
							 duration,
							 m_CharacterPassive->valueMax, m_Information->specialcharacterlevel);

		pCharacter->GenerateBuff(buff);
	}
}


void CLia::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);

	if (m_CharacterPassive->valueCurr > 0.0f)
	{
		if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			m_fPlayTimeElapsed += timeDelta;

			if (m_fPlayTimeElapsed > m_fPassiveDuration) //   리아 본인 것 
			{
				m_CharacterPassive->valueCurr = 0.0f;
				m_fPlayTimeElapsed = 0.0f;

				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

				send_data.add_idplayer(m_Information->id);
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
				send_data.add_valuecurr(0.0f);
				send_data.add_valuemax(0.0f);
				send_data.add_activate(false);
				send_data.add_effecttrigger(false);
				send_data.add_starttime(0.0f);
				send_data.add_hostpermit(true);
				STORE_FBPACKET(builder, message, send_data)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}
		}
	}

}



