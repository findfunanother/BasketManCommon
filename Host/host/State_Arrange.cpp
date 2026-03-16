#include "State_Arrange.h"
#include "BallController.h"

void CState_Arrange::OnEnter(void* pData)
{
    CREATE_BUILDER(builder)
    CREATE_FBPACKET(builder, system_s2c_arrange, message, send_data);
    
    int offenseTeam = m_pHost->GetOffenseTeam();

    if (m_pHost->GetCharacterTeamSize(offenseTeam) == 0)
    {
        offenseTeam = m_pHost->GetIndexOpponentTeam(offenseTeam);
    }

    int defenseTeam = m_pHost->GetIndexOpponentTeam(offenseTeam);

    startIndex = rand() % m_pHost->GetCharacterTeamSize(offenseTeam);

    int ballBoy = -1;
    float distanceCurr = FLT_MAX_EXP;

    switch (m_pHost->BallControllerGet()->GameBallDataGet()->ballState)
    {
    case BALL_STATE::ballState_pick:
    case BALL_STATE::ballState_pickBlock:
    case BALL_STATE::ballState_pickLoose:
    case BALL_STATE::ballState_pickPass:
    case BALL_STATE::ballState_pickRebound:
    case BALL_STATE::ballState_pickSteal:
    case BALL_STATE::ballState_ready:
    {
        ballBoy = m_pHost->BallControllerGet()->GameBallDataGet()->owner;
    }
    break;
    default:
    {
        SVector3 ballPosition = m_pHost->GetBallPosition();

        for (int i = 0; i < m_pHost->GetCharacterTeamSize(offenseTeam); i++)
        {
            int id = m_pHost->GetCharacterSN(offenseTeam, i);
            SVector3 characterPosition = m_pHost->GetCharacterPosition(id);

            float distance = CommonFunction::SVectorDistance(characterPosition, ballPosition);

            if (distance < distanceCurr)
            {
                distanceCurr = distance;
                ballBoy = id;
            }
        }

        //if (ballBoy == -1)
        {
            for (int i = 0; i < m_pHost->GetCharacterTeamSize(defenseTeam); i++)
            {
                int id = m_pHost->GetCharacterSN(defenseTeam, i);
                SVector3 characterPosition = m_pHost->GetCharacterPosition(id);

                float distance = CommonFunction::SVectorDistance(characterPosition, ballPosition);

                if (distance < distanceCurr)
                {
                    distanceCurr = distance;
                    ballBoy = id;
                }
            }
        }
    }
    break;
    }

    int32_t id = SetPlayers(startIndex, offenseTeam, &send_data);
    
    m_pHost->SetReplayAssistCheck(false);
    m_pHost->SetReplayPoint(kFLOAT_INIT);

    send_data.add_idballboy(ballBoy);

    STORE_FBPACKET(builder, message, send_data)

    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

    timeArrange = 10.0f;
    timeElapsed = 0.0f;

    count = 0;
}

void CState_Arrange::OnExit()
{
    CState::OnExit();

    m_pHost->SetCharacterMoveUnLock();
}

void CState_Arrange::OnUpdate(float timeDelta)
{
    CState::OnUpdate(timeDelta);

    timeElapsed += timeDelta;

    if (timeElapsed > timeArrange)
    {
        m_pHost->ChangeState(EHOST_STATE::READY, &startIndex);
    }
}

int32_t CState_Arrange::SetPlayers(int startIndex, DHOST_TYPE_INT32 offenseTeam, F4PACKET::system_s2c_arrange_dataBuilder* data)
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

DHOST_TYPE_BOOL CState_Arrange::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
    if (nullptr == pPacket)
        return false;

    bool retValue = false;

    DHOST_TYPE_USER_ID userid = pPacket->GetUserID();
    
    switch ((PACKET_ID)pPacket->GetPacketID())
    {
        CASE_FB_CALL(system_c2s_arrangeReady);
    default:
        return CState::OnPacket(pPacket, peer, recv_time);
    }
}

const DHOST_TYPE_BOOL CState_Arrange::ConvertPacket_system_c2s_arrangeReady(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_arrangeReady_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

    count++;

    int team1, team2;

    m_pHost->GetCharacterManager()->GetTeamsActive(team1, team2);
    int team1Count = m_pHost->GetCharacterManager()->GetTeamCharacterSize(team1);
    int team2Count = m_pHost->GetCharacterManager()->GetTeamCharacterSize(team2);
    
    if (count == team1Count + team2Count)
    {
        m_pHost->ChangeState(EHOST_STATE::READY, &startIndex);
    }

    return true;
}

const DHOST_TYPE_BOOL CState_Arrange::ConvertPacket_play_c2s_ballClear(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballClear_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
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

    if (data->position() == nullptr)
    {
        string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER position is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
        m_pHost->ToLog(invalid_buffer_log.c_str());

        return false;
    }
    //!

    CREATE_BUILDER(builder)
    CREATE_FBPACKET(builder, play_s2c_ballClear, message, send_data);
    send_data.add_ballcleared(data->ballcleared());
    send_data.add_position(data->position());
    STORE_FBPACKET(builder, message, send_data)

    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

    return true;
}