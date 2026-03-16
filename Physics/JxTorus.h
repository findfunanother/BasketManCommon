#pragma once

#include "JxActors.h"

class CJxScene;
class CJxSphere;
class CJxController;

//
//class CJxtDesc       // sphere desc.
//{
//public:
//	CJxtDesc(){ fRadius = 12.0f;}
//	~CJxtDesc(){}
//public:
//	float			fRadius;
//	float			fDistance;
//	JOVECTOR3		vNormal;
//};
//

class JPS_API CJxTorus : public CJxActor
{
public:
	float			fRadius;
	float			fDistance;
	JOVECTOR3		vNormal;
	//CJxTorus( CJxScene* pJxScene, CJxtDesc* pJxtDesc );
	CJxTorus(void);
	virtual ~CJxTorus(void);
	CJxTorus(JOVECTOR3 _position, JOVECTOR3 _normal,float _fDistance, float _fRadius);

public:
	void							Intergrate( float fTimeDelta );
	void							ReadyToNextStep( float fTimeDelta );

public:
	//..CJxtDesc*						_pJxtDesc;	


public:
	EKindOfActor					GetKindOfActor(void) { return KE_Torus; }


	bool							CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo );
	
	bool							CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo );
	
private:
	// resolve.
	void							ResolveWithSphere( CContactInfo* pContactInfo );

};
