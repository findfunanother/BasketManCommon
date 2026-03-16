#pragma once

#include "State.h"
#include "State_Challenge.h"

class CState_Challenge_Pass : public CState_Challenge
{
public:
	CState_Challenge_Pass(CHost* host) : CState_Challenge(host)
	{
		pChallengeData = nullptr;
	}

public:
	struct ChallengeData
	{
	public:
		int m_userPlayerID;
		int m_point;
		vector<int> m_targets;
	};

protected:
	ChallengeData * pChallengeData;
};

