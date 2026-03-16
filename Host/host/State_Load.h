#pragma once
#include "State.h"

class CState_Load : public CState
{
public:
	CState_Load(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(system_c2s_loadComplete)
};

