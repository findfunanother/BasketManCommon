#include "State_Challenge_JumpShotBlock_Play.h"
#include "BallController.h"
#include "PhysicsHandler.h"
#include "DataManagerShotSolution.h"
void CState_Challenge_JumpShotBlock_Play::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;
	m_PrevShotClock = 0;

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_timeGame, message, send_data);
	send_data.add_time(pChallengeData->m_tryCount);
	send_data.add_timefloat((float)pChallengeData->m_tryCount);
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CState_Challenge_JumpShotBlock_Play::OnUpdate(float timeDelta)
{
	m_pHost->CheckExpiredRoom();

	// 슛 클락 업데이트
	if (m_pHost->GetOffenseTeam() != -1)
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
					CREATE_BUILDER(builder)
					CREATE_FBPACKET(builder, system_s2c_shotClockViolation, message, send_data);
					send_data.add_time(static_cast<int>(m_pHost->GetShotClock()));
					STORE_FBPACKET(builder, message, send_data)

					m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

					if (pChallengeData->m_tryCount > 0)
					{
						m_pHost->ChangeState(EHOST_STATE::CHALLENGE_BREAKTIME, pChallengeData);
					}
					else
					{
						m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, pChallengeData);
					}
				}
			}
		}
	}
}

const DHOST_TYPE_BOOL CState_Challenge_JumpShotBlock_Play::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	bool retValue = CState_Challenge_JumpShotBlock::ConvertPacket_play_c2s_ballRebound(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		switch (data->ballsimulation()->ballstate())
		{
		case BALL_STATE::ballState_looseBlock:
		case BALL_STATE::ballState_passBlock:
		case BALL_STATE::ballState_pickBlock:
		{
			int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_Block"));
			pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_Block, 0.0f);
		}
		}
	}
	return retValue;

}


const DHOST_TYPE_BOOL CState_Challenge_JumpShotBlock_Play::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	bool retValue = CState_Challenge_JumpShotBlock::ConvertPacket_play_c2s_ballSimulation(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		switch (data->ballsimulation()->ballstate())
		{
		case BALL_STATE::ballState_looseBlock:
		case BALL_STATE::ballState_passBlock:
		case BALL_STATE::ballState_pickBlock:
		{
			int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_Block"));
			pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_Block, 0.0f);
		}
		}
	}
	return retValue;

}

