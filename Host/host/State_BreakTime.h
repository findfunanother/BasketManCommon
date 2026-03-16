#pragma once
#include "State.h"

class CState_BreakTime : public CState
{
public:
	CState_BreakTime(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

private:
	float timeBreak = 0.0f;
	float timeStop = 0.0f;
	float timeElapsed = 0.0f;
	bool sendStop = false;
};

