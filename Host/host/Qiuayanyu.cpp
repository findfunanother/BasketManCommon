#include "Qiuayanyu.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

CQiuayanyu::CQiuayanyu(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax)
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{

	m_CharacterPassive->valueMax = passiveValueMax; //

	if (passiveValueMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 0.07f;
	}


	m_fPlayTimeElapsed = 0.0f;
	m_fPassiveDuration = m_pBalanceData->GetValue("VPassive_Qiuyanyu_Time");
}

CQiuayanyu::~CQiuayanyu()
{

}

DHOST_TYPE_BOOL  CQiuayanyu::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{

	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pInfo;
	auto* data = pPacket->GetData();


	// 치유앤유는 모두 서버에서 처리가 가능함 

	return false; // 브로드 캐스팅 안함, 여기서 처리 완료 
}


void CQiuayanyu::OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime)
{
	if (CharacterSN == m_Information->id )
	{
		if (recordType == ECHARACTER_RECORD_TYPE::ASSIST)
		{
			DHOST_TYPE_BOOL circleActivate = false;
			circleActivate = true;


			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

			send_data.add_idplayer(m_Information->id);
			send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
			send_data.add_valuecurr(1.0f);
			send_data.add_valuemax(1.0f);
			send_data.add_activate(circleActivate);
			send_data.add_effecttrigger(circleActivate);
			send_data.add_starttime(0.0f);
			send_data.add_hostpermit(true);
			STORE_FBPACKET(builder, message, send_data)
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);


			//////////////////////
			//m_CharacterPassive->valueCurr = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu");
			DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;

			if (m_Information->specialcharacterlevel == 1)
			{
				DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu_Special");
				DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu");
				DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
				fAddSuccessRate = b + (a - b) * c;
			}
			else
			if (m_Information->specialcharacterlevel == 2)
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu_Special");
			}
			else
			{
				fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu");
			}

			m_CharacterPassive->valueCurr = fAddSuccessRate;

		}
		else
		if (recordType == ECHARACTER_RECORD_TYPE::TRY_2POINT || recordType == ECHARACTER_RECORD_TYPE::TRY_3POINT)
		{
			if (m_CharacterPassive->valueCurr > 0.0f)
			{
				m_CharacterPassive->valueCurr = 0.0f;

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

DHOST_TYPE_FLOAT CQiuayanyu::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	return m_CharacterPassive->valueCurr;
}


void CQiuayanyu::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	// 할게 없음 
}


void CQiuayanyu::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
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
				send_data.add_valuemax(1.0f);
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