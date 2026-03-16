#pragma once

#include "State.h"
#include "State_Challenge_JumpShotBlock.h"

class CState_Challenge_JumpShotBlock_End : public CState_Challenge_JumpShotBlock
{
public:
	CState_Challenge_JumpShotBlock_End(CHost* host) : CState_Challenge_JumpShotBlock(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:

	DECLARATION_PACKET_PROCESS(challenge_c2s_retry)

private:
	float timeWait = 0.0f;
	float timeElapsed = 0.0f;
};

