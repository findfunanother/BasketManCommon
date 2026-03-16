#include "pch.h"
#include "CBallSimulator.h"
#include "JxInPlane.h"
#include "JxTorus.h"
#include "JEnviroment.h"
#include "JxField.h"
#include "JxBox.h"
#include "F4GoalInChecker.h"
void CBallSimulator::Initialize()
{
    pool = new CBallPool();
    //jxSphereObject = new GameObject("SimulationObject");
    jxSimulationScene = new CJxScene();

    jxFisrtCurveSimulationScene = new CJxScene();

    actorSet.clear();
    CJxActor* jxActorPlane = new CJxInPlane(JOVECTOR3(0.0f, 0.0f, 0.0f), JOVECTOR3(0.0f, 1.0f, 0.0f),GROUNDBOUNCE, true, GROUNDFRICTION);
    jxSimulationScene->AddJxActor(jxActorPlane);
    actorSet.push_back(jxActorPlane);
    CJxActor* jxActorRim = new CJxTorus(JOVECTOR3(RIM_POS_X, RIM_POS_Y, RIM_POS_Z()), JOVECTOR3::up, RIM_HALF_DISTANCE, RIM_HALF_CIRCLE);
    jxSimulationScene->AddJxActor(jxActorRim);
    actorSet.push_back(jxActorRim);
    CJxActor* jxActorfield = new CJxField(JOVECTOR3::zero, COURT_WIDTH_HALF(), COURT_DEPTH, RIM_HALF_DISTANCE, JOVECTOR3(RIM_POS_X, RIM_POS_Y, RIM_POS_Z()), JOVECTOR3(RIM_POS_X, RIM_POS_Y, RIM_POS_NEGATIVE_Z()));
    jxSimulationScene->AddJxActor(jxActorfield);
    actorSet.push_back(jxActorfield);

    JOVECTOR3 backBoardPos = JOVECTOR3(0.0f, RIM_POS_Y + BOARD_HALF_HEIGHT - BOARD_EXTEND, RIM_POS_Z() + RIM_HALF_DISTANCE + RIM_EXTEND);
    jxFisrtCurveSimulationSceneBefore = new CJxBox(backBoardPos, JOVECTOR3::up, JOVECTOR3(0.f, 0.f, 1.f), BOARD_HALF_DEPTH_BEFORE, BOARD_HALF_HEIGHT_BEFORE, BOARD_HALF_WIDTH);
    jxFisrtCurveSimulationScene->AddJxActor(jxFisrtCurveSimulationSceneBefore);
    actorSet.push_back(jxFisrtCurveSimulationSceneBefore);

    CJxActor* jxActorBackBoard = new CJxBox(backBoardPos, JOVECTOR3::up, JOVECTOR3(0.f, 0.f, 1.f), BOARD_HALF_DEPTH, BOARD_HALF_HEIGHT, BOARD_HALF_WIDTH);
    jxSimulationScene->AddJxActor(jxActorBackBoard);
    actorSet.push_back(jxActorBackBoard);
    
    //CJxActor* jxF4GoalInChecker = new F4GoalInChecker(JOVECTOR3::zero, COURT_WIDTH_HALF(), COURT_DEPTH, RIM_HALF_DISTANCE, JOVECTOR3(RIM_POS_X, RIM_POS_Y, RIM_POS_Z()),
    //    JOVECTOR3(RIM_POS_X, RIM_POS_Y, RIM_POS_NEGATIVE_Z()));
    //jxSimulationScene->AddJxActor(jxF4GoalInChecker);

    //pool = new BallPool();
}

CBallSimulator::~CBallSimulator()
{
    //for (size_t i = 0; i < actorSet.size(); i++)
    //{
    //    delete(actorSet[i]);
    //}

    if (pool != NULL)
    {
        delete(pool);
    }
    if (jxSimulationScene != NULL)
        delete(jxSimulationScene);
    if (jxFisrtCurveSimulationScene != NULL)
        delete(jxFisrtCurveSimulationScene);
    for (size_t i = 0; i < actorSet.size(); i++)
    {
        delete(actorSet[i]);
    }
    actorSet.clear();
}

