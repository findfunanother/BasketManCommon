//#include "stdafx.h"
#include "JxScene.h"
#include "JxFiPlane.h"
#include "JxSphere.h"
#include "JxPhysics.h"

CJxFiPlane::CJxFiPlane(void)
{
}

//CJxFiPlane::CJxFiPlane(CJxScene* pJxScene, CJxfpDesc* pJxfpDesc)
//{
//	//_pJxfpDesc = pJxfpDesc;
//	m_pJxScene = pJxScene;
//	_vecPos = pJxfpDesc->vPos;
//	_vecPrePos = _vecPos;
//	m_pJxScene->AddJxActor(this);
//}

CJxFiPlane::~CJxFiPlane(void)
{
	m_CollideList.clear();
	//m_pJxScene->RemoveJxActor(this);
	
}

CJxFiPlane::CJxFiPlane(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float width, float depth, float bounciness)
{
	CJxfpDesc* pJxfpDesc = new CJxfpDesc();
	_vecPos = pos;
	pJxfpDesc->vNormal = vecNormal;
	pJxfpDesc->fWidth = width;
	pJxfpDesc->fDepth = depth;
	pJxfpDesc->fBounceness = bounciness;
	pJxfpDesc->vDepthVector = depthVector;
	
	//Vector3[] pointMid4 = new Vector3[4];

	JOVECTOR3 pointMid0 = _vecPos + depthVector.Normalized() * depth;
	JOVECTOR3 dirRight = CrossProduct(vecNormal, depthVector);
	JOVECTOR3 pointMid1 = _vecPos + dirRight.Normalized() * width;
	JOVECTOR3 dirBack = CrossProduct(vecNormal, dirRight);
	JOVECTOR3 pointMid2 = _vecPos + dirBack.Normalized() * depth;
	JOVECTOR3 dirLeft = CrossProduct(vecNormal, dirBack);
	JOVECTOR3 pointMid3 = _vecPos + dirLeft.Normalized() * width;


	pJxfpDesc->vPoint4[0] = depthVector.Normalized() * depth;
	pJxfpDesc->vPoint4[1] = dirRight.Normalized() * width;
	pJxfpDesc->vPoint4[2] = dirBack.Normalized() * depth;
	pJxfpDesc->vPoint4[3] = dirLeft.Normalized() * width;

	/*
	PhysicSimulator.CreateDebugObject(point4[0], "point0");
	PhysicSimulator.CreateDebugObject(point4[1], "point1");
	PhysicSimulator.CreateDebugObject(point4[2], "point2");
	PhysicSimulator.CreateDebugObject(point4[3], "point3");
	*/
}


//CJxFiPlane::CJxFiPlane(JOVECTOR3 pos, float width, float depth, float radius, JOVECTOR3 rightGoalPos, JOVECTOR3 leftGoalPos)
//{
//	
//	_vecPos = pos;
//	vNormal = JOVECTOR3::up;
//	fWidth = width;
//	fDepth = depth;
//	fBounceness = GROUNDBOUNCE;
//	//_pJxfpDesc->vDepthVector = depthVector;
//
//	//Vector3[] pointMid4 = new Vector3[4];
//
//	/*JOVECTOR3 pointMid0 = _vecPos + depthVector.Normalized() * depth;
//	JOVECTOR3 dirRight = CrossProduct(_pJxfpDesc->vNormal, depthVector);
//	JOVECTOR3 pointMid1 = _vecPos + dirRight.Normalized() * width;
//	JOVECTOR3 dirBack = CrossProduct(_pJxfpDesc->vNormal, dirRight);
//	JOVECTOR3 pointMid2 = _vecPos + dirBack.Normalized() * depth;
//	JOVECTOR3 dirLeft = CrossProduct(_pJxfpDesc->vNormal, dirBack);
//	JOVECTOR3 pointMid3 = _vecPos + dirLeft.Normalized() * width;
//
//
//	_pJxfpDesc->vPoint4[0] = depthVector.Normalized() * depth;
//	_pJxfpDesc->vPoint4[1] = dirRight.Normalized() * width;
//	_pJxfpDesc->vPoint4[2] = dirBack.Normalized() * depth;
//	_pJxfpDesc->vPoint4[3] = dirLeft.Normalized() * width;*/
//
//	/*
//	PhysicSimulator.CreateDebugObject(point4[0], "point0");
//	PhysicSimulator.CreateDebugObject(point4[1], "point1");
//	PhysicSimulator.CreateDebugObject(point4[2], "point2");
//	PhysicSimulator.CreateDebugObject(point4[3], "point3");
//	*/
//}

