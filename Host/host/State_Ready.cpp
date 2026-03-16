#include "State_Ready.h"

void CState_Ready::OnEnter(void* pData)
{
    // 재접속 유저 있는지 확인 후 이것저것 처리
    m_pHost->ReconnectUserOperation();

    //! 관전자가 몇명있는지 알려주기
    m_pHost->SendObserverUserCountInfo();

    CREATE_BUILDER(builder)
    CREATE_FBPACKET(builder, system_s2c_readyToPlay, message, send_data);

    int offenseTeam = m_pHost->GetOffenseTeam();

    if (m_pHost->GetCharacterTeamSize(offenseTeam) == 0)
    {
        offenseTeam = m_pHost->GetIndexOpponentTeam(offenseTeam);
    }

    int defenseTeam = m_pHost->GetIndexOpponentTeam(offenseTeam);

    //-------------- 미니게임 모드 ------ 
    CGameManager* pGameManager = m_pHost->GetGameManager();
    if (pGameManager)
    {
        F4PACKET::MinigameCustomInfoT* pCustomMinigame = pGameManager->GetMinigameCustomInfo();
        if (pCustomMinigame)
        {
            if (pCustomMinigame->ishomeattackretained)
            {
                offenseTeam = 0;
                defenseTeam = 1;
            }
        }
    }
    //-------------

    int startIndex = 0;
    bool positionReset;

    if (pData == nullptr)
    {
        size_t character_team_size = m_pHost->GetCharacterTeamSize(offenseTeam);
        if (character_team_size > 0)
        {
            startIndex = rand() % character_team_size;
        }
        else
        {
            string log_message = "[ERROR] RoomID : " + m_pHost->GetHostID() + ", character_team_size is 0";
            m_pHost->ToLog(log_message.c_str());
        }
        positionReset = true;
    }
    else
    {
        startIndex = *static_cast<int*>(pData);
        positionReset = false;
    }
    

    int32_t id11 = SetPlayers(startIndex, offenseTeam, &send_data);


    /*
    map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();
    for (int i = 0; i < characters.size(); i++)
    {
        CCharacter* character = characters[i];

        if (character == nullptr)
            continue;

        character->ClearSereverPositionDeque();
    }*/

    map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();

    for (auto it = characters.begin(); it != characters.end(); ++it) 
    {
        CCharacter* character = it->second;
        if (character)
            character->ClearSereverPositionDeque();
    }

    // 비매너 감지 초기화
    m_pHost->InitMannerManagerData(m_pHost->GetGameTime(), m_pHost->BallNumberGet(), id11);
    
    //! 20210415 replay - by thinkingpig
    // 리플레이 초기화

    m_pHost->SetReplayAssistCheck(false);
    m_pHost->SetReplayPoint(kFLOAT_INIT);
    //! end
    
    timeWait = 3.0f;
    timeCount = static_cast<int>(timeWait);

    timeElapsed = 0.0f;
    
    send_data.add_ballnumber(m_pHost->BallNumberGet());
    send_data.add_positionreset(positionReset);
    STORE_FBPACKET(builder, message, send_data)

    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CState_Ready::OnExit()
{
    CState::OnExit();

    m_pHost->SetCharacterMoveUnLock();
}

void CState_Ready::OnUpdate(float timeDelta)
{
    CState::OnUpdate(timeDelta);

    timeElapsed += timeDelta;

    int currentCount = static_cast<int>((timeWait - timeElapsed));

    if (timeElapsed > timeWait)
    {
        F4PACKET::SPlay pPlay;// = new SPlay;
        pPlay.mutate_teamoffense(m_pHost->GetOffenseTeam());

        CREATE_BUILDER(builder)
        CREATE_FBPACKET(builder, system_s2c_play, message, send_data);
        send_data.add_play(&pPlay);
        STORE_FBPACKET(builder, message, send_data)

        m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

        m_pHost->ChangeState(EHOST_STATE::PLAY);
    }
    else
    {
        if (currentCount != timeCount)
        {
            CREATE_BUILDER(builder)
            CREATE_FBPACKET(builder, system_s2c_count, message, send_data);
            send_data.add_value(timeCount);
            STORE_FBPACKET(builder, message, send_data)

            m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

            timeCount = currentCount;
        }
    }
}

int32_t CState_Ready::SetPlayers(int startIndex, DHOST_TYPE_INT32 offenseTeam, F4PACKET::system_s2c_readyToPlay_dataBuilder* data)
{
    std::vector<DHOST_TYPE_CHARACTER_SN> offensePlayers, defensePlayers;
    offensePlayers.clear();
    defensePlayers.clear();

    for (int i = 0; i < m_pHost->GetCharacterTeamSize(offenseTeam); ++i)
    {
        offensePlayers.push_back(m_pHost->GetCharacterSN(offenseTeam, i));
    }

    DHOST_TYPE_INT32 defenseTeam = m_pHost->GetIndexOpponentTeam(offenseTeam);

    for (int i = 0; i < m_pHost->GetCharacterTeamSize(defenseTeam); ++i)
    {
        defensePlayers.push_back(m_pHost->GetCharacterSN(defenseTeam, i));
    }
    
    int ids[9] = { 0 };
    
    ids[0] = offensePlayers.size() > 0 ? offensePlayers[startIndex % offensePlayers.size()] : 0;
    ids[1] = offensePlayers.size() > 1 ? offensePlayers[(startIndex + 1) % offensePlayers.size()] : 0;
    ids[2] = offensePlayers.size() > 2 ? offensePlayers[(startIndex + 2) % offensePlayers.size()] : 0;
    
	switch (m_pHost->GetModeType())
	{
		case EMODE_TYPE::CONTINUOUS:
		{
			ids[3] = defensePlayers.size() > 0 ? defensePlayers[startIndex % defensePlayers.size()] : 0;
			ids[4] = defensePlayers.size() > 1 ? defensePlayers[(startIndex + 1) % defensePlayers.size()] : 0;
			ids[5] = defensePlayers.size() > 2 ? defensePlayers[(startIndex + 2) % defensePlayers.size()] : 0;

			DHOST_TYPE_INT32 waitTeam = m_pHost->GetIndexTeamWait();

			if (waitTeam != -1)
			{
				std::vector<DHOST_TYPE_CHARACTER_SN> waitPlayers;

				for (int i = 0; i < m_pHost->GetCharacterTeamSize(waitTeam); ++i)
				{
					waitPlayers.push_back(m_pHost->GetCharacterSN(waitTeam, i));
				}

				ids[6] = waitPlayers.size() > 0 ? waitPlayers[startIndex % waitPlayers.size()] : 0;
				ids[7] = waitPlayers.size() > 1 ? waitPlayers[(startIndex + 1) % waitPlayers.size()] : 0;
				ids[8] = waitPlayers.size() > 2 ? waitPlayers[(startIndex + 2) % waitPlayers.size()] : 0;
			}
		}
		break;
		default:
		{
			if (offensePlayers.size() >= defensePlayers.size())
			{
				ids[3] = m_pHost->GetMatchInfoID(ids[0]);
				ids[4] = m_pHost->GetMatchInfoID(ids[1]);
				ids[5] = m_pHost->GetMatchInfoID(ids[2]);
			}
			else
			{
				ids[3] = defensePlayers.size() > 0 ? defensePlayers[startIndex % defensePlayers.size()] : 0;
				ids[4] = defensePlayers.size() > 1 ? defensePlayers[(startIndex + 1) % defensePlayers.size()] : 0;
				ids[5] = defensePlayers.size() > 2 ? defensePlayers[(startIndex + 2) % defensePlayers.size()] : 0;
			}
		}
		break;
	}

    data->add_team1(offenseTeam);
    data->add_team2(defenseTeam);

    data->add_id11(ids[0]);
    data->add_id12(ids[1]);
    data->add_id13(ids[2]);

    data->add_id21(ids[3]);
    data->add_id22(ids[4]);
    data->add_id23(ids[5]);

    data->add_id31(ids[6]);
    data->add_id32(ids[7]);
    data->add_id33(ids[8]);
    
    if (ids[0] != 0)
    {
        m_pHost->SetCharacterReadyPosition(ids[0], ACTION_TYPE::action_standDribble, position[0]);
    }

    for (int i = 1; i < 9; i++)
    {
        if (ids[i] != 0)
        {
            ACTION_TYPE actionType;

            switch (i)
            {
            case 0:
                actionType = ACTION_TYPE::action_standDribble;
                break;
            case 1:
            case 2:
                actionType = ACTION_TYPE::action_standOffense;
                break;
            case 3:
            case 4:
            case 5:
                actionType = ACTION_TYPE::action_standDefense;
                break;
            default:
                actionType = ACTION_TYPE::action_standNormal;
                break;
            }

            m_pHost->SetCharacterReadyPosition(ids[i], actionType, position[i]);
        }
    }

    m_pHost->SetCharacterMoveLock(ids[0], EMOVEMENT_RESTRICTION::OUTSIDE_TO_INSIDE_RESTRICTION);
    m_pHost->SetCharacterMoveLock(ids[1], EMOVEMENT_RESTRICTION::OUTSIDE_TO_INSIDE_RESTRICTION);
    m_pHost->SetCharacterMoveLock(ids[2], EMOVEMENT_RESTRICTION::OUTSIDE_TO_INSIDE_RESTRICTION);

    m_pHost->SetCharacterMoveLock(ids[3], EMOVEMENT_RESTRICTION::INSIDE_TO_OUTSIDE_RESTRICTION);
    m_pHost->SetCharacterMoveLock(ids[4], EMOVEMENT_RESTRICTION::INSIDE_TO_OUTSIDE_RESTRICTION);
    m_pHost->SetCharacterMoveLock(ids[5], EMOVEMENT_RESTRICTION::INSIDE_TO_OUTSIDE_RESTRICTION);

    return ids[0];
}

const DHOST_TYPE_BOOL CState_Ready::ConvertPacket_play_c2s_playerMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

    const SPlayerAction* pData = data->playeraction();

    if (data->runmode() != MOVE_MODE::normal)
    {
        retValue = CState::ConvertPacket_play_c2s_playerMove(userid, peer, pPacket, recv_time);
    }
    
    return retValue;
}

const DHOST_TYPE_BOOL CState_Ready::ConvertPacket_play_c2s_playerStand(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStand_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

    const SPlayerAction* pData = data->playeraction();

    if (data->runmode() != MOVE_MODE::normal)
    {
        retValue = CState::ConvertPacket_play_c2s_playerStand(userid, peer, pPacket, recv_time);
    }

    return retValue;
}