CBallSimulationCurve* CBallSimulator::MakeNewCurveFromTwoCurve(float reachTime, string *addEvent, JOVECTOR3 startPos, JOVECTOR3 targetPos, JOVECTOR3 targetVelocity, bool backboardCheck)
{
    if (isnan(reachTime))
    {
        reachTime = 2.0f;
    }

    int pickIndex = 0;

    CBallSimulationCurve *before,*after;

    if (backboardCheck)
    {
        jxFisrtCurveSimulationScene->RemoveJxActorClear(jxFisrtCurveSimulationSceneBefore); // 유저들이 불편해 해서 백보드 맞는 것을 뺌 
    }
    else
    {
        jxFisrtCurveSimulationScene->AddJxActorUnique(jxFisrtCurveSimulationSceneBefore); // 특정 점프슛외에 나머지 슛들은 백보드 체크를 하자 
    }
    
    int k = static_cast<int>(rint(MAX(1, reachTime / FIXEDUPDATETIME)));
    if (startPos.fX >= 0) // 반대인 경우 
    {
        targetPos.fX = -targetPos.fX;
        JOVECTOR3 vecVelocity;
        
        float fstartPos[] = { startPos.fX,startPos.fY,startPos.fZ };
        float ftargetPos[] = { targetPos.fX,targetPos.fY,targetPos.fZ };

        vecVelocity = GetInitialVelocity(reachTime, fstartPos, ftargetPos);
        before = SimulateCurrentCurve(jxFisrtCurveSimulationScene, startPos, vecVelocity, CURVEVALUE, JOVECTOR3::up, BALL_RADIUS, false, k, pickIndex);
        bool curveChanged = false;

        for (int i = 0; i < k; i++)
        {
            auto simulationBall = &before->ballDataList[i];
            for (size_t j = 0; j < simulationBall->eventList.size(); j++)
            {
                if (simulationBall->eventList[j] == "Event_FirstBackBoard") 
                {
                    simulationBall->firstBound = true;
                    k = i;
                    targetVelocity = simulationBall->_vecVelocity;
                    targetPos = simulationBall->_vecPos;
                    curveChanged = true;
                    *addEvent = "Event_FirstBackBoard";
                    break;
                }
            }
        }
        
        // 첫번째 커브에는 시뮬레이션 오브젝트들이 빠졌음( 날아가는 용도만 ) 
        if(!curveChanged)targetVelocity.fX = -targetVelocity.fX;
       
        // 두번째 커브 
        int count = frameCount - k;
        after = SimulateCurrentCurve(jxSimulationScene, targetPos, targetVelocity, CURVEVALUE, JOVECTOR3::up, BALL_RADIUS, true, count, pickIndex);
    }
    else
    {
        JOVECTOR3 vecVelocity;// = JPhysics.GetInitialVelocity(reachTime, startPos, targetPos, JPhysics.airDrag);
         float fstartPos[] = { startPos.fX,startPos.fY,startPos.fZ };
        float ftargetPos[] = { targetPos.fX,targetPos.fY,targetPos.fZ };

        vecVelocity = GetInitialVelocity(reachTime, fstartPos, ftargetPos);
        before = SimulateCurrentCurve(jxFisrtCurveSimulationScene, startPos, vecVelocity, CURVEVALUE, JOVECTOR3::up, BALL_RADIUS, false, k, pickIndex);

        bool curveChanged = false;

        for (int i = 0; i < k; i++)
        {
            auto simulationBall = &before->ballDataList[i];
            for (size_t j = 0; j < simulationBall->eventList.size(); j++)
            {
                if (simulationBall->eventList[j] == "Event_FirstBackBoard")
                {
                    simulationBall->firstBound = true;
                    k = i;
                    targetVelocity = simulationBall->_vecVelocity;
                    targetPos = simulationBall->_vecPos;
                    curveChanged = true;
                    *addEvent = "Event_FirstBackBoard";
                    break;
                }
            }
        }

        int count = frameCount - k;
        after = SimulateCurrentCurve(jxSimulationScene, targetPos, targetVelocity, CURVEVALUE, JOVECTOR3::up, BALL_RADIUS, true, count, pickIndex);
    }

    bool copyStart = false;
    int targetIndex = 0;
    float elapsedTime = 0.0f;

    float timeReachNogoal = -1.0f;

    int lastBoundIndex = 0;
    int reboundLastIndex = 0;
    int pickLastIndex = 0;
    float timeReachNoGoal = -1.0f;
    float timeReachGround = -1.0f;

    bool changeRimbound = false;
    bool goalInCheck = false;

    for (int i = k; i < BALLDATALISTSIZE; i++)
    {
        elapsedTime = FIXEDUPDATETIME*(i);

        if (!copyStart)
        {
            if (i >= k)
            {
                copyStart = true;
                after->ballDataList[targetIndex].eventList.push_back(*addEvent);
            }
        }
        //int t = sizeof(CJxSimulationSphere);
        
        before->ballDataList[i].CopyFrom(&after->ballDataList[targetIndex]);

        if (after->ballDataList[targetIndex].eventList.size() > 0)
        {
            if (after->ballDataList[targetIndex].eventList[0] == "Event_RimBound" ||
                after->ballDataList[targetIndex].eventList[0] == "Event_FirstRimBound" || 

                after->ballDataList[targetIndex].eventList[0] == "Event_FirstBackBoard" || 
                after->ballDataList[targetIndex].eventList[0] == "Event_BackBoardBound" )

            {
                changeRimbound = true; // 림바운드가 일어 났다면 

                lastBoundIndex = i; // 마지막 바운드 저장 
                timeReachNoGoal = elapsedTime;

                reboundLastIndex = i; // 훅샷이 림에 맞으면 리바운드와 픽이 안되는 경우가 있어서 수정 , 이걸 해줘야 픽이 계산됨 

            }

            if (after->ballDataList[targetIndex].eventList[0] == "Event_BoundSound")
            {
                if (timeReachGround < 0)
                {
                    timeReachGround = elapsedTime;
                }
            }

            // 골인 솔루션인지 체크 
            if (after->ballDataList[targetIndex].eventList[0] == "Event_RightGoalIn")
            {
                goalInCheck = true;
            }
        }

        targetIndex++;
        
        before->ballDataList[i].currentNum = i;

        // 볼이 고점에서 꺽이는 부분을 체크( 클라이언트와 다름, 같게 하려면 부하게 생김, 정호씨가 이해를 잘못한 것 같음, 나중에 수정을 해야 함 by steven, 2023-02-07 )
        // _vecPreVelocity 가 모두 0 보다 작은 경우가 있나 ? reboundLastIndex 값 최초를 이벤트가 발생하는 시점으로 우선 넣음 

        if (changeRimbound && before->ballDataList[i]._vecPreVelocity.fY >= 0.0f && before->ballDataList[i]._vecVelocity.fY < 0.0f) 
        {
            changeRimbound = false;
            reboundLastIndex = i; // 계속 업데이트 , 클라이언트에서는 + 1 
        }

        if (reboundLastIndex > 0 && before->ballDataList[i]._vecPrePos.fY >= 2.0f && before->ballDataList[i]._vecPos.fY < 2.0f) // 2미터 
        {
            pickLastIndex = i;
        }
    }

    const int addFrameIndex = 0;
    int noGoalIndex = static_cast<int>(rint(MAX(1, timeReachNoGoal / FIXEDUPDATETIME))) + addFrameIndex;
    before->timeReachGroundBound = timeReachGround;
    before->timeReachNoGoal = timeReachNoGoal + addFrameIndex * FIXEDUPDATETIME;
    before->timeReachReboundAbleTime = reboundLastIndex * FIXEDUPDATETIME;
    before->timeReachPickAbleTime = pickLastIndex * FIXEDUPDATETIME; // 클라이언트는 2m 

    if (reboundLastIndex > 0 && !goalInCheck) // 노골인 경우와 업데이트가 된 경우 
    {
        before->ballDataList[reboundLastIndex].eventList.push_back("Event_ReboundAble"); // 이것 안쓰는 것 같음, 여기서는 의미 없음, 다시 AddShotBallData 에서 처리 (2025-04-05 )
        before->ballDataList[noGoalIndex].eventList.push_back("Event_NoGoal"); // 이것 안쓰는 것 같음 
        //GameObject sphere1 = JUsageWork.CreatePrimitive(PrimitiveType.Cube, before.ballDataList[rimBoundLastIndex].vecPos, 1.0f, 1.0f, 1.0f, 0.5f, 0.24f, "ReboundBall");
    }

    return before;
}


