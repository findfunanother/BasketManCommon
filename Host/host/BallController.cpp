#include "BallController.h"
#include "PhysicsHandler.h"
#include "DataManagerShotSolution.h"

CBallController::CBallController(CHost* pHost) : m_pHost(pHost), m_pCurrentBallData(nullptr)
{
	ballSimulator = new CBallSimulator();
	ballSimulator->Initialize();
}

CBallController::~CBallController()
{
	RemoveCurrentBallData();

	if (ballSimulator != NULL)
		delete(ballSimulator);
	
	for (map<GameBallData*, float>::iterator iter = m_ReservedBallData.begin(); iter != m_ReservedBallData.end(); ++iter)
	{
		GameBallData* pGameBallData = iter->first;
		SAFE_DELETE(pGameBallData);
	}

	m_ReservedBallData.clear();
}

void CBallController::AddShotBallData(SBallShotT* pBallShot,GameBallData* pGameBallData, float timeRemain)
{
	AddBallData(pGameBallData, timeRemain);
	
	SVector3 positionStart;

	CPhysicsHandler* pPhysics = m_pHost->GetPhysicsHandler();
	auto positionRim = pPhysics->GetRimPosition();
	int sector_num = kINT32_INIT;
	switch (pBallShot->shottype)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
			sector_num = 0;
			positionStart = pPhysics->GetDunkPosition();// .GetDunkPos();
			break;
		default:
			
			sector_num = m_pHost->GetShotSolution()->GetSectorNumber(TB::SVector3(pBallShot->positionball->x(), pBallShot->positionball->y(), pBallShot->positionball->z()),
																  TB::SVector3(positionRim.x(), positionRim.y(), positionRim.z()));
			positionStart = *pBallShot->positionball;
			break;
	}

	bool backboard_check = false;

	switch (pBallShot->shottype)
	{
		case SHOT_TYPE::shotType_middle:
		case SHOT_TYPE::shotType_threePoint:
		case SHOT_TYPE::shotType_post:
			backboard_check = true;
			break;
		default:
			break;
	}
	
	SHOT_BOUND bound = pBallShot->shotbound;
	int index = pBallShot->index;

	auto targetData = m_pHost->GetShotSolution()->FindTargetDataInDictionaryIndex((F4PACKET::SHOT_THROW)pBallShot->shotthrow, sector_num, (F4PACKET::SHOT_BOUND)pBallShot->shotbound, pBallShot->index);
	if (targetData == nullptr)
	{
		string str = "[BALL_SHOT] targetData is null step 1 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", ShotThrow : " + F4PACKET::EnumNameSHOT_THROW(pBallShot->shotthrow) + ", sector_num : " + std::to_string(sector_num) + ", ShotBound : " + F4PACKET::EnumNameSHOT_BOUND(pBallShot->shotbound) 
			+ ", Index : " + std::to_string(pBallShot->index);
		m_pHost->ToLog(str.c_str());

		F4PACKET::SHOT_THROW temp_shot_throw = F4PACKET::SHOT_THROW::low;
		int temp_sector_num = 0;
		F4PACKET::SHOT_BOUND temp_shot_bound = pBallShot->shotbound;
		int temp_index = 0;

		targetData = m_pHost->GetShotSolution()->FindTargetDataInDictionaryIndex(temp_shot_throw, temp_sector_num, temp_shot_bound, temp_index);
	}

	if (targetData == nullptr)
	{
		string str = "[BALL_SHOT] targetData is null step 2 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", ShotThrow : " + F4PACKET::EnumNameSHOT_THROW(pBallShot->shotthrow) + ", sector_num : " + std::to_string(sector_num) + ", ShotBound : " + F4PACKET::EnumNameSHOT_BOUND(pBallShot->shotbound)
			+ ", Index : " + std::to_string(pBallShot->index);
		m_pHost->ToLog(str.c_str());

		return;
	}

	auto postarget = *targetData->contactpoint();
	auto velocitytarget = *targetData->targetvelocity();
	
	JOVECTOR3 posStart;
	posStart.fX = positionStart.x();
	posStart.fY = positionStart.y();
	posStart.fZ = positionStart.z();
	JOVECTOR3 targetPoint;
	targetPoint.fX = postarget.x();
	targetPoint.fY = postarget.y();
	targetPoint.fZ = postarget.z();
	JOVECTOR3 targetVelocity;
	targetVelocity.fX = velocitytarget.x();
	targetVelocity.fY = velocitytarget.y();
	targetVelocity.fZ = velocitytarget.z();

	auto str = targetData->eventname()->str();
	auto ballcurve = ballSimulator->MakeNewCurveFromTwoCurve(pBallShot->targetreachtime, &str, posStart, targetPoint, targetVelocity, backboard_check);
	pGameBallData->curSimulationCurve = ballcurve;

	// 훅샷 백보드 맞고 공이 안주어지는 버그 수정 
	bool firstBackBoard = false;

	const float addSize = 0.04f; // 림사이즈 만큼 아래에
	float goalInHeight = RIM_POS_Y - addSize;
	bool changeRimbound = false;
	bool goalInCheck = false;
	int rimBoundLastIndex = 0;
	auto eventRimbound = pGameBallData->ballEventMap.find("Event_FirstRimBound");
	changeRimbound = pGameBallData->ballEventMap.find("Event_FirstRimBound") != pGameBallData->ballEventMap.end();
	float timerimbound = changeRimbound? eventRimbound->second : 0.f;
	JOVECTOR3 newGoalInPos = JOVECTOR3(RIM_POS_X, goalInHeight, RIM_POS_Z_FIXED);
	for (int i = 1; i < SIMULBALLMAX; i++)
	{
		auto _vecPos = ballcurve->ballDataList[i]._vecPos;
		auto _vecPrePos = ballcurve->ballDataList[i-1]._vecPos; 

		// 첫번째 체크해서 백보드가 맞았다면 거기부터 리바운드 가능 시점 
		if (ballcurve->ballDataList[i].eventList.size() > 0)
		{
			if (ballcurve->ballDataList[i].eventList[0] == "Event_FirstBackBoard")
			{
				firstBackBoard = true;
			}
		}

		if (changeRimbound && timerimbound < i * FIXEDUPDATETIME && ballcurve->ballDataList[i]._vecPreVelocity.fY <= 0.0f)
		{
			changeRimbound = false;
			rimBoundLastIndex = i + 1; // 계속 업데이트 
		}
		
		if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
		{
			float k = DotProduct(JOVECTOR3::up * -1.f, newGoalInPos - _vecPrePos) / DotProduct(JOVECTOR3::up * -1.f, (_vecPos - _vecPrePos));
			JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

			float distance = (newGoalInPos - goalInPoint).Length();
			if (distance < RIM_HALF_DISTANCE)
			{
				ballcurve->ballDataList[i].OnEventRightGoalIn();
				pGameBallData->ballEventMap.insert(pair<string, float>("Event_RightGoalIn", max(i*FIXEDUPDATETIME, 0.0f)));
				goalInCheck = true;
			}
		}
	}

	if (rimBoundLastIndex > 0 && !goalInCheck) // 노골인 경우와 업데이트가 된 경우 
	{
		if (eventRimbound->first == "Event_FirstRimBound")
		{
			// rimBoundLastIndex 계산을 잘못하는 것 같아서 아래와 같이 바꿈  steven 20240108, 이전  MakeNewCurveFromTwoCurve 계산함 
			// pGameBallData->ballEventMap.insert(pair<string, float>("eventReboundAble", max(rimBoundLastIndex * FIXEDUPDATETIME, 0.0f))); 

			float preSendtime = 0.0f; // 0.03f; // 리바운드 가능 패킷을 일찍 보내는 로직 

			if (m_pHost->bTreatDelayMode)
			{
				preSendtime = 0.03f;
			}

			pGameBallData->ballEventMap.insert(pair<string, float>("EventReboundAble", (pGameBallData->curSimulationCurve->timeReachReboundAbleTime - preSendtime) ));
			pGameBallData->ballEventMap.insert(pair<string, float>("EventPickAble", (pGameBallData->curSimulationCurve->timeReachPickAbleTime - preSendtime)));

			/*string log = "diff : " + std::to_string(eventRimbound->second - pGameBallData->curSimulationCurve->timeReachNoGoal) + ", pBallShot->targetreachtime : " + std::to_string(eventRimbound->second) + ", timeReachNoGoal : " + std::to_string(pGameBallData->curSimulationCurve->timeReachNoGoal);
			m_pHost->ToLog(log.c_str());*/

			eventRimbound->second = pGameBallData->curSimulationCurve->timeReachNoGoal;
		}
	}

	// 기존 코드를 유지하기 위해서 이렇게 바꿨으나 통일 되지 못함 
	if (rimBoundLastIndex == 0 && firstBackBoard && !goalInCheck) // ( 훅샷 공 못줍는 버그 수정 ) 
	{
		float preSendtime = 0.0f; // 0.03f; // 리바운드 가능 패킷을 일찍 보내는 로직 
		if (m_pHost->bTreatDelayMode)
		{
			preSendtime = 0.03f;
		}
		pGameBallData->ballEventMap.insert(pair<string, float>("EventReboundAble", (pGameBallData->curSimulationCurve->timeReachReboundAbleTime - preSendtime)));
		pGameBallData->ballEventMap.insert(pair<string, float>("EventPickAble", (pGameBallData->curSimulationCurve->timeReachPickAbleTime - preSendtime)));
	}


}

