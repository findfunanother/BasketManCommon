#pragma once

#include "State.h"
#include "State_Challenge_Pass.h"

class CState_Challenge_Pass_Play : public CState_Challenge_Pass
{
public:
	CState_Challenge_Pass_Play(CHost* host) : CState_Challenge_Pass(host)
	{
	}

protected:
	int m_targetID;

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

protected:
	int m_prevGameTime;
	int m_penaltyPointTotal;
	int m_penaltyPointDelta;
	float m_penaltyTimeRemain;
	float m_penaltyTimeDuration;

protected:
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);
	DECLARATION_PACKET_PROCESS(play_c2s_playerReceivePass);
};

