//#include "stdafx.h"
#include "JxScene.h"
#include "JxCylinder.h"
#include "JxSphere.h"


CJxCylinder::CJxCylinder()
{
}


CJxCylinder::~CJxCylinder()
{
	//m_pJxScene->RemoveJxActor(this);
	delete _pJxcyDesc;
}

CJxCylinder::CJxCylinder(CJxScene* pJxScene, CJxcyDesc* pJxcyDesc)
{
	_pJxcyDesc = pJxcyDesc;
	m_pJxScene = pJxScene;
	_vecPos = pJxcyDesc->vPos;
	_vecPrePos = _vecPos;
	m_pJxScene->AddJxActor(this);

}

bool CJxCylinder::CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo)
{
	return pJxSphere->CollideWithJxCylinder(this, pContactInfo);
}

// collision check.
bool CJxCylinder::CollideWithOtherActor(CJxActor * pJxOtherActor, CContactInfo* pContactInfo)
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch (eActor)
	{
		case KE_Sphere: CollideWithSphere((CJxSphere*)pJxOtherActor, pContactInfo);			break;
		default:
			break;
	}
	return true;
}