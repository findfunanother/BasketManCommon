#include "State_Challenge_JumpShotBlock_BreakTime.h"

void CState_Challenge_JumpShotBlock_BreakTime::OnEnter(void* pData)
{
	pChallengeData = (ChallengeData*)pData;

	pChallengeData->m_tryCount--;

    timeBreak = 3.0f;
	timeElapsed = 0.0f;
    
	m_pHost->ResetShotClock();

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerStop, message, send_data);
	send_data.add_stop(true);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CState_Challenge_JumpShotBlock_BreakTime::OnUpdate(float timeDelta)
{
	CState::OnUpdate(timeDelta);

	timeElapsed += timeDelta;

	if (timeElapsed >= timeBreak)
    {
        m_pHost->ChangeState(EHOST_STATE::CHALLENGE_READY, pChallengeData);
    }
}