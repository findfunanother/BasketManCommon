#include "State_Training.h"
#include "BallController.h"

void CState_Training::OnEnter(void* pData)
{
	m_pHost->SetStageID(kSTAGE_TUTORIAL);
}
void CState_Training::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
	if (eventName == "Event_FirstRimBound")
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_goalFail, message, send_data);
		send_data.add_ownerid(ownerID);
		send_data.add_shottype((F4PACKET::SHOT_TYPE)shotType);
		send_data.add_ballnumber(ballNumber);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
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
	else if (eventName == "Event_RightGoalIn")
	{
		m_pHost->SetBallNumberGoalIn(ballNumber);

		m_pHost->SendSyncInfo(SYNCINFO_TYPE::PICK);
	}
}

DHOST_TYPE_BOOL CState_Training::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2s_loadComplete)
		CASE_FB_CALL(system_c2s_login)
		CASE_FB_CALL(system_c2s_readyToLoad)
		CASE_FB_CALL(local_system_c2s_login)
		CASE_FB_CALL(system_c2s_exitGame)
		CASE_FB_CALL(system_c2s_readyToTraining)
		BASE_CALL()
	}
}
const DHOST_TYPE_BOOL CState_Training::ConvertPacket_system_c2s_loadComplete(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	bool exist = false;

	for (int i = 0; i < m_Readys.size(); ++i)
	{
		if (m_Readys[i] == userID)
		{
			exist = true;
		}
	}

	if (exist == false)
	{
		m_Readys.push_back(userID);
	}

	if (m_Readys.size() == m_pHost->UserCount())
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_trainingStart, message, send_data);
		
		TB::SVector3 pPositionBall(0.f,2.f,10.f);// = new SVector3();
		TB::SVector3 pVelocityBall(0.f,0.f,5.f);// = new SVector3();
		TB::SVector3 pPositionPlayer;// = new SVector3();

		send_data.add_ballnumber(m_pHost->BallNumberGet());
		send_data.add_positionball(&pPositionBall);
		send_data.add_velocityball(&pVelocityBall);
		send_data.add_positioncharacter(&pPositionPlayer);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	m_pHost->SetLoadComplete(true);

	return false;
}

const DHOST_TYPE_BOOL CState_Training::ConvertPacket_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	F4PACKET::RESULT result = F4PACKET::RESULT::fail;
	F4PACKET::EXIT_TYPE exitType = m_pHost->GetConvertExitType();

	auto& HostUserMap = m_pHost->GetHostUserMap();

	if (HostUserMap.empty() == false)
	{
		auto iter = HostUserMap.find(data->userid());

		if (iter != HostUserMap.end())
		{
			result = F4PACKET::RESULT::success;
			iter->second->SetLogin(true);
		}
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_login, message, send_data);
	send_data.add_result(result);
	send_data.add_exittype(exitType);
	send_data.add_logintype(F4PACKET::LOGIN_TYPE::normal);
	send_data.add_remaingametime(m_pHost->GetGameTime());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, data->userid());

	return true;
}

const DHOST_TYPE_BOOL CState_Training::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	F4PACKET::EGAME_MODE gameMode = m_pHost->GetConvertGameMode();

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_trainingInfo, message, send_data);
	send_data.add_gamemode(gameMode);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Training::ConvertPacket_system_c2s_readyToTraining(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToTraining_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
			STORE_FBPACKET(builder, message, databuilder)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}
	}

	// 로딩 시작 명령.
	{
		auto stage = m_pHost->GetStageID();
		F4PACKET::EGAME_MODE gameMode = m_pHost->GetConvertGameMode();

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_startLoad, message, send_data);
		send_data.add_result(RESULT::success);
		send_data.add_stageid(stage);
		send_data.add_gamemode(gameMode);
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

const DHOST_TYPE_BOOL CState_Training::ConvertPacket_system_c2s_exitGame(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_exitGame_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	m_pHost->RedisSaveMatchResult();

	return true;
}

