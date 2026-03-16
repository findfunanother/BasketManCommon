#pragma once
#include "JxFiPlane.h"
class JPS_API CJxBox :public CJxActor
{
public:
    float       _fWidth; // X
    float       _fHeight; // Y
    float       _fDepth;  // Z
    JOVECTOR3   _vecNormal;
    CJxFiPlane fiPlane[6];// = new JxFiPlane[6];

    CJxBox(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float fWidth, float fHeight, float fDepth);
    virtual ~CJxBox();
    virtual bool CollideWithJxSphere(CJxSphere* jxSphere, CContactInfo* infoContact);
    virtual	EKindOfActor					GetKindOfActor(void) { return KE_Box; }

    virtual bool							CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo);
};