void CBallController::AddBallData(const SBallSimulation* pBallsim, GameBallData* pGameBallData, float timeRemain)
{
	const float addSize = 0.04f; // 림사이즈 만큼 아래에
	float goalInHeight = RIM_POS_Y - addSize;
	JOVECTOR3 newGoalInPos = JOVECTOR3(RIM_POS_X, goalInHeight, RIM_POS_Z_FIXED);

	bool check_shot_ball = CheckBallStateShot(pBallsim->ballstate());

	AddBallData(pGameBallData, timeRemain);

	if (((int)pGameBallData->ballState & (int)BALL_STATE::ballState_loose) == (int)BALL_STATE::ballState_loose)
	{
		JOVECTOR3 posStart;
		posStart.fX = pBallsim->positionball().x();
		posStart.fY = pBallsim->positionball().y();
		posStart.fZ = pBallsim->positionball().z();

		JOVECTOR3 vecStart;
		vecStart.fX = pBallsim->velocity().x();
		vecStart.fY = pBallsim->velocity().y();
		vecStart.fZ = pBallsim->velocity().z();

		int pickIndex = 0;

		auto ballcurve = ballSimulator->MakeNewCurveOnly(posStart, vecStart, 0.f, JOVECTOR3::zero, BALL_RADIUS, pickIndex);
		pGameBallData->curSimulationCurve = ballcurve;

		//pGameBallData->ballEventMap.insert(pair<string, float>("EventPickAble", max(pickIndex * FIXEDUPDATETIME, 0.0f)));

		if (check_shot_ball)
		{
			for (int i = 1; i < SIMULBALLMAX; i++)
			{
				//ballcurve->ballDataList[i].eventList.push_back("test");
				auto _vecPos = ballcurve->ballDataList[i]._vecPos;
				auto _vecPrePos = ballcurve->ballDataList[i - 1]._vecPos;

				if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
				{
					float k = DotProduct(JOVECTOR3::up * -1.f, newGoalInPos - _vecPrePos) / DotProduct(JOVECTOR3::up * -1.f, (_vecPos - _vecPrePos));
					JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

					float distance = (newGoalInPos - goalInPoint).Length();
					if (distance < RIM_HALF_DISTANCE)
					{
						ballcurve->ballDataList[i].OnEventRightGoalIn();
						pGameBallData->ballEventMap.insert(pair<string, float>("Event_RightGoalIn", max(i * FIXEDUPDATETIME, 0.0f)));
					}
				}
			}
		}
	}
	/*auto ballcurve = ballSimulator->SimulateCurrentCurve(pBallShot->targetreachtime(), &targetData->eventName,
		posStart, targetPoint, targetVelocity);*/

}

