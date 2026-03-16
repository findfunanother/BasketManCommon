#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#else
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

#include "JxActors.h"

class CJxActor;
class CContactInfo;
class CJxFiPlane;
class CJxInPlane;
class CJxController;
class CJxCloth;
class CJxBox;
class CJxUserContactReport;
class CJxCylinder;
class CJxField;
class CF3GoalIn;
class CJxTorus;
class F4GoalInChecker;
//class CJxsDesc       // sphere desc.
//{
//public:
//	CJxsDesc()
//	{
//		fRadius = 0.12f; pJxUserContactReport = NULL;
//	}
//	~CJxsDesc()
//	{
//		
//	}
//public:
//	
//	void*	pUserData;
//	CJxUserContactReport* pJxUserContactReport;
//};

class JPS_API CJxSphere : public CJxActor
{
public:
	//CJxSphere( CJxScene* pJxScene, CJxsDesc* pJxsDesc );
	//CJxSphere( CJxsDesc* pJxsDesc );
	CJxSphere(void);
	~CJxSphere(void);

	float   fMass;
	float	fRadius;

public:
	void							Intergrate( float fTimeDelta );
	void							ReadyToNextStep( float fTimeDelta );

public:
	// 시뮬레이션 용 
	//std::string						collisionEventName[1000]; // 
	int								currentFrameIndex;
	float						    elapsedCurrentTime;

	float							additionalBounceFactorratio;
	//	EKindOfActor							m_KindOfActor;
	///////////////  감아차기 관련 /////////////////////////////////
	float                           curveValue;
	JOVECTOR3                       curveMoment;
	bool							firstBound;
	///////////////////////////////////////////////////////////////
public:
	//CJxsDesc*						_pJxsDesc;	

public:
	EKindOfActor					GetKindOfActor(void) { return KE_Sphere; }

	// collision check.
	bool							CollideWithOtherActor ( CJxActor * pJxOtherActor, CContactInfo* pContactInfo );

	bool							CollideWithController(  CJxController * pJxController, CContactInfo* pContactInfo );
	bool							CollideWithInPlane( CJxInPlane* pJxInPlane, CContactInfo* pContactInfo );             
	bool							CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo );
	bool							CollideWithJxFiPlane( CJxFiPlane * pJxFiPlane, CContactInfo* pContactInfo );
	bool							CollideWithJxCylinder(CJxCylinder * pJxCylinder, CContactInfo* pContactInfo );
	bool							CollideWithField(CJxField* pF3Field, CContactInfo* pContactInfo);
	void							CollideWithFieldExtraBoundary(CJxField* Field, CContactInfo* infoContact);
	bool							CollideWithJxTorus(CJxTorus* jxTorus, CContactInfo* infoContact);
	bool							CollideWithJxBox(CJxBox* jxTorus, CContactInfo* infoContact);
	bool							CollideWithJxF4GoalInChecker(F4GoalInChecker* f4Field, CContactInfo* pContactInfo);
	bool							CollideWihtJxActor(CJxActor* jxActor, CContactInfo* infoContact);


	virtual void					Update(float fDeltaTime);// { }

	void							ResolveWithJxTorus(CContactInfo* infoContact);
	// resolve.
	void							ResolveWithController( CContactInfo* pContactInfo );
	void							ResolveWithSphere( CContactInfo* pContactInfo );
	void							ResolveWithInPlane( CContactInfo* pContactInfo );
	void							ResolveWithTorus( CContactInfo* pContactInfo );
	void							ResolveWithFiPlane(CContactInfo* pContactInfo);
	void							ResolveWithCylinder(CContactInfo* pContactInfo, JOVECTOR3 newNearCylPnt);
	void							ResolveWithField(CContactInfo* pContactInfo);
	void							ResolveWithF3GoalIn(CContactInfo* pContactInfo);
	virtual void					OnEventFirstRimBound() { };
	virtual void					OnEventRimBound() { };
	virtual void					OnEventFirstBackBoardBound() {};
	virtual void					OnEventBackBoardBound() {};
	virtual void					OnEventBoundary() { };
	
	virtual void OnEventRightGoalIn() {};
	virtual void OnEventLeftGoalIn() {};
	
	void ResolveWithJxFiPlane(CContactInfo* infoContact);

};
