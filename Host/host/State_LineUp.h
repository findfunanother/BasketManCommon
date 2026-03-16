#pragma once
#include "State.h"

class CState_LineUp : public CState
{
public:
	CState_LineUp(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

public:
	void InitMatchInfo();

protected:
	float timeElapsed;
	float timeWait = 3.0f;
};

