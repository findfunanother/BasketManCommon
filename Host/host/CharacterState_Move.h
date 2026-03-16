#pragma once
#include "CharacterState.h"

class CCharacter;

class CCharacterState_Move : public CCharacterState
{
public:
	CCharacterState_Move(CCharacter* pInfo);
	virtual ~CCharacterState_Move();

	F4PACKET::MOVE_MODE m_moveMode = F4PACKET::MOVE_MODE::normal;

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	virtual void OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnExit();

	void Init();
	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();

protected:
	
	DHOST_TYPE_FLOAT m_Speed;
	DHOST_TYPE_FLOAT m_SlideTimePosition;
	DHOST_TYPE_FLOAT m_SlideYaw;
	DHOST_TYPE_FLOAT m_LastAniTime;
	DHOST_TYPE_FLOAT m_AnimationDuration;

	DHOST_TYPE_BOOL	m_Mirror;
	DHOST_TYPE_BOOL m_LastUpdateTick;
	DHOST_TYPE_BOOL m_IsEnableDribble;

	JOVECTOR3 m_SlidePosition;
	JOVECTOR3 m_RimPosition;
	JOVECTOR3 m_LocalPositionBall;
	JOVECTOR3 m_MoveDirection;
};