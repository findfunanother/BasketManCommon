#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable : 4251)
#else
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif
///
/// Created by  findfun 2019 / 03 / 15
/// 


typedef float freal;
typedef freal FREAL;


#include "JOMath3d.h"
#include "string"

//#include "JxScene.h"

using namespace std;

class CContactInfo;

class CJxScene;
class CJxActor;

enum EKindOfActor { KE_NONE, KE_InPlane, KE_Sphere, KE_Cylinder, KE_Torus, KE_FPlane, KE_Controllers, KE_Box, KE_PointCloth, KE_Cloth, KE_F3Field, KE_F3GoalIn, KE_3PointLine, KE_F4GoalInChecker};

class CContactInfo
{
public:
	JOVECTOR3			vNormal;
	JOVECTOR3           vNewPosition;

	CJxActor*			pActorA;
	CJxActor*			pActorB;

	// 2009.10.8	smstock
	float				timeDelta;			// 실행 시간

	CContactInfo():pActorA(NULL),pActorB(NULL),timeDelta(0.f){}
	~CContactInfo(){ pActorA = NULL; pActorB = NULL; }
};

class JPS_API CJxActor
{
public:
	 CJxActor();
	virtual ~CJxActor();

public:
	JOVECTOR3								_vecPrePos;
	JOVECTOR3								_vecPos;
	JOVECTOR3								_vecVelocity;
	JOVECTOR3								_vecPreVelocity;
	JOVECTOR3								_vecAngularVelocity;
	JOQUATERNION							_qOrientation;			// 회전을 나타낼 쿼터니온
	JOVECTOR3								_vMoment;				// 전체 모멘트(토크)
	float									_curveValue;
	void*									userData;
	std::string								actorName;
	int										_actorID;


protected:
	JOMATRIX3								m_mtrInertia;			
	JOMATRIX3								m_mtrInertiaInverse;		


protected:
	bool									m_bSleep;
	bool									m_bMovAble;
	bool									m_bCollidable;
	bool									m_bSpinAble;
	unsigned int							m_uiCollisionGroup;

	CJxScene*								m_pJxScene;

public:
	JPS_API_CAL JOMATRIX3&								GetMtrInertia(void)				{ return m_mtrInertia; } 
	JPS_API_CAL JOMATRIX3&								GetMtrInertiaInverse(void)		{ return m_mtrInertiaInverse; }
	JPS_API_CAL CJxScene*								GetScene(void)					{ return m_pJxScene; }
	JPS_API_CAL void									SetCollidable  ( bool bAble )	{ m_bCollidable = bAble; }
	JPS_API_CAL bool									GetCollidable(void)				{ return m_bCollidable;  } 
	JPS_API_CAL void									SetMovable( bool bMovable )		{ m_bMovAble = bMovable; } 
	JPS_API_CAL bool									GetMovable(void)				{ return m_bMovAble; }
	JPS_API_CAL void									SetSpinAble( bool bAble )		{ m_bSpinAble = bAble; } 
	JPS_API_CAL bool									GetSpinAble(void)				{ return m_bSpinAble; } 
	JPS_API_CAL void									SetSleep( bool bSleep )			{ m_bSleep = bSleep; } 


	JPS_API_CAL void									SetCollisionGroup( unsigned int uiGroup ) { m_uiCollisionGroup = uiGroup; } 
	JPS_API_CAL int										GetCollisionGroup(void)			{ return m_uiCollisionGroup; } 
	JPS_API_CAL virtual void							Intergrate( float fTimeDelta )  { }
	JPS_API_CAL virtual void							ReadyToNextStep(float fTimeDelta);// { }
	JPS_API_CAL virtual	EKindOfActor					GetKindOfActor(void)			= 0;			
	JPS_API_CAL virtual bool							CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo ) = 0;
	JPS_API_CAL virtual void							Update(float fDeltaTime) { }
			    
	JPS_API_CAL virtual void							CreateActor(JOVECTOR3 vecPos) {}


};
