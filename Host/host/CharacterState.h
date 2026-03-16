#pragma once

#include "Character.h"
#include "FileLog.h"

class CAnimationController;

class CCharacterState
{
public:
	CCharacterState(CCharacter* pInfo);
	virtual ~CCharacterState();

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	virtual void OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnExit();

	virtual DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();

protected:
	CCharacter* m_pOwner;
};