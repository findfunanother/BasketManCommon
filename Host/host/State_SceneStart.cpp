#include "State_SceneStart.h"

void CState_SceneStart::OnEnter(void* pData)
{
	sceneIndex = m_pHost->GetSceneIndex();

	if (sceneIndex == HOST_OPTION_STAGE_RANDOM)
	{
		sceneIndex = kDEFAULT_SCENE_INDEX + rand() % HOST_OPTION_SCENE_RANDOM_COUNT;
	}

	//! 20230522 3�� �ʿ��� Ư�������� ����
	if (m_pHost->GetStageID() == 3)
	{
		sceneIndex = 1002;
	}

	{
		string str = "RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", stage : " + std::to_string(m_pHost->GetStageID()) + ", sceneIndex : " + std::to_string(sceneIndex);
		m_pHost->ToLog(str.c_str());
	}

	CREATE_BUILDER(builder);

	std::vector<int32_t> vecData;

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			//string str = "system_s2c_sceneEnter i : " + std::to_string(i) + ", j : " + std::to_string(j) + ", charSN : " + std::to_string(m_pHost->GetCharacterSN(i, j));
			//m_pHost->ToLog(str.c_str());

			vecData.push_back(m_pHost->GetCharacterSN(i, j));
		}
	}

	auto offsetcountinfo = builder.CreateVector(vecData);

	CREATE_FBPACKET(builder, system_s2c_sceneEnter, message, databuilder);

	databuilder.add_result(F4PACKET::RESULT::success);
	databuilder.add_sceneindex(sceneIndex);
	databuilder.add_timegame(static_cast<int>(m_pHost->GetGameTimeInit()));
	databuilder.add_playerlist(offsetcountinfo);

	STORE_FBPACKET(builder, message, databuilder);

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	timeElapsed = 0.0f;
}

void CState_SceneStart::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);
}

DHOST_TYPE_BOOL CState_SceneStart::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID(); 
	
	switch ((PACKET_ID)pPacket->GetPacketID()) 
	{
		CASE_FB_CALL(system_c2s_sceneEnd)

	default:
		return CState::OnPacket(pPacket, peer, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_SceneStart::ConvertPacket_system_c2s_sceneEnd(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_sceneEnd_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
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

	bool lineUp = true;

	string log_message = "ConvertPacket_system_c2s_sceneEnd RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", SceneIndex " + to_string(sceneIndex);
	m_pHost->ToLog(log_message.c_str());

	// sceneEnd 기반 전환에서는 항상 라인업 패킷을 먼저 보낸다.
	// (CState_LineUp::OnEnter 에서 pLineUp == true 일 때만 readyToLineUp 브로드캐스트)

	m_pHost->ChangeState(EHOST_STATE::LINE_UP, &lineUp);

	return true;
}