void CBallController::AddBallData(GameBallData* pGameBallData, float timeRemain)
{
	pGameBallData->curSimulationCurve = NULL;

	std::pair<std::map<GameBallData*, float>::iterator, bool> result;
	result = m_ReservedBallData.insert(pair<GameBallData*, float>(pGameBallData, timeRemain));
	if (result.second == false)
	{
		string log = "AddBallData map insert fail ballNumber : " + std::to_string(pGameBallData->ballNumber);
		m_pHost->ToLog(log.c_str());

		SAFE_DELETE(pGameBallData)
	}
}

void CBallController::Update(float timeDelta)
{
	map<GameBallData*, float>::iterator iter1 = m_ReservedBallData.begin();
	while (iter1 != m_ReservedBallData.end())
	{
		iter1->second -= timeDelta;

		if (iter1->second <= 0.0f)
		{
			if (m_pCurrentBallData != nullptr)
			{
				m_ballStatePrev = m_pCurrentBallData->ballState;
			}

			if (m_pCurrentBallData == nullptr || iter1->first->ballNumber > m_pCurrentBallData->ballNumber)
			{
				RemoveCurrentBallData();
				m_pCurrentBallData = iter1->first;
				m_pCurrentBallData->timeElapsed = 0.f;
			}
			
			auto remove = iter1;

			++iter1;

			m_ReservedBallData.erase(remove);

			/*string log_message = "[CBallController] current ball out numnber : " + to_string(m_pCurrentBallData->ballNumber);
			m_pHost->ToLog(log_message.c_str());*/

			//m_pHost->SendDebugMessage(log_message);
		}
		else
		{
			++iter1;
		}
	}

	if (m_pCurrentBallData != nullptr)
	{
		map<string, float>::iterator iter2 = m_pCurrentBallData->ballEventMap.begin();
		m_pCurrentBallData->timeElapsed += timeDelta;

		while (iter2 != m_pCurrentBallData->ballEventMap.end())
		{
			iter2->second -= timeDelta;

			if (iter2->second <= 0.0f)
			{
				m_pHost->BallOnEvent(m_pCurrentBallData->ballNumber, iter2->first, m_pCurrentBallData->owner, (F4PACKET::BALL_STATE)m_pCurrentBallData->ballState, (F4PACKET::SHOT_TYPE)m_pCurrentBallData->shotType,
					(DHOST_TYPE_INT32)m_pCurrentBallData->team, m_pCurrentBallData->point, m_pCurrentBallData->posStart.fX, m_pCurrentBallData->posStart.fZ);

				auto remove = iter2;

				++iter2;

				m_pCurrentBallData->ballEventMap.erase(remove);
			}
			else
			{
				++iter2;
			}
		}
	}
}

