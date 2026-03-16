#pragma once

#include "State.h"
#include "State_Challenge.h"

class CState_Challenge_JumpShotBlock : public CState_Challenge
{
public:
	CState_Challenge_JumpShotBlock(CHost* host) : CState_Challenge(host) {}

	struct ChallengeData
	{
	public:
		int m_userPlayerID;
		int m_startPlayerID;
		int m_point;
		int m_tryCount;
	};

protected:
	ChallengeData* pChallengeData;
};

