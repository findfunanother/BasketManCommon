//#include "stdafx.h"
#include "JxInPlane.h"
#include "JxScene.h"
#include "JxSphere.h"
#include "JxController.h"
//
//
//CJxInPlane::CJxInPlane( CJxScene* pJxScene, CJxipDesc* pJxipDesc )
//{
//	_pJxipDesc		=	pJxipDesc;
//	m_pJxScene		=	pJxScene;
//	m_pJxScene->AddJxActor( this );
//}


CJxInPlane::CJxInPlane(void)
{

}


CJxInPlane::~CJxInPlane(void)
{
	//m_pJxScene->RemoveJxActor( this );
	//delete _pJxipDesc;
}

CJxInPlane::CJxInPlane(JOVECTOR3 vecPos3, JOVECTOR3 vecNormal3, float bounciness, bool isGround, float friction)
{
	_vecPos = vecPos3;
	_vecPrePos = _vecPos;
	vNormal = vecNormal3;
	fBounceness = bounciness;
	bisground = isGround;
	fFriction = friction;
	fDistance = 0.f;
}


float  CJxInPlane::DistanceToPoint( const JOVECTOR3  A ) const
{
	return fDistance + DotProduct(vNormal, A ); // 평면에서 특정 위치까지의 거리
}


// collision
bool CJxInPlane::CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo )
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch( eActor )
	{
		case KE_Sphere		: CollideWithSphere( (CJxSphere*)pJxOtherActor, pContactInfo );				break; 
		case KE_Controllers : CollideWithController( (CJxController*) pJxOtherActor, pContactInfo );	break;
		default:
			break;
	}
	return true;
}


bool CJxInPlane::CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo )
{
	return pJxSphere->CollideWithInPlane( (CJxInPlane*)this, pContactInfo );
}

bool CJxInPlane::CollideWithController( CJxController* pJxController, CContactInfo* pContactInfo )
{
	return pJxController->CollideWithInPlane( (CJxInPlane*)this, pContactInfo );
}


void CJxInPlane::ResolveWithSphere( CContactInfo* pContactInfo )
{

}

