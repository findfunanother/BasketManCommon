#pragma once
#include "JxActors.h"

class CJxSphere;
class CJxController;

//class CJxipDesc       // infinite palne desc.
//{
//public:
//	CJxipDesc()
//	{
//		fRadius = 200.0f;
//	}
//	~CJxipDesc()
//	{
//
//	}
//public:
//	JOVECTOR3			vNormal;   // 이 평면의 노멀 벡터 
//	FREAL				fDistance; // 원점으로 부터의 거리 
//	FREAL               fBounceness; // 
//	float				fRadius;
//	void*				pUserData;
//	bool				bIsCollisonEvent;
//};


// infinite plane 무한 평면 
class JPS_API CJxInPlane : public CJxActor
{ 
public:

	JOVECTOR3			vNormal;   // 이 평면의 노멀 벡터 
	FREAL				fDistance; // 원점으로 부터의 거리 
	FREAL               fBounceness; // 
	float				fRadius;
	void* pUserData;
	bool				bIsCollisonEvent;
	
	bool bisground;
	float fFriction;

	//CJxInPlane( CJxScene* pJxScene, CJxipDesc* pJxipDesc );
	CJxInPlane(void);
	~CJxInPlane(void);
	CJxInPlane(JOVECTOR3 vecPos3, JOVECTOR3 vecNormal3, float bounciness, bool isGround, float friction);

public:
	float				DistanceToPoint( const JOVECTOR3  A ) const;

public:
	//CJxipDesc*			_pJxipDesc;			

public:
	EKindOfActor		GetKindOfActor(void){ return KE_InPlane; } 			

	// collision check.
	bool				CollideWithOtherActor ( CJxActor * pJxOtherActor, CContactInfo* pContactInfo );
	bool				CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo );
	bool				CollideWithController( CJxController* pJxController, CContactInfo* pContactInfo ); 

private:
	// resolve.
	void				ResolveWithSphere( CContactInfo* pContactInfo );

};
