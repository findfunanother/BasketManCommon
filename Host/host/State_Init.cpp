#include "State_Init.h"
#include "HostUserInfo.h"
#include "VerifyManager.h"

void CState_Init::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);

	if (m_pHost->CheckOption(HOST_OPTION_NO_REDIS))
	{
		return;
	}

	DHOST_TYPE_INT32 game_cap = m_pHost->GetGameCap();

	DHOST_TYPE_BOOL next_step = false;

	DHOST_TYPE_UINT64 bad_connect_user_size = m_pHost->GetBadConnectUserSize();
	DHOST_TYPE_UINT64 login_user_count = m_pHost->GetLoginCompleteUserSize();

	if (game_cap > kINT32_INIT)
	{
		if (login_user_count + bad_connect_user_size == game_cap)
		{
			string log_message = "Force change to the LOAD RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserLoginCount : " + std::to_string(login_user_count) + ", BadConnectUserSize : " + std::to_string(bad_connect_user_size);
			m_pHost->ToLog(log_message.c_str());

			next_step = true;
		}
		else
		if ((login_user_count + bad_connect_user_size) > game_cap)
		{
			string log_message = "ChangeState(EHOST_STATE::LOAD) is Faied, login_user_count : " + std::to_string(login_user_count);
				+ "  RoomID : " + m_pHost->GetHostID()
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", bad_connect_user_size : " + std::to_string(bad_connect_user_size);
			m_pHost->ToLog(log_message.c_str());
		}
	}

	// 레디스 데이터를 로딩한 이후에도 유저가 접속을 못한 경우라면 방이 파괴되어야 한다.
	if (m_pHost->GetRedisLoadRoomElapsedTime() > kFLOAT_INIT)
	{
		if (m_pHost->GetRoomElapsedTime() - m_pHost->GetRedisLoadRoomElapsedTime() > kTIME_AFTER_HOST_CREATION && login_user_count == kINT32_INIT)
		{
			string log_message = "ExpiredRoom because user's cannot connect";
			m_pHost->ToLog(log_message.c_str());

			m_pHost->ForciblyExpireRoom();
		}
	}
		
	if (next_step)
	{
		m_pHost->ChangeState(EHOST_STATE::LOAD);
	}
}

DHOST_TYPE_BOOL CState_Init::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2s_login)
		CASE_FB_CALL(system_c2s_readyToLoad)
		CASE_FB_CALL(local_system_c2s_login)
		CASE_FB_CALL(local_system_c2s_mixPlayer)

		default:
			return CState::OnPacket(pPacket, peer, recv_time);
	}
}

const DHOST_TYPE_BOOL CState_Init::ConvertPacket_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
	F4PACKET::LOGIN_TYPE type = F4PACKET::LOGIN_TYPE::none;
	F4PACKET::EXIT_TYPE exitType = m_pHost->GetConvertExitType();

	auto& HostUserMap = m_pHost->GetHostUserMap();

	if (HostUserMap.empty() == false)
	{
		auto iter = HostUserMap.find(data->userid());

		if (iter != HostUserMap.end())
		{
			result = F4PACKET::RESULT::success;
			type = F4PACKET::LOGIN_TYPE::normal;

			iter->second->SetLogin(true);
		}
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_login, message, send_data);
	send_data.add_result(result);
	send_data.add_logintype(type);
	send_data.add_exittype(exitType);
	send_data.add_remaingametime(m_pHost->GetGameTime());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, data->userid());

	m_pHost->PushLoginCompleteUser(userid);

	string log_message = "system_c2s_login RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", result : " + F4PACKET::EnumNameRESULT(result) + ", GetUserLoginCount : " + to_string(m_pHost->GetLoginCompleteUserSize());
	m_pHost->ToLog(log_message.c_str());

	bool redisReady = m_pHost->CheckOption(HOST_OPTION_NO_REDIS) == true ? true : m_pHost->GetRedisLoadValue();

	if (m_pHost->GetLoginCompleteUserSize() == m_pHost->GetGameCap() && redisReady == true)
	{
		m_pHost->ChangeState(EHOST_STATE::LOAD);
	}

	return true;
}
// 재접속, 로컬모드를 위해서 만든 것 같다. 
const DHOST_TYPE_BOOL CState_Init::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	m_pHost->ChangeState(EHOST_STATE::LOAD);

	return true;
}

