#include "State_Tutorial.h"
#include "BallController.h"

void CState_Tutorial::OnEnter(void* pData)
{
	m_pHost->SetStageID(kSTAGE_TUTORIAL);
}

void CState_Tutorial::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{
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

DHOST_TYPE_BOOL CState_Tutorial::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2s_loadComplete)
		CASE_FB_CALL(system_c2s_login)
		CASE_FB_CALL(system_c2s_readyToLoad)
		CASE_FB_CALL(local_system_c2s_login) // 튜토일얼에서는 로컬시스템 패킷을 쓰는구나, 유의할 것 
		CASE_FB_CALL(system_c2s_exitGame)
		CASE_FB_CALL(system_c2s_readyToTutorial)
		CASE_FB_CALL(system_c2s_tutorialResult)
		BASE_CALL()
	}
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_loadComplete(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
		CREATE_FBPACKET(builder, system_s2c_tutorialStart, message, send_data);
		send_data.add_ballnumber(m_pHost->BallNumberGet());
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	m_pHost->SetLoadComplete(true);

	return false;
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
	send_data.add_logintype(F4PACKET::LOGIN_TYPE::normal);
	send_data.add_exittype(exitType);
	send_data.add_remaingametime(m_pHost->GetGameTime());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, data->userid());

	return true;
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
	CREATE_FBPACKET(builder, system_s2c_tutorialInfo, message, send_data);
	send_data.add_gamemode(gameMode);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_readyToTutorial(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToTutorial_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_exitGame(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_exitGame_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	m_pHost->RedisSaveMatchResult();

	return true;
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_system_c2s_tutorialResult(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_tutorialResult_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	int tutorial_result = data->result() == true ? 1 : 0;

	// 기본 튜토리얼은 성공/실패 판단이 없다고 함. 그냥 무조건 성공으로 보내자
	if (m_pHost->GetModeType() == EMODE_TYPE::TUTORIAL_BASIC)
	{
		tutorial_result = 1;

		string log_message = "ConvertPacket_system_c2s_tutorialResult tutorial_result : " + to_string(tutorial_result);
		m_pHost->ToLog(log_message.c_str());
	}

	m_pHost->RedisSaveMatchResult(tutorial_result);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_tutorialResult, message, send_data);
	send_data.add_result(data->result());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState_Tutorial::ConvertPacket_local_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::local_system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
	if (pGameLogin->characterid <= 0)
		return false;

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

	//for (int i = 0; i < pGameLogin->skills.size(); i++)
	//{
	//	F4PACKET::SSkillInfo pSkillInfo(pGameLogin->skills[i].skillslot(),
	//	pGameLogin->skills[i].index(),
	//	pGameLogin->skills[i].level());
	//	playerInformation->skills.push_back(pSkillInfo);
	//}

	// 2023-05-12 스킬이 필요한 튜토리얼애다가 스킬 추가 (차정철)
	F4PACKET::EGAME_MODE game_mode = m_pHost->GetConvertGameMode();
	switch (game_mode)
	{
		case F4PACKET::EGAME_MODE::eGameMode_tutorial_divingCatch:
			for (int i = 0; i < pGameLogin->skills.size(); i++)
			{
				if (pGameLogin->skills[i].index() != F4PACKET::SKILL_INDEX::skill_divingCatch) continue;
				F4PACKET::SSkillInfo pSkillInfo(pGameLogin->skills[i].skillslot(),
				F4PACKET::SKILL_INDEX::skill_divingCatch,
				pGameLogin->skills[i].level(), pGameLogin->skills[i].forceawaken());
				playerInformation->skills.push_back(pSkillInfo);
			}
			break;

		case F4PACKET::EGAME_MODE::eGameMode_tutorial_boxout:
			for (int i = 0; i < pGameLogin->skills.size(); i++)
			{
				if (pGameLogin->skills[i].index() != F4PACKET::SKILL_INDEX::skill_boxOut) continue;
				F4PACKET::SSkillInfo pSkillInfo(pGameLogin->skills[i].skillslot(),
				F4PACKET::SKILL_INDEX::skill_boxOut,
				pGameLogin->skills[i].level(), pGameLogin->skills[i].forceawaken());
				playerInformation->skills.push_back(pSkillInfo);
			}
			break;
	}
	

	for (int i = 0; i < pGameLogin->signatures.size(); i++)
	{
		bool isSameKindSignatureCategory = false;
		int signatureCategory = pGameLogin->signatures[i] / 1000;
		for (int k = 0; k < i; k++)
		{
			int compareSignatureCategory = pGameLogin->signatures[k] / 1000;
			if (signatureCategory == compareSignatureCategory)
			{
				isSameKindSignatureCategory = true;
				break;
			}
		}

		if (isSameKindSignatureCategory == false)
			playerInformation->signatures.push_back(pGameLogin->signatures[i]);
	}

	//for (int i = 0; i < pGameLogin->medals.size(); i++)
	//{
	//	F4PACKET::SMedalInfo medalInfo
	//	(
	//		pGameLogin->medals[i].index(),
	//		pGameLogin->medals[i].grade(),
	//		pGameLogin->medals[i].level(),
	//		pGameLogin->medals[i].value()
	//	);

	//	playerInformation->medals.push_back(medalInfo);
	//}

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
	CREATE_FBPACKET(builder, local_system_s2c_login, message, databuilder)

	databuilder.add_result(F4PACKET::RESULT::success);
	databuilder.add_gamelogin(offset_gamelogin);
	STORE_FBPACKET(builder, message, databuilder)

	string log_message = "local_system_c2s_login UserName : " + data->gamelogin()->username()->str() + ", Position : " + F4PACKET::EnumNameROLE(playerInformation->role) + ", TotalLoginUserCount : " + to_string(m_pHost->GetHostUserMap().size());
	m_pHost->ToLog(log_message.c_str());

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}