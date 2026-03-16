#include "State_Challenge_Pass_Ready.h"
#include "BallController.h"

void CState_Challenge_Pass_Ready::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;
	m_timeRemain = 3.0f;
	m_timeInt = static_cast<int>(m_timeRemain);
}

void CState_Challenge_Pass_Ready::OnUpdate(float timeDelta)
{
	m_timeRemain -= timeDelta;

	int timeInt = static_cast<int>(m_timeRemain);

	if (timeInt != m_timeInt)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_count, message, send_data);
		send_data.add_value(m_timeInt);
		STORE_FBPACKET(builder, message, send_data)

		m_timeInt = timeInt;
	}

	if (m_timeRemain < 0.0f)
	{
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, challenge_s2c_start, message, send_data);
		send_data.add_userid(0);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		m_pHost->ChangeState(EHOST_STATE::CHALLENGE_PLAY, pChallengeData);
	}
}