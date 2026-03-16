#pragma once

#include "State.h"
#include "State_Challenge_JumpShotBlock.h"

class CState_Challenge_JumpShotBlock_Init : public CState_Challenge_JumpShotBlock
{
public:
	CState_Challenge_JumpShotBlock_Init(CHost* host) : CState_Challenge_JumpShotBlock(host)
	{
		pChallengeData = &m_challengeData;
		pChallengeData->m_userPlayerID = kINT32_INIT;
		pChallengeData->m_point = kINT32_INIT;
	}

	virtual void OnEnter(void* pData);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	void OnLogin(int id);

	DECLARATION_PACKET_PROCESS(system_c2s_loadComplete)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToChallenge)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad)

private:
	ChallengeData m_challengeData;
};

