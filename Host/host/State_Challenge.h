#pragma once

#include "State.h"

class CState_Challenge : public CState
{
public:
	CState_Challenge(CHost* host) : CState(host)
	{
	}

public:
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	virtual void OnLogin(int id);
	void SendMessagePoint(int pointTotal, int pointDelta, CHALLENGE_MESSAGE pMessage, float delay);
	DECLARATION_PACKET_PROCESS(local_system_c2s_login)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad)
	DECLARATION_PACKET_PROCESS(system_c2s_login)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToChallenge)
};

