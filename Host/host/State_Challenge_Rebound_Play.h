#pragma once

#include "State.h"
#include "State_Challenge_Rebound.h"

class CState_Challenge_Rebound_Play : public CState_Challenge_Rebound
{
public:
	CState_Challenge_Rebound_Play(CHost* host) : CState_Challenge_Rebound(host)
	{
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

protected:
	int m_prevGameTime;

	int m_penaltyPointTotal;
	int m_penaltyPointDelta;
	float m_penaltyTimeRemain;
	float m_penaltyTimeDuration;

protected:
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);
	DECLARATION_PACKET_PROCESS_EXTEND(play_c2s_ballRebound);
	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound); // ¾È¾¸ 

};

