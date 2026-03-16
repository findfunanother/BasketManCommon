#include "State_Challenge_Rebound_Init.h"
#include "BallController.h"

void CState_Challenge_Rebound_Init::OnEnter(void* pData)
{
	m_pHost->SetStageID(kSTAGE_TUTORIAL);

	pChallengeData->m_point = kINT32_INIT;

	float time = m_pHost->GetBalanceValue("VChallenge3_Time");

	m_pHost->SetGameTimeInit(time);

	
	CBallController* pBallController = m_pHost->BallControllerGet();
	if (pBallController != nullptr)
	{
		if (pBallController->GameBallDataGet())
			pBallController->GameBallDataGet()->ballState = F4PACKET::BALL_STATE::ballState_none;
	}
	

}

DHOST_TYPE_BOOL CState_Challenge_Rebound_Init::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2s_loadComplete)
		DEFAULT_CALL(CState_Challenge_Rebound)
	}
}

void CState_Challenge_Rebound_Init::OnLogin(int id)
{
	if (pChallengeData->m_userPlayerID == 0)
	{
		pChallengeData->m_userPlayerID = id;
	}
}

const DHOST_TYPE_BOOL CState_Challenge_Rebound_Init::ConvertPacket_system_c2s_loadComplete(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	DHOST_TYPE_USER_ID userID = m_pHost->UserGetID(peer);
	CREATE_BUILDER(builder)
	vector<TB::SVector3> vec_position;
	TB::SVector3 position1(-1.0f, 0.0f, 9.0f);
	vec_position.push_back(position1);
	TB::SVector3 position2(-7.0f, 0.0f, 8.0f);
	vec_position.push_back(position2);
	TB::SVector3 position3(1.0f, 0.0f, 9.0f);
	vec_position.push_back(position3);
	
	auto offset_vec_position = builder.CreateVectorOfStructs<TB::SVector3>(vec_position);
	CREATE_FBPACKET(builder, system_s2c_challengeStart, message, send_data)

	TB::SVector3 pPositionBall(kFLOAT_INIT, kFLOAT_INIT, kFLOAT_INIT);
	pPositionBall.mutate_x(-7.0f);
	pPositionBall.mutate_y(0.25f);
	pPositionBall.mutate_z(10.0f);

	TB::SVector3 pVelocityBall(kFLOAT_INIT, kFLOAT_INIT, kFLOAT_INIT);

	send_data.add_ballnumber(m_pHost->BallNumberGet());
	send_data.add_ownerid(0);
	send_data.add_positionball((const TB::SVector3*) & pPositionBall);
	send_data.add_velocityball((const TB::SVector3*) & pVelocityBall);
	
	send_data.add_positioncharacter(offset_vec_position);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	m_pHost->ChangeState(EHOST_STATE::CHALLENGE_READY, pChallengeData);

	m_pHost->SetLoadComplete(true);

	return true;
}

const DHOST_TYPE_BOOL CState_Challenge_Rebound_Init::ConvertPacket_system_c2s_readyToChallenge(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToChallenge_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	CState_Challenge_Rebound::ConvertPacket_system_c2s_readyToChallenge(userid, peer, pPacket, recv_time);

	// 로딩 시작 명령.
	{
		auto stage = m_pHost->GetStageID();
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_startLoad, message, send_data);
		send_data.add_result(F4PACKET::RESULT::success);
		send_data.add_stageid(stage);
		send_data.add_gamemode(F4PACKET::EGAME_MODE::eGameMode_challenge_rebound);
		send_data.add_isnormalgame(m_pHost->GetIsNormalGame());
		send_data.add_switchtoailevel(m_pHost->GetSwitchToAiLevel());
		send_data.add_handicaprulepoint(m_pHost->GetGameManager()->GetHandicapRulePoint());
		send_data.add_treatdelayoption(m_pHost->bTreatDelayMode);
		send_data.add_serverpossync(m_pHost->bServerSyncPos);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return true;
}

const DHOST_TYPE_BOOL CState_Challenge_Rebound_Init::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_challengeInfo, message, send_data);
	send_data.add_challengestep(2);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}
