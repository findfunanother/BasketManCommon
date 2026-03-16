#pragma once

#include "State.h"
#include "State_Challenge_Pass.h"

class CState_Challenge_Pass_Init : public CState_Challenge_Pass
{
public:
	CState_Challenge_Pass_Init(CHost* host) : CState_Challenge_Pass(host)
	{
		pChallengeData = &m_challengeData;
		pChallengeData->m_userPlayerID = kINT32_INIT;
		pChallengeData->m_point = kINT32_INIT;
	}

public:
	virtual void OnEnter(void* pData);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(local_system_c2s_login)
	DECLARATION_PACKET_PROCESS(system_c2s_loadComplete)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToChallenge)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad)

protected:
	void OnLogin(int id);

private:
	vector<DHOST_TYPE_USER_ID> m_Readys;
	
	ChallengeData m_challengeData;
};

