#pragma once

#include "JxActors.h"
#include <vector>

class CJxSphere;

//class CFieldDesc  // finite palne desc.
//{
//public:
//	CFieldDesc() {}
//	~CFieldDesc() {}
//
//public:
//	JOVECTOR3			vPos;        // 이 평면의 노멀 벡터 
//	JOVECTOR3			vNormal;     // 
//
//	float				fHalfSizeX;
//	float				fHalfSizeZ;
//	float               fGoalPostHeightY;
//	float               fGoalPostWidthZ;
//	bool				bIsCollisonEvent;
//
//	JOVECTOR3			rightGoalInPos;
//	JOVECTOR3			leftGoalInPos;
//	JOVECTOR3			rightGoalInNormal;
//	JOVECTOR3			leftGoalInNormal;
//
//	void*				pUserData;
//};


class JPS_API CJxField : public CJxActor
{
public:
	JOVECTOR3			vPos;        // 이 평면의 노멀 벡터 
	JOVECTOR3			vNormal;     // 

	float				fHalfSizeX;
	float				fHalfSizeZ;
	float               fGoalPostHeightY;
	float               fGoalPostWidthZ;
	bool				bIsCollisonEvent;

	JOVECTOR3			rightGoalInPos;
	JOVECTOR3			leftGoalInPos;
	JOVECTOR3			rightGoalInNormal;
	JOVECTOR3			leftGoalInNormal;

	JOVECTOR3 _rightGoalInPos;
	JOVECTOR3 _leftGoalInPos;

	float   _rimRadius;

	JOVECTOR3 _rightNormal;
	JOVECTOR3 _leftNormal;
	JOVECTOR3 _frontNormal;
	JOVECTOR3 _backNormal;

	float _courtWidth;
	float _courtDepth;

public:
	CJxField();
	~CJxField();
	//CJxField(CJxScene* pJxScene, CFieldDesc* pJxcyDesc);
	CJxField(JOVECTOR3 vecPos, float courtWidth, float courtDepth, float radius, JOVECTOR3 rightGoalInPos, JOVECTOR3 leftGoalInPos);
public:
	//CFieldDesc*			_pfieldDesc;


public:
	virtual EKindOfActor							GetKindOfActor(void) { return KE_F3Field; }

	
	virtual bool							CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo);
	
	// collision check.
	//bool									CollideWithOtherActor(CJxActor * pJxOtherActor, CContactInfo* pContactInfo);
	virtual bool    						CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo);
};