const DHOST_TYPE_BOOL CState_Init::ConvertPacket_local_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::local_system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	bool login_success = true;

	string log_message = "local_system_c2s_login UserID : " + std::to_string(userid);
	m_pHost->ToLog(log_message.c_str());

	m_pHost->PushLoginCompleteUser(userid);

	SLocalGameLoginT Local_Login_data;
	data->gamelogin()->UnPackTo(&Local_Login_data);
	SLocalGameLoginT* pGameLogin = &Local_Login_data;

	if (pGameLogin == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pGameLogin is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (m_pHost->GetLoginCompleteUserSize() == 1)
	{
		m_ClientBuildData = data->str_buildinfo()->str();
	}
	else
	{
		if (m_ClientBuildData.compare(data->str_buildinfo()->str()) != 0)
		{
			login_success = false;
		}
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
		playerInformation->ailevel = (data->gamelogin()->ailevel());

		if (m_pHost->GetModeType() == EMODE_TYPE::TRIO || ( m_pHost->GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED && pGameLogin->teamcontrolluser))
		{
			playerInformation->ailevel = kAI_DEFAULT_LEVEL;
		}

		playerInformation->passivetype = data->gamelogin()->passivetype();

		string log_message = "local_system_c2s_login CharacterSN : " + std::to_string(playerInformation->id) + ", AiLevel : " + std::to_string(playerInformation->ailevel);
		m_pHost->ToLog(log_message.c_str());

		playerInformation->role = (data->gamelogin()->role());

		for (int i = 0; i < pGameLogin->skills.size(); i++)
		{
			F4PACKET::SSkillInfo skillInfo(pGameLogin->skills[i].skillslot(), pGameLogin->skills[i].index(), 
				pGameLogin->skills[i].level(), pGameLogin->skills[i].forceawaken());
			playerInformation->skills.push_back(skillInfo);
		}

		for (int i = 0; i < pGameLogin->signatures.size(); i++)
		{
			playerInformation->signatures.push_back(pGameLogin->signatures[i]);
		}

		for (int i = 0; i < pGameLogin->medals.size(); i++)
		{
			F4PACKET::SMedalInfo medalInfo(pGameLogin->medals[i].index(), pGameLogin->medals[i].grade(), pGameLogin->medals[i].level(), pGameLogin->medals[i].value(), pGameLogin->medals[i].core());
			playerInformation->medals.push_back(medalInfo);
		}

		for (int i = 0; i < pGameLogin->emojis.size(); i++)
		{
			F4PACKET::SEmojiInfo emojiInfo(pGameLogin->emojis[i].slot(), pGameLogin->emojis[i].index());
			playerInformation->emojis.push_back(emojiInfo);
		}

		for (int i = 0; i < pGameLogin->potentials.size(); i++)
		{
			F4PACKET::SPotentialInfo potentialInfo(pGameLogin->potentials[i].potentialindex()
				, pGameLogin->potentials[i].potentiallevel()
				, pGameLogin->potentials[i].potentialvalue()
				, pGameLogin->potentials[i].potentialawaken()
				, pGameLogin->potentials[i].potentialbloomratelevel()
				, pGameLogin->potentials[i].potentialbloomratevalue()
				, pGameLogin->potentials[i].potentialbloombufflevel()
				, pGameLogin->potentials[i].potentialbloombuffvalue());
			playerInformation->potentials.push_back(potentialInfo);
		}


		for (int i = 0; i < pGameLogin->jukebox.size(); i++)
		{
			F4PACKET::SJukeBoxlInfo jukeBoxInfo(pGameLogin->jukebox[i].musicid());
			playerInformation->jukebox.push_back(jukeBoxInfo);
		}

		m_pHost->PushAiHostCandidateUser(playerInformation->userid, playerInformation->id);

		playerInformation->burstvalue = pGameLogin->burstvalue;

		playerInformation->specialcharacterlevel = pGameLogin->specialcharlevel;

		// 팀 능력치 넣기 ( 2025-05-15 ) 
		pGameLogin->teamattributes.swap(playerInformation->teamattributesinfo);

		playerInformation->teamcontrolluser = pGameLogin->teamcontrolluser;

		playerInformation->autoplayuser = pGameLogin->autoplayuser;

		playerInformation->effectpassiveid = pGameLogin->effectpassiveid;

		if (m_pHost->GetCharacterManager()->CreateCharacter(playerInformation, 0, m_pHost->GetVerifyManager()->GetCharacterIndex(playerInformation) ))
		{
			string log_message = "CreateCharacter CharacterID : " + std::to_string(playerInformation->id) + ", Name : " + playerInformation->name.c_str();
			m_pHost->ToLog(log_message.c_str());

			m_pHost->SetScore(playerInformation->team, 0);

			m_pHost->AddLastCharacterSN();
		}
		else
		{
			string log_message = "local_system_c2s_login CreateCharcter fail pCharacter is null UserID : " + to_string(userid);
			m_pHost->ToLog(log_message.c_str());
		}

		m_pHost->SetScore(playerInformation->team, pGameLogin->handicaprulepoint); 
		m_pHost->GetGameManager()->SetHandicapRulePoint(pGameLogin->handicaprulepoint);

		CREATE_BUILDER(builder)
		auto offset_gamelogin = F4PACKET::SLocalGameLogin::Pack(builder, pGameLogin);
		CREATE_FBPACKET(builder, local_system_s2c_login, message, databuilder)
		databuilder.add_result(login_success ? F4PACKET::RESULT::success : F4PACKET::RESULT::fail);
		databuilder.add_gamelogin(offset_gamelogin);
		STORE_FBPACKET(builder, message, databuilder)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		{
			builder.Clear();

			std::vector<const F4PACKET::SLocalGameTeamRoleCount*> vecrolecounter;
			std::vector< flatbuffers::Offset<F4PACKET::SLocalGameTeamRoleCountInfo>> veccounterinfo;

			for (int team = 0; team < 3; ++team)
			{
				F4PACKET::SLocalGameTeamRoleCountInfoT countinfo;

				countinfo.team = team;

				for (int role = 0; role <= (int)ROLE::MAX; ++role)
				{
					auto rolecnt = m_pHost->GetTeamRoleCount(team, (ROLE)role);
					F4PACKET::SLocalGameTeamRoleCount rolecount((F4PACKET::ROLE)role, rolecnt);
					countinfo.sinfo.push_back(rolecount);
				}

				auto vectorsinfo = builder.CreateVector(vecrolecounter);
				auto offsetinfo = F4PACKET::SLocalGameTeamRoleCountInfo::Pack(builder, &countinfo);

				veccounterinfo.push_back(offsetinfo);
			}

			auto offsetcountinfo = builder.CreateVector(veccounterinfo);
			CREATE_FBPACKET(builder, local_system_s2c_loginTeamInfo, second_message, databuilder);
			databuilder.add_sinfo(offsetcountinfo);
			STORE_FBPACKET(builder, second_message, databuilder);
			m_pHost->BroadcastPacket(second_message, kUSER_ID_INIT);
		}
	}

	return true;
}

const DHOST_TYPE_BOOL CState_Init::ConvertPacket_local_system_c2s_mixPlayer(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::local_system_c2s_mixPlayer_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	m_pHost->MixCharacterTeam();
	CREATE_BUILDER(builder)

	CREATE_FBPACKET(builder, local_system_s2c_mixPlayer, message, databuilder);
	databuilder.add_result(F4PACKET::RESULT::success);
	STORE_FBPACKET(builder, message, databuilder);
	
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	
	return true;

}