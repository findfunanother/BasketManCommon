//#include "stdafx.h"
#include "JxScene.h"
#include "JxField.h"
#include "JxSphere.h"


CJxField::CJxField()
{
}


CJxField::~CJxField()
{
	//m_pJxScene->RemoveJxActor(this);
	//delete _pfieldDesc;
}

//CJxField::CJxField(CJxScene* pJxScene, CFieldDesc* pJxcyDesc)
//{
//	_pfieldDesc = pJxcyDesc;
//
//	_pfieldDesc->rightGoalInPos = JOVECTOR3(pJxcyDesc->fHalfSizeX, pJxcyDesc->fGoalPostHeightY * 0.5f, 0.0f);
//	_pfieldDesc->leftGoalInPos = JOVECTOR3(-pJxcyDesc->fHalfSizeX, pJxcyDesc->fGoalPostHeightY * 0.5f, 0.0f);
//
//	JOVECTOR3 rNormal = _pfieldDesc->leftGoalInPos - _pfieldDesc->rightGoalInPos;
//	JOVECTOR3 lNormal = _pfieldDesc->rightGoalInPos - _pfieldDesc->leftGoalInPos;
//
//	_pfieldDesc->rightGoalInNormal = rNormal.Normalized();
//	_pfieldDesc->leftGoalInNormal = lNormal.Normalized();
//
//	m_pJxScene = pJxScene;
//	_vecPos = pJxcyDesc->vPos;
//	_vecPrePos = _vecPos;
//	m_pJxScene->AddJxActor(this);
//}

CJxField::CJxField(JOVECTOR3 vecPos, float courtWidth, float courtDepth, float radius, JOVECTOR3 rightGoalInPos, JOVECTOR3 leftGoalInPos)
{
	JOVECTOR3 vecrightnorm = (leftGoalInPos - rightGoalInPos);
	JOVECTOR3 vecleftnorm = (leftGoalInPos - rightGoalInPos);

	_vecPos = vecPos;
	_courtWidth = courtWidth;
	_courtDepth = courtDepth;
	
	_rimRadius = radius;
	
	_rightGoalInPos = rightGoalInPos;
	_leftGoalInPos = leftGoalInPos;
	
	_rightNormal = vecrightnorm.Normalized();
	_leftNormal = vecleftnorm.Normalized();
	_frontNormal = CrossProduct(JOVECTOR3::up, _rightNormal);
	_backNormal = CrossProduct(JOVECTOR3::up, _leftNormal);
}

bool CJxField::CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo)
{
	return pJxSphere->CollideWithField(this, pContactInfo);
}
//
//// collision check.

bool CJxField::CollideWithOtherActor(CJxActor * pJxOtherActor, CContactInfo* pContactInfo)
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