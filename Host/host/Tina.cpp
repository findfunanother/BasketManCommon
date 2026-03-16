#include "Tina.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

CTina::CTina(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax)
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{

	m_CharacterPassive->valueMax = passiveValueMax; //

	if (passiveValueMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 0.25f;
	}

	m_fPlayTimeElapsed = 0.0f;
}

CTina::~CTina()
{

}

DHOST_TYPE_BOOL CTina::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
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

	// 티나 일단 이것은 0 으로 보내자. 클라이언트에서 이미 처리하므로 
	m_CharacterPassive->valueCurr = m_CharacterPassive->valueMax;

	m_bCheckPassiveValidity = true;

	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL CTina::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		return true;
	}

	return false;
}

DHOST_TYPE_FLOAT CTina::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	return 0.0f; // // 티나 일단 이것은 0 으로 보내자. 클라이언트에서 이미 처리하므로  m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CTina::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT CTina::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}


void CTina::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pData;
	auto* data = pPacket->GetData();

	std::vector<CCharacter*> vecTeamsCharacter;
	vecTeamsCharacter.clear();
	m_pHost->GetCharacterManager()->GetMyTeamsCharacterVector(vecTeamsCharacter, m_Information->team);

	if (data->effecttrigger() == false)// 참인 경우는 블록을 하거나, 슛을 한 상태, 발동 조건은 여기가 false 로  
	{
		m_fPlayTimeElapsed = 0.0f;
		if (m_Information->specialcharacterlevel == 1)
		{
			m_fPassiveDuration = m_pBalanceData->GetValue("VPassive_Tina_Time_Special1");
		}
		else
		if (m_Information->specialcharacterlevel == 2)
		{
			m_fPassiveDuration = m_pBalanceData->GetValue("VPassive_Tina_Time_Special2");
		}
		else
		{
			m_fPassiveDuration = m_pBalanceData->GetValue("VPassive_Tina_Time");
		}
	}

}


void CTina::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
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

