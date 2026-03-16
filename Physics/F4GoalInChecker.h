#pragma once
#include "JxActors.h"
#include "JxSphere.h"
class JPS_API F4GoalInChecker :
    public CJxActor
{
public:
    JOVECTOR3 rightGoalInPos;
    JOVECTOR3 leftGoalInPos;

    float rimRadius;

    JOVECTOR3 rightNormal;
    JOVECTOR3 leftNormal;
    JOVECTOR3 frontNormal;
    JOVECTOR3 backNormal;

    float courtWidth;
    float courtDepth;


    F4GoalInChecker(JOVECTOR3 vecPos, float courtWidth, float courtDepth, float radius, JOVECTOR3 rightGoalInPos, JOVECTOR3 leftGoalInPos);
    virtual bool CollideWihtJxActor(CJxActor* jxActor, CContactInfo* infoContact);

    bool CollideWithJxSphere(CJxSphere* jxSphere, CContactInfo* infoContact);



    // CJxActor을(를) 통해 상속됨
    virtual JPS_API_CAL EKindOfActor GetKindOfActor(void) override;

    virtual JPS_API_CAL bool CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo) override;

};

