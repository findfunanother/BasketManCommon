#include "JxTorus.h"
#include "JxScene.h"
#include "JxSphere.h"

//CJxTorus::CJxTorus( CJxScene* pJxScene, CJxtDesc* pJxtDesc )
//{
//	_pJxtDesc				=	pJxtDesc;
//	m_pJxScene				=	pJxScene;
//	m_pJxScene->AddJxActor( this );
//}


CJxTorus::CJxTorus(void)
{
}

CJxTorus::~CJxTorus(void)
{
	//m_pJxScene->RemoveJxActor( this );
	//delete _pJxtDesc;
}

CJxTorus::CJxTorus(JOVECTOR3 _position, JOVECTOR3 _normal, float _fDistance, float _fRadius)
{
	//_pJxtDesc = new CJxtDesc();
	fDistance = _fDistance;
	fRadius = _fRadius;
	vNormal = _normal;
	_vecPrePos = _vecPos = _position;
}

void CJxTorus::Intergrate( float fTimeDelta )
{

}


void CJxTorus::ReadyToNextStep( float fTimeDelta )
{

}
bool CJxTorus::CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo)
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch (eActor)
	{
	case KE_Sphere: CollideWithSphere((CJxSphere*)pJxOtherActor, pContactInfo);				break;
	//case KE_Controllers: CollideWithController((CJxController*)pJxOtherActor, pContactInfo);	break;
	default:
		break;
	}
	return true;
}

bool CJxTorus::CollideWithSphere(CJxSphere *jxSphere, CContactInfo *infoContact)
{
	return jxSphere->CollideWithJxTorus(this, infoContact);
}

// collision
//bool CJxTorus::CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo )
//{
//	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
//	switch( eActor )
//	{
//		case KE_Sphere		: CollideWithSphere( (CJxSphere*)pJxOtherActor, pContactInfo );		break;
//	}
//	return true;
//}
//
//bool CJxTorus::CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo )
//{
//	return pJxSphere->CollideWithTorus((CJxTorus*)this, pContactInfo);
//}


void CJxTorus::ResolveWithSphere( CContactInfo* pContactInfo )
{

}


