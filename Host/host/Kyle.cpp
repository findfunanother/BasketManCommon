#include "Kyle.h"
#include "Character.h"
#include "Host.h"
#include "DataManagerBalance.h"

/*

"증가치 = [기존 슈팅 성공률] + Passive_JumpShotProbability_Kyle
감소치 = [기존 블록 성공 확률] + Passive_BlockCondition_Probability_Kyle

- 관중을 환호하게 하는 액션을 하면 스택이 정해진 밸런스만큼 증가하게 된다.

1) 세레모니 1회 실행 Passive_Kyle_Stack_Ceremony
2) 동점을 만드는 득점 Passive_Kyle_Stack_Score_Tie
3) 역전을 만드는 득점  Passive_Kyle_Stack_Score_Turn
4) 종료 시간 1분 이하이고 5점차 이하일 때 득점  Passive_Kyle_Stack_Score_Clutch

- 스택이 Passive_Kyle_Stack_Max만큼 쌓이면,
모든 슈팅 성공률이 Passive_JumpShotProbability_Kyle 만큼 증가한다.
- 또한, 모든 슈팅을 블록 당할 확률이 Passive_BlockCondition_Probability_Kyle 만큼 감소한다."
*/

/*
"증가치 = [기존 슈팅 성공률] + Passive_JumpShotProbability_Kyle_Special
감소치 = [기존 블록 성공 확률] + Passive_BlockCondition_Probability_Kyle_Special

- 관중을 환호하게 하는 액션을 하면 스택이 정해진 밸런스만큼 증가하게 된다.

1) 세레모니 1회 실행 Passive_Kyle_Stack_Ceremony
2) 동점을 만드는 득점 Passive_Kyle_Stack_Score_Tie
3) 역전을 만드는 득점  Passive_Kyle_Stack_Score_Turn
4) 종료 시간 1분 이하이고 5점차 이하일 때 득점  Passive_Kyle_Stack_Score_Clutch


- 스택이 Passive_Kyle_Stack_Max만큼 쌓이면,
모든 슈팅 성공률이 Passive_JumpShotProbability_Kyle_Special 만큼 증가한다.
- 또한, 모든 슈팅을 블록 당할 확률이 Passive_BlockCondition_Probability_Kyle_Special 만큼 감소한다."

*/

CKyle::CKyle(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax)
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{

	m_CharacterPassive->valueMax = passiveValueMax; //

	if (passiveValueMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 50;
	}

	m_fPlayTimeElapsed = 0.0f;

	m_currentScoreStatus = EGAME_SCORE_STATUS::NONE;
	m_superStarMode = false;
	m_PlayedCeremony = false;


	//m_CharacterPassive->valueMax = 20;
}


CKyle::~CKyle()
{

}

DHOST_TYPE_BOOL  CKyle::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{

	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPassive_data>*)pInfo;
	auto* data = pPacket->GetData();


	// 치유앤유는 모두 서버에서 처리가 가능함 

	return false; // 브로드 캐스팅 안함, 여기서 처리 완료 
}

void CKyle::OnCharacterPlayerCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time) 
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerCeremony_data>*)pData;
	auto* data = pPacket->GetData();

	CCharacter::OnCharacterPlayerCeremony(pData, time);

	if (data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_custom1
		|| data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_custom2
		|| data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_custom3
		|| data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_couple)
	{
		if (!m_superStarMode)
		{
			if (!m_PlayedCeremony)
			{
				m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Kyle_Stack_Ceremony");

				if (m_CharacterPassive->valueCurr >= m_CharacterPassive->valueMax /*m_pBalanceData->GetValue("VPassive_Kyle_Stack_Max")*/)
				{
					m_superStarMode = true;
				}

				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

				send_data.add_idplayer(m_Information->id);
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
				send_data.add_valuecurr(m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax);
				send_data.add_valuemax(1.0f);

				if (m_superStarMode)
					send_data.add_activate(true);
				else
					send_data.add_activate(false);

				send_data.add_effecttrigger(true); // 이펙트는 터짐 
				send_data.add_starttime(0.0f);
				send_data.add_hostpermit(true);
				STORE_FBPACKET(builder, message, send_data)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

				m_PlayedCeremony = true;
			}
		}
	}
}