CBallSimulationCurve* CBallSimulator::MakeNewCurveOnly(JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize, int& index, bool firstBound/* Firt Bound*/)
{
    int framecount = frameCount;
    return SimulateCurrentCurve(jxSimulationScene, vecStartPos, vecVelocity, curveValue, normal, ballSize, firstBound, framecount, index);
}

JOVECTOR3 CBallSimulator::ReCalcTargetPosition(JOVECTOR3 vecStartPos, float ballSize)
{
    //CJxSimulationSphere simulationBall(0, vecStartPos, JOVECTOR3::zero, 1.0f, JOVECTOR3::zero, ballSize);
    //simulationBall.firstBound = false;
    //jxSimulationScene.SimulateActorOnly(&simulationBall, FIXEDUPDATETIME);

    ////Debug.Log("Simulate Completed");
    return JOVECTOR3::zero;
}

CBallSimulationCurve* CBallSimulator::SimulateCurrentCurve(CJxScene* jxScene, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize, bool firstBound/* Firt Bound*/,int& simCount, int& index)
{
    pool->ReadyNextStep();
    CBallSimulationCurve* ballCurve = pool->GetBallCurve();
    //CBallSimulationCurve* ballSimulationCurve = pool->GetBallCurve();
    
    CJxSimulationSphere* simulationBall = pool->SetCurSimulBallInfo(0, vecStartPos, vecVelocity, JOQUATERNION::identity);
    simulationBall->SetSimulationSphere(0, vecStartPos, vecVelocity, curveValue, normal, ballSize);
    simulationBall->firstBound = firstBound;
    ballCurve->ballDataList[0].CopyFrom(simulationBall);
    JOVECTOR3 pos = simulationBall->_vecPos;
    
    simulationBall->_vecPrePos = vecStartPos; // by findfun, 2021-03-31 코트밖으로 나가는 버그를 수정하기 위해서 

    int pickIndex = 0;

    for (int i = 1; i < simCount; i++)
    {
        simulationBall->eventList.clear();
        
        jxScene->SimulateActorOnly(simulationBall, FIXEDUPDATETIME);
        simulationBall->currentNum = i;
        
        //pool->SetCurSimulBallInfo(i, simulationBall->_vecPos, simulationBall->_vecVelocity, simulationBall->_qOrientation);

        /*for (int k = 0; k < simulationBall->eventList.size(); k++)
        {
            pool->SetCurSimullBallEvent(i, simulationBall->eventList[k]);
        }*/

        //Debug.DrawLine(pos, simulationBall.vecPos, Color.green, 10f);
        
        pos = simulationBall->_vecPos;

        if (simulationBall->_vecPos.fY < 2.0f && pickIndex == 0 )
        {
            pickIndex = i; // 2보다 작은 것 중에서 맨 첫번째 것 
        }

        ballCurve->ballDataList[i].CopyFrom(simulationBall);
    }

    index = pickIndex;

    ballCurve->ready = false;
    //Debug.Log("Simulate Completed");
    return ballCurve;
}

