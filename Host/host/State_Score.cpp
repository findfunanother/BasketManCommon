#include "State_Score.h"
#include "BallController.h"

void CState_Score::OnEnter(void* pData)
{
	m_CheckReplay = m_pHost->CheckOption(HOST_OPTION_NO_CHECK_REPLAY);

	DHOST_TYPE_CHARACTER_SN scorer = static_cast<DHOST_TYPE_CHARACTER_SN>(*(int*)pData);

	solo = kCHARACTER_SN_INIT;
	duo = kCHARACTER_SN_INIT;
	
	//isSendCoupleCeremony = false;
	coupleCeremony.clear();
	//coupleCeremonyCandidate.clear();

	timeCeremony = 3.0f;
	timeElapsed = 0.0f;
	sendStopPacket = false;
	m_replay = false;
	m_BuzzerBeater = m_pHost->GetBuzzerBeater();

	float currentTime = m_pHost->GetGameTime();

	m_pHost->SetScoredTime(currentTime);

	//! 버저비터 연출이 필요한 경우라면
	if (m_BuzzerBeater)
	{
		/*
		timeCeremony = serverWaitTimeAfterGoalIn;
		timeWait = (m_pHost->GetRoomElapsedTime() - m_pHost->GetShotRoomElapsedTime() + startAnimationTime) * divideBuzzerBeaterHighlight + scaleTime;
		m_replay = true;
		*/

		timeCeremony = serverWaitTimeAfterGoalIn;

		float cameraOneTwoDuration = (startAnimationTime + balloutElapsedTime) * 2.0f;
		float cameraLastOneDuration = (m_pHost->GetRoomElapsedTime() - m_pHost->GetShotRoomElapsedTime() + startAnimationTime) + scaleTimeLastCamera;
		timeWait = cameraOneTwoDuration + cameraLastOneDuration;

		m_replay = true;
	}
	else
	{
		switch (m_pHost->GetModeType())
		{
			case EMODE_TYPE::CONTINUOUS:
			{
				timeWait = 1.0f;
			}
			break;
			default:
			{
				// 주목할만한 선수 데이터를 보낸적이 없을 경우만 해당
				if (m_pHost->GetNoTableCharacterSend() == false)
				{
					// condition 1. 특정 시간 이후에 득점을 하였는가?
					if (m_pHost->GetGameTime() < m_pHost->GetBalanceValue("VFocusTime"))
					{
						// condition 2. 주목할만한 캐릭터인가?
						solo = m_pHost->CheckNoTableCharacterSoloCondition(scorer);

						if (solo != kCHARACTER_SN_INIT)
						{
							// condition 3. 상대팀에도 주목할만한 선수가 있는가?
							duo = m_pHost->CheckNoTableCharacterDuoCondition(solo);
							m_pHost->SetNoTableCharacterSend(true);

							timeWait = 6.5f;
						}
					}
				}

				// 주목할선수를 연출하는 득점에서는 리플레이를 스킵한다.
				if (solo == kCHARACTER_SN_INIT)
				{
					if (!m_CheckReplay)
					{
						m_replay = true;
						m_pHost->SetReplayTime(currentTime);
						timeWait = 8.0f;
					}
					else
					{
						if (m_pHost->CheckReplay())
						{
							m_replay = true;
							m_pHost->SetReplayTime(currentTime);
							timeWait = 8.0f;
						}
						else
						{
							timeWait = 0.5f;
						}
					}
				}
			}
			break;
		}
	}
}

