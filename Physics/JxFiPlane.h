#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#ifdef EXPORT_DLL
#define JPS_API __declspec(dllexport)
#else
#define JPS_API __declspec(dllimport) 
#endif
#define JPS_API_CAL
#else
#define JPS_API __attribute__((visibility("default")))
#define JPS_API_CAL __attribute__((visibility("hidden")))
#pragma clang diagnostic ignored "-Wunused-variable"
#endif


#include "JxActors.h"
#include <vector>

class CJxSphere;

class CJxfpDesc  // finite palne desc.
{
public:
	CJxfpDesc():fBounceness(0.f),pUserData(nullptr) {}
	~CJxfpDesc() {}
public:
	JOVECTOR3			vPos;   // 이 평면의 노멀 벡터 
	JOVECTOR3			vNormal;   // 이 평면의 노멀 벡터 
	JOVECTOR3			fWidth;    // 
	JOVECTOR3			fDepth;    // 
	JOVECTOR3           vDepthVector;
	JOVECTOR3           vPoint4[4];
	FREAL               fBounceness; // 
	void*				pUserData;
};


class JPS_API CJxFiPlane : public CJxActor
{
public:
	struct	SCollideInfo
	{
		CJxActor*	Actor;
		bool		PrevCollide;
		bool		CurrentCollide;
	};

public:
	JOVECTOR3			vPos;   // 이 평면의 노멀 벡터 
	JOVECTOR3			vNormal;   // 이 평면의 노멀 벡터 
	JOVECTOR3			fWidth;    // 
	JOVECTOR3			fDepth;    // 
	JOVECTOR3           vDepthVector;
	JOVECTOR3           vPoint4[4];
	FREAL               fBounceness; // 

	//CJxFiPlane(CJxScene* pJxScene, CJxfpDesc* pJxfpDesc);
	CJxFiPlane(void);
	~CJxFiPlane(void);
	CJxFiPlane(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float width, float depth, float bounciness);
	//CJxFiPlane(JOVECTOR3 pos, float width, float depth, float radius,JOVECTOR3 rightGoalPos,JOVECTOR3 leftGoalPos);
public:

	void SetFiPlane(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float width, float depth, float bounciness);

public:

	void									SetPreCollide(CJxActor* Actor,bool bCollide);
	void									SetCollide(CJxActor* Actor,bool bCollide);

	bool									IsPreCollide(CJxActor* Actor);
	bool									IsCollide(CJxActor* Actor);

private:
	SCollideInfo*							GetCollideInfo(CJxActor* Actor);
	std::vector<SCollideInfo>				m_CollideList;


public:
	EKindOfActor							GetKindOfActor(void) { return KE_FPlane; } 
	// collision check.
	bool									CollideWithOtherActor ( CJxActor * pJxOtherActor, CContactInfo* pContactInfo );
	bool									CollideWithSphere(CJxSphere * pJxSphere, CContactInfo* pContactInfo);

	void									ResolveWithSphere(CContactInfo* pContactInfo);

};
