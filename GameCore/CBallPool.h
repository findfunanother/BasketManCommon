#pragma once
#define simulMaxCount 2

#include "JxSimulationSphere.h"
#include "CBallSimulationCurve.h"
#include <vector>


class DECLSPEC CBallPool
{
private:

    int curSimulCount = 0;
    CBallSimulationCurve BallArray[simulMaxCount];
    //CBallSimulationCurve simuBallArray[simulMaxCount];
    
    //CJxSimulationSphere ballDataList[simulMaxCount][SIMULBALLMAX];
public:
    CJxSimulationSphere simulationBall;
    DLLOCAL CBallPool()
    {
        /*for (size_t i = 0; i < simulMaxCount; i++)
        {
            for (size_t j = 0; j < SIMULBALLMAX; j++)
            {
                ballDataList[i][j].fRadius = BALL_RADIUS;
            }
        }*/
    }
    DLLOCAL ~CBallPool()
    {

    }
    DLLOCAL void ReadyNextStep();
    DLLOCAL CBallSimulationCurve* GetBallCurve();
    //DLLOCAL CBallSimulationCurve* GetcurSimulCurve();
    DLLOCAL CJxSimulationSphere* SetCurSimulBallInfo(int index, JOVECTOR3 postion, JOVECTOR3 velocity, JOQUATERNION rotation);

    DLLOCAL void SetCurSimullBallEvent(int index, string eventName);

    DLLOCAL string GetCurSimulBallEventFirst(int index);

    //DLLOCAL CJxSimulationSphere* GetCurSimulBallInfo(int index);


    //DLLOCAL CJxSimulationSphere* GetPreSimulBallInfo(int index);

};