bool CBallController::IsBeforeShotEventTime(float timeDelta)
{
	bool retValue = true;

	if (m_pCurrentBallData != nullptr)
	{
 		switch (m_pCurrentBallData->ballState)
		{
		case BALL_STATE::ballState_shotDunk:
		case BALL_STATE::ballState_shotJump:
		case BALL_STATE::ballState_shotLayUp:
		case BALL_STATE::ballState_shotPost:
		{
			map<string, float>::iterator iter = m_pCurrentBallData->ballEventMap.find("Event_RightGoalIn");

			if (iter != m_pCurrentBallData->ballEventMap.end())
			{
				retValue = timeDelta < iter->second;
			}
		}
		break;
		}
	}

	if (retValue == true)
	{
		for (map<GameBallData*, float>::iterator iter1 = m_ReservedBallData.begin(); iter1 != m_ReservedBallData.end(); ++iter1)
		{
			switch (iter1->first->ballState)
			{
			case BALL_STATE::ballState_shotDunk:
			case BALL_STATE::ballState_shotJump:
			case BALL_STATE::ballState_shotLayUp:
			case BALL_STATE::ballState_shotPost:
			{
				map<string, float>::iterator iter2 = iter1->first->ballEventMap.find("Event_RightGoalIn");

				if (iter2 != iter1->first->ballEventMap.end())
				{
					retValue = timeDelta < iter2->second;
				}
			}
			break;
			}
		}
	}

	return retValue;
}

