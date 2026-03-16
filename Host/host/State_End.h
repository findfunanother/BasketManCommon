#pragma once
#include "State.h"

class CState_End : public CState
{
public:
	CState_End(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

private:
	float timeWait = 0.0f;
	float timeElapsed = 0.0f;

public:
	DECLARATION_PACKET_PROCESS(play_c2s_playerMove);
	DECLARATION_PACKET_PROCESS(play_c2s_playerStand);
};

