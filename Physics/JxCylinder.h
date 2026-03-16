#pragma once

#include "JxActors.h"
#include <vector>

class CJxSphere;

class CJxcyDesc  // finite palne desc.
{
public:
	CJxcyDesc() { fRadius = 0.1f; }
	~CJxcyDesc() {}

public:
	JOVECTOR3			vPos;        // 이 평면의 노멀 벡터 
	JOVECTOR3			vNormal;     // 
	FREAL               fBounceness; // 
	FREAL               fRadius;
	FREAL               fHeight;
	void*				pUserData;
};


class CJxCylinder : public CJxActor
{
public:
	CJxCylinder();
	~CJxCylinder();
	CJxCylinder(CJxScene* pJxScene, CJxcyDesc* pJxcyDesc);

public:
	CJxcyDesc*			_pJxcyDesc;


public:
	EKindOfActor							GetKindOfActor(void) { return KE_Cylinder; }
	// collision check.
	bool									CollideWithOtherActor(CJxActor * pJxOtherActor, CContactInfo* pContactInfo);
	bool									CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo);
};

