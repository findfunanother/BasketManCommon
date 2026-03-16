#include "State_LineUp.h"

void CState_LineUp::OnEnter(void* pData)
{
    m_pHost->SetOverTime(false);

    m_pHost->SetTeamsActive(0, 1);

    int id11 = m_pHost->GetCharacterSN(0, 0);
    int id12 = m_pHost->GetCharacterSN(0, 1);
    int id13 = m_pHost->GetCharacterSN(0, 2);
    int id21 = m_pHost->GetCharacterSN(1, 0);
    int id22 = m_pHost->GetCharacterSN(1, 1);
    int id23 = m_pHost->GetCharacterSN(1, 2);
    int id31 = m_pHost->GetCharacterSN(2, 0);
    int id32 = m_pHost->GetCharacterSN(2, 1);
    int id33 = m_pHost->GetCharacterSN(2, 2);

    //CProtoBufPacket<ProtoBuf::system_s2c_readyToLineUp_data> message(ProtoBuf::system_s2c_readyToLineUp);
    
    m_pHost->PushMatchInfo(id11, id21);
    m_pHost->PushMatchInfo(id12, id22);
    m_pHost->PushMatchInfo(id13, id23);
    m_pHost->PushMatchInfo(id21, id11);
    m_pHost->PushMatchInfo(id22, id12);
    m_pHost->PushMatchInfo(id23, id13);

    bool* pLineUp = (bool *)pData;

    if (*pLineUp == true)
    {
        string log_message = "system_s2c_readyToLineUp RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime());
        m_pHost->ToLog(log_message.c_str());

        CREATE_BUILDER(builder)
        CREATE_FBPACKET(builder, system_s2c_readyToLineUp, message, send_data);
        send_data.add_ballnumber(m_pHost->BallNumberGet());
        send_data.add_team1(0);
        send_data.add_team2(1);
        send_data.add_id11(id11);
        send_data.add_id12(id12);
        send_data.add_id13(id13);
        send_data.add_id21(id21);
        send_data.add_id22(id22);
        send_data.add_id23(id23);
        send_data.add_id31(id31);
        send_data.add_id32(id32);
        send_data.add_id33(id33);
        STORE_FBPACKET(builder, message, send_data)

        m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
        timeWait = 3.0f;
    }
    else
    {
        timeWait = 0.0f;
    }
    
    //send_data.mutate_ballnumber(m_pHost->BallNumberGet());

    timeElapsed = 0.0f;

    m_pHost->SendGameScore();
}

void CState_LineUp::InitMatchInfo()
{
    m_pHost->SetOverTime(false);

    m_pHost->SetTeamsActive(0, 1);

    int id11 = m_pHost->GetCharacterSN(0, 0);
    int id12 = m_pHost->GetCharacterSN(0, 1);
    int id13 = m_pHost->GetCharacterSN(0, 2);
    int id21 = m_pHost->GetCharacterSN(1, 0);
    int id22 = m_pHost->GetCharacterSN(1, 1);
    int id23 = m_pHost->GetCharacterSN(1, 2);
    int id31 = m_pHost->GetCharacterSN(2, 0);
    int id32 = m_pHost->GetCharacterSN(2, 1);
    int id33 = m_pHost->GetCharacterSN(2, 2);

    m_pHost->PushMatchInfo(id11, id21);
    m_pHost->PushMatchInfo(id12, id22);
    m_pHost->PushMatchInfo(id13, id23);
    m_pHost->PushMatchInfo(id21, id11);
    m_pHost->PushMatchInfo(id22, id12);
    m_pHost->PushMatchInfo(id23, id13);

}

void CState_LineUp::OnUpdate(float timeDelta)
{
    CState::OnUpdate(timeDelta);

    timeElapsed += timeDelta;

    if (timeElapsed > timeWait)
    {
        m_pHost->ChangeState(EHOST_STATE::JUMP_BALL);
    }
}

DHOST_TYPE_BOOL CState_LineUp::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
    DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

    return CState::OnPacket(pPacket, peer, recv_time);
}