#include "State_Load.h"

void CState_Load::OnEnter(void* pData)
{
	//! 리바운드 능력치 순으로 정렬
	m_pHost->SortCharacterInformation();
	//! 1번을 제외한 나머지 캐릭터들은 포지션별로 정렬
	m_pHost->SortCharacterInformationWithRole();

	//! 점프볼하는 캐릭터 확인용 로그
	if (m_pHost->GetCharacterManager() != nullptr)
	{
		std::vector<std::vector<CCharacter*>> vecTeamsCharacter;
		vecTeamsCharacter.clear();
		m_pHost->GetCharacterManager()->GetTeamsCharacterVector(vecTeamsCharacter);

		for (int i = 0; i < vecTeamsCharacter.size(); i++)
		{
			for (int j = 0; j < vecTeamsCharacter[i].size(); ++j)
			{
				CCharacter* pCharacter = vecTeamsCharacter[i][j];

				if (pCharacter != nullptr && pCharacter->GetCharacterInformation() != nullptr)
				{
					SPlayerInformationT* pCharacterInfo = pCharacter->GetCharacterInformation();

					F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pCharacterInfo->characterid / 100);

					string verify_log = "[JUMP_BALL_SORT]RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
						+ ", Team : " + std::to_string(pCharacterInfo->team)
						+ ", Name : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
						+ ", Role : " + F4PACKET::EnumNameROLE(pCharacterInfo->role)
						+ ", Rebound : " + std::to_string(pCharacterInfo->ability->rebound);
					m_pHost->ToLog(verify_log.c_str());
				}
				else
				{
					string verify_log = "[JUMP_BALL_SORT] pCharacter_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime());
					m_pHost->ToLog(verify_log.c_str());
				}
				
			}
		}
	}

	std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
	vecCharacters.clear();

	m_pHost->GetCharacterInformationVec(vecCharacters);

	// 플레이어들의 정보 전송.
	if (m_pHost->GetRedisLoadValue())	// Host to InGameServer
	{
		CREATE_BUILDER(builder)

		std::vector<flatbuffers::Offset<F4PACKET::SPlayerInformation>> vecchars;

		for (int i = 0; i < vecCharacters.size(); ++i)
		{
			auto packedcharecter = F4PACKET::SPlayerInformation::Pack(builder, vecCharacters[i]);

			vecchars.push_back(packedcharecter);
		}
		auto offvecchars = builder.CreateVector(vecchars);
		CREATE_FBPACKET(builder, system_s2c_playerInfo, message, databuilder)
		databuilder.add_playerlist(offvecchars);

		STORE_FBPACKET(builder, message, databuilder)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);


	}
	else // Host to LocalHost
	{
		for (int i = 0; i < vecCharacters.size(); ++i)
		{	
			CREATE_BUILDER(builder)

			auto offset_character = F4PACKET::SPlayerInformation::Pack(builder, vecCharacters[i]);
			CREATE_FBPACKET(builder, local_system_s2c_playerInfo, message, databuilder)

			databuilder.add_playerinfomation(offset_character);
			STORE_FBPACKET(builder,message,databuilder)
			//uint32_t size = builder.GetSize();
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}
	}

	// 로딩 시작 명령.
	{
		auto stage = m_pHost->GetStageID();

		if (m_pHost->GetStageID() == kSTAGE_INIT)
		{
			stage = Util::GetRandom(0, 5);
		}

		if (stage == 2)
		{
			m_pHost->CreateEnvironment();
		}

		//---  로그인한 유저 로딩 시작 시간 기록하기  ---- by steven
		std::vector<DHOST_TYPE_USER_ID> loingUser = m_pHost->GetLoginCompleteUser();
		for (int i = 0; i < loingUser.size(); i++)
		{
			CHostUserInfo* pUser = m_pHost->FindUser(loingUser[i]);
			if (pUser != nullptr)
			{
				pUser->SetLoadingStartime(m_pHost->GetRoomElapsedTime());

				string verify_log = "[LoadingStart] system_s2c_startLoad RoomElapsedTime RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", RoomID : "   + m_pHost->GetHostID()
					+ ", UserID : " + std::to_string(pUser->GetUserID());

				m_pHost->ToLog(verify_log.c_str());

			}
		}
		//----------------------------------------------------

		F4PACKET::EGAME_MODE game_mode = m_pHost->GetConvertGameMode();

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_startLoad, message, send_data);
		send_data.add_result(F4PACKET::RESULT::success);
		send_data.add_stageid(stage);
		send_data.add_gamemode(game_mode);
		send_data.add_isnormalgame(m_pHost->GetIsNormalGame());
		send_data.add_switchtoailevel(m_pHost->GetSwitchToAiLevel());
		send_data.add_handicaprulepoint(m_pHost->GetGameManager()->GetHandicapRulePoint());
		send_data.add_treatdelayoption(m_pHost->bTreatDelayMode);
		send_data.add_serverpossync(m_pHost->bServerSyncPos);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		//string log_message = "system_s2c_startLoad RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime());
		//m_pHost->ToLog(log_message.c_str());

	}
}

