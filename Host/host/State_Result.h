#pragma once
#include "State.h"

class CState_Result : public CState
{
public:
	CState_Result(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

private:
	float timeWait = 0.0f;
	float timeElapsed = 0.0f;
	bool bEnd;
};

