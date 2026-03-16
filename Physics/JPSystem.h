/**
*
*
*
*
*/
#pragma once

#include "JPSApi.h"



class NxPhysicsSDK;
class NxScene;
class NxVec3;
class NxControllerManager;
class UserAllocator;
class NxController;
class NxControllerDesc;


class	CJxScene;
class   CJxControllerManager;
class   CJxController;
class   CJxcDesc;
class   CJxActor;
class   CJxsDesc;
class   JOVECTOR3;



// 충돌 그룹 지정 


// 바닥     공      벽       캐릭터 
//  10      1111      01          01
#define COLLISION_GROUP_A  0x0001 // 01				// 캐릭터
#define COLLISION_GROUP_B  0x0002 // 10				// 바닥, 링
#define COLLISION_GROUP_C  0x0004 //100				// 옆에 벽
#define COLLISION_GROUP_D  0x0008 //1000			// 백보드


#ifdef  OUT
#undef  OUT
#define OUT
#else
#endif 

class GPS2Ball;//for AI PJM
class ContactReport;


#define MAX_SIMULBALL  1000
#define EVENT_NAME_LENGTH 128

struct SimulBallInfo
{
	float fPos[3];
	float fVel[3];
	float fRot[4];
	char  eventName[EVENT_NAME_LENGTH];
};

class JPS_API JPSystem
{

public:

	static			JPSystem*	GetInstance();

	
	bool					Initialize();
	bool					Destroy();
	void					Simulate(float fTimeDelta);
	void					SimulateJxActor(CJxActor* pJxActor, float fTimeDelta);
	void					SimulateJxActor( CJxActor* pJxActor, float fTimeDelta, JOVECTOR3& vecPos, JOVECTOR3& vecVel);
	void					SimulateJxActorForAI( CJxActor* pJxActor, float fTimeDelta, JOVECTOR3& vecPos );//for AI PJM

	float					m_fRemainTime;	// 이번 tick에서 처리하고 남은 시간  sclee


	void					DebugRender(void);



//	void					SetFrameSync( bool bFrameSync ) { m_bFrameSync = bFrameSync; m_fRemainTime = 0.0f; } 

	// About Simulation 
public:
	SimulBallInfo*			SimulateBallFixedFrame(int frameCount, float inPos[], float inVel[], float inRotation[], float inTorqueNormal[], float curveValue);
	SimulBallInfo*			SimulateBallStartFixedFrame(int ballIndex, int frameCount, float inPos[], float inVel[], float inRotation[]);
private:
	SimulBallInfo*			m_pSimulBallInfoBuffer;


private:

	void					JCallBackUpdate( void );
	void				    ControllerCallBackUpdate( void );


private:

	int						m_iCharacterCreateCount;

	//JOVECTOR3				m_vecTargetPos;

	bool					m_bFrameSync;

	// 시뮬레이션 체크용 
	CJxActor*				m_pNewActor;
	CJxsDesc*				m_pJxsDesc;


////////////// new about JX /////////////////////////////
private: //
	/* 시뮬레이션 방법이 캐릭터와 볼은 다르므로 따로 관리*/
	CJxScene*				m_pJxScene;
	CJxControllerManager*	m_pJxControllerManager;

public:
	void					InitJx(void);
	void					ExitJx(void);

	CJxController*			CreateController( CJxcDesc* pjxcDesc, int& iID );
	CJxControllerManager*	GetJxControllerManager(){ return m_pJxControllerManager;}
	CJxScene*				GetScene(void) { return m_pJxScene; } 


public:
	const ContactReport*	GetContactReport(){ return m_pContactReport; }		
private:
	ContactReport*			m_pContactReport;			



public:
	JPSystem(void);
public:
	~JPSystem(void);
};
