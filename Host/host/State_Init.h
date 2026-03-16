#pragma once
#include "State.h"

class CState_Init : public CState
{
public:
	CState_Init(CHost* host) : CState(host)
	{
		
	}

	virtual void OnUpdate(float timeDelta);

	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(system_c2s_login)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad) 

		// 1. -> system_s2c_playerInfo 2. -> system_s2c_startLoad  3. -> c2s_loadComplete
		// 1. -> system_s2c_tutorialInfo -> 2. system_c2s_readyToTutorial -> 3. system_s2c_playerInfo -> system_s2c_startLoad

	DECLARATION_PACKET_PROCESS(local_system_c2s_login)
	DECLARATION_PACKET_PROCESS(local_system_c2s_mixPlayer)

	string m_ClientBuildData;
};