const DHOST_TYPE_BOOL CState_Challenge_JumpShotBlock_Play::ConvertPacket_play_c2s_ballShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->ballshot() == nullptr || data->ballshot()->positionball() == nullptr || data->ballshot()->positionowner() == nullptr || data->ballshot()->medalindices() == nullptr || data->ballshot()->hindrances() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballshot is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	SBallShotT ballShot;
	data->ballshot()->UnPackTo(&ballShot);
	SBallShotT* pBallShot = &ballShot;


	float basic_shot_suc_rate = kFLOAT_INIT;
	
	if (m_pHost->BallNumberGet() <= pBallShot->ballnumber)
	{
		pBallShot->success = (true);

		m_pHost->BallNumberSet(pBallShot->ballnumber + 1);

		m_pHost->SetBallOwner(pBallShot->owner);

		CPhysicsHandler* pPhysics = m_pHost->GetPhysicsHandler();

		F4PACKET::SPlayerInformationT* pPlayer = m_pHost->FindCharacterInformation(pBallShot->owner);

		if (pPhysics != nullptr && pPlayer != nullptr)
		{
			if (pBallShot->shottype == SHOT_TYPE::shotType_threePoint)
			{
				pBallShot->point = (3);
			}
			else
			{
				pBallShot->point = (2);
			}

			auto rimPos = pPhysics->GetRimPosition();
			rimPos.mutate_y(0.0f);

			bool success = false;

			float successRate = 0.0f;

			BALL_STATE ballState = BALL_STATE::ballState_none;
			SVector3 pos = *pBallShot->positionowner;
			float distanceOwnerRim = VECTOR3_DISTANCE(pos, rimPos);

			float targetReachTime = 0.0f;

			if (data->ballshot()->shotbound() == SHOT_BOUND::success_clean)
			{
				switch (pBallShot->shottype)
				{
				case SHOT_TYPE::shotType_dunkDriveIn:
				case SHOT_TYPE::shotType_dunkFingerRoll:
				case SHOT_TYPE::shotType_dunkPost:
				{
					ballState = BALL_STATE::ballState_shotDunk;
					pBallShot->shotthrow = (SHOT_THROW::low);
				}
				break;
				case SHOT_TYPE::shotType_layUpDriveIn:
				case SHOT_TYPE::shotType_layUpFingerRoll:
				case SHOT_TYPE::shotType_layUpPost:
				{
					ballState = BALL_STATE::ballState_shotLayUp;
					pBallShot->shotthrow = (SHOT_THROW::low);
				}
				break;
				case SHOT_TYPE::shotType_middle:
				{
					switch (data->ballshot()->skillindex())
					{
					case SKILL_INDEX::skill_spotUpShot:
						successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_SpotUpJumpShot");
						break;
					default:
						successRate = m_pHost->GetBalanceValue("VMidRangeDefense_Prob_SpotUpJumpShot");
						break;
					}

					ballState = BALL_STATE::ballState_shotJump;
					pBallShot->shotthrow = (SHOT_THROW::middle);
				}
				break;
				case SHOT_TYPE::shotType_middleUnstready:
				{
					ballState = BALL_STATE::ballState_shotJump;
					pBallShot->shotthrow = (SHOT_THROW::middle);
				}
				break;
				case SHOT_TYPE::shotType_hook:
				case SHOT_TYPE::shotType_post:
				{
					ballState = BALL_STATE::ballState_shotPost;
					pBallShot->shotthrow = (SHOT_THROW::middle);
				}
				break;
				case SHOT_TYPE::shotType_closePostUnstready:
				case SHOT_TYPE::shotType_postUnstready:
				{
					ballState = BALL_STATE::ballState_shotPost;
					pBallShot->shotthrow = (SHOT_THROW::middle);
				}
				break;
				case SHOT_TYPE::shotType_threePoint:
				{
					int pointGain;
					CHALLENGE_MESSAGE pointMessage;

					switch (data->ballshot()->skillindex())
					{
					case SKILL_INDEX::skill_spotUpShot:
						pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_SpotUpThreePointShot"));
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_SpotUpThreePointShot;
						successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_SpotUpThreePointShot");
						break;
					case SKILL_INDEX::skill_fadeAway3:
						pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_3ptFadeAway"));
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_3ptFadeAway;
						successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_3ptFadeAway");
						break;
					default:
						pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_ThreePointShot"));
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_ThreePointShot;
						successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_ThreePointShot");
						break;
					}

					pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
					SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);

					ballState = BALL_STATE::ballState_shotJump;
					pBallShot->shotthrow = (SHOT_THROW::middle);
				}
				break;
				}

				pBallShot->targetreachtime = (CalculateTargetReachTime(m_pHost->GetBalanceTable(), pBallShot, distanceOwnerRim));

				float hindrance = 0.0f;

				for (int i = 0; i < pBallShot->hindrances.size(); i++)
				{
					hindrance += pBallShot->hindrances[i].pointresult();
				}

				if (hindrance > m_pHost->GetBalanceValue("VMidRangeDefense_Prob_Hindrance_Standard2"))
				{
					successRate -= m_pHost->GetBalanceValue("VMidRangeDefense_Prob_Hindrance_Value2");

					int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_GreatHindrance"));
					pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
					SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_GreatDefense, 0.0f);
				}
				else if (hindrance > m_pHost->GetBalanceValue("VMidRangeDefense_Prob_Hindrance_Standard1"))
				{
					successRate -= m_pHost->GetBalanceValue("VMidRangeDefense_Prob_Hindrance_Value1");

					int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_GoodHindrance"));
					pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
					SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_GoodDefense, 0.0f);
				}
				else
				{
					int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_BadHindrance"));
					pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
					SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_BadDefense, 0.0f);
				}

				float randomSuccess = abs(Util::GetRandom(1.0f));

				success = randomSuccess <= successRate;

				if (success == true)
				{
					pBallShot->shotbound = (SHOT_BOUND::success_clean);
				}
				else
				{
					pBallShot->shotbound = (SHOT_BOUND::fail_forward_weak_bound);
				}
			}

			SBallActionInfo sInfo;
			sInfo.BallNumber = m_pHost->BallNumberGet();
			sInfo.BallOwner = m_pHost->GetBallOwner();
			sInfo.BallOwnerTeam = m_pHost->ConvertToStringTeam(m_pHost->GetCharacterTeam(sInfo.BallOwner));
			sInfo.Ballstate = m_pHost->ConvertToEnumBallState((BALL_STATE)ballState);
			sInfo.bAssist = false;
			sInfo.fGameTime = m_pHost->GetGameTime();
			sInfo.SkillIndex = (DHOST_TYPE_UINT32)pBallShot->skillindex;
			m_pHost->PushBallActionInfoDeque(sInfo);

			int sectorNumber;

			switch (data->ballshot()->shottype())
			{
			case SHOT_TYPE::shotType_dunkDriveIn:
			case SHOT_TYPE::shotType_dunkFingerRoll:
			case SHOT_TYPE::shotType_dunkPost:
				sectorNumber = 0;
				break;
			default:
				sectorNumber = m_pHost->GetShotSolution()->GetSectorNumber(*pBallShot->positionball, rimPos);
				break;
			}

			unsigned int key = m_pHost->GetShotSolution()->GetPointDataKey((F4PACKET::SHOT_THROW)pBallShot->shotthrow,
				sectorNumber,
				(F4PACKET::SHOT_BOUND)pBallShot->shotbound);
			int count = m_pHost->GetShotSolution()->GetCount(key);

			if (count > 0)
			{
				pBallShot->index = (rand() % count);
			}
			/*
			ShotSolution::SolutionTargetData* pSolutionTargetData =
				m_pHost->GetShotSolution()->FindTargetDataInDictionaryIndex(pBallShot->shotthrow(), sectorNumber, pBallShot->shotbound(), pBallShot->index());
				*/
			CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

			pGameBallData->ballNumber = m_pHost->BallNumberGet();
			pGameBallData->ballState = ballState;
			pGameBallData->shotType = data->ballshot()->shottype();
			pGameBallData->owner = pBallShot->owner;
			pGameBallData->point = pBallShot->point;
			pGameBallData->team = pPlayer->team;
			pGameBallData->posStart = *pBallShot->positionball;
			pGameBallData->timeElapsed = -pBallShot->time;
			ECHARACTER_RECORD_TYPE try_shot_record = ECHARACTER_RECORD_TYPE::TRY_2POINT;

			if (data->ballshot()->shottype() == SHOT_TYPE::shotType_threePoint)
			{
				try_shot_record = ECHARACTER_RECORD_TYPE::TRY_3POINT;
			}

			if (success == false)
			{
				pGameBallData->ballEventMap.insert(pair<string, float>("Event_FirstRimBound", pBallShot->targetreachtime));
			}
			
			m_pHost->BallControllerGet()->AddShotBallData(pBallShot, pGameBallData, pBallShot->time);

			pBallShot->successrate = (successRate);
		}
		else
		{
			pBallShot->successrate = (0.0f);
		}

		pBallShot->ballnumber = (m_pHost->BallNumberGet());

		BB_STATE state = (BB_STATE)m_pHost->GetBackBoardState();

		if (F4PACKET::EnumNameBB_STATE(state))
		{
			pBallShot->bbstate = ((BB_STATE)m_pHost->GetBackBoardState());
		}
	}
	else
	{
		pBallShot->success = (false);
		pBallShot->successrate = (0.0f);
	}

	CREATE_BUILDER(builder);
	auto offset_ballshot_data = F4PACKET::CreateSBallShot(builder, pBallShot);
	CREATE_FBPACKET(builder, play_s2c_ballShot, message, databuilder);
	databuilder.add_ballshot(offset_ballshot_data);
	STORE_FBPACKET(builder, message, databuilder);
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

