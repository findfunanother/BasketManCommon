#include "pch.h"
#include "JxSimulationSphere.h"

//string CJxSimulationSphere::eventFirstBound = "Event_FirstBound";     // 그라운드 
//string CJxSimulationSphere::eventFirstRimBound = "Event_FirstRimBound";  // 림이 첫음 바운드 됐을 때 
//string CJxSimulationSphere::eventRimBound = "Event_RimBound";       // 림이 첫음 바운드 됐을 때 
//string CJxSimulationSphere::eventReboundAble = "Event_ReboundAble";        // 리바운드가 가능한 이벤트 
//string CJxSimulationSphere::eventFirstBackBoardBound = "Event_FirstBackBoard"; // 백보드 
//string CJxSimulationSphere::eventRightGoalIn = "Event_RightGoalIn";
//string CJxSimulationSphere::eventLeftGoalIn = "Event_LeftGoalIn";
//string CJxSimulationSphere::eventBoundSound = "Event_BoundSound";
//string CJxSimulationSphere::eventBoundary = "Event_Boundary";
//string eventBackBoardBound = "Event_BackBoardBound";

CJxSimulationSphere::CJxSimulationSphere():currentNum(0)
{
    
}

CJxSimulationSphere::~CJxSimulationSphere()
{
}

CJxSimulationSphere CJxSimulationSphere::Copy(CJxSimulationSphere jxActor)
{
    jxActor._vecPos = _vecPos;
    jxActor._vecPrePos = _vecPrePos;
    jxActor._vecPos = jxActor._vecPos;
    jxActor._vecVelocity = _vecVelocity;
    jxActor._qOrientation = _qOrientation;
    jxActor._actorID = _actorID;
    jxActor.eventList = eventList;
    return jxActor;
}

void CJxSimulationSphere::CopyFrom(CJxSimulationSphere* jxActor)
{
    _vecPos = jxActor->_vecPos;
    _vecPrePos = jxActor->_vecPrePos;
    //jxActor._vecPos = jxActor->_vecPos;
    _vecVelocity = jxActor->_vecVelocity;
    _qOrientation = jxActor->_qOrientation;
    _actorID = jxActor->_actorID;
    eventList = jxActor->eventList;
    
}

CJxSimulationSphere::CJxSimulationSphere(JOVECTOR3 vecPos3, float fRadius):currentNum(0)// : base(gameObject, vecPos3, fRadius)
{
}

CJxSimulationSphere::CJxSimulationSphere(int simulationNum, JOVECTOR3 vecPos3, JOVECTOR3 velocity, float curvalue, JOVECTOR3 curveNormal, float fradius)
{
    currentNum = simulationNum;
    firstBound = false;
    _vecPos = vecPos3;
    _vecVelocity = velocity;
    _curveValue = curvalue;
    _vMoment = curveNormal;
    fRadius = fradius;
    //this.fDrag = AIRDRA;
}
void CJxSimulationSphere::SetSimulationSphere(int simulationNum, JOVECTOR3 vecPos3, JOVECTOR3 velocity, float curvalue, JOVECTOR3 curveNormal, float fradius)
{
    currentNum = simulationNum;
    firstBound = false;
    _vecPos = vecPos3;
    _vecVelocity = velocity;
    _curveValue = curvalue;
    _vMoment = curveNormal;
    fRadius = fradius;
    //this.fDrag = AIRDRA;
}
void CJxSimulationSphere::OnEventFirstBound()
{}

void CJxSimulationSphere::OnEventBoundSound()
{
    eventList.push_back("Event_BoundSound");
}

void CJxSimulationSphere::OnEventContactToCylinder()
{
    eventList.push_back("OnEventContactToCylinder");
}
void CJxSimulationSphere::OnEventRightGoalIn()
{
    eventList.push_back("Event_LeftGoalIn");
}

void CJxSimulationSphere::OnEventContactToField(string eventName)
{
    eventList.push_back(eventName);
}

void CJxSimulationSphere::OnEventBoundary()
{
    //Debug.LogError("OnEventBoundary: " +currentNum);
    eventList.push_back("Event_Boundary");
}

void CJxSimulationSphere::OnEventLeftGoalIn()
{
}

void CJxSimulationSphere::OnEventFirstRimBound()
{
#if SHOW_LOG
    Debug.LogError("OnEventFirstRimBound!!: " + currentNum);
#endif
    //Debug.LogError("OnEventFirstRimBound!!: " + currentNum);
    eventList.push_back("Event_FirstRimBound");
}

void CJxSimulationSphere::OnEventRimBound()
{
    eventList.push_back("Event_RimBound");
}

void CJxSimulationSphere::OnEventFirstBackBoardBound()
{
    eventList.push_back("Event_FirstBackBoard");
}

void CJxSimulationSphere::OnEventBackBoardBound()
{

    eventList.push_back("Event_BackBoardBound");

}