void CState_Load::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);
	
	//1. 로딩을 시작했으나 너무 오래 걸리는 유저 체크, 디버깅 모드에서는 빼자 
	std::vector<DHOST_TYPE_USER_ID> loingUser = m_pHost->GetLoginCompleteUser();
	for (int i = 0; i < loingUser.size(); i++)
	{
		CHostUserInfo* pUser = m_pHost->FindUser(loingUser[i]);
		if (pUser != nullptr)
		{
			DHOST_TYPE_FLOAT gap = m_pHost->GetRoomElapsedTime() - pUser->GetLoadingStartime();

			if (gap > kTIME_AFTER_USER_LOADING)
			{
				if (!m_pHost->IsLoadCompleteUser(loingUser[i]))
				{
					if (pUser->GetConnectState() != ECONNECT_STATE::DISCONNECT)
					{
#ifndef _DEBUG
						string invalid_buffer_log = "[SYSTEM] Loading is too long and kick!! RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + " , RoomID : " + m_pHost->GetHostID() + " , UserID : " + std::to_string(loingUser[i]);
						m_pHost->ToLog(invalid_buffer_log.c_str());
						m_pHost->BadConnectProcess(pUser->GetUserID(), false);
						break;
#endif
					}
				}
			}
		}
	}
	

	DHOST_TYPE_INT32 game_cap = m_pHost->GetGameCap();
	DHOST_TYPE_BOOL next_step = false;

	if (game_cap > kINT32_INIT)
	{
		DHOST_TYPE_UINT64 bad_connect_user_size = m_pHost->GetBadConnectUserSize();
		DHOST_TYPE_UINT64 load_complete_user_count = m_pHost->GetSendLoadCompleteUserSize();

		if (load_complete_user_count + bad_connect_user_size == game_cap)
		{
			string log_message = "Force change to the SCENE_START RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", LoadCompleteUserCount : " + std::to_string(load_complete_user_count) + ", BadConnectUserSize : " + std::to_string(bad_connect_user_size);
			m_pHost->ToLog(log_message.c_str());

			next_step = true;
		}
	}

	if (next_step)
	{
		m_pHost->SetLoadComplete(true);
		m_pHost->ChangeState(EHOST_STATE::SCENE_START);
	}
}

DHOST_TYPE_BOOL CState_Load::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID(); 
	
	switch ((PACKET_ID)pPacket->GetPacketID()) 
	{
		CASE_FB_CALL(system_c2s_loadComplete)

	default:
		return CState::OnPacket(pPacket, peer, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Load::ConvertPacket_system_c2s_loadComplete(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	// 이미 끊긴 유저라면 해당 패킷을 무시한다.
	if (m_pHost->CheckDisconnectUser(userid) == false)
	{
		m_pHost->PushSendLoadCompleteUser(userid);

		if (m_pHost->GetSendLoadCompleteUserSize() == m_pHost->GetGameCap())
		{
			m_pHost->SetLoadComplete(true);
			m_pHost->ChangeState(EHOST_STATE::SCENE_START);
		}
	}

	return true;
}
