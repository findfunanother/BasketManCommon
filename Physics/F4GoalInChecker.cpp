#include "F4GoalInChecker.h"
#include <typeinfo>

F4GoalInChecker::F4GoalInChecker(JOVECTOR3 vecPos, float _courtWidth, float _courtDepth, float _radius, JOVECTOR3 _rightGoalInPos, JOVECTOR3 _leftGoalInPos)
{

    _vecPos = vecPos;
    courtWidth = _courtWidth;
    courtDepth = _courtDepth;

    rimRadius = _radius;

    rightGoalInPos = _rightGoalInPos;
    leftGoalInPos = _leftGoalInPos;

    rightNormal = (_leftGoalInPos - _rightGoalInPos);
    leftNormal = (_rightGoalInPos - _leftGoalInPos);
    rightNormal.Normalize();
    leftNormal.Normalize();

    frontNormal = CrossProduct(JOVECTOR3::up, rightNormal);
    backNormal = CrossProduct(JOVECTOR3::up, leftNormal);


}

bool F4GoalInChecker::CollideWihtJxActor(CJxActor* jxActor, CContactInfo* infoContact)
{

    if (typeid(jxActor) == typeid(CJxSphere))
    {
        return CollideWithJxSphere((CJxSphere*)jxActor, infoContact);
    }
    return false;
}

bool F4GoalInChecker::CollideWithJxSphere(CJxSphere* jxSphere, CContactInfo* infoContact)
{
    return jxSphere->CollideWithJxF4GoalInChecker(this, infoContact);
}

JPS_API_CAL EKindOfActor F4GoalInChecker::GetKindOfActor(void)
{
    return KE_F4GoalInChecker;
}

JPS_API_CAL bool F4GoalInChecker::CollideWithOtherActor(CJxActor* pJxOtherActor, CContactInfo* pContactInfo)
{
    return true;
}