void CBallSimulator::SimulateSync(unsigned int ballNumber, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize)
{
    pool->ReadyNextStep();
    CBallSimulationCurve* ballSimulationCurve = pool->GetBallCurve();
    ballSimulationCurve->ballNumber = ballNumber;
    CJxSimulationSphere simulationBall(0, vecStartPos, vecVelocity, curveValue, normal, ballSize);

    
    //CJxSimulationSphere *startBall = pool.SetCurSimulBallInfo(0, simulationBall._vecPos, simulationBall._vecVelocity, JOQUATERNION::identity);
    for (int i = 0; i < frameCount; i++)
    {
        simulationBall.eventList.clear();

        jxSimulationScene->SimulateActorOnly(&simulationBall, FIXEDUPDATETIME);

        pool->SetCurSimulBallInfo(i, simulationBall._vecPos, simulationBall._vecVelocity, simulationBall._qOrientation);

        for (int k = 0; k < simulationBall.eventList.size(); k++)
        {
            pool->SetCurSimullBallEvent(i, simulationBall.eventList[k]);
        }

        //ballSimulationCurve->ballDataList[i] = (pool.GetCurSimulBallInfo(i));
        auto iter = simulationCurveDictionary.find(ballNumber);
        if (iter == simulationCurveDictionary.end())
        {
            simulationCurveDictionary.insert(pair<unsigned int, CBallSimulationCurve*>(ballSimulationCurve->ballNumber, ballSimulationCurve));
        }
    }

    ballSimulationCurve->ready = true;
}

