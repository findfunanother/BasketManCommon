#pragma once

#include "State.h"
#include "State_Challenge.h"

class CState_Challenge_OffBallMove : public CState_Challenge
{
public:
	CState_Challenge_OffBallMove(CHost* host) : CState_Challenge(host)
	{
	}

public:
	struct ChallengeData
	{
	public:
		int m_userPlayerID;
		int m_point;
	};

protected:
	ChallengeData* pChallengeData;
};

