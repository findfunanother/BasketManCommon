#include "State_Challenge_Pass_Play.h"
#include "BallController.h"

void CState_Challenge_Pass_Play::OnEnter(void* pData)
{
	m_targetID = 0;

	pChallengeData = (ChallengeData*)pData;
	m_prevGameTime = 0;
	
	m_penaltyPointDelta = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_TimePenalty_Value"));
	m_penaltyTimeDuration = m_pHost->GetBalanceValue("VPass_Point_TimePenalty_Time");

	m_penaltyPointTotal = 0;
	m_penaltyTimeRemain = m_penaltyTimeDuration;
}

void CState_Challenge_Pass_Play::OnUpdate(float timeDelta)
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

const DHOST_TYPE_BOOL CState_Challenge_Pass_Play::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_Pass::ConvertPacket_play_c2s_ballLerp(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		if (data->balllerp()->owner() == pChallengeData->m_userPlayerID)
		{
			int pointGain = kUINT32_INIT;

			if (data->balllerp()->target() == m_targetID)
			{
				switch (data->balllerp()->passtype())
				{
				case PASS_TYPE::overBad:
				case PASS_TYPE::overCommon:
				case PASS_TYPE::overGood:
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_Target_OverPass"));
					break;
				default:
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_Target_NormalPass"));
					break;
				}
			}
			else
			{
				switch (data->balllerp()->passtype())
				{
				case PASS_TYPE::overBad:
				case PASS_TYPE::overCommon:
				case PASS_TYPE::overGood:
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_NonTarget_OverPass"));
					break;
				default:
					pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_NonTarget_NormalPass"));
					break;
				}
			}

			pointGain += m_penaltyPointTotal;

			pChallengeData->m_point = max(pChallengeData->m_point + pointGain, 0);

			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_None, 0.0f);
		}
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Challenge_Pass_Play::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_BOOL retValue = CState_Challenge_Pass::ConvertPacket_play_c2s_ballSimulation(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		if (data->ballsimulation()->ballstate() == BALL_STATE::ballState_pickPass)
		{
			if (data->ballsimulation()->owner() == pChallengeData->m_userPlayerID)
			{
				m_targetID = pChallengeData->m_targets[rand() % pChallengeData->m_targets.size()];

				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, challenge_s2c_setTarget, message, send_data);
				send_data.add_targetid(m_targetID);
				STORE_FBPACKET(builder, message, send_data)

				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}
		}
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Challenge_Pass_Play::ConvertPacket_play_c2s_playerReceivePass(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerReceivePass_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	DHOST_TYPE_BOOL retValue = CState_Challenge_Pass::ConvertPacket_play_c2s_playerReceivePass(userid, peer, pPacket, recv_time);

	if (retValue == true)
	{
		auto* data = pPacket->GetData();

		if (data->playeraction()->id() == pChallengeData->m_userPlayerID)
		{
			int pointGain = kINT32_INIT;

			if (data->animid() == 8)
			{
				pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_Receive_OverPass"));
			}
			else
			{
				pointGain = static_cast<int>(m_pHost->GetBalanceValue("VPass_Point_Receive"));
			}

			pChallengeData->m_point = max(pChallengeData->m_point + pointGain, 0);

			SendMessagePoint(pChallengeData->m_point, pointGain, CHALLENGE_MESSAGE::challengeMessage_None, 0.0f);

			m_penaltyPointTotal = kINT32_INIT;
			m_penaltyTimeRemain = m_penaltyTimeDuration;
		}
	}

	return retValue;
}