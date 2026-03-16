#pragma once

#include "State.h"
#include "State_Challenge_Rebound.h"

class CState_Challenge_Rebound_Ready : public CState_Challenge_Rebound
{
public:
	CState_Challenge_Rebound_Ready(CHost* host) : CState_Challenge_Rebound(host)
	{
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

protected:
	float m_timeRemain;
	int m_timeInt;
};