void CBallSimulator::SimulateASync(unsigned int ballNumber, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize)
{
    //CBallSimulationCurve ballSimulationCurve(ballNumber);

    //CJxSimulationSphere simulationBall(0, vecStartPos, vecVelocity, curveValue, normal, ballSize);

    pool->ReadyNextStep();
    CJxSimulationSphere* startBall = pool->SetCurSimulBallInfo(0, vecStartPos, vecVelocity, JOQUATERNION::identity);
    //ballSimulationCurve.ballDataList[0] = startBall;

   
}

void  CBallSimulator::RunSimulate(CBallSimulationCurve* ballSimulationCurve, CJxSimulationSphere *simulationBall)
{
    for (int i = 1; i < frameCount; i++)
    {
        simulationBall->eventList.clear();

        jxSimulationScene->SimulateActorOnly(simulationBall, FIXEDUPDATETIME);

        pool->SetCurSimulBallInfo(i, simulationBall->_vecPos, simulationBall->_vecVelocity, simulationBall->_qOrientation);

        //CreateSimulationResultFrame(simulationBall.vecPos, i);

        for (int k = 0; k < simulationBall->eventList.size(); k++)
        {
            pool->SetCurSimullBallEvent(i, simulationBall->eventList[k]);
        }

        if (pool->GetCurSimulBallEventFirst(i) == "Event_FirstBound")
        {
            ballSimulationCurve->indexFirstBound = i;
            ballSimulationCurve->nextBoundPosition = simulationBall->_vecPos;
        }


        //ballSimulationCurve->ballDataList[i] = (pool.GetCurSimulBallInfo(i));


        auto iter = simulationCurveDictionary.find(ballSimulationCurve->ballNumber);
        if (iter == simulationCurveDictionary.end())
        {
            simulationCurveDictionary.insert(pair<unsigned int, CBallSimulationCurve*>(ballSimulationCurve->ballNumber, ballSimulationCurve));
            //ballSimulationList.Add(ballSimulationCurve);
        }

    }

    ballSimulationCurve->ready = true;

    if (ballSimulationCurve->ready)
    {
        //ballcontroller.ApplyBallArgument((BallPosition)ballPositionSimulation);
    }

}