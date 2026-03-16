#include "State_Challenge.h"


DHOST_TYPE_BOOL CState_Challenge::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2s_login)
		CASE_FB_CALL(system_c2s_readyToLoad)
		CASE_FB_CALL(system_c2s_readyToChallenge)
		CASE_FB_CALL(local_system_c2s_login)
		DEFAULT_CALL(CState)
	}
}

void CState_Challenge::OnLogin(int id)
{

}

void CState_Challenge::SendMessagePoint(int pointTotal, int pointDelta, CHALLENGE_MESSAGE pMessage, float delay)
{
	string log_message = "SendMessagePoint pointTotal : " + std::to_string(pointTotal) + ", pointDelta : " + std::to_string(pointDelta);
	m_pHost->ToLog(log_message.c_str());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, challenge_s2c_point, message, send_data);
	send_data.add_point(pointTotal);
	send_data.add_gain(pointDelta);
	send_data.add_message(pMessage);
	send_data.add_delay(delay);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

const DHOST_TYPE_BOOL CState_Challenge::ConvertPacket_local_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::local_system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	bool retValue = false;

	if (nullptr != pPacket)
	{
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
			int playerID = m_pHost->GetLastCharacterSN();

			OnLogin(playerID);

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


			if (m_pHost->GetCharacterManager()->CreateCharacter(playerInformation))
			{
				m_pHost->SetScore(playerInformation->team, kINT32_INIT);
				m_pHost->AddLastCharacterSN();
			}
			else
			{
				string log_message = "CState_Challenge::ConvertPacket_local_system_c2s_login CreateCharcter fail pCharacter is null UserID : " + to_string(userid);
				m_pHost->ToLog(log_message.c_str());
			}

			CREATE_BUILDER(builder)
				auto offset_gamelogin = F4PACKET::SLocalGameLogin::Pack(builder, pGameLogin);
			CREATE_FBPACKET(builder, local_system_s2c_login, message, databuilder)
			databuilder.add_result(F4PACKET::RESULT::success);
			//send_data->set_result(RESULT::success);
			databuilder.add_gamelogin(offset_gamelogin);
			STORE_FBPACKET(builder, message, databuilder)

			string log_message = "local_system_c2s_login UserName : " + data->gamelogin()->username()->str() + ", Position : " + F4PACKET::EnumNameROLE(playerInformation->role) + ", TotalLoginUserCount : " + to_string(m_pHost->GetHostUserMap().size());
			m_pHost->ToLog(log_message.c_str());

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

			retValue = true;
		}
	}

	return retValue;
}

const DHOST_TYPE_BOOL CState_Challenge::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
	// 그냥 리턴하는거면 패킷을 왜 보내지?? 나중에 확인 요망 - by thinkingpig
}

const DHOST_TYPE_BOOL CState_Challenge::ConvertPacket_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

	return true;
}

const DHOST_TYPE_BOOL CState_Challenge::ConvertPacket_system_c2s_readyToChallenge(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToChallenge_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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
		STORE_FBPACKET(builder,message,databuilder)
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

	return true;
}
