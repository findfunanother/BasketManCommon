#pragma once
#include "CBallCurveInfo.h"
#include "JOMath3d.h"
#include "JxPhysics.h"
#include "JxSimulationSphere.h"


typedef void(*EVENT_CALLBACK_FUNC)(JOVECTOR3 ,string);
class DECLSPEC JOVECTOR3;
class DECLSPEC CJxSimulationSphere;


extern "C"
{ 
	class DECLSPEC CBallSimulationCurve :
		public CBallCurveInfo
	{
	public:
	
	
		int addInfo = -1;       // 1¿Ã∏È ∞Ò¿Œ 
		//JOVECTOR3 airBallTouchBallPoint;// { get; set; }
		float timeReachGroundBound;
		float timeReachNoGoal;
		float timeReachReboundAbleTime;
		float timeReachPickAbleTime;
		JOVECTOR3 nextBoundPosition;// { get; set; }
		CJxSimulationSphere ballDataList[SIMULBALLMAX];// = new BallData[300];
		CBallSimulationCurve();
		CBallSimulationCurve(unsigned int ballNumber);
		JOQUATERNION GetRotation(float time);
		JOVECTOR3 GetPosition(int index);
		JOVECTOR3 GetPosition(float time);
		JOVECTOR3 GetVelocity(float time);
		JOVECTOR3 GetVelocity(int index);
		float GetTime(JOVECTOR3 position);
		void PopEvent(float time, EVENT_CALLBACK_FUNC eventCallback);
		void PopEvent(int index, EVENT_CALLBACK_FUNC eventCallback);
		int GetIndex(float timeElapsed);
	
	
	};

}