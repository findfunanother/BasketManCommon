#pragma once

#include <string>
#include <vector>
#include <map>

#include "Host.h"
#include "JPhysics.h"
#include "CBallSimulationCurve.h"
#include "CBallSimulator.h"


class CBallSimulationCurve;

	class CBallController
	{
	private:

	public:
		typedef struct _GameBallData
		{
			int ballNumber;
			int owner;
			DHOST_TYPE_INT32 team;
			int point;
			F4PACKET::BALL_STATE ballState;
			F4PACKET::SHOT_TYPE shotType;
			map<string, float> ballEventMap;
			JOVECTOR3 posStart;
			float timeElapsed;
			CBallSimulationCurve* curSimulationCurve;

			bool checkReboundValid; // sendsyncpos 이후에 이것을 빼도록 하자 
			float  checkReboudEnableTime = 0.0f;

		} GameBallData;


		CBallSimulator* ballSimulator;


	public:
		CBallController(CHost* pHost);
		~CBallController();
		void AddShotBallData(SBallShotT* pballShot, GameBallData* pGameBallData, float timeRemain);
		void AddBallData(const SBallSimulation* pBallsim, GameBallData* pGameBallData, float timeRemain);
		void AddBallData(GameBallData* pGameBallData, float timeRemain);
		void Update(float timeDelta);
		bool IsBeforeShotEventTime(float timeDelta);
		void GetBallPosition(float timeEvent, JOVECTOR3& ballPos);

		JOVECTOR3 GetBallPosition();
		JOVECTOR3 GetBallPosition(float timeEvent);
		
		JOVECTOR3 GetBallVelocity();
		JOVECTOR3 GetBallVelocity(float timeEvent);
		
		GameBallData* GameBallDataGet()	{return m_pCurrentBallData;}
		F4PACKET::BALL_STATE PrevBallStateGet() { return m_ballStatePrev; }
		bool CheckBallStateShot(F4PACKET::BALL_STATE value);
		bool CheckBallStateShot();
		bool CheckBallStateEnableRebound();
		bool CheckBallStateEnablePick();
		bool IsReserved();

	public:
		void RemoveCurrentBallData();
		bool isOwnerChanaging();

	protected:
		CHost* m_pHost;
		GameBallData* m_pCurrentBallData;
		
		F4PACKET::BALL_STATE m_ballStatePrev = F4PACKET::BALL_STATE::ballState_none;

		map<GameBallData*, float> m_ReservedBallData;
	};