void CState_Score::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);

	timeElapsed += timeDelta;

	if (sendStopPacket == false)
	{
		if (timeElapsed >= timeCeremony)
		{
			sendStopPacket = true;

			if (m_replay == false)
			{
				if (solo == kCHARACTER_SN_INIT)
				{
					CREATE_BUILDER(builder)
					CREATE_FBPACKET(builder, play_s2c_playerStop, message, send_data);
					send_data.add_stop(true);
					STORE_FBPACKET(builder, message, send_data)

					m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
				}
				else
				{
					CREATE_BUILDER(builder)
					CREATE_FBPACKET(builder, system_s2c_manOfWatch, message, send_data);
					

					if (duo == kCHARACTER_SN_INIT)
					{
						send_data.add_scenetype(F4PACKET::POST_GOAL_SCENE_TYPE::postGoalSceneType_solo);
					}
					else
					{
						send_data.add_scenetype(F4PACKET::POST_GOAL_SCENE_TYPE::postGoalSceneType_duo);
					}

					send_data.add_sceneid1(static_cast<DHOST_TYPE_INT32>(solo));
					send_data.add_sceneid2(static_cast<DHOST_TYPE_INT32>(duo));
					STORE_FBPACKET(builder, message, send_data)

					m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
				}
			}
			else
			{
				CREATE_BUILDER(builder)
				CREATE_FBPACKET(builder, system_s2c_replayStart, message, send_data);
				send_data.add_duration(timeWait);
				send_data.add_buzzerbeater(m_BuzzerBeater);
				STORE_FBPACKET(builder, message, send_data)

				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
			}
		}
	}
	else
	{
		if (timeElapsed >= timeCeremony + timeWait)
		{
			if (m_pHost->GetGameTime() < 1.0f || m_pHost->GetBuzzerBeater())
			{
				m_pHost->ChangeState(EHOST_STATE::END);
			}
			else
			{
				m_pHost->ChangeState(EHOST_STATE::READY);
			}
		}
	}
}

