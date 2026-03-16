#include "State_End.h"

void CState_End::OnEnter(void* pData)
{
	// 재접속 유저 있는지 확인 후 이것저것 처리
	m_pHost->ReconnectUserOperation();

	m_pHost->SetWinnerTeam();
	m_pHost->CalcMVPScore();

	timeWait = 3.0f;
	timeElapsed = 0.0f;

	CREATE_BUILDER(builder);

	CREATE_FBPACKET(builder, system_s2c_end, message, databuilder);

	STORE_FBPACKET(builder, message, databuilder);

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CState_End::OnUpdate(float timeDelta)
{
	curTimeDelta = timeDelta;

	// 비정상 방 종료일 때는 호스트 상태를 즉시 변경한다.
	if (m_pHost->GetAbnormalExpireRoom())
	{
		m_pHost->ChangeState(EHOST_STATE::RESULT);
	}

	timeElapsed += timeDelta;

	if (timeElapsed >= timeWait)
	{
		//! 20231222 컨티모드에서 end는 종료를 의미한다.
		if (m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS)
		{
			m_pHost->ChangeState(EHOST_STATE::RESULT);
		}
		else
		{
			if (m_pHost->GetNoTieMode()) // 이 모드에서는 바로 종료를 한다 
			{
				m_pHost->ChangeState(EHOST_STATE::RESULT);
			}
			else
			{
				if (m_pHost->GetScore(0) == m_pHost->GetScore(1))
				{
					m_pHost->SetOverTime(true);
					m_pHost->BallNumberAdd(1);
					m_pHost->SetGameTimeInit(60.0f);
					m_pHost->SetGameTime(60.0f);
					m_pHost->ResetShotClock();
					m_pHost->SetOffenseTeam(-1);
					m_pHost->ChangeState(EHOST_STATE::JUMP_BALL);
				}
				else
				{
					m_pHost->ChangeState(EHOST_STATE::RESULT);
				}
			}
		}
	}
}

const DHOST_TYPE_BOOL CState_End::ConvertPacket_play_c2s_playerMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
}

const DHOST_TYPE_BOOL CState_End::ConvertPacket_play_c2s_playerStand(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStand_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
}

