
#include "Niky.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"


CNiky::CNiky(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	if (passiveDuration <= 0.0f)
	{
		string position_log = "[HACK_CHECK] VPassive_Niky_Time table is none";
	    m_pHost->ToLog(position_log.c_str());
		passiveDuration = 10.0f;
	}

	m_fPassiveDuration = passiveDuration;

	if (shotBonusMax <= 0.0f) shotBonusMax = 0.1f;

	if (m_Information->specialcharacterlevel > 0)
	{
		if (m_Information->specialcharacterlevel == 1)
		{
			m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Niky_Special"); 
		}
		else
		{
			m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Niky_Special") * m_pBalanceData->GetValue("VPassive_Special_2");
		}

		if (m_CharacterPassive->valueMax <= 0.0f)
		{
			m_CharacterPassive->valueMax = 0.12f;
		}
	}
	else
	{
		m_CharacterPassive->valueMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Niky");

		if (m_CharacterPassive->valueMax <= 0.0f)
		{
			m_CharacterPassive->valueMax = 0.1f;
		}
	}


	m_CharacterPassive->valueCurr = 0.0f;

	m_fPlayTimeElapsed = 0.0f;
}

CNiky::~CNiky()
{
}


DHOST_TYPE_BOOL CNiky::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	/*
	// 1. 핵체크, 클라이언트에서 오는 맥스값이 테이블값보다 클 수 없다
	if (m_CharacterPassive->valueMax < passiveMax)
	{
		m_bCheckPassiveValidity = false;
		return m_bCheckPassiveValidity;
	}

	// 2. 핵체크
	if (m_CharacterPassive->valueCurr < passiveCurrent)
	{
		m_bCheckPassiveValidity = false;
		return m_bCheckPassiveValidity;
	}
	*/

	m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;

	m_bCheckPassiveValidity = true;

	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CNiky::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		return true;
	}

	return false;
}

DHOST_TYPE_FLOAT CNiky::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle)
	{
		return m_CharacterPassive->valueCurr;
	}
	else
	if (sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
	{
		DHOST_TYPE_FLOAT re = 0.0f;

		if (m_Information->specialcharacterlevel > 0)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				re = m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Niky_Special");
			}
			else
			{
				re = m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Niky_Special") * m_pBalanceData->GetValue("VPassive_Special_2");
			}
		}
		else
		{
			re = m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Niky");
		}

		return re;
	}
	
	return 0.0f;
}

DHOST_TYPE_FLOAT CNiky::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CNiky::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}


void CNiky::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
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

		if (pCharacter->GetCharacterInformation()->id == m_Information->id) continue; // 나는 제외 , 나는 패시브로 따로 받음, 기존 캐릭터오 구조를 맞추다 보니. 

		DHOST_TYPE_FLOAT duration = m_pBalanceData->GetValue("VPassive_Niky_Time");

		if (duration <= 0.0f )
		{
			string position_log = "[HACK_CHECK] VPassive_Niky_Time table is none";
			m_pHost->ToLog(position_log.c_str());

			duration = 10.0f;

		}

		SCharacterBuff buff(m_Information->id, pCharacter->GetCharacterInformation()->id, // 버프가 되는 캐릭터 
			F4PACKET::CHARACTER_BUFF_TYPE::Niky_Passive,
			F4PACKET::ACTION_TYPE::action_shot,
			DHOST_TYPE_BOOL(true),
			duration,
			m_CharacterPassive->valueMax, m_Information->specialcharacterlevel);

		pCharacter->GenerateBuff(buff);
	}
}


void CNiky::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);

	if (m_CharacterPassive->valueCurr > 0.0f)
	{
		if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			m_fPlayTimeElapsed += timeDelta;

			if (m_fPlayTimeElapsed > m_fPassiveDuration)
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

