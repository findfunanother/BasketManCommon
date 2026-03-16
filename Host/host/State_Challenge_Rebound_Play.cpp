#include "State_Challenge_Rebound_Play.h"
#include "BallController.h"

void CState_Challenge_Rebound_Play::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;
	m_prevGameTime = 0;

	m_penaltyPointDelta = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_TimePenalty_Value"));
	m_penaltyTimeDuration = m_pHost->GetBalanceValue("VRebound_Point_TimePenalty_Time");

	m_penaltyPointTotal = 0;
	m_penaltyTimeRemain = m_penaltyTimeDuration;
}

void CState_Challenge_Rebound_Play::OnUpdate(float timeDelta)
{
	m_pHost->CheckExpiredRoom();

	float gameTimeFloat = m_pHost->GetGameTime() - timeDelta;

	m_pHost->SetGameTime(gameTimeFloat);

	m_penaltyTimeRemain -= timeDelta;

	if (m_penaltyTimeRemain < 0.0f)
	{
		m_penaltyPointTotal -= m_penaltyPointDelta;

		m_penaltyTimeRemain += m_penaltyTimeDuration;
	}

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

void CState_Challenge_Rebound_Play::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	if (eventName == "EventReboundAble")
	{
		m_penaltyPointTotal = kINT32_INIT;
		m_penaltyTimeRemain = m_penaltyTimeDuration;

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
	else if (eventName == "Event_RightGoalIn")
	{
		m_pHost->SetBallNumberGoalIn(ballNumber);

		m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);
	}
}

const DHOST_TYPE_BOOL CState_Challenge_Rebound_Play::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_Rebound::ConvertPacket_play_c2s_ballLerp(userid, peer, pPacket, recv_time);

	return retValue;
}


const DHOST_TYPE_BOOL CState_Challenge_Rebound_Play::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_Rebound::ConvertPacket_play_c2s_ballRebound(userid, peer, pPacket, recv_time);

	CBallController* pBallController = m_pHost->BallControllerGet();
	if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
	{
		if (retValue == true && pBallController->GameBallDataGet()->checkReboundValid == true)
		{
			if (data->ballsimulation()->ballstate() == BALL_STATE::ballState_pickRebound)
			{
				int pointGain;
				CHALLENGE_MESSAGE pointMessage;

				pBallController->GameBallDataGet()->checkReboundValid = false;

				if (data->ballsimulation()->owner() == pChallengeData->m_userPlayerID)
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Max") + m_penaltyPointTotal);

					if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GreatRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GreatRebound;
					}
					else if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GoodRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GoodRebound;
					}
					else
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_BadRebound;
					}
				}
				else
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Fail"));
					pointMessage = CHALLENGE_MESSAGE::challengeMessage_Fail;
				}

				pChallengeData->m_point = max(pChallengeData->m_point + pointGain, 0);

				SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);
			}
		}
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Challenge_Rebound_Play::ConvertPacketExtend_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const char* pData, int size, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	// 이 챌린지에서는 이전 리바운드를 사용 
	DHOST_TYPE_BOOL retValue = CState_Challenge_Rebound::ConvertPacket_play_c2s_ballRebound(userid, peer, pPacket, recv_time);

	CBallController* pBallController = m_pHost->BallControllerGet();
	if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
	{
		if (retValue == true && pBallController->GameBallDataGet()->checkReboundValid == true)
		{
			if (data->ballsimulation()->ballstate() == BALL_STATE::ballState_pickRebound)
			{
				int pointGain;
				CHALLENGE_MESSAGE pointMessage;

				pBallController->GameBallDataGet()->checkReboundValid = false;

				if (data->ballsimulation()->owner() == pChallengeData->m_userPlayerID)
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Max") + m_penaltyPointTotal);

					if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GreatRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GreatRebound;
					}
					else if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GoodRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GoodRebound;
					}
					else
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_BadRebound;
					}
				}
				else
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Fail"));
					pointMessage = CHALLENGE_MESSAGE::challengeMessage_Fail;
				}

				pChallengeData->m_point = max(pChallengeData->m_point + pointGain, 0);

				SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);
			}
		}
	}

	return retValue;
}


const DHOST_TYPE_BOOL CState_Challenge_Rebound_Play::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_Rebound::ConvertPacket_play_c2s_ballSimulation(userid, peer, pPacket, recv_time);

	CBallController* pBallController = m_pHost->BallControllerGet();
	if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
	{
		if (retValue == true && pBallController->GameBallDataGet()->checkReboundValid == true)
		{
			if (data->ballsimulation()->ballstate() == BALL_STATE::ballState_pickRebound)
			{
				int pointGain;
				CHALLENGE_MESSAGE pointMessage;

				pBallController->GameBallDataGet()->checkReboundValid = false;

				if (data->ballsimulation()->owner() == pChallengeData->m_userPlayerID)
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Max") + m_penaltyPointTotal);

					if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GreatRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GreatRebound;
					}
					else if (pointGain > m_pHost->GetBalanceValue("VRebound_Point_GoodRebound"))
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_GoodRebound;
					}
					else
					{
						pointMessage = CHALLENGE_MESSAGE::challengeMessage_BadRebound;
					}
				}
				else
				{
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VRebound_Point_Fail"));
					pointMessage = CHALLENGE_MESSAGE::challengeMessage_Fail;
				}

				pChallengeData->m_point = max(pChallengeData->m_point + pointGain, 0);

				SendMessagePoint(pChallengeData->m_point, pointGain, pointMessage, 0.0f);
			}
		}
	}

	return retValue;
}