JOVECTOR3 CBallController::GetBallPosition()
{
	if (!m_pCurrentBallData)
		return JOVECTOR3();

	map<string, float>::iterator iter2 = m_pCurrentBallData->ballEventMap.begin();
	if (((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_loose) == (int)BALL_STATE::ballState_loose ||
		((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_shot) == (int)BALL_STATE::ballState_shot)
	{
		if (m_pCurrentBallData->curSimulationCurve == nullptr)
		{
			return JOVECTOR3();
		}

		int ind = m_pCurrentBallData->curSimulationCurve->GetIndex(m_pCurrentBallData->timeElapsed);
		JOVECTOR3 pos = m_pCurrentBallData->curSimulationCurve->GetPosition(ind);

		return pos;
	}
	JOVECTOR3 posPlayer;
	auto owner = m_pHost->GetCharacterPosition(m_pCurrentBallData->owner);
	posPlayer.fX = owner.x();
	posPlayer.fY = owner.y();
	posPlayer.fZ = owner.z();
	
	return posPlayer;
}

JOVECTOR3 CBallController::GetBallPosition(float timeEvent)
{
	JOVECTOR3 result = JOVECTOR3::zero;

	if (m_pCurrentBallData == nullptr)
	{
		return result;
	}
	
	if (m_pCurrentBallData->curSimulationCurve == nullptr)
	{
		return result;
	}

	int index = m_pCurrentBallData->curSimulationCurve->GetIndex(m_pCurrentBallData->timeElapsed + timeEvent);

	if (index > kINT32_INIT)
	{
		result = m_pCurrentBallData->curSimulationCurve->GetPosition(index);
	}
	
	return result;
}

JOVECTOR3 CBallController::GetBallVelocity()
{
	if (!m_pCurrentBallData)
		return JOVECTOR3();

	map<string, float>::iterator iter2 = m_pCurrentBallData->ballEventMap.begin();
	if (((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_loose) == (int)BALL_STATE::ballState_loose ||
		((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_shot) == (int)BALL_STATE::ballState_shot)
	{
		if (m_pCurrentBallData->curSimulationCurve == nullptr)
		{
			return JOVECTOR3();
		}

		int ind = m_pCurrentBallData->curSimulationCurve->GetIndex(m_pCurrentBallData->timeElapsed);
		JOVECTOR3 pos = m_pCurrentBallData->curSimulationCurve->GetVelocity(ind);

		return pos;
	}

	return JOVECTOR3();
}

JOVECTOR3 CBallController::GetBallVelocity(float timeEvent)
{
	JOVECTOR3 result = JOVECTOR3::zero;

	if (m_pCurrentBallData == nullptr)
	{
		return result;
	}

	if (m_pCurrentBallData->curSimulationCurve == nullptr)
	{
		return result;
	}

	int index = m_pCurrentBallData->curSimulationCurve->GetIndex(m_pCurrentBallData->timeElapsed + timeEvent);

	if (index > kINT32_INIT)
	{
		result = m_pCurrentBallData->curSimulationCurve->GetVelocity(index);
	}

	return result;
}

bool CBallController::CheckBallStateShot(F4PACKET::BALL_STATE value)
{
	switch (value)
	{
		case F4PACKET::BALL_STATE::ballState_shot:
		case F4PACKET::BALL_STATE::ballState_shotDunk:
		case F4PACKET::BALL_STATE::ballState_shotJump:
		case F4PACKET::BALL_STATE::ballState_shotLayUp:
		case F4PACKET::BALL_STATE::ballState_shotPost:
			return true;
		default:
			return false;
	}

	return false;
}

bool CBallController::CheckBallStateShot()
{
	bool result = false;

	if (m_pCurrentBallData != nullptr)
	{
		F4PACKET::BALL_STATE value = m_pCurrentBallData->ballState;

		switch (value)
		{
			case F4PACKET::BALL_STATE::ballState_shot:
			case F4PACKET::BALL_STATE::ballState_shotDunk:
			case F4PACKET::BALL_STATE::ballState_shotJump:
			case F4PACKET::BALL_STATE::ballState_shotLayUp:
			case F4PACKET::BALL_STATE::ballState_shotPost:
			{
				result = true;
			}
			break;
			default:
			{

			}
			break;
		}
	}

	return result;
}


bool CBallController::CheckBallStateEnablePick()
{
	bool result = false;

	if (m_pCurrentBallData != nullptr)
	{
		F4PACKET::BALL_STATE value = m_pCurrentBallData->ballState;

		switch (value)
		{
		case F4PACKET::BALL_STATE::ballState_shot:
		case F4PACKET::BALL_STATE::ballState_shotDunk:
		case F4PACKET::BALL_STATE::ballState_shotJump:
		case F4PACKET::BALL_STATE::ballState_shotLayUp:
		case F4PACKET::BALL_STATE::ballState_shotPost:			
		/*case F4PACKET::BALL_STATE::ballState_looseBlock:, block 상황은 공이 빨라 클라와 서버의 공을 동기화하기가 어렵기 때문에 일단 뺌 */
		case F4PACKET::BALL_STATE::ballState_looseRebound:
		case F4PACKET::BALL_STATE::ballState_looseSteal:
		{
			result = true;
		}
		break;
		default:
		{

		}
		break;
		}
	}

	return result;
}

bool CBallController::CheckBallStateEnableRebound()
{
	bool result = false;

	if (m_pCurrentBallData != nullptr)
	{
		F4PACKET::BALL_STATE value = m_pCurrentBallData->ballState;

		switch (value)
		{
			case F4PACKET::BALL_STATE::ballState_shot:
			case F4PACKET::BALL_STATE::ballState_shotDunk:
			case F4PACKET::BALL_STATE::ballState_shotJump:
			case F4PACKET::BALL_STATE::ballState_shotLayUp:
			case F4PACKET::BALL_STATE::ballState_shotPost:
			case F4PACKET::BALL_STATE::ballState_looseBlock:
			case F4PACKET::BALL_STATE::ballState_looseRebound:
			case F4PACKET::BALL_STATE::ballState_looseSteal:
			{
				result = true;
			}
			break;
			default:
			{

			}
			break;
		}
	}

	return result;
}

bool CBallController::isOwnerChanaging()
{
	if (m_pCurrentBallData != nullptr)
	{
		if (
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pass ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_passNormal ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_passRebound ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_passBlock /* ||

			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pickBlock ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pickLoose ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pickPass ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pickRebound ||
			GameBallDataGet()->ballState == F4PACKET::BALL_STATE::ballState_pickSteal*/
			)
		{

			return true;
		}
	}

	return false;
}


void CBallController::RemoveCurrentBallData()
{
	if (m_pCurrentBallData != nullptr)
	{
		SAFE_DELETE(m_pCurrentBallData);
	}
}

bool CBallController::IsReserved()
{
	return m_ReservedBallData.size() > 0;
}

void CBallController::GetBallPosition(float timeEvent, JOVECTOR3& ballPos)
{
	if (!m_pCurrentBallData)
		return;

	if (((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_loose) == (int)BALL_STATE::ballState_loose ||
		((int)m_pCurrentBallData->ballState & (int)BALL_STATE::ballState_shot) == (int)BALL_STATE::ballState_shot)
	{
		int ind = m_pCurrentBallData->curSimulationCurve->GetIndex(timeEvent);
		ballPos = m_pCurrentBallData->curSimulationCurve->GetPosition(ind);
		//ballPos = m_pCurrentBallData->curSimulationCurve->GetPosition(timeEvent);
	}	
}