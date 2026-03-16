#include "State_Play.h"
#include "BallController.h"

CState_Play::CState_Play(CHost* host) : CState(host)
{
	m_EnableStop = true;
	m_LastSendGameTime = false;

	m_CheckScore = m_pHost->CheckOption(HOST_OPTION_NO_CHECK_GOAL);
	m_CheckGameTime = m_pHost->CheckOption(HOST_OPTION_NO_CHECK_TIME);
	m_CheckShotClock = m_pHost->CheckOption(HOST_OPTION_NO_CHECK_SHOTCLOCK);

	m_PrevGameTime = kINT32_INIT;
	m_PrevShotClock = kINT32_INIT;
}

void CState_Play::OnEnter(void* pData)
{
	m_EnableStop = true;
	m_LastSendGameTime = false;

	m_PrevGameTime = static_cast<int>(m_pHost->GetGameTime());
	m_PrevShotClock = static_cast<int>(m_pHost->GetShotClock());

	// 게임 시작 알림
	//CFlatBufPacket<system_s2c_play_data> message(system_s2c_play);
	auto teamoff = F4PACKET::SPlay(m_pHost->GetOffenseTeam());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_play, message, send_data);
	send_data.add_play(&teamoff);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	m_pHost->ResetShotClock();
}

void CState_Play::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);

	// 게임타임 업데이트
	if (m_CheckGameTime == true)
	{
		//! 20231128 CONTINUOUS 일 때 대기상태에서는 자리비움 체크를 하지 않는다 - 중국팀 요청사항
		if (m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS)
		{
			DHOST_TYPE_INT32 waitTeam = m_pHost->GetIndexTeamWait();

			if (waitTeam != -1)
			{
				for (int i = 0; i < m_pHost->GetCharacterTeamSize(waitTeam); ++i)
				{
					DHOST_TYPE_CHARACTER_SN characterSN = m_pHost->GetCharacterSN(waitTeam, i);

					if (characterSN != kCHARACTER_SN_INIT)
					{
						m_pHost->UpdateAwayFromKeyboard(characterSN, m_pHost->GetGameTime());
					}
				}
			}
		}
		//!
		float gameTimeFloat = m_pHost->GetGameTime() - timeDelta;

		m_pHost->SetGameTime(gameTimeFloat);

		int gameTimeInt = (int)gameTimeFloat;

		if (gameTimeInt != m_PrevGameTime && gameTimeInt > -1)
		{
			m_PrevGameTime = gameTimeInt;

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_s2c_timeGame, message, send_data);
			send_data.add_time(gameTimeInt + 1);
			send_data.add_timefloat(gameTimeFloat);
			STORE_FBPACKET(builder, message, send_data)

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}

		if (gameTimeFloat <= kFLOAT_INIT)
		{
			if (m_LastSendGameTime == false)
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, system_s2c_timeGame, message, send_data);
				send_data.add_time(gameTimeInt);
				send_data.add_timefloat(gameTimeFloat);
				STORE_FBPACKET(builder, message, send_data)

				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

				m_LastSendGameTime = true;
			}
			
			if (m_pHost->BallControllerGet()->GameBallDataGet() == nullptr)
			{
				string log_message = "ConvertPacket_play_c2s_ballSimulation GameBallDataGet() is null";
				m_pHost->ToLog(log_message.c_str());
			}

			const CBallController::GameBallData* pGameBallData = m_pHost->BallControllerGet()->GameBallDataGet();

			auto ball_event_size = 0;

			if (pGameBallData != nullptr)
			{
				ball_event_size = static_cast<int>(pGameBallData->ballEventMap.size());
			}

			if (ball_event_size > 0) // 볼관련 이벤트 체크 
			{
				return;
			}

			m_pHost->ChangeState(EHOST_STATE::END);
		}
	}

	// 슛 클락 업데이트
	if (m_CheckShotClock == true && m_pHost->GetOffenseTeam() != -1)
	{
		float shotClockFloat = m_pHost->GetShotClock() - timeDelta;

		m_pHost->SetShotClock(shotClockFloat);

		int shotClockInt = ceil(shotClockFloat);

		if (shotClockInt != m_PrevShotClock)
		{
			m_PrevShotClock = shotClockInt;

			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, system_s2c_timeShot, message, send_data);
				send_data.add_time(shotClockInt);
				STORE_FBPACKET(builder, message, send_data)

				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}

			if (shotClockInt <= 0)
			{
				if (m_pHost->BallControllerGet()->GameBallDataGet() == nullptr)
				{
					string log_message = "ConvertPacket_play_c2s_ballSimulation GameBallDataGet() is null";
					m_pHost->ToLog(log_message.c_str());
				}

				const CBallController::GameBallData* pGameBallData = m_pHost->BallControllerGet()->GameBallDataGet();

				bool ignore = false;

				if (pGameBallData != nullptr)
				{
					switch (pGameBallData->ballState)
					{
						case BALL_STATE::ballState_shotDunk:
						case BALL_STATE::ballState_shotJump:
						case BALL_STATE::ballState_shotLayUp:
						case BALL_STATE::ballState_shotPost:
							{
								int ball_event_size = static_cast<int>(pGameBallData->ballEventMap.size());

								if (ball_event_size > 0)
								{
									ignore = true;
								}
							}
							break;
						default:
							break;
					}
				}

				if (ignore == false)
				{
					m_pHost->ResetShotClock();
					m_pHost->SetOffenseTeam(m_pHost->GetIndexOpponentTeam(m_pHost->GetOffenseTeam()));

					CREATE_BUILDER(builder)
					CREATE_FBPACKET(builder, system_s2c_shotClockViolation, message, send_data);
					send_data.add_time(static_cast<int>(m_pHost->GetShotClock()));
					STORE_FBPACKET(builder, message, send_data)

					m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

					m_pHost->ChangeState(EHOST_STATE::BREAK_TIME);
				}
			}
		}
	}

}

