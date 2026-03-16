#pragma once

#include "State.h"
#include "State_Challenge_Pass.h"

class CState_Challenge_Pass_Ready : public CState_Challenge_Pass
{
public:
	CState_Challenge_Pass_Ready(CHost* host) : CState_Challenge_Pass(host)
	{
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

protected:
	float m_timeRemain;
	int m_timeInt;
};

