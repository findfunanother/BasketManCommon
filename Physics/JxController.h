#pragma once
#include "JxActors.h"

class CJxSphere;
class CContactInfo;
class CJxIControllerHitReport;
class CJxInPlane;
class CJx3PointLine;


class CJxcDesc
{
public:
	CJxcDesc(){ fRadius = 200.0f; fHeight = 200.0f;}
	~CJxcDesc(){}
public:
	float	fRadius;
	float   fHeight;
	void*   pControllerHitReport;
};


class CJxController : public CJxActor
{
public:
	CJxController( CJxScene* pJxScene, CJxcDesc* pCJxcDesc );
	CJxController(void){};
	~CJxController(void);

public:
	void							Move( JOVECTOR3 dispVector );
	void							Intergrate( float fTimeDelta );
	void	 						ReadyToNextStep( float fTimeDelta );
	EKindOfActor					GetKindOfActor(void) { return KE_Controllers; }

    
	// collision check.
	bool							CollideWithOtherActor ( CJxActor * pJxOtherActor, CContactInfo* pContactInfo );

	bool							CollideWithController(  CJxController * pJxController, CContactInfo* pContactInfo );
	bool							CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo );
	bool							CollideWithInPlane( CJxInPlane* pJxInPlane,  CContactInfo* pContactInfo );

	// resolve.
	void							ResolveWithController( CContactInfo* pContactInfo );
	void							ResolveWithSphere( CContactInfo* pContactInfo );
	void							ResolveWithInPlane( CContactInfo* pContactInfo );

	void							SetRadius( float fRadius ) { _pJxcDesc->fRadius = fRadius; } 
	
	void							SetEllipse( bool enable )	{_ellipse = enable; }
	void							SetFront( float front )		{ _front = front; }
	
public:
	CJxcDesc*						_pJxcDesc;
	int								_nPriority; 	/* 충돌시 우선권으로 이것이 높은 객체는 다른 객체를 밀고 나간다 */ 

private:
	CJxIControllerHitReport*        m_pJxIControllerHitReport;

	bool							_ellipse;		// 충돌 범위 타원으로 변환
	float							_front;			// 캐릭터 정면

public:
	void*							GetUserData(void){ return userData; } 


};