void CState_Challenge_JumpShotBlock_Play::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	if (eventName == "Event_ballSimulation")
	{
		auto bReset = true;

		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(ownerID);

		if (pCharacter->team == 0)
		{
			int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_DefSuccess"));
			pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_DefenseSuccess, 0.0f);

			if (pChallengeData->m_tryCount > 0)
			{
				m_pHost->ChangeState(EHOST_STATE::CHALLENGE_BREAKTIME, pChallengeData);
			}
			else
			{
				m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, pChallengeData);
			}
		}
		else
		{
			if (pCharacter != nullptr)
			{
				switch (m_pHost->ConvertToEnumBallState((BALL_STATE)ballState))
				{
				case EBALL_STATE::LOOSE_BLOCK:
				{
					bReset = false;
				}
				break;
				case EBALL_STATE::PASS_BLOCK:
				{
					bReset = false;
				}
				break;
				case EBALL_STATE::LOOSE_STEAL:
				{
					bReset = false;
				}
				break;
				}
			}

			if (team != m_pHost->GetOffenseTeam() && bReset)
			{
				m_pHost->ResetShotClock();

				m_pHost->SetOffenseTeam(team);
			}
		}
	}
	else if (eventName == "Event_FirstRimBound")
	{
		int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_DefSuccess"));
		pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
		SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_DefenseSuccess, 0.0f);

		if (pChallengeData->m_tryCount > 0)
		{
			m_pHost->ChangeState(EHOST_STATE::CHALLENGE_BREAKTIME, pChallengeData);
		}
		else
		{
			m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, pChallengeData);
		}
	}
	else if (eventName == "Event_RightGoalIn")
	{
		switch (point)
		{
		case 2:
		{
			int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_2pt"));
			pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_2pt, 0.0f);
		}
			break;
		case 3:
		{
			int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VMidRangeDefense_Point_3pt"));
			pChallengeData->m_point = max(0, pointGain + pChallengeData->m_point);
			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_3pt, 0.0f);
		}
			break;
		}

		m_pHost->SetBallNumberGoalIn(ballNumber);

		m_pHost->BallNumberAdd(1);

		if (pChallengeData->m_tryCount > 0)
		{
			m_pHost->ChangeState(EHOST_STATE::CHALLENGE_BREAKTIME, pChallengeData);
		}
		else
		{
			m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, pChallengeData);
		}
	}
}
