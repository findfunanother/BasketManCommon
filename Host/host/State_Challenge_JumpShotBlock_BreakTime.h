#pragma once
#include "State_Challenge_JumpShotBlock.h"

class CState_Challenge_JumpShotBlock_BreakTime : public CState_Challenge_JumpShotBlock
{
public:
	CState_Challenge_JumpShotBlock_BreakTime(CHost* host) : CState_Challenge_JumpShotBlock(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

private:
	float timeBreak = 0.0f;
	float timeElapsed = 0.0f;
};

