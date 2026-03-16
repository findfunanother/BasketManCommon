#include "State_Result.h"

void CState_Result::OnEnter(void* pData)
{
    bEnd = false;
    timeWait = kFLOAT_INIT;
    timeElapsed = kFLOAT_INIT;

    auto MVP = m_pHost->GetMVP();

    string log_message = "MVP : " + to_string(MVP);
    m_pHost->ToLog(log_message.c_str());

    CREATE_BUILDER(builder)
    CREATE_FBPACKET(builder, system_s2c_result, message, send_data);
    send_data.add_mvpid(MVP);
    STORE_FBPACKET(builder, message, send_data)

    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

    switch (m_pHost->GetModeType())
    {
        case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
        case EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE:
        case EMODE_TYPE::SKILL_CHALLENGE_PASS:
        case EMODE_TYPE::SKILL_CHALLENGE_REBOUND:
        {
            timeWait = kSKILL_CHALLENGE_END_SCENE_WAIT_TIME;
        }
        break;
        case EMODE_TYPE::TRAINING:
        case EMODE_TYPE::TRAINING_REBOUND:
        case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
        case EMODE_TYPE::TRAINING_RIMATTACKBLOCK:
        case EMODE_TYPE::TRAINING_OFFBALLMOVE:

        case EMODE_TYPE::TUTORIAL_BASIC:
        case EMODE_TYPE::TUTORIAL_BOXOUT:
        case EMODE_TYPE::TUTORIAL_DIVINGCATCH:
        case EMODE_TYPE::TUTORIAL_JUMPSHOT:
        case EMODE_TYPE::TUTORIAL_JUMPSHOTBLOCK:
        case EMODE_TYPE::TUTORIAL_PASS:
        case EMODE_TYPE::TUTORIAL_REBOUND:
        case EMODE_TYPE::TUTORIAL_RIMATTACK:
        case EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK:
        case EMODE_TYPE::TUTORIAL_STEAL:
        case EMODE_TYPE::TUTORIAL_PENETRATE:
        case EMODE_TYPE::TUTORIAL_SHOOTINGDISTURB:
        case EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER:
        case EMODE_TYPE::TUTORIAL_REBOUND_BEGINNER:
        {
            timeWait = kFLOAT_INIT;
        }
        break;
        default:
        {
            timeWait = kMVP_SCENE_WAIT_TIME;
        }
        break;
    }

    if (m_pHost->GetAbnormalExpireRoom() == true && m_pHost->UserCount() <= 1)
    {
        timeWait = kFLOAT_INIT;
    }
}

void CState_Result::OnUpdate(float timeDelta)
{
	timeElapsed += timeDelta;

	if (timeElapsed >= timeWait && bEnd == false)
	{
        m_pHost->PrintPacketCountReport();

        m_pHost->RedisSaveMatchResult();

        bEnd = true;
	}
}
