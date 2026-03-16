#pragma once
#include "CharacterState.h"

class CCharacter;

class CCharacterState_Pass : public CCharacterState
{
public:
	CCharacterState_Pass(CCharacter* pInfo);
	virtual ~CCharacterState_Pass();

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	virtual void OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnExit();

	void Init();
	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();

protected:

	DHOST_TYPE_FLOAT m_SlideTimePosition;
	DHOST_TYPE_FLOAT m_SlideYaw;
	DHOST_TYPE_FLOAT m_LastAniTime;
	DHOST_TYPE_FLOAT m_AnimationDuration;

	DHOST_TYPE_BOOL	m_Mirror;
	DHOST_TYPE_BOOL m_LastUpdateTick;

	JOVECTOR3 m_SlidePosition;
	JOVECTOR3 m_RimPosition;
	JOVECTOR3 m_LocalPositionBall;
};