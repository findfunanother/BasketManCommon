#pragma once

#include "State.h"
#include "State_Challenge_Rebound.h"

class CState_Challenge_Rebound_End : public CState_Challenge_Rebound
{
public:
	CState_Challenge_Rebound_End(CHost* host) : CState_Challenge_Rebound(host){}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:

	DECLARATION_PACKET_PROCESS(challenge_c2s_retry)

private:
	float timeWait = 0.0f;
	float timeElapsed = 0.0f;
};

