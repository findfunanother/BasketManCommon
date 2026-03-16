#pragma once
#include "CharacterState.h"

class CCharacter;

class CCharacterState_Shot : public CCharacterState
{
public:
	CCharacterState_Shot(CCharacter* pInfo);
	virtual ~CCharacterState_Shot();

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	virtual void OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnExit();

	void Init();
	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();
	void SetSecondAnimation(DHOST_TYPE_FLOAT CalcElapsedTime);

protected:

	DHOST_TYPE_INT32 m_BallNumber;
	F4PACKET::SHOT_TYPE m_ShotType;
	JOVECTOR3 m_BallPosition;
	DHOST_TYPE_FLOAT m_Speed;
	DHOST_TYPE_BOOL m_EnableDribble;
	DHOST_TYPE_BOOL m_ReadyAnimation;
	DHOST_TYPE_BOOL m_IsLeftHanded;
	DHOST_TYPE_BOOL m_Mirror;

	JOVECTOR3 m_Direction;
	JOVECTOR3 m_SlidePosition;
	DHOST_TYPE_FLOAT m_SlideTimePosition;
	DHOST_TYPE_FLOAT m_SlideYaw;
	DHOST_TYPE_FLOAT m_LastAniTime;
	DHOST_TYPE_FLOAT m_AnimationDuration;
	DHOST_TYPE_FLOAT m_YawDest;
	JOVECTOR3 m_RimPosition;
	DHOST_TYPE_BOOL m_DoubleClutchPacketReceive;
	DHOST_TYPE_FLOAT m_DoubleClutchYawDest;
	DHOST_TYPE_BOOL m_LastUpdateTick;
	DHOST_TYPE_BOOL m_LastSlideUpdateTick;
	JOVECTOR3 m_LocalPositionBall;

	//! ¾Ù¸®¿ó
	DHOST_TYPE_INT32 m_SecondAnimationID;
	DHOST_TYPE_BOOL m_bCheckSecondAnimationExcute;
	JOVECTOR3 m_SecondLocalPositionBall;
	JOVECTOR3 m_FirstSlidePositionA;
	JOVECTOR3 m_SecondSlidePositionB;
	DHOST_TYPE_FLOAT m_SecondSlideYawB;
	//! End
};