void CKyle::OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime)
{
	if (!m_superStarMode) //CharacterSN == m_Information->id)
	{
		int addedScore = 0;

		if (recordType == ECHARACTER_RECORD_TYPE::TRY_3POINT || recordType == ECHARACTER_RECORD_TYPE::TRY_2POINT)
		{
			m_PlayedCeremony = false;
		}
		else
		if (recordType == ECHARACTER_RECORD_TYPE::SUC_2POINT || recordType == ECHARACTER_RECORD_TYPE::SUC_3POINT)
		{
			if (recordType == ECHARACTER_RECORD_TYPE::SUC_2POINT)
				addedScore = 2;
			else 
				addedScore = 3;

			DHOST_TYPE_BOOL sendStatus = false;
			DHOST_TYPE_BOOL applyStatus = false;

			CCharacter* pActionOwner = this;


			std::vector<DHOST_TYPE_INT32> teamScores = m_pHost->GetVecScores();


			DHOST_TYPE_INT32 ActionOwnerTeamScore = pActionOwner->GetCharacterInformation()->team < teamScores.size() ? teamScores[pActionOwner->GetCharacterInformation()->team] : 0;
			DHOST_TYPE_INT32 OppornentTeamScore = 0;

			if (teamScores.size() > 1)
			{
				OppornentTeamScore = teamScores[m_pHost->GetCharacterManager()->GetIndexOpponentTeam(pActionOwner->GetCharacterInformation()->team)];
			}

			// 동점에서 점수를 넣는 것은 역전이 아니다( 양팀 모두 ) 

			if (ActionOwnerTeamScore > OppornentTeamScore)
			{
				if (m_currentScoreStatus == EGAME_SCORE_STATUS::LOOSING_IN_SCORE) // 역전 
				{
					if (CharacterSN == m_Information->id)
					{
						// 스택 증가 
						m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Kyle_Stack_Score_Turn");
						sendStatus = true;

					}
				}

				m_currentScoreStatus = EGAME_SCORE_STATUS::WINNING_IN_SCORE;
			}
			else if (ActionOwnerTeamScore < OppornentTeamScore)
			{
				
				m_currentScoreStatus = EGAME_SCORE_STATUS::LOOSING_IN_SCORE;
			}
			else
			{
				if (m_currentScoreStatus == EGAME_SCORE_STATUS::LOOSING_IN_SCORE) // 동점 
				{
					if (CharacterSN == m_Information->id)
					{
						// 스택 증가 
						m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Kyle_Stack_Score_Tie");
						sendStatus = true;
					}

				}

				m_currentScoreStatus = EGAME_SCORE_STATUS::TIE_IN_SCORE;
			}


			// 종료시간 1분이하 5점차 이하일때의 득점  
			float gameTime = m_pHost->GetGameTime();
			DHOST_TYPE_INT32 scoreGap = abs(ActionOwnerTeamScore - addedScore - OppornentTeamScore) ;
			if (gameTime <= m_pBalanceData->GetValue("VPassive_Kyle_Time") && scoreGap <= 5)
			{
				if (CharacterSN == m_Information->id)
				{
					m_CharacterPassive->valueCurr += m_pBalanceData->GetValue("VPassive_Kyle_Stack_Score_Clutch");
					sendStatus = true;
				}
			}


			if (m_CharacterPassive->valueCurr >= m_CharacterPassive->valueMax/*m_pBalanceData->GetValue("VPassive_Kyle_Stack_Max")*/)
			{
				m_superStarMode = true;
			}


			if (sendStatus)
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

				send_data.add_idplayer(m_Information->id);
				send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
				send_data.add_valuecurr(m_CharacterPassive->valueCurr / m_CharacterPassive->valueMax);
				send_data.add_valuemax(1.0f);

				if(m_superStarMode)
					send_data.add_activate(true);
				else
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

DHOST_TYPE_FLOAT CKyle::GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{

	DHOST_TYPE_FLOAT fAddSuccessRate = 0.0f;
	
	DHOST_TYPE_BOOL circleActivate = false;

	if ( m_superStarMode )
	{
		DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VPassive_Mina_Max");

		circleActivate = true;

		fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Kyle");


		if (m_Information->specialcharacterlevel == 1)
		{
			DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Kyle_Special");
			DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Kyle");
			DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
			fAddSuccessRate = b + (a - b) * c;
		}
		else
		if (m_Information->specialcharacterlevel == 2)
		{
			fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Kyle_Special");
		}
		else
		{
			fAddSuccessRate = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Kyle");
		}

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
	}

	return fAddSuccessRate;
}


void CKyle::OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	// 할게 없음 
}


void CKyle::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacter::UpdateCharacter(timeDelta);
}