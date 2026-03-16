#include "State_Challenge_OffBallMove_Play.h"
#include "BallController.h"
#include "PhysicsHandler.h"
#include "DataManagerShotSolution.h"
void CState_Challenge_OffBallMove_Play::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;
	m_prevGameTime = 0;
}

void CState_Challenge_OffBallMove_Play::OnUpdate(float timeDelta)
{
	m_pHost->CheckExpiredRoom();

	float gameTimeFloat = m_pHost->GetGameTime() - timeDelta;

	m_pHost->SetGameTime(gameTimeFloat);

	int gameTimeInt = (int)gameTimeFloat;

	if (gameTimeInt != m_prevGameTime)
	{
		m_prevGameTime = gameTimeInt;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_timeGame, message, send_data);
		send_data.add_time(gameTimeInt + 1);
		send_data.add_timefloat(gameTimeFloat);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	if (gameTimeFloat <= 0)
	{
		{
			gameTimeInt = 0;

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_s2c_timeGame, message, send_data);
			send_data.add_time(gameTimeInt);
			send_data.add_timefloat(gameTimeFloat);
			STORE_FBPACKET(builder, message, send_data)
			
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
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

		if (ball_event_size > 0)
		{
			return;
		}

		{

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, challenge_s2c_result, message, send_data);
			send_data.add_point(pChallengeData->m_point);
			STORE_FBPACKET(builder, message, send_data)

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}

		m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, pChallengeData);
	}
}

const DHOST_TYPE_BOOL CState_Challenge_OffBallMove_Play::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_OffBallMove::ConvertPacket_play_c2s_ballLerp(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		auto& data = *pPacket->GetData();
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Challenge_OffBallMove_Play::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	switch (data->ballsimulation()->ballstate())
	{
	case BALL_STATE::ballState_looseBlock:
	case BALL_STATE::ballState_passBlock:
	case BALL_STATE::ballState_pickBlock:
		return false;
	default:
		return CState_Challenge_OffBallMove::ConvertPacket_play_c2s_ballRebound(userid, peer, pPacket, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Challenge_OffBallMove_Play::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	switch (data->ballsimulation()->ballstate())
	{
		case BALL_STATE::ballState_looseBlock:
		case BALL_STATE::ballState_passBlock:
		case BALL_STATE::ballState_pickBlock:
			return false;
		default:
			return CState_Challenge_OffBallMove::ConvertPacket_play_c2s_ballSimulation(userid, peer, pPacket, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Challenge_OffBallMove_Play::ConvertPacket_play_c2s_ballShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
			const TB::SVector3 pos = *data->ballshot()->positionball();//positionball();
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
						int pointGain;
						CHALLENGE_MESSAGE pointMessage;

						switch (data->ballshot()->skillindex())
						{
							case SKILL_INDEX::skill_spotUpShot:
								pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_SpotUpJumpShot"));
								pointMessage = CHALLENGE_MESSAGE::challengeMessage_SpotUpJumpShot;
								successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_SpotUpJumpShot");
								break;
							case SKILL_INDEX::skill_fadeAway:
								pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_2ptFadeAway"));
								pointMessage = CHALLENGE_MESSAGE::challengeMessage_2ptFadeAway;
								successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_2ptFadeAway");
								break;
							default:
								pointMessage = CHALLENGE_MESSAGE::challengeMessage_MidRangeShot;
								pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_MidRangeShot"));
								successRate = m_pHost->GetBalanceValue("VOffBallMove_Prob_MidRangeShot");
								break;
						}

						pChallengeData->m_point = max(0, pChallengeData->m_point + pointGain);

						SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);

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
							case SKILL_INDEX::skill_fadeAway:
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

						pChallengeData->m_point = max(0, pChallengeData->m_point + pointGain);

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

				if (hindrance > m_pHost->GetBalanceValue("VOffBallMove_Prob_Hindrance_Standard"))
				{
					successRate -= m_pHost->GetBalanceValue("VOffBallMove_Prob_Hindrance_Value");

					int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_BadShot"));

					pChallengeData->m_point = max(0, pChallengeData->m_point + pointGain);
					SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_BadShot, 0.5f);
				}
				else
				{
					int pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_GoodShot"));

					pChallengeData->m_point = max(0, pChallengeData->m_point + pointGain);
					SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_GoodShot, 0.5f);
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
					sectorNumber = m_pHost->GetShotSolution()->GetSectorNumber(
						TB::SVector3(pBallShot->positionball->x(),
						pBallShot->positionball->y(),
						pBallShot->positionball->z()),
						TB::SVector3(rimPos.x(),
							rimPos.y(),
							rimPos.z()));
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
			SVector3 posball = *pBallShot->positionball;
			JOVECTOR3 posstart(posball.x(), posball.y(), posball.z());
			pGameBallData->posStart = posstart;
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
	auto offset_ball_shot_data  = SBallShot::Pack(builder, pBallShot);
	CREATE_FBPACKET(builder, play_s2c_ballShot, message, databuilder);
	databuilder.add_ballshot(offset_ball_shot_data);
	STORE_FBPACKET(builder, message, databuilder);
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

void CState_Challenge_OffBallMove_Play::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	if (eventName == "Event_RightGoalIn")
	{
		int pointGain;
		CHALLENGE_MESSAGE pointMessage;

		switch ((SHOT_TYPE)shotType)
		{
			case SHOT_TYPE::shotType_threePoint:
				pointMessage = CHALLENGE_MESSAGE::challengeMessage_3pt;
				pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_3pt"));
				break;
			default:
				pointMessage = CHALLENGE_MESSAGE::challengeMessage_2pt;
				pointGain = static_cast<int>(m_pHost->GetBalanceValue("VOffBallMove_Point_2pt"));
				break;
		}

		pChallengeData->m_point = max(0, pChallengeData->m_point + pointGain);

		SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);

		m_pHost->SetBallNumberGoalIn(ballNumber);

		m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);
	}
	else
	if (eventName == "EventReboundAble") // 2024-01-08 by steven, 서버가 판단하는 리바운드 가능시점 부터 리바운드가 가능하다( 핵킹 방지 ) 
	{
		CBallController* pBallController = m_pHost->BallControllerGet();
		if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
		{
			pBallController->GameBallDataGet()->checkReboundValid = true;
			pBallController->GameBallDataGet()->checkReboudEnableTime = m_pHost->GetRoomElapsedTime();
			m_pHost->SendSyncInfo(SYNCINFO_TYPE::REBOUND);
		}
	}
	else
	if (eventName == "EventPickAble")
	{
		CBallController* pBallController = m_pHost->BallControllerGet();
		if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
		{
			m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);
		}
	}

}
