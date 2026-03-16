#pragma once

#include "State.h"
#include "State_Challenge.h"

class CState_Challenge_Rebound : public CState_Challenge
{
public:
	CState_Challenge_Rebound(CHost* host) : CState_Challenge(host)
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

