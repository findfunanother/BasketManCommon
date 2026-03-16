#pragma once

#include "State.h"
#include "State_Challenge.h"

class CState_ChallengeResult : public CState_Challenge
{
public:
	CState_ChallengeResult(CHost* host) : CState_Challenge(host)
	{
	}

	virtual void OnEnter(void* pData);

public:
	DECLARATION_PACKET_PROCESS(play_c2s_playerMove);
	DECLARATION_PACKET_PROCESS(play_c2s_playerStand);
};