void CState_Play::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	auto loose = (int)ballState & (int)BALL_STATE::ballState_loose;
	auto pick = (int)ballState & (int)BALL_STATE::ballState_pick;

	//! 버스트 모든 관련 변수
	std::vector<DHOST_TYPE_INT32>& team_scores = m_pHost->GetVecScores();
	DHOST_TYPE_CHARACTER_SN action_owner = ownerID;						// 액션의 주체
	//! end

	auto bReset = true;

	if (eventName == "Event_ballSimulation")
	{
		//m_pHost->DevBallActionInfoLog();

		DHOST_TYPE_CHARACTER_SN TurnOverCharacterSN = m_pHost->CheckTurnOver(ballNumber, (BALL_STATE)ballState, team);

		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(ownerID);
		if (pCharacter != nullptr)
		{
			ECHARACTER_RECORD_TYPE record_type = ECHARACTER_RECORD_TYPE::NONE;

			switch (m_pHost->ConvertToEnumBallState((BALL_STATE)ballState))
			{
			case EBALL_STATE::LOOSE_BLOCK:
			{
				record_type = ECHARACTER_RECORD_TYPE::BLOCK;
				bReset = false;
			}
			break;
			case EBALL_STATE::LOOSE_REBOUND:
			{
				bReset = false;
			}
			break;
			case EBALL_STATE::PASS_REBOUND:
			{
				record_type = ECHARACTER_RECORD_TYPE::REBOUND;
				bReset = false;
			}
			break;
			case EBALL_STATE::PASS_BLOCK:
			{
				record_type = ECHARACTER_RECORD_TYPE::BLOCK;
				bReset = false;
			}
			break;
			case EBALL_STATE::PASS_LOOSE:
			{
				record_type = ECHARACTER_RECORD_TYPE::LOOSE_BALL;
				bReset = false;
			}
			break;
			case EBALL_STATE::PICK_BLOCK:
			{
				record_type = ECHARACTER_RECORD_TYPE::BLOCK;
			}
			break;
			case EBALL_STATE::PICK_REBOUND:
			{
				record_type = ECHARACTER_RECORD_TYPE::REBOUND;

				DHOST_TYPE_BALL_NUMBER target_ball = ballNumber - 1;

				SBallActionInfo prevBallInfo;
				if (m_pHost->FindTargetBallActionInfo(target_ball, prevBallInfo))
				{
					if (prevBallInfo.BallNumber == target_ball)
					{
						// 직전공이 앨리웁패스라면 리바운드 카운팅을 하지 않는다.
						if (prevBallInfo.Ballstate == EBALL_STATE::ALLEYOOP)
						{
							break;
						}

						// 직전공이 블락이었다면 리바운드가 아닌 루즈볼 획득으로 기록처리 해야됨
						if (prevBallInfo.Ballstate == EBALL_STATE::LOOSE_BLOCK)
						{
							record_type = ECHARACTER_RECORD_TYPE::LOOSE_BALL;
							break;
						}

						// 리바운드 액션에서 공을 잡았는데 직전공이 어퍼컷 스틸인 경우에는 스틸 기록을 추가해준다. (좀 더 정확하게 하려면 스킬 인덱스로 비교해야됨)
						if (prevBallInfo.Ballstate == EBALL_STATE::LOOSE_STEAL)
						{
							if (prevBallInfo.BallOwnerTeam == m_pHost->ConvertToStringTeam((DHOST_TYPE_INT32)pCharacter->team))
							{
								m_pHost->UpdateRecord(prevBallInfo.BallOwner, prevBallInfo.BallNumber, ECHARACTER_RECORD_TYPE::STEAL);
								SendRecordData(prevBallInfo.BallOwner, m_pHost->ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE::STEAL));
								m_pHost->CheckCharacterBurstGauge(prevBallInfo.BallOwner, m_pHost->GetGameTime(), (BURST_ACTION_TYPE | BURST_ACTION_TYPE_STEAL), team_scores);

								record_type = ECHARACTER_RECORD_TYPE::LOOSE_BALL;
								break;
							}
						}
					}
				}

				if (ballNumber <= 3)	// 점프볼 공은 리바운드 취급하지 않는다.
				{
					record_type = ECHARACTER_RECORD_TYPE::NONE;
				}
			}
			break;
			case EBALL_STATE::PICK_STEAL:
			{
				record_type = ECHARACTER_RECORD_TYPE::STEAL;
			}
			break;
			case EBALL_STATE::LOOSE_STEAL:
			{
				bReset = false;
			}
			break;
			case EBALL_STATE::PICK:
			case EBALL_STATE::PICK_LOOSE:
			{
				record_type = ECHARACTER_RECORD_TYPE::LOOSE_BALL;

				auto pre_ball = ballNumber - 1;
				if (pre_ball > 1)
				{
					SBallActionInfo sInfo;

					if (m_pHost->FindTargetBallActionInfo(pre_ball, sInfo))
					{
						if (sInfo.BallNumber == pre_ball)
						{
							if (sInfo.Ballstate == EBALL_STATE::SHOT ||
								sInfo.Ballstate == EBALL_STATE::SHOT_DUNK ||
								sInfo.Ballstate == EBALL_STATE::SHOT_JUMP ||
								sInfo.Ballstate == EBALL_STATE::SHOT_LAYUP ||
								sInfo.Ballstate == EBALL_STATE::SHOT_POST)
							{
								//record_type = ECHARACTER_RECORD_TYPE::REBOUND;
							}
							else if (sInfo.Ballstate == EBALL_STATE::LOOSE_STEAL)
							{
								if (sInfo.BallOwnerTeam == m_pHost->ConvertToStringTeam((DHOST_TYPE_INT32)pCharacter->team))
								{
									m_pHost->UpdateRecord(sInfo.BallOwner, sInfo.BallNumber, ECHARACTER_RECORD_TYPE::STEAL);
									SendRecordData(sInfo.BallOwner, m_pHost->ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE::STEAL));
									m_pHost->CheckCharacterBurstGauge(sInfo.BallOwner, m_pHost->GetGameTime(), (BURST_ACTION_TYPE | BURST_ACTION_TYPE_STEAL), team_scores);
								}
							}
						}
					}
				}
			}
			break;
			}

			if (record_type == ECHARACTER_RECORD_TYPE::BLOCK)
			{
				auto pre_ball = ballNumber - 1;
				if (pre_ball > 1)
				{
					SBallActionInfo sInfo;
					if (m_pHost->FindTargetBallActionInfo(pre_ball, sInfo))
					{
						if (sInfo.BallNumber == pre_ball)
						{
							if (sInfo.Ballstate == EBALL_STATE::SHOT ||
								sInfo.Ballstate == EBALL_STATE::SHOT_DUNK ||
								sInfo.Ballstate == EBALL_STATE::SHOT_JUMP ||
								sInfo.Ballstate == EBALL_STATE::SHOT_LAYUP ||
								sInfo.Ballstate == EBALL_STATE::SHOT_POST)
							{
								uint64_t burst_action_type = BURST_ACTION_TYPE | BURST_ACTION_TYPE_SHOT;

								if (shotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
								{
									burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE_FAIL;
								}
								else
								{
									burst_action_type |= BURST_ACTION_TYPE_SHOT_FAIL;
								}

								m_pHost->CheckCharacterBurstGauge(sInfo.BallOwner, m_pHost->GetGameTime(), (burst_action_type | BURST_ACTION_TYPE_BLOCKED), team_scores);
							}
						}
					}
				}
			}

			if (record_type != ECHARACTER_RECORD_TYPE::NONE)
			{
				m_pHost->UpdateRecord(ownerID, ballNumber, record_type);
				SendRecordData(ownerID, m_pHost->ConvertToProtobufRecordType(record_type));

				m_pHost->CheckCharacterBurstGauge(ownerID, m_pHost->GetGameTime(), m_pHost->GetBurstActionTypeWithRecordType(record_type), team_scores);
			}

			if (TurnOverCharacterSN != kCHARACTER_SN_INIT)
			{
				map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();

				for (auto it = characters.begin(); it != characters.end(); ++it)
				{
					CCharacter* character = it->second;
					if (character)
						character->OnTurnOver(action_owner, TurnOverCharacterSN);
				}
				

				m_pHost->UpdateRecord(TurnOverCharacterSN, kBALL_NUMBER_INIT, ECHARACTER_RECORD_TYPE::TURN_OVER);
				SendRecordData(TurnOverCharacterSN, m_pHost->ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE::TURN_OVER));
				m_pHost->CheckCharacterBurstGauge(TurnOverCharacterSN, m_pHost->GetGameTime(), m_pHost->GetBurstActionTypeWithRecordType(ECHARACTER_RECORD_TYPE::TURN_OVER), team_scores);
			}
		}

		//! 비매너 감지
		if (pick == (int)BALL_STATE::ballState_pick)
		{
			//std::string pickStr = "Ower : " + std::to_string(ownerID) + ", BallState : " + F4PACKET::EnumNameBALL_STATE(ballState) + ", Pick : " + std::to_string(pick);
			//m_pHost->SendDebugMessage(pickStr);
			m_pHost->UpdateIntentionalBallHogPlay(ownerID, m_pHost->GetGameTime(), ballNumber);
		}
		else
		{
			m_pHost->UpdateIntentionalBallHogPlay(kCHARACTER_SN_INIT, m_pHost->GetGameTime(), ballNumber);
		}

		if (team != m_pHost->GetOffenseTeam() && bReset)
		{
			m_EnableStop = true;
			m_pHost->ResetShotClock();
			m_pHost->SetOffenseTeam(team);
		} 
	}
	else if (eventName == "EventReboundAble") // 2024-01-08 by steven, 서버가 판단하는 리바운드 가능시점 부터 리바운드가 가능하다( 핵킹 방지 ) 
	{
		CBallController* pBallController = m_pHost->BallControllerGet();
		if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
		{
			pBallController->GameBallDataGet()->checkReboundValid = true;
			pBallController->GameBallDataGet()->checkReboudEnableTime = m_pHost->GetRoomElapsedTime();
			m_pHost->SendSyncInfo(SYNCINFO_TYPE::REBOUND);
		}
    }
	else if (eventName == "EventPickAble")
	{
		m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);
	}
	else if (eventName == "Event_FirstRimBound")
	{
		if (m_pHost->GetGameTime() <= 0)
		{
			return;
		}

		m_EnableStop = true;
		m_pHost->ResetShotClock();

		//string log_message = "[SHOT_INFO] shotPosition x : " + std::to_string(ballPositionX) + ", z : " + std::to_string(ballPositionZ) + ", Distance : " + std::to_string(m_pHost->GetShotPositionDistanceFromRim(ballPositionX, ballPositionZ)) + ", degree : " + std::to_string(m_pHost->GetShotPositionDegree(ballPositionX, ballPositionZ)) + ", Sector : " + std::to_string(m_pHost->GetShotPositionZone(ballPositionX, ballPositionZ));
		//m_pHost->ToLog(log_message.c_str());

		m_pHost->AddCharacterShootInfo(ownerID, ballPositionX, ballPositionZ, m_pHost->GetShotPositionZone(ballPositionX, ballPositionZ), false);

		uint64_t burst_action_type = BURST_ACTION_TYPE | BURST_ACTION_TYPE_SHOT | BURST_ACTION_TYPE_SHOT_FGM_FAIL;

		if (shotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
		{
			burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE_FAIL;
			m_pHost->CheckCharacterBurstGauge(ownerID, m_pHost->GetGameTime(), burst_action_type, team_scores);
		}
		else
		{
			burst_action_type |= BURST_ACTION_TYPE_SHOT_FAIL;

			m_pHost->CheckCharacterBurstGauge(ownerID, m_pHost->GetGameTime(), burst_action_type, team_scores);
		}

		std::vector<DHOST_TYPE_CHARACTER_SN> vBallShotHindrance;
		vBallShotHindrance.clear();
		vBallShotHindrance = m_pHost->GetBallShotHindrance();

		// 골 실패 처리하는 곳 
		map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();

		for (auto it = characters.begin(); it != characters.end(); ++it)
		{
			CCharacter* character = it->second;
			if (character)
				character->OnGoalFailed(vBallShotHindrance);

			if (character)
				character->OnGoalResult(ownerID, shotType, false);
		}




		CREATE_BUILDER(builder)
		auto offsetcountinfo = builder.CreateVector(vBallShotHindrance);
		CREATE_FBPACKET(builder, system_s2c_goalFail, message, send_data);
		send_data.add_ownerid(ownerID);
		send_data.add_shottype((F4PACKET::SHOT_TYPE)shotType);
		send_data.add_ballnumber(ballNumber);
		send_data.add_hindrances(offsetcountinfo);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
	else if (eventName == "Event_RightGoalIn")
	{
		m_pHost->SetBallNumberGoalIn(ballNumber);

		if (m_pHost->GetOffenseTeam() != team)
		{
			team = m_pHost->GetOffenseTeam();
			point = 2;
			DHOST_TYPE_CHARACTER_SN teammember = m_pHost->GetNeariestTeamCharacterFromRim(team);
			if (teammember != kCHARACTER_SN_INIT)
			{
				ownerID = teammember;
			}
		}
		if (m_pHost->BallControllerGet()->GameBallDataGet() == nullptr)
		{
			string log_message = "ConvertPacket_play_c2s_ballSimulation GameBallDataGet() is null";
			m_pHost->ToLog(log_message.c_str());
		}

		// 골인을 해도 pick을 허락해 주자 
		m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);

		if (m_CheckScore == true && ballNumber == m_pHost->BallControllerGet()->GameBallDataGet()->ballNumber &&
			m_pHost->BallControllerGet()->IsReserved() == false)
		{
			uint64_t burst_action_type = BURST_ACTION_TYPE | BURST_ACTION_TYPE_SHOT | BURST_ACTION_TYPE_SHOT_FGM;

			int opponent_score = m_pHost->GetScore(m_pHost->GetIndexOpponentTeam(team));

			int newScore = m_pHost->GetScore(team) + point;
			
			if (m_pHost->GetScore(team) == 0 && opponent_score == 0)
			{
				if (shotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
				{
					burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE;
				}
				else
				{
					burst_action_type |= BURST_ACTION_TYPE_SHOT_TWO;
				}
			}
			else
			{
				if (shotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
				{
					burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE;

					if (m_pHost->GetScore(team) < opponent_score)
					{
						if (newScore == opponent_score)
						{
							burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE_TIE;
						}
						else if (newScore > opponent_score)
						{
							burst_action_type |= BURST_ACTION_TYPE_SHOT_THREE_REVERSAL;
						}
					}
				}
				else
				{
					burst_action_type |= BURST_ACTION_TYPE_SHOT_TWO;

					if (shotType == F4PACKET::SHOT_TYPE::shotType_dunkDriveIn || shotType == F4PACKET::SHOT_TYPE::shotType_dunkFingerRoll || shotType == F4PACKET::SHOT_TYPE::shotType_dunkPost)
					{
						burst_action_type |= BURST_ACTION_TYPE_SHOT_DUNK;
					}

					if (m_pHost->GetScore(team) < opponent_score)
					{
						if (newScore == opponent_score)
						{
							burst_action_type |= BURST_ACTION_TYPE_SHOT_TWO_TIE;
						}
						else if (newScore > opponent_score)
						{
							burst_action_type |= BURST_ACTION_TYPE_SHOT_REVERSAL;
						}
					}
				}
			}

			m_pHost->CheckCharacterBurstGauge(ownerID, m_pHost->GetGameTime(), burst_action_type, team_scores);

			m_pHost->SetScore(team, newScore);
			
			// 주목할 선수 컨텐츠로 인한 캐릭터 기록 보내는 시점 변경
			// 변경 전 : 골인패킷 -> 기록
			// 변경 후 : 기록 -> 골인패킷
			SBallActionInfo sInfo;
			if (m_pHost->FindTargetBallActionInfo(ballNumber, sInfo))
			{
				if (sInfo.BallNumber == ballNumber)
				{
					ECHARACTER_RECORD_TYPE type = (point == 2) ? ECHARACTER_RECORD_TYPE::SUC_2POINT : ECHARACTER_RECORD_TYPE::SUC_3POINT;

					DHOST_TYPE_CHARACTER_SN assistCharacterSN = kCHARACTER_SN_INIT;

					assistCharacterSN = m_pHost->UpdateRecord(ownerID, sInfo.BallNumber, type, sInfo.bAssist);
					PushSendRecordInfo(ownerID, type);

					m_pHost->UpdateRecordBasicTrainingSystem(ownerID, sInfo.BallNumber, (F4PACKET::SKILL_INDEX)sInfo.SkillIndex, shotType);

					if (assistCharacterSN != kCHARACTER_SN_INIT)
					{
						m_pHost->CheckCharacterBurstGauge(assistCharacterSN, m_pHost->GetGameTime(), (BURST_ACTION_TYPE | BURST_ACTION_TYPE_ASSIST), m_pHost->GetVecScores());
						PushSendRecordInfo(assistCharacterSN, ECHARACTER_RECORD_TYPE::ASSIST);
						m_pHost->SetReplayAssistCheck(true);
					}
					// 레이업 골 카운트
					if (shotType == F4PACKET::SHOT_TYPE::shotType_layUpDriveIn || shotType == F4PACKET::SHOT_TYPE::shotType_layUpFingerRoll || shotType == F4PACKET::SHOT_TYPE::shotType_layUpPost)
					{
						m_pHost->IncreaseActionLayUpGoalCount(ownerID);
					}
					// 덩크 골 카운트
					if (shotType == F4PACKET::SHOT_TYPE::shotType_dunkDriveIn || shotType == F4PACKET::SHOT_TYPE::shotType_dunkFingerRoll || shotType == F4PACKET::SHOT_TYPE::shotType_dunkPost)
					{
						m_pHost->IncreaseActionDunkGoalCount(ownerID);
					}

					SendRecordData();
				}
			}
			
			//string log_message = "[SHOT_INFO] shotPosition x : " + std::to_string(ballPositionX) + ", z : " + std::to_string(ballPositionZ) + ", Distance : " + std::to_string(m_pHost->GetShotPositionDistanceFromRim(ballPositionX, ballPositionZ)) + ", degree : " + std::to_string(m_pHost->GetShotPositionDegree(ballPositionX, ballPositionZ)) + ", Sector : " + std::to_string(m_pHost->GetShotPositionZone(ballPositionX, ballPositionZ));
			//m_pHost->ToLog(log_message.c_str());

			m_pHost->AddCharacterShootInfo(ownerID, ballPositionX, ballPositionZ, m_pHost->GetShotPositionZone(ballPositionX, ballPositionZ), true);

			//CFlatBufPacket<system_s2c_goal_data> message(system_s2c_goal);

			m_pHost->GetCharacterManager()->EndValidatePassiveAll(); // 기본으로 해주는 것 들 

			CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(ownerID); // 골인이 됐을 때 특정 캐릭터만 해주는 것 
			if (pCharacter != nullptr)
			{
				SCharacterPassive* pCharacterPassive = pCharacter->GetCharacterPassive();
				if (pCharacterPassive != nullptr)
				{
					float before = pCharacterPassive->valueCurr;
					pCharacter->EndValidatePassive(shotType);
					pCharacter->OnGoalResult(ownerID, shotType, true);

					//string log_message = "[PASSIVE] [EndValidatePassive] ownerID : " + std::to_string(ownerID) + "Before value: " + std::to_string(before)
						//+ " After value: " + std::to_string(pCharacterPassive->valueCurr);
					//m_pHost->ToLog(log_message.c_str());
				}
			
			}

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_s2c_goal, message, send_data);
			send_data.add_scorerid(ownerID);
			send_data.add_point(point);
			send_data.add_scoreteam((DHOST_TYPE_INT32)team);
			send_data.add_score(m_pHost->GetScore(team));
			send_data.add_shottype((F4PACKET::SHOT_TYPE)shotType);
			send_data.add_remaintime(m_pHost->GetGameTime());
			STORE_FBPACKET(builder, message, send_data)

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

			if (m_pHost->GetGameTime() <= kBUZZER_BEATER_TIME)
			{
				// 일반경기? 에서만 버저비터 작동하게
				if (m_pHost->GetModeType() != EMODE_TYPE::CONTINUOUS && m_pHost->CheckSingleMode() == false)
				{
					m_pHost->CheckBuzzerBeater(team, point, ballPositionX, ballPositionZ);

					if (m_pHost->GetBuzzerBeater())
					{
						m_pHost->ChangeState(EHOST_STATE::SCORE, &ownerID);
					}
				}
			}

			if (m_pHost->GetGameTime() <= kFLOAT_INIT)
			{
				return;
			}

			if (m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS && newScore >= 11)
			{
				m_pHost->ChangeState(EHOST_STATE::END);
			}
			else
			{
				DHOST_TYPE_INT32 offenseTeam;

				if (m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS)
				{
					DHOST_TYPE_INT32 waitTeam = m_pHost->GetIndexTeamWait();

					if (waitTeam != -1)
					{
						m_pHost->SetTeamsActive(team, waitTeam);
					}

					offenseTeam = waitTeam;
				}
				else
				{
					offenseTeam = m_pHost->GetIndexOpponentTeam(team);
				}

				m_pHost->SetOffenseTeam(m_pHost->GetIndexOpponentTeam(team));
				m_pHost->ResetShotClock();
				m_pHost->ChangeState(EHOST_STATE::SCORE, &ownerID);
			}
		}
	}
}

DHOST_TYPE_BOOL CState_Play::OnPacket(CPacketBase* pPacket, const char* pData, int size, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (nullptr == pPacket)
		return false;

	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL_EXTEND(play_c2s_ballRebound);
	default:
		return false;
	}
}