const DHOST_TYPE_BOOL CState_Training::ConvertPacket_local_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::local_system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	if (data->gamelogin() == nullptr || data->str_buildinfo() == nullptr || data->gamelogin()->username() == nullptr || data->gamelogin()->handedness() == nullptr || data->gamelogin()->ability() == nullptr || data->gamelogin()->abilitymodification() == nullptr || data->gamelogin()->skills() == nullptr || data->gamelogin()->signatures() == nullptr || data->gamelogin()->medals() == nullptr || data->gamelogin()->emojis() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER str_buildinfo is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	SLocalGameLoginT Local_Login_data;
	data->gamelogin()->UnPackTo(&Local_Login_data);
	SLocalGameLoginT* pGameLogin = &Local_Login_data;

	if (pGameLogin == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pGameLogin is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	pGameLogin->userid = (userid);

	if (pGameLogin->characterid != 0)
	{
		// 플레이어 정보 세팅 및 추가.
		F4PACKET::SPlayerInformationT* playerInformation = new F4PACKET::SPlayerInformationT();

		playerInformation->userid = (userid);
		playerInformation->team = (data->gamelogin()->team());
		playerInformation->id = (m_pHost->GetLastCharacterSN());
		playerInformation->characterid = (data->gamelogin()->characterid());
		playerInformation->name = (data->gamelogin()->username()->str());
		playerInformation->radius = (0.35f);

		playerInformation->height = data->gamelogin()->height();
		playerInformation->winspan = data->gamelogin()->winspan();
		playerInformation->gender = data->gamelogin()->gender();
		playerInformation->handedness = data->gamelogin()->handedness()->str();

		pGameLogin->ability.swap(playerInformation->ability);
		pGameLogin->abilitymodification.swap(playerInformation->abilitymodification);

		playerInformation->ceremony1 = (data->gamelogin()->ceremony1());
		playerInformation->ceremony2 = (data->gamelogin()->ceremony2());
		playerInformation->ceremony3 = (data->gamelogin()->ceremony3());
		playerInformation->ceremonywin = (data->gamelogin()->ceremonywin());
		playerInformation->ceremonylose = (data->gamelogin()->ceremonylose());
		playerInformation->ceremonydraw = (data->gamelogin()->ceremonydraw());
		playerInformation->ceremonymvp = (data->gamelogin()->ceremonymvp());

		if (data->gamelogin()->team() == 0)
		{
			playerInformation->ailevel = 0; // m_pHost->GetHomeAiLevel();
		}
		else if (data->gamelogin()->team() == 1)
		{
			playerInformation->ailevel = 0; // (m_pHost->GetAwayAiLevel());
		}

		playerInformation->role = (data->gamelogin()->role());

		for (int i = 0; i < pGameLogin->skills.size(); i++)
		{
			F4PACKET::SSkillInfo pSkillInfo(pGameLogin->skills[i].skillslot(),
				pGameLogin->skills[i].index(),
				pGameLogin->skills[i].level(), pGameLogin->skills[i].forceawaken());// = playerInformation->add_skills();
			playerInformation->skills.push_back(pSkillInfo);
			//pSkillInfo->set_skillslot(pGameLogin->skills().Get(i).skillslot());
			//pSkillInfo->set_index(pGameLogin->skills().Get(i).index());
			//pSkillInfo->set_level(pGameLogin->skills().Get(i).level());
		}

		for (int i = 0; i < pGameLogin->signatures.size(); i++)
		{
			playerInformation->signatures.push_back(pGameLogin->signatures[i]);
		}

		for (int i = 0; i < pGameLogin->medals.size(); i++)
		{
			F4PACKET::SMedalInfo medalInfo(pGameLogin->medals[i].index(),
				pGameLogin->medals[i].grade(),
				pGameLogin->medals[i].level(),
				pGameLogin->medals[i].value(), 
				pGameLogin->medals[i].core());

			playerInformation->medals.push_back(medalInfo);
		}

		m_pHost->PushAiHostCandidateUser(playerInformation->userid, playerInformation->id);

		if (m_pHost->GetCharacterManager()->CreateCharacter(playerInformation))
		{
			m_pHost->SetScore(playerInformation->team, kINT32_INIT);
			m_pHost->AddLastCharacterSN();
		}
		else
		{
			string log_message = "CState_Init::ConvertPacket_local_system_c2s_login CreateCharcter fail pCharacter is null UserID : " + to_string(userid);
			m_pHost->ToLog(log_message.c_str());
		}


		CREATE_BUILDER(builder)
			auto offset_gamelogin = F4PACKET::SLocalGameLogin::Pack(builder, pGameLogin);
		CREATE_FBPACKET(builder, local_system_s2c_login, message, databuilder )

			//auto& send_data = message.GetData();
			databuilder.add_result(F4PACKET::RESULT::success);
		//send_data->set_result(RESULT::success);
		databuilder.add_gamelogin(offset_gamelogin);// .gamelogin = set_allocated_gamelogin(pGameLogin);
		STORE_FBPACKET(builder, message, databuilder)

			//delete(playerInformation);

			string log_message = "local_system_c2s_login UserName : " + data->gamelogin()->username()->str() + ", Position : " + F4PACKET::EnumNameROLE(playerInformation->role) + ", TotalLoginUserCount : " + to_string(m_pHost->GetHostUserMap().size());
		m_pHost->ToLog(log_message.c_str());

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return true;
}
