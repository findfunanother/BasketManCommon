#pragma once

#include "ICallBackGPS2.h" // GPS2 
#include <list>
#include "JOMath3d.h"



class ControllerHitReport;
class CJxActor;
class CJxController;



// Actor Group Define
#define  GRP_DEFAULT  0
#define  GRP_BOUNDARY 1
#define  GRP_BALL	  2
#define  GRP_POST	  3
#define  GRP_RIM	  4
#define  GRP_BOTTOM	  5
#define  GRP_NET      6
#define  GRP_BARRIER  7

#define  CHARACTER_COL_LV1	-1
#define  CHARACTER_COL_LV2	-2
#define  CHARACTER_COL_LV3	-3
#define  CHARACTER_COL_LV4	-4
#define  CHARACTER_COL_LV5	-5
#define  CHARACTER_COL_LV6	-6
#define  CHARACTER_COL_LV7	-7
#define  CHARACTER_COL_LV8	-8
#define  CHARACTER_COL_LV9	-9

// 바닥     공      벽       캐릭터 
//  10      1111      01          01
#define COLLISION_GROUP_A  0x0001 // 01
#define COLLISION_GROUP_B  0x0002 // 10
#define COLLISION_GROUP_C  0x0004 //100
#define COLLISION_GROUP_D  0x0008 //1000


//// GPS2 character, Phyx 컨트롤러를 쓰지 않기 위해 제공 
class GPS2Character  :   public ICallBackGPS2
{
public:
	//int					CreateActor(const JOVECTOR3& vPos , float fHeight , float fRadius, ControllerHitReport* pHitReport , void* pCharacter);
	void				Destroy();
	void				Move( JOVECTOR3& vMoveDelta);
	JOVECTOR3&			GetCharactersPosition();
	CJxActor*			GetActor(void);
	CJxController*		GetController(void);

	void				AddForce(JOVECTOR3& vFroce);
	void				SetCollisionEnable(bool bEnable); 
	bool				IsSetCollision();
	void				SetCollisionGroup(unsigned int uiGroup);
	unsigned int	    GetCollisionGroup();

	void				SetGPSPosition(JOVECTOR3& Point);
	void				SetRadius(float fRadius);
	float				GetRadius();	/// sclee 추가
	void				SetCollisionLevel(int iLevel);
	int					GetCollisionLevel();

	void				SetFront(float front);		// 캐릭터 정면을 물리연산에서도 사용
	void				SetEllipse(bool enable);	// 캐릭터 충돌범위를 타원으로 할 시에 사용

private:

	//int					m_iControllerID;	
	//bool				m_bSetCollision;
	CJxController*		m_pJxController;
	JOVECTOR3			m_vGPSPosition;


public:
	GPS2Character(void);
	~GPS2Character(void);
};


class GPS2Ball : public ICallBackGPS2
{


public:

	void								CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall);
	void								Destroy();

	JOVECTOR3&							GetActorPosition();
	JOQUATERNION&						GetActorRotate();
	JOVECTOR3&							GetActorVelocity();
	JOVECTOR3&							GetActorAngleVelocity();

	void								SetActorVelocity( JOVECTOR3& vVelocity );
	void								SetActorAngleVelocity( JOVECTOR3& vAngleVelocity );

	void								SetActorPosition(const JOVECTOR3& vPos);
	void								ResetActor(const JOVECTOR3& vPos);

	// 슛 , 패스등 .. 볼을 던져야 할 상황에서 쓰임 
	void								RaiseActorFlag( bool actorFlag = false );
	void								ClearActorFlag( bool actorFlag = true);

	// 라인 아웃시 볼을 고정시키기위해 사용
	void								SetSleep( bool actorFlag );

	// 2009.1.9 smstock
	// 상황에 따른 충돌 그룹 설정
//	void								SetCollisionGroup( unsigned int uiGroupNum );

	JOVECTOR3							GetSimulateActorPosition( float fTime );
	JOVECTOR3							GetSimulateActorPositionForAI( float fTime);//for AI PJM

	bool								GetSimulateActorVeoloticy( float fTime , JOVECTOR3& vBallPos ,JOVECTOR3& vBallVelocity );


protected: /// 추가함 sclee // gps에서 직접 접근하지 않도록 수정해야겠음

	void								ThrowBall(JOVECTOR3& vStartPos, JOVECTOR3& vVelocity, JOVECTOR3& vecAngleVelocity );


protected:
	CJxActor*							m_pActor;


private:
	JOVECTOR3			m_vGPSPosition;
	JOQUATERNION		m_qGPSRotate;
	JOVECTOR3			m_vGPSVelocity;
	JOVECTOR3			m_vGPSAngleVelocity;

public:
	GPS2Ball();
	~GPS2Ball();

};