DHOST_TYPE_BOOL CState_Play::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (nullptr == pPacket)
		return false;

	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(play_c2s_ballClear)
		CASE_FB_CALL(play_c2s_ballLerp)
		CASE_FB_CALL(play_c2s_ballShot)
		CASE_FB_CALL(play_c2s_ballSimulation)
	default:
		return CState::OnPacket(pPacket, peer, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Play::ConvertPacket_play_c2s_ballClear(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballClear_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->position() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER position is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballClear, message, send_data);
	send_data.add_ballcleared(data->ballcleared());
	send_data.add_position(data->position());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Play::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->balllerp() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER balllerp is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_EnableStop = true;

	return CState::ConvertPacket_play_c2s_ballLerp(userid, peer, pPacket, recv_time);
}

const DHOST_TYPE_BOOL CState_Play::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballsimulation() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballsimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_EnableStop = true;

	return CState::ConvertPacket_play_c2s_ballSimulation(userid, peer, pPacket, recv_time);
}

const DHOST_TYPE_BOOL CState_Play::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
}


const DHOST_TYPE_BOOL CState_Play::ConvertPacketExtend_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const char* pData, int size, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
	//! 
	
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballsimulation() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballsimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	
	//!
	//! 

	m_EnableStop = true;

	return CState::ConvertPacketExtend_play_c2s_ballRebound(userid, peer, pPacket, pData, size, recv_time);
}



