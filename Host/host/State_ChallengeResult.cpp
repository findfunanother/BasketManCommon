#include "State_ChallengeResult.h"

void CState_ChallengeResult::OnEnter(void* pData)
{
	
}

const DHOST_TYPE_BOOL CState_ChallengeResult::ConvertPacket_play_c2s_playerMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
}

const DHOST_TYPE_BOOL CState_ChallengeResult::ConvertPacket_play_c2s_playerStand(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStand_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	return true;
}
