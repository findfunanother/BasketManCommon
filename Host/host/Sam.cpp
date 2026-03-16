#include "Sam.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"


CSam::CSam(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration) : CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	if (passiveDuration <= 0.0f) passiveDuration = 10.0f;

	m_fPassiveDuration = passiveDuration;

	m_CharacterPassive->valueCurr = 0.0f;

	m_fPlayTimeElapsed = 0.0f;

	m_CharacterPassive->valueMax = SHOT_BONUS_SIGNATURE_MAX;
}

CSam::~CSam()
{
}


DHOST_TYPE_BOOL CSam::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{

	//m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;

	m_bCheckPassiveValidity = true;

	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CSam::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	// 안씀 , 모든 것을 서버에서 시작하는 최초 캐릭터 ? 
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		return false;
	}
	
	return false;
}

DHOST_TYPE_FLOAT CSam::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	if (m_CharacterPassive->valueCurr > 0.0f) // 샘패시브를 받을때 
	{
		// 샘 패시브 버프 받았다는 이펙트 보내기 
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

		send_data.add_idplayer(m_Information->id);
		send_data.add_type(CHARACTER_PASSIVE_TYPE::pass);
		send_data.add_valuecurr(1.0f);
		send_data.add_valuemax(1.0f);
		send_data.add_activate(true);
		send_data.add_effecttrigger(true); // 
		send_data.add_starttime(0.0f);
		send_data.add_hostpermit(true);
		STORE_FBPACKET(builder, message, send_data)
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CSam::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CSam::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}

void CSam::OnPlayerPass(DHOST_TYPE_CHARACTER_SN SendCharacterSN, DHOST_TYPE_CHARACTER_SN RecvCharacterSN, F4PACKET::CHARACTER_BUFF_TYPE buffType)
{

	if (m_Information->id == SendCharacterSN)
	{
		std::vector<CCharacter*> vecTeamsCharacter;
		vecTeamsCharacter.clear();
		m_pHost->GetCharacterManager()->GetMyTeamsCharacterVector(vecTeamsCharacter, m_Information->team);

		DHOST_TYPE_FLOAT buffShotRate = 0.0f;

		
		int scoreDelta = kINT32_INIT;
		if (m_Information->team == 0)
			scoreDelta = m_pHost->GetScore(1) - m_pHost->GetScore(0);
		else
			scoreDelta = m_pHost->GetScore(0) - m_pHost->GetScore(1);

		if (scoreDelta >= 7)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam3_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam3");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				buffShotRate = b + (a - b) * c;

			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam3_Special");
			}
			else
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam3");
			}

		}
		else
		if (scoreDelta >= 4)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam2_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam2");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				buffShotRate = b + (a - b) * c;

			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam2_Special");
			}
			else
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam2");
			}
		}
		else
		if (scoreDelta > 0)
		{
			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam1_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam1");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				buffShotRate = b + (a - b) * c;

			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam1_Special");
			}
			else
			{
				buffShotRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Sam1");
			}
		}

		//buffShotRate = 0.1f;
		//m_fPassiveDuration = 10.0f; // m_pBalanceData->GetValue("VPassive_Sam_Time");

		if (buffShotRate > 0.0f) // 
		{
			for (int i = 0; i < vecTeamsCharacter.size(); i++)
			{
				CCharacter* pCharacter = vecTeamsCharacter[i];
				if (pCharacter)
				{

					if (pCharacter->GetCharacterInformation()->id == m_Information->id) continue; // 나는 제외 


					if (pCharacter->GetCharacterInformation()->id == RecvCharacterSN && buffType == F4PACKET::CHARACTER_BUFF_TYPE::Pass) // 패스 받는 캐릭터만 
					{

						SCharacterBuff buff(m_Information->id, pCharacter->GetCharacterInformation()->id, // 버프가 되는 캐릭터 
							F4PACKET::CHARACTER_BUFF_TYPE::Sam_Passive,
							F4PACKET::ACTION_TYPE::action_shot,
							DHOST_TYPE_BOOL(true),
							m_fPassiveDuration,
							buffShotRate, m_Information->specialcharacterlevel);

						pCharacter->GenerateBuff(buff);

						break;
					}

					if (buffType == F4PACKET::CHARACTER_BUFF_TYPE::NicePass)
					{
						SCharacterBuff buff(m_Information->id, pCharacter->GetCharacterInformation()->id, // 버프가 되는 캐릭터 
							F4PACKET::CHARACTER_BUFF_TYPE::Sam_Passive,
							F4PACKET::ACTION_TYPE::action_shot,
							DHOST_TYPE_BOOL(true),
							m_fPassiveDuration,
							buffShotRate, m_Information->specialcharacterlevel);

						pCharacter->GenerateBuff(buff); // 나 빼고 모든 팀원 
					}
				}
			}
		}


		// 패시브 본인 적용하기 
		if (buffShotRate > 0.0f) // 
		{
			m_fPlayTimeElapsed = 0.0f;

			// 나이스 패스면 샘 본인도 버프를 받는다
			if (buffType == F4PACKET::CHARACTER_BUFF_TYPE::NicePass)
			{
				m_CharacterPassive->valueCurr = buffShotRate;

				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

				send_data.add_idplayer(m_Information->id);
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
				send_data.add_valuecurr(1.0f);
				send_data.add_valuemax(1.0f);
				send_data.add_activate(true);
				send_data.add_effecttrigger(true); // 
				send_data.add_starttime(0.0f);
				send_data.add_hostpermit(true);
				STORE_FBPACKET(builder, message, send_data)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}
			else // 아니면 이펙트만 뿌려주기 
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

				send_data.add_idplayer(m_Information->id);
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
				send_data.add_valuecurr(0.0f);
				send_data.add_valuemax(1.0f);
				send_data.add_activate(false);
				send_data.add_effecttrigger(true); // 이펙트는 터짐 
				send_data.add_starttime(0.0f);
				send_data.add_hostpermit(true);
				STORE_FBPACKET(builder, message, send_data)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}
		}
	}
}

void CSam::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{

}


void CSam::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
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
				send_data.add_type(CHARACTER_PASSIVE_TYPE::pass);
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
