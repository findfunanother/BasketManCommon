#pragma once
#include "CharacterState.h"

class CCharacterState_Stand : public CCharacterState
{
public:
	CCharacterState_Stand(CCharacter* pInfo);
	virtual ~CCharacterState_Stand();

	F4PACKET::MOVE_MODE m_moveMode = F4PACKET::MOVE_MODE::normal;

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	virtual void OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnExit();

	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();
};