DHOST_TYPE_BOOL CState_Score::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(play_c2s_playerEmotion);
		CASE_FB_CALL(play_c2s_playerCeremonyEnd);
		CASE_FB_CALL(play_c2s_playerCoupleCeremony);

		default:
			return CState::OnPacket(pPacket, peer, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_playerEmotion(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerEmotion_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	const SPlayerAction* pData = data->playeraction();

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerEmotion, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	F4PACKET::play_c2s_ballLerp_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballLerp_dataT* pBallLerpData = &dataT;

	if (pBallLerpData == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallLerpData is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	pBallLerpData->balllerp->mutate_success(false);
	pBallLerpData->balllerp->mutate_ballnumber(m_pHost->BallNumberGet());

	F4PACKET::SPlayerInformationT* pPlayer = m_pHost->FindCharacterInformation(pBallLerpData->balllerp->owner());

	if (pBallLerpData->balllerp->success() == true)
	{
		CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

		pGameBallData->ballNumber = m_pHost->BallNumberGet();
		pGameBallData->ballState = pBallLerpData->balllerp->ballstate();
		pGameBallData->shotType = SHOT_TYPE::shotType_none;
		pGameBallData->owner = pBallLerpData->balllerp->owner();
		pGameBallData->point = 0;
		pGameBallData->team = pPlayer->team;

		m_pHost->BallControllerGet()->AddBallData(pGameBallData, pBallLerpData->balllerp->time());
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballLerp, message, send_data);
	send_data.add_balllerp(pBallLerpData->balllerp.get());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_ballShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	SBallShotT data_ballshot;
	data->ballshot()->UnPackTo(&data_ballshot);
	
	SBallShotT* pBallShot = &data_ballshot;

	DHOST_TYPE_BOOL bAssist = false;

	ECHARACTER_RECORD_TYPE recordType = ECHARACTER_RECORD_TYPE::NONE;

	string log_message = "";

	pBallShot->ballnumber = (m_pHost->BallNumberGet());

	CREATE_BUILDER(builder);
	auto offset_ballshot_data = SBallShot::Pack(builder, pBallShot);
	CREATE_FBPACKET(builder, play_s2c_ballShot, message, databuilder);
	databuilder.add_ballshot(offset_ballshot_data);
	STORE_FBPACKET(builder, message, databuilder);
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	F4PACKET::play_c2s_ballRebound_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballRebound_dataT* pBallSimulation = &dataT;

	if (pBallSimulation == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallSimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (pBallSimulation->ballsimulation->success() == true)
	{
		pBallSimulation->ballsimulation->mutate_success(false);
	}

	F4PACKET::SPlayerInformationT* pPlayer = m_pHost->FindCharacterInformation(pBallSimulation->ballsimulation->owner());

	if (pPlayer != nullptr)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_ballRebound, message, send_data);
		send_data.add_ballsimulation(pBallSimulation->ballsimulation.get());
		STORE_FBPACKET(builder, message, send_data)
			
		return true;
	}

	return false;
}


const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	F4PACKET::play_c2s_ballSimulation_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballSimulation_dataT* pBallSimulation = &dataT;

	if (pBallSimulation == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallSimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (pBallSimulation->ballsimulation->success() == true)
	{
		pBallSimulation->ballsimulation->mutate_success(false);
	}

	F4PACKET::SPlayerInformationT* pPlayer = m_pHost->FindCharacterInformation(pBallSimulation->ballsimulation->owner());

	if (pPlayer != nullptr)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_ballSimulation, message, send_data);
		send_data.add_ballsimulation(pBallSimulation->ballsimulation.get());
		STORE_FBPACKET(builder, message, send_data)

		return true;
	}

	return false;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_playerCeremonyEnd(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCeremonyEnd_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
	//!

	bool retValue = false;
	
	for (vector<DHOST_TYPE_CHARACTER_SN>::iterator iter = coupleCeremony.begin(); iter != coupleCeremony.end(); ++iter)
	{
		if (*iter == data->id())
		{
			coupleCeremony.erase(iter);

			retValue = true;

			break;
		}
	}

	return retValue;;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_playerCoupleCeremony(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCoupleCeremony_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->position1() == nullptr || data->position2() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER position1 is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;
	bool find = false;

	for (vector<DHOST_TYPE_CHARACTER_SN>::iterator iter = coupleCeremony.begin(); iter != coupleCeremony.end(); ++iter)
	{
		if (*iter == data->id1() || *iter == data->id2())
		{
			find = true;

			break;
		}
	}

	if (find == false)
	{
		map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();
		for (auto it = characters.begin(); it != characters.end(); ++it)
		{
			CCharacter* character = it->second;
			if (character)
				character->OnCharacterPlayerCoupleCeremony(pPacket, recv_time);
		}

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerCoupleCeremony, message, send_data);

		int id = rand() % 4 + 1;

		send_data.add_ceremonyid(id);
		send_data.add_id1(data->id1());
		send_data.add_id2(data->id2());
		send_data.add_position1(data->position1());
		send_data.add_position2(data->position2());
		send_data.add_rotation1(data->rotation1());
		send_data.add_rotation2(data->rotation2());

		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		coupleCeremony.push_back(data->id1());
		coupleCeremony.push_back(data->id2());

		retValue = true;
	}

	return retValue;;
}


const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_playerMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;

	if (sendStopPacket == false)
	{
		bool find = false;

		for (int i = 0; i < coupleCeremony.size(); i++)
		{
			if (coupleCeremony[i] == data->playeraction()->id())
			{
				find = true;

				break;
			}
		}

		if (find == false)
		{
			retValue = CState::ConvertPacket_play_c2s_playerMove(userid, peer, pPacket, recv_time);
		}
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Score::ConvertPacket_play_c2s_playerStand(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStand_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;

	if (sendStopPacket == false)
	{
		bool find = false;

		for (int i = 0; i < coupleCeremony.size(); i++)
		{
			if (coupleCeremony[i] == data->playeraction()->id())
			{
				find = true;

				break;
			}
		}

		if (find == false)
		{
			retValue = CState::ConvertPacket_play_c2s_playerStand(userid, peer, pPacket, recv_time);
		}
	}

	return retValue;
}