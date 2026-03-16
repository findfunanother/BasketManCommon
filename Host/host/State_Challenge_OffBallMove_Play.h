#pragma once

#include "State.h"
#include "State_Challenge_OffBallMove.h"

class CState_Challenge_OffBallMove_Play : public CState_Challenge_OffBallMove
{
public:
	CState_Challenge_OffBallMove_Play(CHost* host) : CState_Challenge_OffBallMove(host)
	{
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

protected:
	int m_prevGameTime;
	
protected:
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);
	DECLARATION_PACKET_PROCESS(play_c2s_ballShot);
	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound);
	
};

