#include <limits>
#include <iostream>

#include "pch.h"
#include "CBallSimulationCurve.h"

CBallSimulationCurve::CBallSimulationCurve()
{
    timeReachGroundBound = 0.0f;
    timeReachNoGoal = 0.0f;
    timeReachReboundAbleTime = 0.0f;
}

CBallSimulationCurve::CBallSimulationCurve(unsigned int _ballNumber)
{
    ballNumber = _ballNumber;
}

JOQUATERNION CBallSimulationCurve::GetRotation(float time)
{
    JOQUATERNION quaternion;

    if (BALLDATALISTSIZE > 0)
    {
        int index =  CLAMP((int)(time / FIXEDUPDATETIME), 0, BALLDATALISTSIZE - 1);

        if (index < BALLDATALISTSIZE - 1)
        {
            if (index < 0) return quaternion;
            //float ratio = (time - index * JPhysics.fixedUpdateTime) / JPhysics.fixedUpdateTime;

            return ballDataList[index]._qOrientation;
        }
        else
        {
            return ballDataList[index]._qOrientation;
        }
    }

    return quaternion;
}

JOVECTOR3 CBallSimulationCurve::GetPosition(int index)
{
    JOVECTOR3 vector3;
    if (BALLDATALISTSIZE > 0)
    {
        return ballDataList[MIN(index, BALLDATALISTSIZE - 1)]._vecPos;
    }

    return vector3;
}

JOVECTOR3 CBallSimulationCurve::GetPosition(float time)
{
    JOVECTOR3 vector3;
    if (BALLDATALISTSIZE > 0)
    {
        int index = CLAMP((int)(time / FIXEDUPDATETIME), 0, BALLDATALISTSIZE - 1);

        if (index < BALLDATALISTSIZE - 1)
        {
            float ratio = (time - index * FIXEDUPDATETIME) / FIXEDUPDATETIME;
            return JOVECTOR3::Lerp(ballDataList[index]._vecPos, ballDataList[index + 1]._vecPos, ratio);
        }
        else
        {
            return ballDataList[index]._vecPos;
        }
    }
    else
    {
        return vector3;
    }
}

JOVECTOR3 CBallSimulationCurve::GetVelocity(float time)
{
    if (BALLDATALISTSIZE > 0)
    {
        int index = CLAMP((int)(time / FIXEDUPDATETIME), 0, BALLDATALISTSIZE - 1);

        if (index < BALLDATALISTSIZE - 1)
        {
            float ratio = (time - index * FIXEDUPDATETIME) / FIXEDUPDATETIME;
            return JOVECTOR3::Lerp(ballDataList[index]._vecVelocity, ballDataList[index + 1]._vecVelocity, ratio);
        }
        else
        {
            return ballDataList[index]._vecVelocity;
        }
    }
    else
    {
        JOVECTOR3 vector3;
        return vector3;
    }
}

JOVECTOR3 CBallSimulationCurve::GetVelocity(int index)
{
    JOVECTOR3 vector3;
    if (BALLDATALISTSIZE > 0)
    {
        return ballDataList[MIN(index, BALLDATALISTSIZE - 1)]._vecVelocity;
    }

    return vector3;
}

float CBallSimulationCurve::GetTime(JOVECTOR3 position)
{
    int index = -1;
    float distance = std::numeric_limits<float>::infinity();
    for (int i = 0; i < BALLDATALISTSIZE; i++)
    {
        float x = position.fX - ballDataList[i]._vecPos.fX;
        float z = position.fZ - ballDataList[i]._vecPos.fZ;

        float distanceCurr = static_cast<float>(sqrt(x * x + z * z));

        if (distanceCurr < distance)
        {
            distance = distanceCurr;
            index = i;
        }
        else
        {
            return index * FIXEDUPDATETIME;
        }
    }

    return -1.0f;

}

void CBallSimulationCurve::PopEvent(float time, EVENT_CALLBACK_FUNC eventCallback)
{
    if (BALLDATALISTSIZE > 0)
    {
        int index = GetIndex(time);

        for (int i = 0; i < ballDataList[index].eventList.size(); i++)
        {
            eventCallback(ballDataList[index]._vecPos, ballDataList[index].eventList[i]);
            //Debug.Log("EventName : "  +ballDataList[index].eventList[i] +", FrameNum: " + ballDataList[index].currentNum);
        }
    }
}

void CBallSimulationCurve::PopEvent(int index, EVENT_CALLBACK_FUNC eventCallback)
{
    if (0 < index && index < BALLDATALISTSIZE)
    {
        for (int i = 0; i < ballDataList[index].eventList.size(); i++)
        {
            eventCallback(ballDataList[index]._vecPos, ballDataList[index].eventList[i]);
        }
    }
}

int CBallSimulationCurve::GetIndex(float timeElapsed)
{
    if (BALLDATALISTSIZE > 0)
    {
        return static_cast<int>(CLAMP((roundl(timeElapsed / FIXEDUPDATETIME)), 0.0f, static_cast<float>(BALLDATALISTSIZE - 1)));
    }

    return 0;
}

