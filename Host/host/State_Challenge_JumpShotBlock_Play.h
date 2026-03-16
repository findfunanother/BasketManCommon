#pragma once

#include "State.h"
#include "State_Challenge_JumpShotBlock.h"

class CState_Challenge_JumpShotBlock_Play : public CState_Challenge_JumpShotBlock
{
public:
	CState_Challenge_JumpShotBlock_Play(CHost* host) : CState_Challenge_JumpShotBlock(host){}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

protected:
	int m_PrevShotClock;

	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);
	DECLARATION_PACKET_PROCESS(play_c2s_ballShot);
};

