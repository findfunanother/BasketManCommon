#include "State_BreakTime.h"

void CState_BreakTime::OnEnter(void* pData)
{
	/*switch (m_pHost->GetModeType())
	{
	case EMODE_TYPE::CONTINUOUS:
	case EMODE_TYPE::TRIO:
		timeBreak = 0.5f;
		timeStop = 0.5f;
		break;
	default:
		timeBreak = 2.0f;
		timeStop = 1.0f;
		break;
	}*/

	timeBreak = 2.0f;
	timeStop = 1.0f;

	timeElapsed = 0.0f;
    sendStop = false;
}

void CState_BreakTime::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);

	timeElapsed += timeDelta;

	if (sendStop == false && timeElapsed >= timeBreak)
	{
		sendStop = true;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerStop, message, send_data);
		send_data.add_stop(true);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	if (timeElapsed >= timeBreak + timeStop)
	{
		m_pHost->ChangeState(EHOST_STATE::READY);
	}

    /*if (timeElapsed >= timeBreak + timeStop)
    {
		switch (m_pHost->GetModeType())
		{
		case EMODE_TYPE::CONTINUOUS:
		case EMODE_TYPE::TRIO:
			m_pHost->ChangeState(EHOST_STATE::READY);
			break;
		default:
			m_pHost->ChangeState(EHOST_STATE::ARRANGE);
			break;
		}
    }*/
}