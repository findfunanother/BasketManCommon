#pragma once
#include "State.h"

class CState_Tutorial : public CState
{
public:
	CState_Tutorial(CHost* host) : CState(host)
	{
	}

public:
	virtual void OnEnter(void* pData);

	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

protected:
		DECLARATION_PACKET_PROCESS(system_c2s_login) // 여기서 호출 안하는 것 같음 

		DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad) // 1. ->system_s2c_tutorialInfo 
		DECLARATION_PACKET_PROCESS(local_system_c2s_login) // 2. -> x
		DECLARATION_PACKET_PROCESS(system_c2s_readyToTutorial) // -> 3. system_s2c_playerInfo, 
		DECLARATION_PACKET_PROCESS(system_c2s_loadComplete)

		DECLARATION_PACKET_PROCESS(system_c2s_exitGame)
		DECLARATION_PACKET_PROCESS(system_c2s_tutorialResult)

private:
	vector<DHOST_TYPE_USER_ID> m_Readys;
};

