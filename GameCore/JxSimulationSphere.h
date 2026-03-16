#pragma once

#include "JxSphere.h"


#include <vector>
#ifndef DECLSPEC

#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport) 
#endif

#define DLLOCAL
#else
#define DECLSPEC __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif
#endif

class DECLSPEC CJxSimulationSphere : public CJxSphere
{
public:
    //DLLOCAL static string eventFirstBound;// = "Event_FirstBound";     // 그라운드 
    //DLLOCAL static string eventFirstRimBound;// = "Event_FirstRimBound";  // 림이 첫음 바운드 됐을 때 
    //DLLOCAL static string eventRimBound;// = "Event_RimBound";       // 림이 첫음 바운드 됐을 때 
    //DLLOCAL static string eventReboundAble;// = "Event_ReboundAble";        // 리바운드가 가능한 이벤트 
    //DLLOCAL static string eventFirstBackBoardBound;// = "Event_FirstBackBoard"; // 백보드 
    //DLLOCAL static string eventRightGoalIn;// = "Event_RightGoalIn";
    //DLLOCAL static string eventLeftGoalIn;// = "Event_LeftGoalIn";
    //DLLOCAL static string eventBoundSound;// = "Event_BoundSound";
    //DLLOCAL static string eventBoundary;// = "Event_Boundary";
    //DLLOCAL static string eventBackBoardBound;// = "Event_BackBoardBound";
    vector<string> eventList;// = new list<string>();
    int currentNum;// { set; get; }

    ~CJxSimulationSphere();

    DLLOCAL CJxSimulationSphere Copy(CJxSimulationSphere jxActor);
    DLLOCAL void CopyFrom(CJxSimulationSphere* jxActor);

    DLLOCAL CJxSimulationSphere();// { }
    DLLOCAL CJxSimulationSphere(JOVECTOR3 vecPos3, float fRadius);


    DLLOCAL CJxSimulationSphere(int simulationNum, JOVECTOR3 vecPos3, JOVECTOR3 velocity, float curvalue, JOVECTOR3 curveNormal, float fRadius);
    DLLOCAL void SetSimulationSphere(int simulationNum, JOVECTOR3 vecPos3, JOVECTOR3 velocity, float curvalue, JOVECTOR3 curveNormal, float fRadius);
    DLLOCAL void OnEventFirstBound();
    DLLOCAL void OnEventBoundSound();
    DLLOCAL void OnEventContactToCylinder();
    DLLOCAL virtual void OnEventRightGoalIn();
    DLLOCAL void OnEventLeftGoalIn();
    DLLOCAL void OnEventContactToField(string eventName);
    DLLOCAL void OnEventBoundary();
    DLLOCAL virtual void OnEventFirstRimBound();
    DLLOCAL virtual void OnEventRimBound();
    DLLOCAL virtual void OnEventFirstBackBoardBound();
    DLLOCAL virtual void OnEventBackBoardBound();
    
    
};
