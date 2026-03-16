#pragma once
#include <map>
#include "JxScene.h"
#include "CBallPool.h"
#include "CBallSimulationCurve.h"
#include "JPhysics.h"


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
class CJxScene;
class CJxInPlane;
class F4GoalInChecker;

	class DECLSPEC CBallSimulator
	{
	public:
		CJxScene* jxSimulationScene;
		CJxScene* jxFisrtCurveSimulationScene;
		CJxActor* jxFisrtCurveSimulationSceneBefore;
	private:
		CBallPool* pool;
		vector<CJxActor*> actorSet;
	public:
		map<unsigned int, CBallSimulationCurve*> simulationCurveDictionary;// = new Dictionary<uint, BallSimulationCurve>();
	
	protected:
		//CJxSimulationSphere simulationBall;
		//private     GameObject jxSphereObject;
	
	public:
		~CBallSimulator();
		//IEnumerator runCoroutine = null;
		//IEnumerator runCoroutine2;
	private:
		//JCoroutineJob job;
	
	private:
		const int frameCount = 300;
	
	public:
		void Initialize();
		CBallSimulationCurve* MakeNewCurveFromTwoCurve(float reachTime, string* addEvent, JOVECTOR3 startPos, JOVECTOR3 targetPos, JOVECTOR3 targetVelocity, bool backboardCheck = false);
	
		CBallSimulationCurve* MakeNewCurveOnly(JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize, int &index, bool firstBound = false /* Firt Bound*/);
	
	
		JOVECTOR3 ReCalcTargetPosition(JOVECTOR3 vecStartPos, float ballSize);
	
	
		CBallSimulationCurve* SimulateCurrentCurve(CJxScene* jxScene, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize, bool firstBound, int& simCount, int& index/* Firt Bound*/);
	
	
	
		void SimulateSync(unsigned int ballNumber, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize);
	
	
		void SimulateASync(unsigned int ballNumber, JOVECTOR3 vecStartPos, JOVECTOR3 vecVelocity, float curveValue, JOVECTOR3 normal, float ballSize);
	
	
		void RunSimulate(CBallSimulationCurve* ballSimulationCurve, CJxSimulationSphere* simulationBall);
	};