void CJxFiPlane::SetFiPlane(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float width, float depth, float bounciness)
{
	
	_vecPos = pos;
	vNormal = vecNormal;
	fWidth = width;
	fDepth = depth;
	fBounceness = bounciness;
	vDepthVector = depthVector;
	
	//Vector3[] pointMid4 = new Vector3[4];

	JOVECTOR3 pointMid0 = _vecPos + depthVector.Normalized() * depth;
	JOVECTOR3 dirRight = CrossProduct(vecNormal, depthVector);
	JOVECTOR3 pointMid1 = _vecPos + dirRight.Normalized() * width;
	JOVECTOR3 dirBack = CrossProduct(vecNormal, dirRight);
	JOVECTOR3 pointMid2 = _vecPos + dirBack.Normalized() * depth;
	JOVECTOR3 dirLeft = CrossProduct(vecNormal, dirBack);
	JOVECTOR3 pointMid3 = _vecPos + dirLeft.Normalized() * width;


	vPoint4[0] = depthVector.Normalized() * depth;
	vPoint4[1] = dirRight.Normalized() * width;
	vPoint4[2] = dirBack.Normalized() * depth;
	vPoint4[3] = dirLeft.Normalized() * width;
}

bool CJxFiPlane::CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo)
{
	return pJxSphere->CollideWithJxFiPlane(this, pContactInfo);;
}

void CJxFiPlane::ResolveWithSphere(CContactInfo* pContactInfo)
{
	//CJxFiPlane* pfiPlane = (CJxFiPlane*)pContactInfo->pActorA;

	CJxSphere* psphereB = (CJxSphere*)pContactInfo->pActorB;

	// Reflect
	JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(psphereB->_vecVelocity * (-1.0f), pContactInfo->vNormal)) * pContactInfo->vNormal + psphereB->_vecVelocity;
	vecNewVelocity.Normalize();

	psphereB->_vecPos = pContactInfo->vNewPosition;
	psphereB->_vecVelocity = psphereB->_vecVelocity.Length() * vecNewVelocity * fBounceness;
}

// collision check.
bool CJxFiPlane::CollideWithOtherActor ( CJxActor * pJxOtherActor, CContactInfo* pContactInfo )
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch (eActor)
	{
		case KE_Sphere: CollideWithSphere((CJxSphere*)pJxOtherActor, pContactInfo);				break;
		default:
			break;
	}
	return true;
}


void CJxFiPlane::SetPreCollide(CJxActor* Actor,bool bCollide)
{
	SCollideInfo* Info = GetCollideInfo(Actor);
	if( Info == NULL )
	{
		SCollideInfo NewInfo;
		NewInfo.Actor = Actor;
		NewInfo.PrevCollide = bCollide;
		NewInfo.CurrentCollide = false;

		m_CollideList.push_back(NewInfo);
	}
	else
	{
		Info->PrevCollide = bCollide;
	}
}

void CJxFiPlane::SetCollide(CJxActor* Actor,bool bCollide)
{

	SCollideInfo* Info = GetCollideInfo(Actor);
	if( Info == NULL )
	{
		SCollideInfo NewInfo;
		NewInfo.Actor = Actor;
		NewInfo.PrevCollide = false;
		NewInfo.CurrentCollide = bCollide;

		m_CollideList.push_back(NewInfo);
	}
	else
	{
		Info->CurrentCollide = bCollide;
	}
}


bool CJxFiPlane::IsPreCollide(CJxActor* Actor)
{
	SCollideInfo* Info = GetCollideInfo(Actor);
	if( Info == NULL )
	{
		return false;
	}
	else
	{
		return Info->PrevCollide ;
	}
}


bool CJxFiPlane::IsCollide(CJxActor* Actor)
{
	SCollideInfo* Info = GetCollideInfo(Actor);
	if( Info == NULL )
	{
		return false;
	}
	else
	{
		return Info->CurrentCollide;
	}
}

CJxFiPlane::SCollideInfo* CJxFiPlane::GetCollideInfo(CJxActor* Actor)
{
	for( int i = 0 ; i<m_CollideList.size() ; i++ )
	{
		if( m_CollideList[i].Actor == Actor )
		{
			return &m_CollideList[i];
		}
	}

return NULL;
}
			