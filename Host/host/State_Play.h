#pragma once
#include "State.h"

class CState_Play : public CState
{
public:
	CState_Play(CHost* host);

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

	DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, const char* pData, int size, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

private:
	bool m_CheckScore;
	bool m_CheckGameTime;
	bool m_CheckShotClock;
	bool m_EnableStop;
	bool m_LastSendGameTime;

	int m_PrevGameTime;
	int m_PrevShotClock;


protected:
	DECLARATION_PACKET_PROCESS(play_c2s_ballClear);
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);

	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound);
	DECLARATION_PACKET_PROCESS_EXTEND(play_c2s_ballRebound);


};

