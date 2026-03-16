#pragma once

#include "State.h"
#include "State_Challenge_OffBallMove.h"

class CState_Challenge_OffBallMove_Ready : public CState_Challenge_OffBallMove
{
public:
	CState_Challenge_OffBallMove_Ready(CHost* host) : CState_Challenge_OffBallMove(host)
	{
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

protected:
	float m_timeRemain;
	int m_timeInt;
};

