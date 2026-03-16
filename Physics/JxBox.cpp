#include "JxScene.h"
#include "JxBox.h"
#include "JxSphere.h"

CJxBox::CJxBox(JOVECTOR3 pos, JOVECTOR3 vecNormal, JOVECTOR3 depthVector, float fWidth, float fHeight, float fDepth)
{
    
    _vecPos = pos;
    _vecNormal = vecNormal;

    _fWidth = fWidth;
    _fHeight = fHeight;
    _fDepth = fDepth;

    const float bounceFactor = 0.5f;

    JOVECTOR3 normal[6];// = new Vector3[6];
    JOVECTOR3 realPos[6];// = new Vector3[6];

    normal[0] = CrossProduct(vecNormal, depthVector);
    realPos[0] = pos + normal[0] * fWidth;
    fiPlane[0].SetFiPlane(realPos[0], normal[0].Normalized(), depthVector, fHeight, fDepth, bounceFactor);

    normal[1] = CrossProduct(vecNormal, normal[0]);
    realPos[1] = pos + normal[1] * fDepth;
    fiPlane[1].SetFiPlane(realPos[1], normal[1].Normalized(), vecNormal, fWidth, fHeight, bounceFactor);

    normal[2] = CrossProduct(vecNormal, normal[1]);
    realPos[2] = pos + normal[2] * fWidth;
    fiPlane[2].SetFiPlane(realPos[2], normal[2].Normalized(), depthVector, fHeight, fDepth, bounceFactor);

    normal[3] = vecNormal;
    realPos[3] = pos + normal[3] * fHeight;
    fiPlane[3].SetFiPlane(realPos[3], normal[3].Normalized(), depthVector, fWidth, fDepth, bounceFactor);

    normal[4] = CrossProduct(vecNormal, normal[2]);
    realPos[4] = pos + normal[4] * fDepth;
    fiPlane[4].SetFiPlane(realPos[4], normal[4].Normalized(), vecNormal, fWidth, fHeight, bounceFactor);

    normal[5] = JOVECTOR3::zero - vecNormal;
    realPos[5] = pos + normal[5] * fHeight;
    fiPlane[5].SetFiPlane(realPos[5], normal[5].Normalized(), depthVector, fWidth, fDepth, bounceFactor);
}

CJxBox::~CJxBox()
{
    //m_pJxScene->RemoveJxActor(this);
    //delete _pJxcyDesc;
}

bool CJxBox::CollideWithJxSphere(CJxSphere* jxSphere, CContactInfo* infoContact)
{
    
    return jxSphere->CollideWithJxBox(this, infoContact);
    
}

bool CJxBox::CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo)
{
    return  CollideWithJxSphere((CJxSphere*)pJxOtherActor, pContactInfo);
}

