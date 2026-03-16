#pragma once
#include "State.h"

class CState_JumpBall : public CState
{
public:
	CState_JumpBall(CHost* host) : CState(host)
	{
		timeElapsed = 0.0f;
		timeWait = 3.0f;
	}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	float timeElapsed;
	float timeWait = 3.0f;

private:
	virtual void MedalCheckAndApply();
	virtual void SendModificationAbilitiesMedalTwoRoleGame(int32_t id, ABILITY_TYPE typeA, ABILITY_TYPE typeB, float valueA, float valueB);
	bool justOneApply = false;
};

