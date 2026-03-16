#include "pch.h"
#include "CBallPool.h"

void CBallPool::ReadyNextStep()
{
    curSimulCount++;
    if (curSimulCount >= simulMaxCount) { curSimulCount = 0; }
}

CBallSimulationCurve* CBallPool::GetBallCurve()
{
    return &BallArray[curSimulCount];
}




CJxSimulationSphere* CBallPool::SetCurSimulBallInfo(int index, JOVECTOR3 postion, JOVECTOR3 velocity, JOQUATERNION rotation)
{
    simulationBall._vecPos = postion;
    simulationBall._qOrientation = rotation;
    //simuBallArray[curSimulCount][index].eventList = eventList;
    return &simulationBall;
}


void CBallPool::SetCurSimullBallEvent(int index, string eventName)
{
    simulationBall.eventList.push_back(eventName);
}

string CBallPool::GetCurSimulBallEventFirst(int index)
{
    return "None";
}

