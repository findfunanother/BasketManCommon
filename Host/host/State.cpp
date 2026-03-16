#define _CRT_SECURE_NO_WARNINGS

#include "State.h"
#include "PhysicsHandler.h"
#include "BallController.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "JEnviroment.h"
#include "Ability.h"
#include "DataManagerShotSolution.h"
#include "DataManagerBalance.h"
//#include "CharacterState.h"
#include "CharacterState_Stand.h"
#include "CharacterState_Move.h"

#include "ProcessPacket.h"
#include "Host.h"

#include "Play_c2s_common.h"
#include "Play_c2s_ballShot.h"

// internal method



//! global function - by thinkingPig (���� �Լ��� ���°� �� ������ ������ ������...)
int FindMedalInfo(F4PACKET::SPlayerInformationT* pPlayerInformation, F4PACKET::MEDAL_INDEX index)
{
	if (pPlayerInformation != nullptr)
	{
		for (int i = 0; i < pPlayerInformation->medals.size(); i++)
		{
			if (pPlayerInformation->medals[i].index() == index)
			{
				return i;
			}
		}
	}

	return -1;
}

SHOT_THROW RouletteShotThrow(int rateThrowLow, int rateThrowMid, int rateThrowHigh, CHost* host = nullptr)
{
	auto totalRate = rateThrowLow + rateThrowMid + rateThrowHigh;

	if (totalRate <= 0)
	{
		cout << endl;
		cout << "[Error] Shot Throw Rate";

		totalRate = 100;
	}

	int randomThrow = Util::GetRandom(totalRate);

	if (randomThrow <= rateThrowLow)
	{
		return SHOT_THROW::low;
	}
	else if (randomThrow <= rateThrowLow + rateThrowMid)
	{
		return SHOT_THROW::middle;
	}
	else
	{
		return SHOT_THROW::high;
	}
}

float GetDistance(SVector3 a, SVector3 b)
{
	float x = a.x() - b.x();
	float y = a.y() - b.y();
	float z = a.z() - b.z();

	return sqrt(a * a + y * y + z * z);
}

F4PACKET::ACTION_TYPE ConvertShotTypeToActionType(F4PACKET::SHOT_TYPE shotType)
{
	F4PACKET::ACTION_TYPE action_type;

	switch (shotType)
	{
		case SHOT_TYPE::shotType_closePostUnstready:
		case SHOT_TYPE::shotType_postUnstready:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		case SHOT_TYPE::shotType_middle:
		case SHOT_TYPE::shotType_middleUnstready:
		case SHOT_TYPE::shotType_threePoint:
		{
			action_type = ACTION_TYPE::action_shotJump;
		}
		break;
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			action_type = ACTION_TYPE::action_shotDunk;
		}
		break;
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			action_type = ACTION_TYPE::action_shotLayUp;
		}
		break;
		default:
		{
			action_type = ACTION_TYPE::action_none;
		}
		break;
	}

	return action_type;
}

DHOST_TYPE_BOOL CheckShotTypeDunk(F4PACKET::SHOT_TYPE shotType)
{
	if (shotType == SHOT_TYPE::shotType_dunkDriveIn || shotType == SHOT_TYPE::shotType_dunkFingerRoll || shotType == SHOT_TYPE::shotType_dunkPost)
	{
		return true;
	}

	return false;
}

DHOST_TYPE_BOOL CheckShotTypeJump(F4PACKET::SHOT_TYPE shotType)
{
	DHOST_TYPE_BOOL jump_shot_type = false;

	switch (shotType)
	{
		case SHOT_TYPE::shotType_middle:
		case SHOT_TYPE::shotType_middleUnstready:
		case SHOT_TYPE::shotType_threePoint:
		case SHOT_TYPE::shotType_closePostUnstready:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		case SHOT_TYPE::shotType_postUnstready:
		{
			jump_shot_type = true;
		}
		break;
		default:
		{

		}
		break;
	}

	return jump_shot_type;
}

DHOST_TYPE_BOOL CheckShotTypeUnstready(F4PACKET::SHOT_TYPE shotType)
{
	if (shotType == SHOT_TYPE::shotType_closePostUnstready || shotType == SHOT_TYPE::shotType_middleUnstready || shotType == SHOT_TYPE::shotType_postUnstready)
	{
		return true;
	}

	return false;
}

DHOST_TYPE_BOOL CheckThreePoint(F4PACKET::SHOT_TYPE shotType)
{
	if (shotType == SHOT_TYPE::shotType_threePoint)
	{
		return true;
	}

	return false;
}

DHOST_TYPE_BOOL CheckHandednessShotType(F4PACKET::SHOT_TYPE shotType)
{
	DHOST_TYPE_BOOL result = false;

	switch (shotType)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		{
			result = true;
		}
		break;
		default:
		{

		}
		break;
	}

	return result;
}

DHOST_TYPE_BOOL GetEnableRescue(F4PACKET::SHOT_TYPE shotType, CAbility* pInfo)
{
	DHOST_TYPE_BOOL enableRescue = false;
	
	switch (shotType)
	{
		case SHOT_TYPE::shotType_middle:
		{
			if (pInfo->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot) > 40.0f)
			{
				enableRescue = true;
			}
		}
		break;
		case SHOT_TYPE::shotType_threePoint:
		{
			if (pInfo->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot) > 40.0f)
			{
				enableRescue = true;
			}
		}
		break;
		default:
		{
			enableRescue = true;
		}
		break;
	}

	return enableRescue;
}
//! end

F4PACKET::SHOT_BOUND CState::GetSuccessShotBoundByShotType(float bonus, F4PACKET::SHOT_TYPE shotType)
{
	int boundClean = 0;
	int boundOnce = 0;
	int boundMulti = 0;

	if (bonus < 0)
	{
		switch (shotType)
		{
			case SHOT_TYPE::shotType_dunkDriveIn:
			case SHOT_TYPE::shotType_dunkFingerRoll:
			case SHOT_TYPE::shotType_dunkPost:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_layUpDriveIn:
			case SHOT_TYPE::shotType_layUpFingerRoll:
			case SHOT_TYPE::shotType_layUpPost:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_middle:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_middleUnstready:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_closePostUnstready:
			case SHOT_TYPE::shotType_hook:
			case SHOT_TYPE::shotType_post:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_postUnstready:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_threePoint:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Multi"));
			}
			break;
		}
	}
	else
	{
		switch (shotType)
		{
			case SHOT_TYPE::shotType_dunkDriveIn:
			case SHOT_TYPE::shotType_dunkFingerRoll:
			case SHOT_TYPE::shotType_dunkPost:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Dunk_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_layUpDriveIn:
			case SHOT_TYPE::shotType_layUpFingerRoll:
			case SHOT_TYPE::shotType_layUpPost:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_LayUp_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_middle:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Middle_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_middleUnstready:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyMiddle_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_hook:
			case SHOT_TYPE::shotType_post:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Hook_And_Post_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_closePostUnstready:
			case SHOT_TYPE::shotType_postUnstready:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_UnsteadyPost_Bonus_Multi"));
			}
			break;
			case SHOT_TYPE::shotType_threePoint:
			{
				boundClean = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Bonus_Clean"));
				boundOnce = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Bonus_Once"));
				boundMulti = static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBound_Three_Bonus_Multi"));
			}
			break;
		}
	}

	int randomBound = Util::GetRandomSetMax(boundClean + boundOnce + boundMulti);

	F4PACKET::SHOT_BOUND shot_bound = F4PACKET::SHOT_BOUND::MIN;

	if (randomBound < boundClean)
	{
		shot_bound = SHOT_BOUND::success_clean;
	}
	else if (randomBound < boundClean + boundOnce)
	{
		shot_bound = SHOT_BOUND::success_rim_bound;
	}
	else
	{
		shot_bound = SHOT_BOUND::success_rim_multi_bound;
	}

	return shot_bound;
}

F4PACKET::SHOT_BOUND CState::GetFailShotBoundByShotType(float bonus, F4PACKET::SHOT_TYPE shotType)
{
	int rateBoundForwardWeak = 0;
	int rateBoundForwardStrong = 0;
	int rateBoundBackwardWeak = 0;
	int rateBoundBackwardStrong = 0;
	int rateBoundMulti = 0;

	switch (shotType)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Dunk_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_LayUp_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_middle:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Middle_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_middleUnstready:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyMiddle_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Bonus_Multi")) :static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Hook_And_Post_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_closePostUnstready:
		case SHOT_TYPE::shotType_postUnstready:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_UnsteadyPost_Multi"));
		}
		break;
		case SHOT_TYPE::shotType_threePoint:
		{
			rateBoundForwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Bonus_ForwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_ForwardWeak"));
			rateBoundForwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Bonus_ForwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_ForwardStrong"));
			rateBoundBackwardWeak = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Bonus_BackwardWeak")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_BackwardWeak"));
			rateBoundBackwardStrong = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Bonus_BackwardStrong")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_BackwardStrong"));
			rateBoundMulti = (bonus > 0) ? static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Bonus_Multi")) : static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotBoundDirection_Three_Multi"));
		}
		break;
		default:
		{
			std::string log_message = "Invalid shotType";
			m_pHost->ToLog(log_message.c_str());
		}
		break;
	}

	int randomBound = Util::GetRandomSetMax(rateBoundForwardWeak + rateBoundForwardStrong + rateBoundBackwardWeak + rateBoundBackwardStrong + rateBoundMulti);

	F4PACKET::SHOT_BOUND shot_bound = F4PACKET::SHOT_BOUND::MAX;

	if (randomBound < rateBoundForwardWeak)
	{
		shot_bound = SHOT_BOUND::fail_forward_weak_bound;
	}
	else if (randomBound < rateBoundForwardWeak + rateBoundForwardStrong)
	{
		shot_bound = SHOT_BOUND::fail_forward_strong_bound;
	}
	else if (randomBound < rateBoundForwardWeak + rateBoundForwardStrong + rateBoundBackwardWeak)
	{
		shot_bound = SHOT_BOUND::fail_backward_weak_bound;
	}
	else if (randomBound < rateBoundForwardWeak + rateBoundForwardStrong + rateBoundBackwardWeak + rateBoundBackwardStrong)
	{
		shot_bound = SHOT_BOUND::fail_backward_strong_bound;
	}
	else
	{
		shot_bound = SHOT_BOUND::fail_rim_multi_bound;
	}

	return shot_bound;
}

DHOST_TYPE_FLOAT CState::ApplyMedalEffectToShotSuccessRate(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	DHOST_TYPE_FLOAT success_rate = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	for (int i = 0; i < pPlayerInfo->medals.size(); ++i)
	{
		F4PACKET::SMedalInfo medalInfo = pPlayerInfo->medals[i];
		F4PACKET::MEDAL_INDEX medal = medalInfo.index();

		// ��ũ�� ���̽� 
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::sacrifice)
		{
			switch (shot_type)
			{
			case SHOT_TYPE::shotType_middle:
			case SHOT_TYPE::shotType_threePoint:
			default:
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_Sacrifice") * medalInfo.value();
				break;
			}

			continue;
		}

		
		// ���ܹ� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::stepBackJumper)
		{
			switch (pShotInfo->skillindex)
			{
			case F4PACKET::SKILL_INDEX::skill_stepBackJumper:
				success_rate += m_pHost->GetBalanceTable()->GetValue("VStepBackJumper_Probability_Medal") * medalInfo.value();
				break;

			case F4PACKET::SKILL_INDEX::skill_stepBackThreePointShot:
				success_rate += m_pHost->GetBalanceTable()->GetValue("VStepBackThreePointShot_Probability_Medal") * medalInfo.value();
				break;

			default:
				break;
			}

			continue;
		}

		// Ǯ�� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::pullUpJumper)
		{
			switch (pShotInfo->skillindex)
			{
			case F4PACKET::SKILL_INDEX::skill_pullUpJumper:
				success_rate += m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_Probability_Medal") * medalInfo.value();
				break;

			default:
				break;
			}

			continue;
		}

		// ���̵����� ������
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::fadeAwayMaster)
		{
			switch (pShotInfo->skillindex)
			{
			case F4PACKET::SKILL_INDEX::skill_fadeAway:
			case F4PACKET::SKILL_INDEX::skill_fadeAway3:
			case F4PACKET::SKILL_INDEX::skill_oneLegedFadeAway:
			case F4PACKET::SKILL_INDEX::skill_turnAroundFadeAway:
				success_rate += m_pHost->GetBalanceTable()->GetValue("VFadeAwayProbability_Medal_FadeAwayMaster") * medalInfo.value();
				break;
			default:
				break;
			}

			continue;
		}

		// ���̾� ��ũ�ϼ�
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::layUpTechnician)
		{
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_doubleCluch)
				success_rate += m_pHost->GetBalanceTable()->GetValue("VSkillDblClutchProbability_Medal_LayUpTechnician") * medalInfo.value();

			continue;
		}

		// Ŭ��ġ ���ھ
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::clutchScorer)
		{
			if (m_pHost->GetScoreDifference() <= 3)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_ClutchPlayer_Shoot") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}

			continue;
		}

		// ���̾� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::spotUpShooter)
		{
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_spotUpShot
				|| pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_spotUpShot3)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VSpotUp_Probability_Medal") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}

			continue;
		}

		// �ε巯�� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::smoothSnap)
		{
			if (shot_type == F4PACKET::SHOT_TYPE::shotType_hook	|| shot_type == F4PACKET::SHOT_TYPE::shotType_post)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VPostShotProbability_SmoothSnap") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}

			continue;
		}

		// ���� ������
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::scoopShotMaster)
		{
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_scoopShot)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VScoopShot_Probability_Medal") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}

			continue;
		}

		// ���� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::scoreSupport)
		{
			if (m_coolTimer->withinTime(CCoolTimer::TYPE::RECEIVE_PASS, 2) == true)
			{
				switch (shot_type)
				{
				case F4PACKET::SHOT_TYPE::shotType_post:
				case F4PACKET::SHOT_TYPE::shotType_hook:
				case F4PACKET::SHOT_TYPE::shotType_middle:
				case F4PACKET::SHOT_TYPE::shotType_threePoint:
				{
					success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_ScoreSupport") * medalInfo.value();
					pShotInfo->medalindices.push_back(medalInfo.index());
				}
				break;
				default:
					break;
				}
			}

			continue;
		}

		// ������ �ճ�����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::delicateTouch)
		{
			switch (shot_type)
			{
			case F4PACKET::SHOT_TYPE::shotType_layUpPost:
			case F4PACKET::SHOT_TYPE::shotType_layUpDriveIn:
			case F4PACKET::SHOT_TYPE::shotType_layUpFingerRoll:
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VLayUp_Probability_Medal_DelicateTouch") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}
			break;
			default:
				break;
			}

			continue;
		}

		// ���� ���
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::aerialAcrobat)
		{
			switch (shot_type)
			{
			case F4PACKET::SHOT_TYPE::shotType_layUpPost:
			case F4PACKET::SHOT_TYPE::shotType_layUpDriveIn:
			case F4PACKET::SHOT_TYPE::shotType_layUpFingerRoll:
			case F4PACKET::SHOT_TYPE::shotType_dunkPost:
			case F4PACKET::SHOT_TYPE::shotType_dunkDriveIn:
			case F4PACKET::SHOT_TYPE::shotType_dunkFingerRoll:
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VRimAttack_Probability_Medal_AerialAcrobat") * medalInfo.value();
				pShotInfo->medalindices.push_back(medalInfo.index());
			}
			break;
			default:
				break;
			}

			continue;
		}

		// Ƽ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::tearDrop)
		{	// �÷��� ��ų
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_floater)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VFloater_Probability_Medal_TearDrop") * medalInfo.value();
			}

			continue;
		}

		// ���κ���
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::rainbow)
		{	// ���κ��� ��ų
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_rainbowShot)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VRainbowShot_Probability_Medal_Rainbow") * medalInfo.value();
			}

			continue;
		}

		// �Ϻ��� ǲ��
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::perfectPutback)
		{	// ����, �ǵ�ũ ��ų
			if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_tipIn)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VTipIn_Probability_Medal_Tap") * medalInfo.value();
			}
			else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_tapDunk)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VTapDunk_Probability_Medal_Tap") * medalInfo.value();
			}

			continue;
		}

		// �۷���
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::glove)
		{
			switch (shot_type)
			{
			case SHOT_TYPE::shotType_middle:
			case SHOT_TYPE::shotType_threePoint:
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_Glove") * medalInfo.value();
			}
			break;
			default:
				break;
			}

			continue;
		}

		// ��Ÿ� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::longDistanceShooter)
		{
			if (shot_type == SHOT_TYPE::shotType_threePoint
			 || shot_type == SHOT_TYPE::shotType_middle)
			{
				// ���� ������ �Ÿ� ���
				float shotDistance = m_pHost->GetBalanceTable()->GetValue("VLongDistanceShooter_Distance");
				float shotFromRim = m_pHost->GetShotPositionDistanceFromRim(pShotInfo->positionowner->x(), pShotInfo->positionowner->z());

				if (shotFromRim >= shotDistance)
				{
					success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_LongDistanceShooter") * medalInfo.value();
					pShotInfo->medalindices.push_back(medal);
				}
			}

			continue;
		}

		// 3 & D
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::threeND)
		{
			if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VThreePointShotProbability_3D") * medalInfo.value();
				pShotInfo->medalindices.push_back(medal);
			}

			continue;
		}

		// ��ź�� ó����
		if (medal == F4PACKET::MEDAL_INDEX::medal_explosiveOrdnanceDisposal)
		{
			if (m_pHost->GetShotClock() < 3.0f && CheckShotTypeJump(shot_type))
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_EOD") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// ȭ���� ����(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_hotStartShoot)
		{
			if (m_pHost->GetOverTime() == false && (m_pHost->GetGameTimeInit() - m_pHost->GetGameTime() < 60.0f))
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_HotStart_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// �ºο�(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_competitiveSpiritShoot)
		{
			if (m_pHost->GetScoreLoosing(pPlayerInfo->team))
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_CompetitiveSpirit_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// Ŭ��ġ �÷��̾�(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_clutchPlayerShoot)
		{
			if (m_pHost->GetScoreDifference() <= 3)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_ClutchPlayer_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// �������� ������(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_stableFinishShoot)
		{
			if (m_pHost->GetOverTime() == false && m_pHost->GetGameTime() <= 60.0f)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_StableFinish_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// ������ ������(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_overTimeRulerShoot)
		{
			if (m_pHost->GetOverTime() == true)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_OverTimeRuler_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// �Ⱦ�����(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_pickAndPoperShoot)
		{
			if (m_coolTimer->withinTime(CCoolTimer::TYPE::SCREEN_COLLISION, 5) == true && CheckShotTypeJump(shot_type))
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_PickAndPoper") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// �� ���潺(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_earlyOffenseShooting)
		{
			// ��Ŭ�� ���� �ķ� 3�� �̳��� �ð����� üũ
			if ((kSHOT_CLOCK_TIME_INIT - m_pHost->GetShotClock()) < 3.0f && CheckShotTypeUnstready(shot_type) == false)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_EarlyOffense_Shoot") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
			continue;
		}

		// �㽽�÷���(��)
		if (medal == F4PACKET::MEDAL_INDEX::medal_hustlePlayShoot)
		{
			CBallController* pBallController = m_pHost->BallControllerGet();

			if (pBallController != nullptr)
			{
				bool enable = false;

				switch (pBallController->PrevBallStateGet())
				{
					case BALL_STATE::ballState_looseBlock:
					case BALL_STATE::ballState_looseCollision:
					case BALL_STATE::ballState_loosePass:
					case BALL_STATE::ballState_looseRebound:
					case BALL_STATE::ballState_looseSteal:
					{
						enable = true;
					}
					break;
					case BALL_STATE::ballState_shotDunk:
					case BALL_STATE::ballState_shotJump:
					case BALL_STATE::ballState_shotLayUp:
					case BALL_STATE::ballState_shotPost:
					{
						if (pBallController->GameBallDataGet() != nullptr)
						{
							switch (pBallController->GameBallDataGet()->ballState)
							{
								case BALL_STATE::ballState_pickLoose:
								{
									enable = true;
								}
								break;
							}
						}
					}
					break;
				}

				if (enable == true)
				{
					CBallController::GameBallData* pGameBallData = pBallController->GameBallDataGet();

					if (pGameBallData != nullptr && pGameBallData->timeElapsed <= 3.0f)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VShootProbability_Medal_HustlePlay") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}
			continue;
		}

		switch (shot_type)
		{
			case SHOT_TYPE::shotType_dunkDriveIn:
			case SHOT_TYPE::shotType_dunkFingerRoll:
			case SHOT_TYPE::shotType_dunkPost:
			{
				if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_tapDunk)
				{
					// �� (������)
					if (medal == F4PACKET::MEDAL_INDEX::medal_tapProbability)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VTapDunk_Probability_Medal_Tap") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}

				// �Ⱦطѷ� ��ũ
				if (medal == F4PACKET::MEDAL_INDEX::medal_pickAndRollerDunk)
				{
					if (m_coolTimer->withinTime(CCoolTimer::TYPE::SCREEN_COLLISION, 5) == true)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VDunkProbability_Medal_PickAndRoller") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}
			break;
			case SHOT_TYPE::shotType_layUpDriveIn:
			case SHOT_TYPE::shotType_layUpFingerRoll:
			case SHOT_TYPE::shotType_layUpPost:
			{
				// ������ �ճ�����
				if (medal == F4PACKET::MEDAL_INDEX::medal_delicateTouch)
				{
					success_rate += m_pHost->GetBalanceTable()->GetValue("VLayUp_Probability_Medal_DelicateTouch") * pPlayerInfo->medals[i].value();
					pShotInfo->medalindices.push_back(medal);
				}

				// �Ⱦطѷ� ���̾�
				if (medal == F4PACKET::MEDAL_INDEX::medal_pickAndRollerLayUp)
				{
					if (m_coolTimer->withinTime(CCoolTimer::TYPE::SCREEN_COLLISION, 5) == true)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VLayUpProbability_Medal_PickAndRoller") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}
			break;
			case SHOT_TYPE::shotType_middle:
			{
				if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_fadeAway || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_turnAroundFadeAway || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_oneLegedFadeAway)
				{
					// ���̵����� ������
					if (medal == F4PACKET::MEDAL_INDEX::medal_fadeAwayMaster)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VFadeAwayProbability_Medal_FadeAwayMaster") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_pullUpJumper || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_stepBackJumper)
				{
					// ���� ����
					if (medal == F4PACKET::MEDAL_INDEX::medal_rhythmShooter)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShot_Probability_Medal_RhythmShooter") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_spotUpShot)
				{
					// ���̾� ����
					if (medal == F4PACKET::MEDAL_INDEX::medal_spotUpShooter)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VSpotUp_Probability_Medal") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_runner)
				{
					// ����
					if (medal == F4PACKET::MEDAL_INDEX::medal_runner)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VRunner_ProbabilityA_Medal") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}  
			break;
			case SHOT_TYPE::shotType_hook:
			case SHOT_TYPE::shotType_post:
			{
				if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_hookShot || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_hookShotPost || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_hookShotPostSky || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_hookShotSky)
				{
					// �ż� ������
					if (medal == F4PACKET::MEDAL_INDEX::medal_hookShotMaster)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VHookShot_Probability_Medal_HookShotMaster") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_tipIn)
				{
					// �� (������)
					if (medal == F4PACKET::MEDAL_INDEX::medal_tapProbability)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VTipIn_Probability_Medal_Tap") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}
			break;
			case SHOT_TYPE::shotType_threePoint:
			{
				if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_fadeAway3)
				{
					// ���̵����� ������
					if (medal == F4PACKET::MEDAL_INDEX::medal_fadeAwayMaster)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VFadeAwayProbability_Medal_FadeAwayMaster") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_pullUpJumper || pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_stepBackJumper)
				{
					// ���� ����
					if (medal == F4PACKET::MEDAL_INDEX::medal_rhythmShooter)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VJumpShot_Probability_Medal_RhythmShooter") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
				else if (pShotInfo->skillindex == F4PACKET::SKILL_INDEX::skill_spotUpShot3)
				{
					// ���̾� ����
					if (medal == F4PACKET::MEDAL_INDEX::medal_spotUpShooter)
					{
						success_rate += m_pHost->GetBalanceTable()->GetValue("VSpotUp_Probability_Medal") * pPlayerInfo->medals[i].value();
						pShotInfo->medalindices.push_back(medal);
					}
				}
			}
			break;
			default:
			{
				
			}
			break;
		}
	}

	return success_rate;
}

DHOST_TYPE_FLOAT CState::ApplyMedalEffectToShotRange(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	DHOST_TYPE_FLOAT shot_range = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	for (int i = 0; i < pPlayerInfo->medals.size(); i++)
	{
		F4PACKET::MEDAL_INDEX medal = pPlayerInfo->medals[i].index();

		if (medal == F4PACKET::MEDAL_INDEX::medal_midRangeMaster && (shot_type == F4PACKET::SHOT_TYPE::shotType_middle || shot_type == F4PACKET::SHOT_TYPE::shotType_middleUnstready))
		{
			shot_range += m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRange_Tag") * pPlayerInfo->medals[i].value();
			pShotInfo->medalindices.push_back(medal);
			break;
		}

		if (medal == F4PACKET::MEDAL_INDEX::medal_longRange && shot_type == F4PACKET::SHOT_TYPE::shotType_threePoint)
		{
			shot_range += m_pHost->GetBalanceTable()->GetValue("VThreePointShotMinimumRange_Tag") * pPlayerInfo->medals[i].value();
			pShotInfo->medalindices.push_back(medal);
			break;
		}

		if (medal == F4PACKET::MEDAL_INDEX::midRangeMaster && (shot_type == F4PACKET::SHOT_TYPE::shotType_middle || shot_type == F4PACKET::SHOT_TYPE::shotType_middleUnstready))
		{
			shot_range += m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRange_Medal") * pPlayerInfo->medals[i].value();
			pShotInfo->medalindices.push_back(medal);
			break;
		}

		if (medal == F4PACKET::MEDAL_INDEX::longRange && shot_type == F4PACKET::SHOT_TYPE::shotType_threePoint)
		{
			shot_range += m_pHost->GetBalanceTable()->GetValue("VThreePointShotMinimumRange_Medal") * pPlayerInfo->medals[i].value();
			pShotInfo->medalindices.push_back(medal);
			break;
		}
	}

	return shot_range;
}

DHOST_TYPE_FLOAT CState::ApplyMedalEffectToShotPosition(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	DHOST_TYPE_FLOAT success_rate = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	for (int i = 0; i < pPlayerInfo->medals.size(); i++)
	{
		F4PACKET::SMedalInfo medalInfo = pPlayerInfo->medals[i];
		F4PACKET::MEDAL_INDEX medal = medalInfo.index();

		// �ڳ� ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::cornerShooter)
		{
			if (shot_type == F4PACKET::SHOT_TYPE::shotType_threePoint)
			{
				success_rate += medalInfo.value() * kTYPE_CAST_TO_FLOAT;
				pShotInfo->medalindices.push_back(medalInfo.index());
			}

			continue;
		}


		switch (shot_type)
		{
			case SHOT_TYPE::shotType_threePoint:
			{
				if (medal == F4PACKET::MEDAL_INDEX::medal_cornerShooter)
				{
					success_rate += pPlayerInfo->medals[i].level() * kTYPE_CAST_TO_FLOAT;
					pShotInfo->medalindices.push_back(medal);
				}
			}
			break;
			default:
			{

			}
			break;
		}
	}

	return success_rate;
}

DHOST_TYPE_FLOAT CState::ApplyMedalEffectToShotWideOpen(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility)
{
	DHOST_TYPE_FLOAT success_rate = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	DHOST_TYPE_BOOL jump_shot_type = false;

	float a, b, c, d, e = kFLOAT_INIT;

	if (pShotInfo->wideopen)
	{
		switch (shot_type)
		{
			case SHOT_TYPE::shotType_dunkDriveIn:
			case SHOT_TYPE::shotType_dunkFingerRoll:
			case SHOT_TYPE::shotType_dunkPost:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_dunk);
				b = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_DunkA");
				c = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_DunkB");
				d = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_DunkC");
			}
			break;
			case SHOT_TYPE::shotType_layUpDriveIn:
			case SHOT_TYPE::shotType_layUpFingerRoll:
			case SHOT_TYPE::shotType_layUpPost:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_layUp);
				b = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_LayupA");
				c = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_LayupB");
				d = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_LayupC");
			}
			break;
			case SHOT_TYPE::shotType_middle:
			case SHOT_TYPE::shotType_middleUnstready:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot);
				b = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShotA");
				c = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShotB");
				d = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShotC");
			}
			break;
			case SHOT_TYPE::shotType_threePoint:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot);
				b = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShotA");
				c = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShotB");
				d = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShotC");
			}
			break;
			case SHOT_TYPE::shotType_closePostUnstready:
			case SHOT_TYPE::shotType_hook:
			case SHOT_TYPE::shotType_post:
			case SHOT_TYPE::shotType_postUnstready:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_postShot);
				b = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_PostShotA");
				c = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_PostShotB");
				d = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_PostShotC");
			}
			break;
			default:
			{

			}
			break;
		}
	}

	jump_shot_type = CheckShotTypeJump(shot_type);
	
	DHOST_TYPE_FLOAT ToughShotMania_Penalty = kFLOAT_INIT;
	DHOST_TYPE_FLOAT SensitiveShooter_Benefit = kFLOAT_INIT;
	DHOST_TYPE_FLOAT SensitiveShooter_Delta = kFLOAT_INIT;
	DHOST_TYPE_FLOAT ToughShotMania_Delta = kFLOAT_INIT;
	DHOST_TYPE_FLOAT DeadEye_Delta = kFLOAT_INIT;
	
	for (int i = 0; i < pPlayerInfo->medals.size(); i++)
	{
		F4PACKET::SMedalInfo medalInfo = pPlayerInfo->medals[i];
		F4PACKET::MEDAL_INDEX medal = medalInfo.index();

		// ������ �ŴϾ�, ������ ���ʹ� �� Ÿ���� �������� ���� ������
		if (jump_shot_type)
		{
			if (medal == F4PACKET::MEDAL_INDEX::medal_toughShotMania)
			{
				ToughShotMania_Penalty += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_ToughShotMania_Penalty") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}

			if (medal == F4PACKET::MEDAL_INDEX::medal_sensitiveShooter)
			{
				SensitiveShooter_Benefit += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_SensitiveShooter_Benefit") * pPlayerInfo->medals[i].value();
				pShotInfo->medalindices.push_back(medal);
			}
		}

		// ������ ����
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::sensitiveShooter)
		{
			if (jump_shot_type)
			{
				if (pShotInfo->wideopen)
				{
					SensitiveShooter_Delta += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_SensitiveShooter_Benefit") * medalInfo.value();
					pShotInfo->medalindices.push_back(medalInfo.index());
				}
				else
				{
					SensitiveShooter_Delta += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_SensitiveShooter_Penalty") * medalInfo.value();
				}
			}

			continue;
		}

		// ������ �ŴϾ�
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::toughShotMania)
		{
			if (jump_shot_type)
			{
				if (pShotInfo->wideopen)
				{
					ToughShotMania_Delta += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_ToughShotMania_Penalty") * medalInfo.value();
				}
				else
				{
					ToughShotMania_Delta += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_ToughShotMania_Benefit") * medalInfo.value();
					pShotInfo->medalindices.push_back(medalInfo.index());
				}
			}

			continue;
		}

		// �������
		if (medalInfo.index() == F4PACKET::MEDAL_INDEX::deadEye)
		{
			if (jump_shot_type)
			{
				if (!pShotInfo->wideopen)
				{
					DeadEye_Delta += m_pHost->GetBalanceTable()->GetValue("VJumpShotProbability_Medal_DeadEye")* medalInfo.value();
					pShotInfo->medalindices.push_back(medalInfo.index()); // �̰��� �ؾ�, Ŭ���̾�Ʈ�� UI�� ǥ�õ� 
				}
			}

			continue;
		}

		// ���̵� ����
		if (medal == F4PACKET::MEDAL_INDEX::wideOpen)
		{
			if (pShotInfo->wideopen)
			{
				e += pPlayerInfo->medals[i].value() * kTYPE_CAST_TO_FLOAT;

				switch (shot_type)
				{
				case SHOT_TYPE::shotType_middle:
				case SHOT_TYPE::shotType_threePoint:
				case SHOT_TYPE::shotType_post:
				case SHOT_TYPE::shotType_hook:
					pShotInfo->medalindices.push_back(medal);
					break;
				default:
					break;
				}
			}

			continue;
		}
	}

	if (pShotInfo->wideopen)
	{
		success_rate = a * b + c + d * e;
		success_rate -= ToughShotMania_Penalty;
		success_rate += SensitiveShooter_Benefit;
	}
	else
	{
		success_rate += ToughShotMania_Penalty;
		success_rate -= SensitiveShooter_Benefit;
	}

	success_rate += SensitiveShooter_Delta;
	success_rate += ToughShotMania_Delta;
	success_rate += DeadEye_Delta;

	return success_rate;
}

DHOST_TYPE_FLOAT CState::CalcShotTypeToSuccessRate(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility, DHOST_TYPE_FLOAT distanceRimToOwner, F4PACKET::BALL_STATE& ballState)
{
	DHOST_TYPE_FLOAT success_rate = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	float a, b, c, d, e, f, g, h, i = kFLOAT_INIT;

	switch (shot_type)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_dunk);
			b = m_pHost->GetBalanceTable()->GetValue("VDunkProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VDunkProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VDunkProbabilityC");

			success_rate = LOG(a, b) / c + d;

			ballState = F4PACKET::BALL_STATE::ballState_shotDunk;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Dunk_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Dunk_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Dunk_High"))));

			if (pShotInfo->skillindex == SKILL_INDEX::skill_popUpDunk)
			{
				success_rate += m_pHost->GetBalanceTable()->GetValue("VPopUpDunk_ProbabilityA") + m_pHost->GetBalanceTable()->GetValue("VPopUpDunk_ProbabilityB") * m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			}
		}
		break;
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			switch (pShotInfo->skillindex)
			{
			case SKILL_INDEX::skill_scoopShot:
			case SKILL_INDEX::skill_floater:
				break;
			default:
			{
				a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_layUp);
				b = m_pHost->GetBalanceTable()->GetValue("VLayUpProbabilityA");
				c = m_pHost->GetBalanceTable()->GetValue("VLayUpProbabilityB");
				d = m_pHost->GetBalanceTable()->GetValue("VLayUpProbabilityC");

				success_rate = LOG(a, b) / c - d;
			}
			break;
			}


			ballState = F4PACKET::BALL_STATE::ballState_shotLayUp;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_High"))));
		}
		break;
		case SHOT_TYPE::shotType_middle:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot);
			b = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityC");
			e = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRangeA");
			f = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRangeB");
			g = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMaximumRange");
			h = distanceRimToOwner;

			float calculated = LOG(a, b) / c - d;
			float midRangeMin = e + a * f;
			if (!isfinite(calculated)) calculated = 0.0f;

			if (h > midRangeMin)
			{
				midRangeMin += ApplyMedalEffectToShotRange(pPlayerInfo, pShotInfo);
			}


			float midRange;
			if (static_cast<int>(pPlayerInfo->characterid / 100) == 12)	// characterid: 12 == character:sarang, �̵鼦�� ��� �Ÿ��� ���� ������ ����.
				midRange = 0;
			else
				{
					float denomMiddle = g - midRangeMin;
					midRange = (fabsf(denomMiddle) > 1e-6f) ? MAX(0.0f, (h - midRangeMin) / denomMiddle) : 0.0f;
				}


			success_rate = calculated - (calculated * midRange);

			ballState = F4PACKET::BALL_STATE::ballState_shotJump;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Middle_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Middle_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Middle_High")), m_pHost));
		}
		break;
		case SHOT_TYPE::shotType_middleUnstready:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_midRangeShot);
			b = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbabilityC");
			e = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotProbability_Unsteady");
			f = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRangeA");
			g = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMinimumRangeB");
			h = m_pHost->GetBalanceTable()->GetValue("VMidRangeShotMaximumRange");
			i = distanceRimToOwner;

			float calculated = LOG(a, b) / c - d - e;

			float midRangeMin = f + a * g;

			if (!isfinite(calculated)) calculated = 0.0f;

			float denomUnsteady = h - midRangeMin;
			float rangeRatioUnsteady = (fabsf(denomUnsteady) > 1e-6f) ? MAX(0.0f, (i - midRangeMin) / denomUnsteady) : 0.0f;
			success_rate = calculated - calculated * rangeRatioUnsteady;

			ballState = F4PACKET::BALL_STATE::ballState_shotJump;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyMiddle_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyMiddle_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyMiddle_High")), m_pHost));
		}
		break;
		case SHOT_TYPE::shotType_threePoint:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot);
			b = m_pHost->GetBalanceTable()->GetValue("VThreePointShotProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VThreePointShotProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VThreePointShotProbabilityC");
			e = m_pHost->GetBalanceTable()->GetValue("VThreePointShotMinimumRange");
			f = m_pHost->GetBalanceTable()->GetValue("VThreePointShotMaximumRange");
			g = distanceRimToOwner;

			float calculated = MAX(LOG(a, b) / c - d, kFLOAT_INIT);
			if (!isfinite(calculated)) calculated = 0.0f;

			if (distanceRimToOwner > POINT3_LENGTH)
			{
				if (e < distanceRimToOwner)
				{
					e += ApplyMedalEffectToShotRange(pPlayerInfo, pShotInfo);
					e += CalcSkillToShotRangeBonus(pPlayerInfo, pShotInfo);
				}
			}

			float denomThree = f - e;
			float rangeRatioThree = (fabsf(denomThree) > 1e-6f) ? MAX(0.0f, (g - e) / denomThree) : 0.0f;
			success_rate = calculated - calculated * rangeRatioThree;

			float check3PointZ = POINT3_CHECK_Z();
			float courtWidthHalf = COURT_WIDTH_HALF();

			float checkZ = check3PointZ + (courtWidthHalf - check3PointZ) * 0.5f;

			if (pShotInfo->positionball->z() > checkZ)
			{
				float a1, b1, c1, d1, e1;

				a1 = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_threePointShot);
				b1 = m_pHost->GetBalanceTable()->GetValue("VBaseLineBonusA");
				c1 = m_pHost->GetBalanceTable()->GetValue("VBaseLineBonusB");
				d1 = m_pHost->GetBalanceTable()->GetValue("VBaseLineBonusC");

				e1 = ApplyMedalEffectToShotPosition(pPlayerInfo, pShotInfo);

				success_rate += a1 * b1 + c1 + d1 * e1;
			}

			ballState = F4PACKET::BALL_STATE::ballState_shotJump;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Three_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Three_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Three_High")), m_pHost));
		}
		break;
		case SHOT_TYPE::shotType_closePostUnstready:
		case SHOT_TYPE::shotType_postUnstready:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_postShot);
			b = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityC");
			e = m_pHost->GetBalanceTable()->GetValue("VPostShotProbability_Unsteady");

			success_rate = LOG(a, b) / c - d - e;

			ballState = F4PACKET::BALL_STATE::ballState_shotPost;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyPost_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyPost_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_UnsteadyPost_High")), m_pHost));
		}
		break;
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		{
			a = pAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_postShot);
			b = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityA");
			c = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VPostShotProbabilityC");

			success_rate = LOG(a, b) / c - d;


			ballState = F4PACKET::BALL_STATE::ballState_shotPost;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_High")), m_pHost));
		}
		break;
		default:
		{
			string log_message = "CState::CalcShotTypeToSuccessRate Invalid ShotType CharacterID : " + std::to_string(pPlayerInfo->id) + ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(shot_type);
			m_pHost->ToLog(log_message.c_str());
		}
		break;
	}

	return success_rate;
}

DHOST_TYPE_FLOAT CState::CalcHandednessToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility)
{
	DHOST_TYPE_FLOAT success_rate_bonus = kFLOAT_INIT;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	if (CheckHandednessShotType(shot_type))
	{
		if (pShotInfo->ownhanded)
		{
			success_rate_bonus = m_pHost->GetBalanceTable()->GetValue("VHandedness_Driving_Benefit");
		}
		else
		{
			success_rate_bonus = -m_pHost->GetBalanceTable()->GetValue("VHandedness_Driving_Penalty");
		}
	}
	
	return success_rate_bonus;
}

DHOST_TYPE_FLOAT CState::CalcSkillToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility, DHOST_TYPE_FLOAT successRate, DHOST_TYPE_BOOL bSkillForce)
{
	DHOST_TYPE_FLOAT result = successRate;

	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	DHOST_TYPE_FLOAT a, b, c, d, e = kFLOAT_INIT;

	switch (pShotInfo->skillindex)
	{
		case SKILL_INDEX::behindStepBackJumper:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VBehindStepBackJumper_MidRange_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VBehindStepBackJumper_MidRange_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VBehindStepBackJumper_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VBehindStepBackJumper_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;

		case SKILL_INDEX::oneDribblePullUpJumper:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VOneDribblePullUp_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VOneDribblePullUp_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VOneDribblePullUp_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VOneDribblePullUp_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_hopStep:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VHopStepShot_ProbabilityA");
			b = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			c = m_pHost->GetBalanceTable()->GetValue("VHopStepShot_ProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VHopStepShot_ProbabilityC");

			result = successRate * a + b * c + d;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_High"))));
		}
		break;
		case SKILL_INDEX::skill_closeReverseDunk:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VCloseReverseDunk_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VCloseReverseDunk_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_reverseDunk:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VReverseDunk_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VReverseDunk_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_spinMove:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_ProbabilityA");
			b = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			c = m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_ProbabilityB");
			d = m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_ProbabilityC");

			result = successRate * a + b * c + d;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_HopStep_High"))));
		}
		break;
		case SKILL_INDEX::skill_tapDunk:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VTapDunk_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VTapDunk_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_tripleCluch:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VTripleClutch_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VTripleClutch_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_doubleCluch:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VSkillDblClutchProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VSkillDblClutchProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_DoubleClutch_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_DoubleClutch_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_DoubleClutch_High"))));
		}
		break;
		case SKILL_INDEX::skill_jellyLayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VJellyLayup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VJellyLayup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_closeReverseLayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VCloseReverseLayup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VCloseReverseLayup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_reverseLayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VReverseLayup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VReverseLayup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_Reverse_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_Reverse_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_LayUp_Reverse_High"))));
		}
		break;
		case SKILL_INDEX::euroStepDunk:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VEuroStepDunk_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VEuroStepDunk_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_euroStepLayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::degree360LayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("V360Layup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("V360Layup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_scoopShot:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VScoopShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VScoopShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_rainbowShot:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VRainbowShot_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VRainbowShot_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VRainbowShot_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VRainbowShot_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_floater:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VFloater_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VFloater_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_stepBackThreePointShot:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type != SHOT_TYPE::shotType_threePoint)
				break;

			a = m_pHost->GetBalanceTable()->GetValue("VStepBackThreePointShot_ThreePointShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VStepBackThreePointShot_ThreePointShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_shakeAndBake:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VShakeAndBakeLayup_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VShakeAndBakeLayup_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_fadeAway:
		{
			a = m_pHost->GetBalanceTable()->GetValue("V2PTFadeAwayProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("V2PTFadeAwayProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_pullUpJumper:
		{
			DHOST_TYPE_FLOAT skillForceValue = 0.0f;

			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_MidRangeShot_ProbabilityB");
				skillForceValue = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_MidRangeShot_Probability_Upgrade");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_ThreePointShot_ProbabilityB");
				skillForceValue = m_pHost->GetBalanceTable()->GetValue("VPullUpJumper_ThreePointShot_Probability_Upgrade");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			if (bSkillForce)
			{
				result = successRate - a + b * c + skillForceValue;
			}
			else
			{
				result = successRate - a + b * c;
			}
		}
		break;
		case SKILL_INDEX::spinningPullBack:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VSpinningPullBack_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VSpinningPullBack_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VSpinningPullBack_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VSpinningPullBack_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_stepBackJumper:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VStepBackJumper_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VStepBackJumper_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VStepBackJumper_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VStepBackJumper_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_spotUpShot:
		{
			DHOST_TYPE_FLOAT skillForceValue = 0.0f;

			a = m_pHost->GetBalanceTable()->GetValue("VSpotUp_MidRangeShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VSpotUp_MidRangeShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			skillForceValue = m_pHost->GetBalanceTable()->GetValue("VSpotUpJumpShot_Probability_Upgrade");

			result = successRate - a + b * c;

			if (bSkillForce)
			{
				result = successRate - a + b * c + skillForceValue;
			}
			else
			{
				result = successRate - a + b * c;
			}
		}
		break;
		case SKILL_INDEX::skill_spotUpShot3:
		{
			DHOST_TYPE_FLOAT skillForceValue = 0.0f;

			a = m_pHost->GetBalanceTable()->GetValue("VSpotUp_ThreePointShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VSpotUp_ThreePointShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			skillForceValue = m_pHost->GetBalanceTable()->GetValue("VSpotUpThreePointShot_Probability_Upgrade");
			result = successRate - a + b * c;
			if (bSkillForce)
			{
				result = successRate - a + b * c + skillForceValue;
			}
			else
			{
				result = successRate - a + b * c;
			}
		}
		break;
		case SKILL_INDEX::skill_turnAroundFadeAway:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VTurnAroundFadeAway_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VTurnAroundFadeAway_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_oneLegedFadeAway:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VOneLeggedFadeAway_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VOneLeggedFadeAway_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_runner:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VRunner_ProbabilityA");
			b = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			c = m_pHost->GetBalanceTable()->GetValue("VRunner_ProbabilityB");

			result = successRate - a + b * c;
		}
		break;

		case SKILL_INDEX::skill_aimShot:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VAimShot_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VAimShot_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VAimShot_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VAimShot_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			result = successRate - a + b * c;
		}

		break;
		case SKILL_INDEX::skill_hookShot:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VHookShotProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VHookShotProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_High"))));
		}
		break;
		case SKILL_INDEX::skill_hookShotPost:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VPostHookShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VPostHookShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Post_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Post_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Post_High"))));
		}
		break;
		case SKILL_INDEX::skill_hookShotPostSky:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VPostSkyHookShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VPostSkyHookShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_PostRunning_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_PostRunning_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_PostRunning_High"))));
		}
		break;
		case SKILL_INDEX::skill_hookShotSky:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VSkyHookShot_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VSkyHookShot_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Sky_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Sky_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Sky_High"))));
		}
		break;
		case SKILL_INDEX::skill_tipIn:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VTipIn_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VTipIn_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;

			pShotInfo->shotthrow = (RouletteShotThrow(static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Tipin_Low")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Tipin_Middle")), static_cast<int>(m_pHost->GetBalanceTable()->GetValue("VShotThrow_Hook_And_Post_Tipin_High"))));
		}
		break;
		case SKILL_INDEX::skill_fadeAway3:
		{
			a = m_pHost->GetBalanceTable()->GetValue("V3PTFadeAwayProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("V3PTFadeAwayProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_turningShot:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VTurningShot_ProbabilityA");
			b = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);
			c = m_pHost->GetBalanceTable()->GetValue("VTurningShot_ProbabilityB");

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::skill_putback:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VPutback_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VPutback_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::skill_kneeKick:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VKneeKick_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VKneeKick_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::catchAndShot:
		{
			F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;
			if (shot_type == SHOT_TYPE::shotType_middle)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VCatchAndShot_MidRangeShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VCatchAndShot_MidRangeShot_ProbabilityB");
			}
			else if (shot_type == SHOT_TYPE::shotType_threePoint)
			{
				a = m_pHost->GetBalanceTable()->GetValue("VCatchAndShot_ThreePointShot_ProbabilityA");
				b = m_pHost->GetBalanceTable()->GetValue("VCatchAndShot_ThreePointShot_ProbabilityB");
			}
			else
			{
				a = b = kFLOAT_INIT;
			}

			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate - a + b * c;
		}
		break;
		case SKILL_INDEX::quickLayUp:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VQuickLayUp_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VQuickLayUp_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::passFake:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VPassFake_LayUp_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VPassFake_LayUp_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		case SKILL_INDEX::jumpHook:
		{
			a = m_pHost->GetBalanceTable()->GetValue("VJumpHook_ProbabilityA");
			b = m_pHost->GetBalanceTable()->GetValue("VJumpHook_ProbabilityB");
			c = m_pHost->GetSkillLevel(pPlayerInfo->id, pShotInfo->skillindex);

			result = successRate + a + b * c;
		}
		break;
		default:
		{
			
		}
		break;
	}

	return result;
}

DHOST_TYPE_FLOAT CState::CalcSkillToShotRangeBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	DHOST_TYPE_FLOAT result, a, b, c;
	result = a = b = c = kFLOAT_INIT;
	F4PACKET::SHOT_TYPE shot_type = pShotInfo->shottype;

	switch (pShotInfo->skillindex)
	{
		case SKILL_INDEX::skill_rainbowShot:
		{
			if (shot_type != F4PACKET::SHOT_TYPE::shotType_threePoint)
				break;

			a = m_pHost->GetBalanceTable()->GetValue("VThreePointShotMinimumRange_RainbowShotA");
			b = m_pHost->GetSkillLevel(pPlayerInfo->id, SKILL_INDEX::skill_rainbowShot);
			c = m_pHost->GetBalanceTable()->GetValue("VThreePointShotMinimumRange_RainbowShotB");

			result = a * b + c;
		}
		break;
	}

	return result;
}

// 5. ����ɷ����� ���� ���ʽ� (��ȭ ����)
DHOST_TYPE_FLOAT CState::CalcPotentialToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	//! Ʈ���� ����Ʈ �ܰ� �ʱ�ȭ
	pShotInfo->potentialfxlevel = 0;

	DHOST_TYPE_FLOAT success_rate = kFLOAT_INIT;

	if (pPlayerInfo != nullptr && pShotInfo != nullptr)
	{
		{
			string verify_log = "[POTENTIAL_SHOT] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				//+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				//+ ", UserID : " + std::to_string(pPlayerInfo->userid)
				+ ", BallNo : " + std::to_string(pShotInfo->ballnumber)
				+ ", CharacterSN : " + std::to_string(pShotInfo->owner)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pShotInfo->skillindex)
				+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pShotInfo->shottype);
			m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
		}

		SPotentialInfo sInfo;
		DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(pShotInfo->owner, ACTION_TYPE::action_shot, pShotInfo->shottype, pShotInfo->skillindex, sInfo);

		{
			string verify_log = "[POTENTIAL_SHOT] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", CharacterSN : " + std::to_string(pShotInfo->owner)
				+ ", BallNo : " + std::to_string(pShotInfo->ballnumber)
				+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
				+ ", PotentialLv : " + std::to_string(sInfo.potentiallevel())
				+ ", PotentialValue : " + std::to_string(sInfo.potentialvalue())
				+ ", awaken : " + std::to_string(sInfo.potentialawaken())
				+ ", BloomRateLv : " + std::to_string(sInfo.potentialbloomratelevel())
				+ ", BloomRateValue : " + std::to_string(sInfo.potentialbloomratevalue())
				+ ", BloomBuffLv : " + std::to_string(sInfo.potentialbloombufflevel())
				+ ", BloomBuffValue : " + std::to_string(sInfo.potentialbloombuffvalue());
			m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
		}

		//! �� ���� ���� �ɷ��� �����Ѵٸ�
		if (bPotential)
		{
			//! 1. ���� �ɷ� ����
			success_rate += sInfo.potentialvalue();

			//! 2. ���� ���� Ȯ��
			if (sInfo.potentialawaken())
			{
				//! 3. ��ȭ �ߵ����� Ȯ��(�� Ÿ���� �������̶�� ��ȭ �ߵ� Ȯ�� �����ϰ� ��ȭ���� üũ - �׼ǿ��� �̹� ����)
				DHOST_TYPE_BOOL Bloom = false;

				if (m_pHost->CheckShotTypeRimAttack(pShotInfo->shottype))
				{
					//! �������� �� ��ȭ�ƴ��� Ȯ��
					DHOST_TYPE_BALL_NUMBER ballNo = m_pHost->GetCharacterActionPotentialBloom(pShotInfo->owner);
					if (ballNo > kBALL_NUMBER_INIT && ballNo == pShotInfo->ballnumber)
					{
						Bloom = true;
					}
				}
				else
				{
					DHOST_TYPE_FLOAT random_value = m_pHost->GetRandomValueAlgorithm(pPlayerInfo->userid, false);

					if (random_value <= sInfo.potentialbloomratevalue())
					{
						Bloom = true;
					}
				}

				//! ��ȭ �ߴٸ� ��ȭ ���� ���ְ�, Ʈ���� ����Ʈ �ܰ� ����
				if (Bloom)
				{
					success_rate += sInfo.potentialbloombuffvalue();
					pShotInfo->potentialfxlevel = (sInfo.potentialbloombufflevel() >= 5 && sInfo.potentialbloomratelevel() >= 5) ? 2 : 1;

					//! �ı����� ��ũ��� �ݵ�� �����ؾ� �Ѵ�. �׷��� ���� Ȯ���� ��������� ��½����ش�
					if (pShotInfo->shottype == SHOT_TYPE::shotType_dunkDriveIn || pShotInfo->shottype == SHOT_TYPE::shotType_dunkFingerRoll || pShotInfo->shottype == SHOT_TYPE::shotType_dunkPost)
					{
						success_rate += 100.0f;
					}
				}
			}
		}
	}

	{
		string verify_log = "[POTENTIAL_SHOT] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) 
			//+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			//+ ", UserID : " + std::to_string(pPlayerInfo->userid)
			+ ", BallNo : " + std::to_string(pShotInfo->ballnumber)
			+ ", CharacterSN : " + std::to_string(pShotInfo->owner)
			//+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pShotInfo->skillindex)
			//+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pShotInfo->shottype)
			+ ", success_rate : " + std::to_string(success_rate)
			+ ", potentialfxlevel : " + std::to_string(pShotInfo->potentialfxlevel);
		m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
	}

	return success_rate;
}

DHOST_TYPE_FLOAT CState::CalcPotentialToHindrancesBonus(DHOST_TYPE_CHARACTER_SN id, SBallShotT* pShotInfo, DHOST_TYPE_INT32* fxLevel, ACTION_TYPE actionType)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;
	*fxLevel = kINT32_INIT;

	//! �̵巹����, 3���� �� ����
	if (pShotInfo->shottype == SHOT_TYPE::shotType_middle || pShotInfo->shottype == SHOT_TYPE::shotType_middleUnstready || pShotInfo->shottype == SHOT_TYPE::shotType_threePoint)
	{
		//! ���� ������ �� ĳ����
		F4PACKET::SPlayerInformationT* pPlayerInfo = m_pHost->FindCharacterInformation(id);

		if (pPlayerInfo != nullptr)
		{
			SPotentialInfo sInfo;
			DHOST_TYPE_BOOL bPotential = m_pHost->GetCharacterPotentialInfo(id, POTENTIAL_INDEX::breathless_perimeter_def, sInfo);

			if (bPotential)
			{
				//! ���� �ɷ�ġ ����
				result = sInfo.potentialvalue();

				//! ���� ����
				if (sInfo.potentialawaken())
				{
					DHOST_TYPE_FLOAT random_value = m_pHost->GetRandomValueAlgorithm(pPlayerInfo->userid, false);

					//! ���ù����ϴ� ĳ������ �׼�
					if ( actionType == ACTION_TYPE::action_standFaceUp || actionType == ACTION_TYPE::action_moveFaceUp
						|| actionType == ACTION_TYPE::action_standShadowFaceUp || actionType == ACTION_TYPE::action_moveShadowFaceUp
						|| actionType == ACTION_TYPE::action_standDefense || actionType == ACTION_TYPE::action_moveDefense)
					{
						//! ��ȭ �ߵ� ����
						if (random_value <= sInfo.potentialbloomratevalue())
						{
							result += sInfo.potentialbloombuffvalue();
							*fxLevel = (sInfo.potentialbloomratelevel() >= 5 && sInfo.potentialbloombufflevel() >= 5) ? 2 : 1;
						}
					}	
				}
			}
		}
	}

	return result;
}

DHOST_TYPE_INT32 CState::GetPotentialForPass(DHOST_TYPE_CHARACTER_SN id, PASS_TYPE type, SKILL_INDEX skill, DHOST_TYPE_USER_ID userid)
{
	DHOST_TYPE_INT32 fxLevel = kINT32_INIT;

	//! �����н��� ��� ����ɷ� ���� ����
	if (type != PASS_TYPE::overBad
		&& type != PASS_TYPE::overCommon
		&& type != PASS_TYPE::overGood
		&& type != PASS_TYPE::shootHookBad
		&& type != PASS_TYPE::shootJumpBad
		&& type != PASS_TYPE::shootLayUpBad)
	{
		SPotentialInfo sInfo;
		DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(id, ACTION_TYPE::action_pass, SHOT_TYPE::shotType_none, skill, sInfo);

		if (bPotential)
		{
			//! ������ �ƴٸ�
			if (sInfo.potentialawaken())
			{
				//! �̾ƿ� ���� ��ȭ �ߵ� Ȯ������ ���ٸ� ��ȭ ������ߵ�
				DHOST_TYPE_FLOAT random_value = m_pHost->GetRandomValueAlgorithm(userid, false);

				if (random_value <= sInfo.potentialbloomratevalue())
				{
					fxLevel = (sInfo.potentialbloombufflevel() >= 5 && sInfo.potentialbloomratelevel() >= 5) ? 2 : 1;

					{
						string verify_log = "[POTENTIAL_PASS_ACTION] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", CharacterSN : " + std::to_string(id)
							+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
							+ ", PassType : " + F4PACKET::EnumNamePASS_TYPE(type)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(skill)
							//+ ", PotentialLv : " + std::to_string(sInfo.potentiallevel())
							//+ ", PotentialValue : " + std::to_string(sInfo.potentialvalue())
							//+ ", awaken : " + std::to_string(sInfo.potentialawaken())
							//+ ", BloomRateLv : " + std::to_string(sInfo.potentialbloomratelevel())
							//+ ", BloomRateValue : " + std::to_string(sInfo.potentialbloomratevalue())
							//+ ", BloomBuffLv : " + std::to_string(sInfo.potentialbloombufflevel())
							//+ ", BloomBuffValue : " + std::to_string(sInfo.potentialbloombuffvalue())
							+", potentialfxlevel : " + std::to_string(fxLevel);
						m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
					}
				}
			}
		}
	}

	return fxLevel;
}

DHOST_TYPE_FLOAT CState::CalcTeamAttributesShotRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	// ���ɷ�ġ ���� ��, ���� ���� �� 
	// 1. ���� �� ���� ���� 
	// 3. ���ŷ� ���� ���� 
	// 2. ������� ���� �������� ó�� 

	float returnValue = 0.0f;

	returnValue += TeamAttackAttributesBonus(pPlayerInfo, pShotInfo);
	returnValue += TeamMentalAttributesBonus(pPlayerInfo, pShotInfo);

	return returnValue;
}

DHOST_TYPE_FLOAT CState::TeamAttackAttributesBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	// ������� �ƹ��� ���ٸ� ���� 
	std::vector<CCharacter*> vecOppenetTeamsCharacter;
	vecOppenetTeamsCharacter.clear();
	m_pHost->GetCharacterManager()->GetOppenentCharacterVector(vecOppenetTeamsCharacter, pPlayerInfo->id);
	if (vecOppenetTeamsCharacter.size() <= 0) return 0.0f;

	// �� ���� �� ���̶� ����Ʈ�� ������ �ƴϸ� ���� 
	if (!vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamcontrolluser || !pPlayerInfo->teamcontrolluser) return 0.0f;

	DHOST_TYPE_FLOAT returnValue = 0.0f;

	DHOST_TYPE_FLOAT a = 0.0f;
	DHOST_TYPE_FLOAT b = 0.0f;
	DHOST_TYPE_FLOAT c = 1.0f;
	DHOST_TYPE_FLOAT d = 0.0f;
	DHOST_TYPE_FLOAT e = 0.0f;

	//1. ���� �� ���� ����

	switch (pShotInfo->shottype)
	{
	case SHOT_TYPE::shotType_layUpDriveIn: // ���̾� 
	case SHOT_TYPE::shotType_layUpFingerRoll:
	case SHOT_TYPE::shotType_layUpPost:
		a = pPlayerInfo->teamattributesinfo->finishingatpost -
			(vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->finishingatpost
				* m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbability_Offset1")
				+ vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->shotcontesting
				* m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbability_Offset2"));

		b = m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_LayUpProbabilityD");

		break;

	case SHOT_TYPE::shotType_dunkDriveIn: // ��ũ 
	case SHOT_TYPE::shotType_dunkFingerRoll:
	case SHOT_TYPE::shotType_dunkPost:
		a = pPlayerInfo->teamattributesinfo->finishingatpost -
			(vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->finishingatpost
				* m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbability_Offset1")
				+ vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->shotcontesting
				* m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbability_Offset2"));

		b = m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_DunkProbabilityD");

		break;
	case SHOT_TYPE::shotType_post: // ��ؼ� 
	case SHOT_TYPE::shotType_postUnstready:
	case SHOT_TYPE::shotType_hook:
	case SHOT_TYPE::shotType_closePostUnstready:
		a = pPlayerInfo->teamattributesinfo->finishingatpost -
			(vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->finishingatpost
				* m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbability_Offset1")
				+ vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->shotcontesting
				* m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbability_Offset2"));

		b = m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_PostShotProbabilityD");

		break;
	case SHOT_TYPE::shotType_middle: // �̵巹���� 
	case SHOT_TYPE::shotType_middleUnstready:
		a = pPlayerInfo->teamattributesinfo->perimetershooting -
			(vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->perimeterdefense
				* m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbability_Offset1")
				+ vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->shotcontesting
				* m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbability_Offset2"));

		b = m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_MidRangeShotProbabilityD");

		break;
	case SHOT_TYPE::shotType_threePoint: // 
		a = pPlayerInfo->teamattributesinfo->perimetershooting -
			(vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->perimeterdefense
				* m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbability_Offset1")
				+ vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->shotcontesting
				* m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbability_Offset2"));

		b = m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_ThreePointShotProbabilityD");

		break;

	default:
		return 0.0f;
		
	}

	try
	{
		if (a > 0.0f)
		{
			returnValue = (SafeLog(a, b) / c + d) * e;
		}
		else
		if (a < 0.0f)
		{
			a = abs(a);
			returnValue = (SafeLog(a, b) / c + d) * e;
			returnValue = returnValue * -1.0f;
		}
		else
		{
			returnValue = SafeLog(1, b) / c + d;
		}
	}
	catch (std::exception& e)
	{
		m_pHost->ToLog(e.what());
	}

	return returnValue;
}


DHOST_TYPE_FLOAT CState::TeamMentalAttributesBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo)
{
	// ������� �ƹ��� ���ٸ� ���� 
	std::vector<CCharacter*> vecOppenetTeamsCharacter;
	vecOppenetTeamsCharacter.clear();
	m_pHost->GetCharacterManager()->GetOppenentCharacterVector(vecOppenetTeamsCharacter, pPlayerInfo->id);
	if (vecOppenetTeamsCharacter.size() <= 0) return 0.0f;

	// �� ���� �� ���̶� ����Ʈ�� ������ �ƴϸ� ���� 
	if (!vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamcontrolluser || !pPlayerInfo->teamcontrolluser) return 0.0f;

	DHOST_TYPE_FLOAT reValue = 0.0f;

	// 1. ��� ���� 2�� ����, 5���� �̳��� �� ��� ���� �������� ���� 
	int scoreDelta = kINT32_INIT;
	if (pPlayerInfo->team == 0)
		scoreDelta = m_pHost->GetScore(0) - m_pHost->GetScore(1);
	else
		scoreDelta = m_pHost->GetScore(1) - m_pHost->GetScore(0);

	if (m_pHost->GetGameTime() < 120.0f && abs(scoreDelta) < 5)
	{
		reValue += GetMentalAttributesShotBonus(pShotInfo, pPlayerInfo->teamattributesinfo->clutch, vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->clutch);
	}

	// 2. ��� ���� �� 1�� ���� ��� ���� �������� ����
	if (m_pHost->GetGameElapsedTimeSinceGameStart() < 60.0f)
	{
		reValue += GetMentalAttributesShotBonus(pShotInfo, pPlayerInfo->teamattributesinfo->startingmomentum, vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->startingmomentum);
	}

	// 3. ���� ���� ���� �� ��� ���� �������� ����
	if (scoreDelta < 0)
	{
		reValue += GetMentalAttributesShotBonus(pShotInfo, pPlayerInfo->teamattributesinfo->tenacity, vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->tenacity);
	}

	// 4. ���� �̱�� ���� �� ��� ���� �������� ����
	if (scoreDelta > 0)
	{
		reValue += GetMentalAttributesShotBonus(pShotInfo, pPlayerInfo->teamattributesinfo->concentration, vecOppenetTeamsCharacter[0]->GetCharacterInformation()->teamattributesinfo->concentration);
	}

	return reValue;
}


DHOST_TYPE_FLOAT CState::GetMentalAttributesShotBonus(SBallShotT* pShotInfo, DHOST_TYPE_FLOAT attackMentalAttribute, DHOST_TYPE_FLOAT defenceMentalAttribute)
{

	float returnValue = 0.0f;

	DHOST_TYPE_FLOAT a = 0.0f;
	DHOST_TYPE_FLOAT b = 0.0f;
	DHOST_TYPE_FLOAT c = 1.0f;
	DHOST_TYPE_FLOAT d = 0.0f;
	DHOST_TYPE_FLOAT e = 0.0f;

	//1. ���� �� ���� ����

	switch (pShotInfo->shottype)
	{
	case SHOT_TYPE::shotType_layUpDriveIn: // ���̾� 
	case SHOT_TYPE::shotType_layUpFingerRoll:
	case SHOT_TYPE::shotType_layUpPost:
		a = attackMentalAttribute - defenceMentalAttribute;
		b = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_LayUpProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_LayUpProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_LayUpProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_LayUpProbabilityD");

		break;

	case SHOT_TYPE::shotType_dunkDriveIn: // ��ũ 
	case SHOT_TYPE::shotType_dunkFingerRoll:
	case SHOT_TYPE::shotType_dunkPost:
		a = attackMentalAttribute - defenceMentalAttribute;
		b = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_DunkProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_DunkProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_DunkProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_DunkProbabilityD");

		break;
	case SHOT_TYPE::shotType_post: // ��ؼ� 
	case SHOT_TYPE::shotType_postUnstready:
	case SHOT_TYPE::shotType_hook:
	case SHOT_TYPE::shotType_closePostUnstready:
		a = attackMentalAttribute - defenceMentalAttribute;
		b = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_PostShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_PostShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_PostShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_PostShotProbabilityD");

		break;
	case SHOT_TYPE::shotType_middle: // �̵巹���� 
	case SHOT_TYPE::shotType_middleUnstready:
		a = attackMentalAttribute - defenceMentalAttribute;
		b = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_MidRangeShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_MidRangeShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_MidRangeShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_MidRangeShotProbabilityD");

		break;
	case SHOT_TYPE::shotType_threePoint: // 
		a = attackMentalAttribute - defenceMentalAttribute;
		b = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_ThreePointShotProbabilityA");
		c = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_ThreePointShotProbabilityB");
		d = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_ThreePointShotProbabilityC");
		e = m_pHost->GetBalanceTable()->GetValue("VTvT_Mental_ThreePointShotProbabilityD");

		break;

	default:
		return 0.0f;
	}

	try
	{
		if (a > 0.0f)
		{
			returnValue = (SafeLog(a, b) / c + d) * e;
		}
		else
		if (a < 0.0f)
		{
			a = abs(a);
			returnValue = (SafeLog(a, b) / c + d) * e;
			returnValue = returnValue * -1.0f;
		}
		else
		{
			returnValue = SafeLog(1, b) / c + d;
		}

	}
	catch (std::exception& e)
	{
		m_pHost->ToLog(e.what());
	}

	return returnValue;
}

float CState::CalculateTargetReachTime(CDataManagerBalance* pTable, SBallShotT* pBallShot, float distance)
{
	if (pTable == nullptr || pBallShot == nullptr)
	{
		if (m_pHost)
		{
			m_pHost->ToLog("[FILE_ERROR] CalculateTargetReachTime invalid parameter");
		}
		return 0.0f;
	}

	if (!isfinite(distance))
	{
		distance = 0.0f;
	}

	float retValue = 0;


	switch (pBallShot->skillindex)
	{
	case SKILL_INDEX::oneDribblePullUpJumper:
	{
		float a = pTable->GetValue("VShotTargetReachTimeOneDribblePullUpHighA");
		float b = pTable->GetValue("VShotTargetReachTimeOneDribblePullUpHighB");
		float c = pTable->GetValue("VShotTargetReachTimeOneDribblePullUpRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::skill_kneeKick:
	{
		float a = pTable->GetValue("VShotTargetReachTimeKneeKickHighA");
		float b = pTable->GetValue("VShotTargetReachTimeKneeKickHighB");
		float c = pTable->GetValue("VShotTargetReachTimeKneeKickHighRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::skill_scoopShot:
	{
		float a = pTable->GetValue("VShotTargetReachTimeScoopShotHighA");
		float b = pTable->GetValue("VShotTargetReachTimeScoopShotHighB");
		float c = pTable->GetValue("VShotTargetReachTimeScoopShotRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::skill_rainbowShot:
	{
		float a = pTable->GetValue("VShotTargetReachTimeRainbowShotHighA");
		float b = pTable->GetValue("VShotTargetReachTimeRainbowShotHighB");
		float c = pTable->GetValue("VShotTargetReachTimeRainbowShotRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::skill_tripleCluch:
	{
		float a = pTable->GetValue("VShotTargetReachTimeTripleClutchHighA");
		float b = pTable->GetValue("VShotTargetReachTimeTripleClutchHighB");
		float c = pTable->GetValue("VShotTargetReachTimeTripleClutchHighRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::skill_floater:
	{
		float a = pTable->GetValue("VShotTargetReachTimeFloaterHighA");
		float b = pTable->GetValue("VShotTargetReachTimeFloaterHighB");
		float c = pTable->GetValue("VShotTargetReachTimeFloaterHighRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	case SKILL_INDEX::quickLayUp:
	{
		float a = pTable->GetValue("VShotTargetReachTimeQuickLayUpHighA");
		float b = pTable->GetValue("VShotTargetReachTimeQuickLayUpHighB");
		float c = pTable->GetValue("VShotTargetReachTimeQuickLayUpHighRandom");

		retValue = a * distance + b + Util::GetRandom(c);
	}
	break;
	default:
	{
		switch (pBallShot->shottype)
		{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			float a, b, c;

			switch (pBallShot->shotthrow)
			{
			case SHOT_THROW::low:
				a = pTable->GetValue("VShotTargetReachTimeDunkLowA");
				b = pTable->GetValue("VShotTargetReachTimeDunkLowB");
				break;
			case SHOT_THROW::middle:
				a = pTable->GetValue("VShotTargetReachTimeDunkMiddleA");
				b = pTable->GetValue("VShotTargetReachTimeDunkMiddleB");
				break;
			case SHOT_THROW::high:
				a = pTable->GetValue("VShotTargetReachTimeDunkHighA");
				b = pTable->GetValue("VShotTargetReachTimeDunkHighB");
				break;
			default:
				a = 0;
				b = 0;
				break;
			}

			c = pTable->GetValue("VShotTargetReachTimeDunkRandom");

			retValue = a * distance + b + Util::GetRandom(c);
		}
		break;
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			float a, b, c;

			switch (pBallShot->shotthrow)
			{
			case SHOT_THROW::low:
				a = pTable->GetValue("VShotTargetReachTimeLayUpLowA");
				b = pTable->GetValue("VShotTargetReachTimeLayUpLowB");
				break;
			case SHOT_THROW::middle:
				a = pTable->GetValue("VShotTargetReachTimeLayUpMiddleA");
				b = pTable->GetValue("VShotTargetReachTimeLayUpMiddleB");
				break;
			case SHOT_THROW::high:
				a = pTable->GetValue("VShotTargetReachTimeLayUpHighA");
				b = pTable->GetValue("VShotTargetReachTimeLayUpHighB");
				break;
			default:
				a = 0;
				b = 0;
				break;
			}

			c = pTable->GetValue("VShotTargetReachTimeLayUpRandom");

			retValue = a * distance + b + Util::GetRandom(c);
		}
		break;
		case SHOT_TYPE::shotType_closePostUnstready:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_post:
		case SHOT_TYPE::shotType_postUnstready:
		{
			float a, b, c;

			switch (pBallShot->shotthrow)
			{
			case SHOT_THROW::low:
				a = pTable->GetValue("VShotTargetReachTimePostLowA");
				b = pTable->GetValue("VShotTargetReachTimePostLowB");
				break;
			case SHOT_THROW::middle:
				a = pTable->GetValue("VShotTargetReachTimePostMiddleA");
				b = pTable->GetValue("VShotTargetReachTimePostMiddleB");
				break;
			case SHOT_THROW::high:
				a = pTable->GetValue("VShotTargetReachTimePostHighA");
				b = pTable->GetValue("VShotTargetReachTimePostHighB");
				break;
			default:
				a = 0;
				b = 0;
				break;
			}

			c = pTable->GetValue("VShotTargetReachTimePostRandom");

			retValue = a * distance + b + Util::GetRandom(c);
		}
		break;
		default:
		{
			float a, b, c;

			switch (pBallShot->shotthrow)
			{
			case SHOT_THROW::low:
				a = pTable->GetValue("VShotTargetReachTimeJumpLowA");
				b = pTable->GetValue("VShotTargetReachTimeJumpLowB");
				break;
			case SHOT_THROW::middle:
				a = pTable->GetValue("VShotTargetReachTimeJumpMiddleA");
				b = pTable->GetValue("VShotTargetReachTimeJumpMiddleB");
				break;
			case SHOT_THROW::high:
				a = pTable->GetValue("VShotTargetReachTimeJumpHighA");
				b = pTable->GetValue("VShotTargetReachTimeJumpHighB");
				break;
			default:
				a = 0;
				b = 0;
				break;
			}

			c = pTable->GetValue("VShotTargetReachTimeJumpRandom");

			retValue = a * distance + b + Util::GetRandom(c);
		}
		break;
		}
	}
	break;
	}


	return retValue;
}

float CState::GetYaw(float directionX, float directionZ)
{
	return atan2(directionX, directionZ) * 57.29578f;
}

bool CState::IsPointInLine(SVector3 start, SVector3 end, SVector3 target, float distance)
{
	bool retValue = false;

	if (end.x() != start.x())
	{
		// y = ax + b
		float a1 = (end.z() - start.z()) / (end.x() - start.x());
		float b1 = start.z() - a1 * start.x();

		float a2 = -1 / a1;
		float b2 = target.z() - a2 * target.x();

		float crossX = (b2 - b1) / (a1 - a2);
		float crossZ = a2 * crossX + b2;

		SVector3 cross;
		
		cross.mutate_x(crossX);
		cross.mutate_y(0.0f);
		cross.mutate_z(crossZ);

		float distanceStartCross = GetDistance(start, cross);
		float distanceEndCross = GetDistance(end, cross);
		float distanceStartEnd = GetDistance(start, end);

		if (distanceStartCross < distanceStartEnd && distanceEndCross < distanceStartEnd && GetDistance(target, cross) < distance)
		{
			retValue = true;
		}
	}

	return retValue;
}

SVector3* CState::CopyVector(SVector3 src)
{
	SVector3* pVector = new SVector3();

	pVector->mutate_x(src.x());
	pVector->mutate_y(src.y());
	pVector->mutate_z(src.z());

	return pVector;
}

SVector3* CState::NewVector(float x, float y, float z)
{
	SVector3* pVector = new SVector3();

	pVector->mutate_x(x);
	pVector->mutate_y(y);
	pVector->mutate_z(z);

	return pVector;
}

void CState::PushSendRecordInfo(DHOST_TYPE_CHARACTER_SN characterSN, ECHARACTER_RECORD_TYPE type)
{
	//! ���������ʹ� ��� ������ ���� �ʴ´� (�߱��� ��û) 20240325 - by thinkingpig
	map<DHOST_TYPE_CHARACTER_SN, ECHARACTER_RECORD_TYPE> sendInfo;
	sendInfo.insert(pair<DHOST_TYPE_CHARACTER_SN, ECHARACTER_RECORD_TYPE>(characterSN, type));

	m_SendRecordInfo.push_back(sendInfo);
}

void CState::SendRecordData()
{
	SendRecordInfoDeque::iterator iter;

	for (iter = m_SendRecordInfo.begin(); iter != m_SendRecordInfo.end(); ++iter)
	{
		auto mapInfo = *iter->begin();

		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(mapInfo.first);

		if (pCharacter != nullptr)
		{
			CREATE_BUILDER(builder)
			auto offset_string_name = builder.CreateString(pCharacter->name);
			CREATE_FBPACKET(builder,play_s2c_gameRecord,message,send_data)
			send_data.add_userid(pCharacter->userid);
			send_data.add_characterid(pCharacter->characterid);
			send_data.add_id(pCharacter->id);
			send_data.add_name(offset_string_name);
			send_data.add_recordtype(m_pHost->ConvertToProtobufRecordType(mapInfo.second));
			send_data.add_recordcount(m_pHost->GetCharacterRecordCountWithType(pCharacter->id, (F4PACKET::RECORD_TYPE)m_pHost->ConvertToProtobufRecordType(mapInfo.second)));
			STORE_FBPACKET(builder, message, send_data);
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}
	}
	m_SendRecordInfo.clear();
}

void CState::SendRecordData(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE recordType)
{
	//! ���������ʹ� ��� ������ ���� �ʴ´� (�߱��� ��û) 20240325 - by thinkingpig
	F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(characterSN);

	if (pCharacter != nullptr)
	{
		if (recordType == RECORD_TYPE::Assist ||
			recordType == RECORD_TYPE::Block ||
			recordType == RECORD_TYPE::LooseBall ||
			recordType == RECORD_TYPE::PointThree ||
			recordType == RECORD_TYPE::PointTwo ||
			recordType == RECORD_TYPE::Rebound ||
			recordType == RECORD_TYPE::Steal ||
			recordType == RECORD_TYPE::TurnOver)
		{

			CREATE_BUILDER(builder);
			auto offset_string_name = builder.CreateString(pCharacter->name);
			CREATE_FBPACKET(builder, play_s2c_gameRecord, message, send_data);
			send_data.add_userid(pCharacter->userid);
			send_data.add_characterid(pCharacter->characterid);
			send_data.add_id(characterSN);
			send_data.add_name(offset_string_name);
			send_data.add_recordtype(recordType);
			send_data.add_recordcount(m_pHost->GetCharacterRecordCountWithType(characterSN, recordType));
			STORE_FBPACKET(builder, message, send_data)
				m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
		}
	}
}

// cooltimmer
CCoolTimer::CCoolTimer(CHost* host) : m_pHost(host)
{
}

CCoolTimer::~CCoolTimer() {}

void CCoolTimer::bookMark(TYPE type)
{
	if (datas.count(type) > 0)
		datas[type] = m_pHost->GetRoomElapsedTime();
	else
		datas.insert(std::make_pair(type, m_pHost->GetRoomElapsedTime()));
}

bool CCoolTimer::withinTime(TYPE type, float time)
{
	if (datas.count(type) <= 0)
		return false;

	return (datas[type] + time) >= m_pHost->GetRoomElapsedTime();
}

CState* CProcessPacket::m_pState = nullptr;
CHost* CProcessPacket::m_pHost = nullptr;

CState::CState(CHost* host) : m_pHost(host)
{
	m_coolTimer = new CCoolTimer(host);
	m_SendRecordInfo.clear();

	CProcessPacket::m_pState = this;
	CProcessPacket::m_pHost = host;

	curTimeDelta = 0.0f;
}

CState::~CState()
{
	if (m_coolTimer)
	{
		delete(m_coolTimer);
	}

	m_coolTimer = nullptr;
}

void CState::OnEnter(void* pData)
{

}

void CState::OnExit()
{

}

void CState::OnUpdate(float timeDelta)
{
	curTimeDelta = timeDelta;
		
	if (m_pHost->CheckOption(HOST_OPTION_NO_AUTO_AI) == false)
	{
		m_pHost->CheckExpiredRoom();
	}

	ProcessReboundBallSimulationPackets();
}

void CState::OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ)
{

}


DHOST_TYPE_BOOL CState::OnPacket(CPacketBase* pPacket, const char* pData, int size, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (nullptr == pPacket)
		return false;

	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();
	PACKET_ID packetID = (PACKET_ID)pPacket->GetPacketID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		// StatePlay�� ������ ������ �͵� , �� �Լ��� �����Լ��� �Ǿ� �־�, StatePlay ���� ���� �Ǿ� �����Ƿ� ������ ������Ʈ���� ����� ���´�. 
		//CASE_FB_CALL(play_c2s_ballRebound)
		CASE_FB_CALL_EXTEND(play_c2s_ballRebound)

	default:
		return false;
	}
}

// on packet

DHOST_TYPE_BOOL CState::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (nullptr == pPacket)
		return false;

	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();
	PACKET_ID packetID = (PACKET_ID)pPacket->GetPacketID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(system_c2c_loadingPercentage);
		CASE_FB_CALL(system_c2s_login);
		CASE_FB_CALL(system_c2s_readyToLoad);
		CASE_FB_CALL(system_c2s_loadComplete);

		CASE_FB_CALL(system_c2s_chat);
		CASE_FB_CALL(system_c2c_ping);
		CASE_FB_CALL(system_c2s_devGameTime); 
		CASE_FB_CALL(system_c2s_exitGame);
		CASE_FB_CALL(system_c2s_awayFromKeyboard);
		CASE_FB_CALL(system_c2s_useJoystick);
		CASE_FB_CALL(system_c2s_surrenderVote);
		CASE_FB_CALL(system_c2s_versionInfo);

		CASE_FB_CALL(play_c2s_ballClear)
		CASE_FB_CALL(play_c2s_ballLerp)
		CASE_FB_CALL(play_c2s_ballShot)
		CASE_FB_CALL(play_c2s_ballSimulation)

		//CASE_FB_CALL(play_c2s_ballRebound) ���⼭�� ���� 

		CASE_FB_CALL(play_c2s_environmentCat)
		CASE_FB_CALL(play_c2s_onFireModeInfo)

		CASE_FB_CALL(play_c2s_playerAlleyOopCut)
		CASE_FB_CALL(play_c2s_playerAlleyOopPass)
		CASE_FB_CALL(play_c2s_playerAlleyOopShot)
		CASE_FB_CALL(play_c2s_playerBlock)
		CASE_FB_CALL(play_c2s_playerBuff)
		CASE_FB_CALL(play_c2s_playerCatchAndShotMove)
		CASE_FB_CALL(play_c2s_playerInYourFace)
		CASE_FB_CALL(play_c2s_playerCeremony)
		CASE_FB_CALL(play_c2s_playerHit)
		CASE_FB_CALL(play_c2s_playerCloseOut)
		CASE_FB_CALL(play_c2s_playerChaseContest)
		CASE_FB_CALL(play_c2s_playerGoAndCatch)
		CASE_FB_CALL(play_c2s_playerCollision)
		CASE_FB_CALL(play_c2s_playerCrossOver)
		CASE_FB_CALL(play_c2s_playerCutIn)
		CASE_FB_CALL(play_c2s_playerDenyDefense)
		CASE_FB_CALL(play_c2s_playerFakeShot)
		CASE_FB_CALL(play_c2s_playerHandCheck)
		CASE_FB_CALL(play_c2s_playerHookHook)
		CASE_FB_CALL(play_c2s_playerHopStep)
		CASE_FB_CALL(play_c2s_playerIntercept)
		CASE_FB_CALL(play_c2s_playerJumpBallTapOut)
		CASE_FB_CALL(play_c2s_playerModificationAbility);
		CASE_FB_CALL(play_c2s_playerMove)
		CASE_FB_CALL(play_c2s_playerReceivePass)
		CASE_FB_CALL(play_c2s_playerPass)
		CASE_FB_CALL(play_c2s_playerPassive)
		CASE_FB_CALL(play_c2s_playerPenetrate)
		CASE_FB_CALL(play_c2s_playerPenetratePostUp)
		CASE_FB_CALL(play_c2s_playerPenetrateReady)
		CASE_FB_CALL(play_c2s_playerPick) 
		CASE_FB_CALL(play_c2s_playerPickAndMove)
		CASE_FB_CALL(play_c2s_playerPickAndSlip)
		CASE_FB_CALL(play_c2s_playerPositionCorrect)
		CASE_FB_CALL(play_c2s_playerSyncInfo)
		CASE_FB_CALL(play_c2s_playerPostUpCollision)
		CASE_FB_CALL(play_c2s_playerPostUpPenetrate)
		CASE_FB_CALL(play_c2s_playerPostUpReady)
		CASE_FB_CALL(play_c2s_playerPostUpStepBack)
		CASE_FB_CALL(play_c2s_playerRebound)
		CASE_FB_CALL(play_c2s_playerScreen) 
		CASE_FB_CALL(play_c2s_playerShakeAndBake)
		CASE_FB_CALL(play_c2s_playerShot)
		CASE_FB_CALL(play_c2s_playerSlipAndSlide)
		CASE_FB_CALL(play_c2s_playerSlideStep) 
		CASE_FB_CALL(play_c2s_playerShadowFaceupDash)
		CASE_FB_CALL(play_c2s_playerSpeaking)
		CASE_FB_CALL(play_c2s_playerSpinMove) 
		CASE_FB_CALL(play_c2s_playerStand) 
		CASE_FB_CALL(play_c2s_playerSteal) 
		CASE_FB_CALL(play_c2s_playerStealFail) 
		CASE_FB_CALL(play_c2s_playerStun) 
		CASE_FB_CALL(play_c2s_playerSwitchActive)
		CASE_FB_CALL(play_c2s_playerSwitchRequest) 
		CASE_FB_CALL(play_c2s_playerSwitchResponse)
		CASE_FB_CALL(play_c2s_playerBurstRequest)
		CASE_FB_CALL(play_c2s_playerDoubleClutch)
		CASE_FB_CALL(play_c2s_playerTapOut)
		CASE_FB_CALL(play_c2s_playerTapPass) 
		CASE_FB_CALL(play_c2s_playerTipIn) 
		CASE_FB_CALL(play_c2s_playerVCut)
		CASE_FB_CALL(play_c2s_playerGiveAndGoMove)
		CASE_FB_CALL(play_c2s_playerOneDribblePullUpJumperMove)
		CASE_FB_CALL(play_c2s_playerEscapeDribble)
		CASE_FB_CALL(play_c2s_playerShotInterfere)
		CASE_FB_CALL(play_c2s_fxDisplayPotential)
		CASE_FB_CALL(play_c2s_playerShammgod)
		CASE_FB_CALL(play_c2s_playerStunIntercept)
		CASE_FB_CALL(play_c2s_playerDash)
		CASE_FB_CALL(play_c2s_playerPassiveDataSet);
		CASE_FB_CALL(play_c2s_playerBehindStepBackJumperShot);
		CASE_FB_CALL(play_c2s_playerIllegalScreen)
		CASE_FB_CALL(play_c2s_uiDisplayMedal) 
		CASE_FB_CALL(play_c2s_playerEmoji)
		CASE_FB_CALL(play_c2s_greatDefense)
		CASE_FB_CALL(system_s2s_clientAliveCheck)
		CASE_FB_CALL(system_s2s_systemTimeCheck)
		CASE_FB_CALL(system_c2s_test)
		CASE_FB_CALL(system_c2s_blackBox)
		CASE_FB_CALL(system_c2s_wkawofur)
		CASE_FB_CALL(system_c2s_skinChange)
	default:
		return false;
	}
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2c_loadingPercentage(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2c_loadingPercentage_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_c2c_loadingPercentage, message, send_data);
	send_data.add_userid(userid);
	send_data.add_percentage(data->percentage());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_login(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_login_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	F4PACKET::RESULT result = F4PACKET::RESULT::fail;
	F4PACKET::LOGIN_TYPE type = F4PACKET::LOGIN_TYPE::none;
	F4PACKET::EXIT_TYPE exitType = m_pHost->GetConvertExitType();

	auto& HostUserMap = m_pHost->GetHostUserMap();

	if (HostUserMap.empty() == false)
	{
		auto iter = HostUserMap.find(data->userid());

		if (iter != HostUserMap.end())
		{
			result = F4PACKET::RESULT::success;
			type = F4PACKET::LOGIN_TYPE::reconnect;

			iter->second->SetLogin(true);

			// ȣ��Ʈ�� PLAY ���·� �ٲ������ �������ϴ� ������ �ִٸ�
			m_pHost->ReconnectUserOperationBeforeLoading(userid);
		}
	}

	m_pHost->PushLoginCompleteUser(userid);

	string log_message = "[RECONNECT] LOGIN RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", RemainTime : " + std::to_string(m_pHost->GetGameTime());
	m_pHost->ToLog(log_message.c_str());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_login, message, send_data);
	send_data.add_result(result);
	send_data.add_logintype(type);
	send_data.add_exittype(exitType);
	send_data.add_remaingametime(m_pHost->GetGameTime());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, data->userid());

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_readyToLoad(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_readyToLoad_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	string log_message = "[RECONNECT] READY_TO_LOAD RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
	m_pHost->ToLog(log_message.c_str());

	{
		std::vector<F4PACKET::SPlayerInformationT*> vecCharacters;
		vecCharacters.clear();

		m_pHost->GetCharacterInformationVec(vecCharacters);

		CREATE_BUILDER(builder)
		std::vector<flatbuffers::Offset<F4PACKET::SPlayerInformation>> vecchars;

		for (int i = 0; i < vecCharacters.size(); ++i)
		{
			auto packedcharecter = F4PACKET::SPlayerInformation::Pack(builder, vecCharacters[i]);

			vecchars.push_back(packedcharecter);
		}

		auto offvecchars = builder.CreateVector(vecchars);

		CREATE_FBPACKET(builder, system_s2c_playerInfo, message, databuilder)
		databuilder.add_playerlist(offvecchars);
		STORE_FBPACKET(builder, message, databuilder)

		m_pHost->SendPacket(message, data->userid());
	}

	{
		auto stage = m_pHost->GetStageID();

		F4PACKET::EGAME_MODE game_mode = m_pHost->GetConvertGameMode();
		
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_startLoad, message, send_data);
		send_data.add_result(F4PACKET::RESULT::success);
		send_data.add_stageid(stage);
		send_data.add_gamemode(game_mode);
		send_data.add_isnormalgame(m_pHost->GetIsNormalGame());
		send_data.add_switchtoailevel(m_pHost->GetSwitchToAiLevel());
		send_data.add_handicaprulepoint(m_pHost->GetGameManager()->GetHandicapRulePoint());
		send_data.add_treatdelayoption(m_pHost->bTreatDelayMode);
		send_data.add_serverpossync(m_pHost->bServerSyncPos);

		STORE_FBPACKET(builder, message, send_data)

		m_pHost->SendPacket(message, data->userid());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_loadComplete(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_loadComplete_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	string log_message = "[RECONNECT] LOAD_COMPLETE RoomElapsedTime : " + std::to_string(recv_time) + ", UserID : " + std::to_string(userid);
	m_pHost->ToLog(log_message.c_str());

	// �����ӽÿ� Ŭ�� �ε尡 ���� ������ WAIT ���·� �������ش�.
	CHostUserInfo* pUser = m_pHost->FindUser(userid);
	if (pUser != nullptr)
	{
		pUser->SetConnectState(ECONNECT_STATE::WAIT);
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_reconnectEnter, message, send_data);
	send_data.add_result(F4PACKET::RESULT::success);
	send_data.add_timegame(static_cast<int>(m_pHost->GetGameTime()));
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, data->userid());
	
	return true;
}

vector<string> split(string input, char delimiter) 
{
	vector<string> answer;
	stringstream ss(input);
	string temp;

	while (getline(ss, temp, delimiter)) {
		answer.push_back(temp);
	}

	return answer;
}

bool CState::Cheat(int id, const flatbuffers::String* context)
{
	string str = context->str();

	vector<string> words = split(str, ' ');

	if (words.size() > 0 && words[0] == "!" && m_pHost->GetDebug())
	{
//#ifdef _DEBUG
		try
		{
			auto data = words.at(1);
		}
		catch (std::exception& e)
		{
			m_pHost->ToLog(e.what());

			string message = "Invalid cheat message";
			CREATE_BUILDER(builder);
			CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
			return false;
		}

		if (words[1].c_str() == nullptr)
		{
			string message = "Invalid cheat message";
			CREATE_BUILDER(builder);
			CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
			return false;
		}

		if (strcmp(words[1].c_str(), "burst") == 0) // ����Ʈ ������ ���� �� 
		{
			//m_pHost->SetGameTime(3.0f);

			{
				//string message = "show me the burst guage.";
				m_pHost->SendCharacterBurstGauge();

			}

			return true;
		}

		if (strcmp(words[1].c_str(), "sd") == 0)
		{
			m_pHost->SetGameTime(3.0f);

			{
				string message = "Game shut down.";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
			}

			return true;
		}

		if (strcmp(words[1].c_str(), "st") == 0 && words.size() >= 3)
		{
			float time_seconds = 10.0f;

			try
			{
				time_seconds = std::stoi(words[2]);
			}
			catch (std::exception& e)
			{
				m_pHost->ToLog(e.what());

				string message = "Invalid cheat message";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
				return false;
			}

			m_pHost->SetGameTime(time_seconds);

			{
				string message = "set game time : " + std::to_string(time_seconds);
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
			}

			return true;
		}

		if (strcmp(words[1].c_str(), "kick") == 0)
		{
			{
				string message = "kick byebye!";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
			}

			DHOST_TYPE_USER_ID UserID = m_pHost->FindUserIDWithCharacterSN(id);

			m_pHost->KickToTheLobby(UserID, F4PACKET::EKICK_TYPE::cheat_kick);

			return true;
		}
		else if (strcmp(words[1].c_str(), "score") == 0 && words.size() >= 4)
		{
			int team = kINT32_INIT;
			int score = kINT32_INIT;

			try
			{
				team = std::stoi(words[2]);
				score = std::stoi(words[3]);
			}
			catch (std::exception& e)
			{
				m_pHost->ToLog(e.what());

				string message = "Invalid cheat message";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
				return false;
			}

			if (m_pHost->BallControllerGet()->GameBallDataGet() == nullptr)
			{
				string message = "touch ball!!";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
				return false;
			}

			if (m_pHost->GetTeamCharacterMap(team) == false)
			{
				string message = "[Cheat Fail] Invalid team index";
				CREATE_BUILDER(builder);
				CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());

				return false;
			}

			m_pHost->SetOffenseTeam(team);
			m_pHost->SetScore(team, score);


			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, system_s2c_goal, message, send_data);
			send_data.add_scorerid(id);
			send_data.add_point(0);
			send_data.add_scoreteam((DHOST_TYPE_INT32)team);
			send_data.add_shottype(F4PACKET::SHOT_TYPE::shotType_none);
			send_data.add_remaintime(m_pHost->GetGameTime());
			STORE_FBPACKET(builder, message, send_data)

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

			m_pHost->ChangeState(EHOST_STATE::SCORE, &id);

			return true;
		}
		else if (strcmp(words[1].c_str(), "rs") == 0 && words.size() >= 4)
		{
			int index = std::stoi(words[2]);
			int level = std::stoi(words[3]);

			CCharacter* character = m_pHost->GetCharacterManager()->GetCharacter(id);

			if (character != nullptr)
			{
				F4PACKET::SPlayerInformationT* info = character->GetCharacterInformation();

				bool exist = false;

				for (int i = 0; i < info->skills.size(); i++)
				{
					if ((int)info->skills[i].index() == index)
					{
						exist = true;
					}
				}

				if (exist == false)
				{
					F4PACKET::SSkillInfo skillInfo;
					skillInfo.mutate_index((F4PACKET::SKILL_INDEX)index);
					skillInfo.mutate_level(level);

					info->skills.push_back(skillInfo);

					{
						CREATE_BUILDER(builder)
						CREATE_FBPACKET(builder, system_s2c_devRegistSkill, message, send_data);
						send_data.add_id(id);
						send_data.add_index(index);
						send_data.add_level(level);
						send_data.add_regist(true);
						STORE_FBPACKET(builder, message, send_data)

						m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
					}

					{
						string message = "[Cheat Success] Skill is registered. ";

						message += words[2].c_str();

						CREATE_BUILDER(builder);
						CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
					}

					{

						string message = "Registered skill list : ";

						for (int i = 0; i < info->skills.size(); i++)
						{
							char buffer[16] = { 0 };
							snprintf(buffer, 10, "%d", (int)info->skills[i].index());

							message += buffer;
							message += " ";
						}

						CREATE_BUILDER(builder);
						CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
					}
				}
				else
				{
					string message = "[Cheat Fail] You already have the skills. ";

					message += words[2].c_str();

					CREATE_BUILDER(builder);
					CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
				}

				return true;
			}
		}
		else if (strcmp(words[1].c_str(), "us") == 0 && words.size() >= 3)
		{
			CCharacter* character = m_pHost->GetCharacterManager()->GetCharacter(id);

			if (character != nullptr)
			{
				F4PACKET::SPlayerInformationT* info = character->GetCharacterInformation();

				if (strcmp(words[2].c_str(), "all") == 0)
				{
					for (std::vector<F4PACKET::SSkillInfo>::iterator iter = info->skills.begin(); iter != info->skills.end(); iter++)
					{
						CREATE_BUILDER(builder)
						CREATE_FBPACKET(builder, system_s2c_devRegistSkill, message, send_data);
						send_data.add_id(id);
						send_data.add_index((int)iter->index());
						send_data.add_level(iter->level());
						send_data.add_regist(false);
						STORE_FBPACKET(builder, message, send_data)

						m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
					}

					info->skills.clear();

					{
						string message = "[Cheat Success] All skill is unregistered.";

						CREATE_BUILDER(builder);
						CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
					}
				}
				else
				{
					int index = std::stoi(words[2]);

					for (std::vector<F4PACKET::SSkillInfo>::iterator iter = info->skills.begin(); iter != info->skills.end(); iter++)
					{
						if ((int)iter->index() == index)
						{
							CREATE_BUILDER(builder)
							CREATE_FBPACKET(builder, system_s2c_devRegistSkill, message, send_data);
							send_data.add_id(id);
							send_data.add_index((int)iter->index());
							send_data.add_level(iter->level());
							send_data.add_regist(false);
							STORE_FBPACKET(builder, message, send_data)

							m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

							info->skills.erase(iter);

							{
								string message = "[Cheat Success] Skill is unregistered. ";

								message += words[2].c_str();

								CREATE_BUILDER(builder);
								CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
							}

							{
								string message = "Registered skill list : ";

								for (int i = 0; i < info->skills.size(); i++)
								{
									char buffer[8] = { 0 };
									snprintf(buffer, 10, "%d", (int)info->skills[i].index());

									message += buffer;
									message += " ";
								}

								CREATE_BUILDER(builder);
								CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
							}

							return true;
						}
					}


					{
						string message = "[Cheat Fail] You don't have skills. ";

						message += words[2].c_str();

						CREATE_BUILDER(builder);
						CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, id, message.c_str());
					}
				}
			}

			return true;
		}
//#endif
	}

	return false;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_chat(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_chat_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->context() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER context is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	string str_log = "[CHAT] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", Chat :  " + data->context()->str();
	m_pHost->ToLog(str_log.c_str());

	if (data->chattype() == F4PACKET::CHAT_TYPE::macro)
	{
		m_pHost->IncreaseUseQuickChatCount(data->characterid());
	}

	if(Cheat(data->characterid(), data->context()) == false)
	{
		CREATE_BUILDER(builder)
		CREATE_AND_BROADCAST_FBPACKET_DIRECT(builder, system_s2c_chat, data->characterid(), data->context()->c_str(), data->chattype());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2c_ping(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2c_ping_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! 
	//! 
	
	CHostUserInfo* pUser = m_pHost->FindUser(userid);
	if (pUser)
	{
		pUser->SetCurFPS(data->fps());
	}
		
	// ������ ��Ʈ�� �����̸� ���ϱ� ���ؼ� ������ ������Ʈ �ð��� ������ 
	DHOST_TYPE_INT32 delta = (int)(curTimeDelta * 1000);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_c2c_ping, message, send_data);
	send_data.add_time(data->time()); 
	send_data.add_serverupdatetime(delta);
	send_data.add_fps(data->fps());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_devGameTime(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_devGameTime_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->gametime() >= 0.0f && data->gametime() < 300.0f)
	{
		m_pHost->SetGameTime(data->gametime());

		string str = "[SYSTEM_C2S_DEV_GAME_TIME] RoomID : " + m_pHost->GetHostID();
		m_pHost->ToLog(str.c_str());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_exitGame(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_exitGame_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (m_pHost->UserCount() == 1)
	{
		string log_message = "system_c2s_exitGame_single RoomElapsedTime : " + std::to_string(recv_time) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(log_message.c_str());

		m_pHost->SetAbnormalExpireRoom(true);

		if (m_pHost->CheckSkillChallengeMode())
		{
			m_pHost->ChangeState(EHOST_STATE::CHALLENGE_END, nullptr);
		}
		else
		{
			m_pHost->ChangeState(EHOST_STATE::END);
		}
	}
	else
	{
		string log_message = "system_c2s_exitGame RoomElapsedTime : " + std::to_string(recv_time) 
			+ " RoomID : " + m_pHost->GetHostID()
			+ ", UserID : " + std::to_string(userid);
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_awayFromKeyboard(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_awayFromKeyboard_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->result() == true) // ������ 
	{
		string str = "ComebackKeyboard RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid) + ", CharacterID : " + std::to_string(data->characterid());
		m_pHost->ToLog(str.c_str());

		m_pHost->ComebackAwayFromKeyboard(m_pHost->GetGameTime(), userid, data->characterid());
	}
	else // AI �� 
	{
		m_pHost->AwayFromKeyboard(userid, data->characterid());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_useJoystick(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_useJoystick_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->result() == true)
	{
		m_pHost->UseJoystick(data->characterid());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_surrenderVote(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::system_c2s_surrenderVote_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (m_pHost->GetTeamSurrenderActivation(data->characterid()))
	{
		std::string str_log = "system_c2s_surrenderVote RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", CharacterID : " + std::to_string(data->characterid()) + ", Vote : " + std::to_string(data->vote());
		m_pHost->ToLog(str_log.c_str());

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, system_s2c_surrenderVote, message, send_data);
		send_data.add_characterid(data->characterid());
		send_data.add_vote(data->vote());
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		m_pHost->AddTeamSurrenderVote(data->characterid(), data->vote());

		// ������ �θ��̻� �ߴٸ�
		if (m_pHost->GetTeamSurrenderVoteCount(data->characterid(), true) >= 2)
		{
			m_pHost->AddTeamSurrenderGameEndTime(data->characterid(), m_pHost->GetGameTime());
		}
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_ballClear(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballClear_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->position() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER position is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballClear, message, send_data);
	send_data.add_ballcleared(data->ballcleared());
	send_data.add_position(data->position());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_ballLerp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballLerp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->balllerp() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER balllerp is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	F4PACKET::play_c2s_ballLerp_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballLerp_dataT* pBallLerpData = &dataT;

	if (pBallLerpData == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallLerpData is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	
	if (m_pHost->BallNumberGet() <= pBallLerpData->balllerp->ballnumber())
	{
		int ballNumber = pBallLerpData->balllerp->ballnumber() + 1;

		m_pHost->BallNumberSet(ballNumber);

		pBallLerpData->balllerp->mutate_ballnumber(ballNumber);

		pBallLerpData->balllerp->mutate_success(true);

		m_pHost->SetBallOwner(pBallLerpData->balllerp->owner());

		if (pBallLerpData->balllerp->passtype() == F4PACKET::PASS_TYPE::overBad || pBallLerpData->balllerp->passtype() == F4PACKET::PASS_TYPE::overCommon || pBallLerpData->balllerp->passtype() == F4PACKET::PASS_TYPE::overGood)
		{
			m_pHost->UpdateRecord(pBallLerpData->balllerp->owner(), m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::OVER_PASS);
		}
		
		SBallActionInfo sInfo;
		sInfo.BallNumber = m_pHost->BallNumberGet();
		sInfo.BallOwner = m_pHost->GetBallOwner();
		sInfo.BallOwnerTeam = m_pHost->ConvertToStringTeam(m_pHost->GetCharacterTeam(sInfo.BallOwner));
		sInfo.Ballstate = (EBALL_STATE)m_pHost->ConvertToEnumBallState(pBallLerpData->balllerp->ballstate());
		sInfo.bAssist = false;
		sInfo.fGameTime = m_pHost->GetGameTime();
		sInfo.SkillIndex = (DHOST_TYPE_UINT32)pBallLerpData->balllerp->skillindex();

		m_pHost->PushBallActionInfoDeque(sInfo);

		//! ���̺�ĳġ�н��� ��� �н��� �̷������ �ʾƵ� ��ų�� ��ĳ�н��� ���� ������ Ÿ���� �ִ��� Ȯ���ؾ� �Ѵ�.
		if (pBallLerpData->balllerp->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatchPass)
		{
			m_pHost->IncreaseSkillSucCount(pBallLerpData->balllerp->owner(), (DHOST_TYPE_UINT32)SKILL_INDEX::skill_divingCatchPass);
		}

		DHOST_TYPE_INT32 fxLevel = GetPotentialForPass(pBallLerpData->balllerp->owner(), pBallLerpData->balllerp->passtype(), pBallLerpData->balllerp->skillindex(), userid);
		pBallLerpData->balllerp->mutate_potentialfxlevelcreativepass(fxLevel);
	}
	else
	{
		string str = "[play_c2s_ballLerp] DIFF_BALL_NO RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", HostBallNumber : " + std::to_string(m_pHost->BallNumberGet()) + ", PakcetBallNumber : " + std::to_string(pBallLerpData->balllerp->ballnumber()) + ", Owner : " + std::to_string(pBallLerpData->balllerp->owner()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime());
		m_pHost->ToLog(str.c_str());

		pBallLerpData->balllerp->mutate_success(false);
	}

	pBallLerpData->balllerp->mutate_ballnumber(m_pHost->BallNumberGet());

	if (pBallLerpData->balllerp->success() == true)
	{
		CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

		pGameBallData->ballNumber = m_pHost->BallNumberGet();
		pGameBallData->ballState = data->balllerp()->ballstate();
		pGameBallData->shotType = SHOT_TYPE::shotType_none;
		pGameBallData->owner = pBallLerpData->balllerp->owner();
		pGameBallData->point = 0;
		pGameBallData->team = m_pHost->GetCharacterTeam(pBallLerpData->balllerp->owner());

		pGameBallData->ballEventMap.insert(pair<string, float>("Event_ballSimulation", pBallLerpData->balllerp->time()));
		m_pHost->BallControllerGet()->AddBallData(pGameBallData, pBallLerpData->balllerp->time());
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballLerp, message, send_data);
	send_data.add_balllerp(pBallLerpData->balllerp.get());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_ballShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (!CPlay_c2s_common::CheckNullPtr(m_pHost, pPacket, userid))
		return false;

	if (!CPlay_c2s_ballShot::CheckNullPtr(m_pHost, pPacket, userid))
		return false;

	

    auto* data = pPacket->GetData();
	

	SBallShotT ballShot;

	data->ballshot()->UnPackTo(&ballShot);

	SBallShotT* pBallShot = &ballShot;

	DHOST_TYPE_BOOL bAssist = false;

	ECHARACTER_RECORD_TYPE recordType = ECHARACTER_RECORD_TYPE::NONE;

	string shot_rate_log = "";

	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! �� �������� Ŭ�� �˷��� �� �������� ���� ��ߵ� (1��, ������ ����, �ñ״���)
	DHOST_TYPE_FLOAT decreaseShotSuccessRate = kFLOAT_INIT;

	bool clientBallShotSucces = pBallShot->success; // by steven 2024-01-14, Ŭ���̾�Ʈ�� ��ü �Ǵ��ϴ� ��������( ���̱�, ���� �������� �߰���, False �� ������ ���¡�� �����Ƿ� ���� ó���Ѵ�

	if (m_pHost->BallNumberGet() <= pBallShot->ballnumber && pBallShot->shottype != SHOT_TYPE::shotType_none)
	{
		// �� �α�
		{
			string str = "[BALL_SHOT] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid)
				+ ", BallNo : " + std::to_string(pBallShot->ballnumber) + ", Owner : " + std::to_string(pBallShot->owner)
				+ ", OwnerPosX : " + std::to_string(pBallShot->positionowner->x()) + ", OwnerPosZ : " + std::to_string(pBallShot->positionowner->z())
				//+ ", BallPosX : " + std::to_string(pBallShot->positionball->x()) + ", BallPosZ : " + std::to_string(pBallShot->positionball->z())
				+ ", WideOpen : " + std::to_string(pBallShot->wideopen) + ", Bonus : " + std::to_string(pBallShot->bonus)
				+ ", bonuspassive : " + std::to_string(pBallShot->bonuspassive) + ", bonussignature : " + std::to_string(pBallShot->bonussignature)
				+ ", bonusnicepass : " + std::to_string(pBallShot->bonusnicepass) + ", bonusanklebreakesuccess : " + std::to_string(pBallShot->bonusanklebreakesuccess)
				+ ", signaturekey : " + std::to_string(pBallShot->signaturekey)
				+ ", skillindex : " + std::to_string((int)pBallShot->skillindex) + ", successrate : " + std::to_string(pBallShot->successrate)
				+ ", clientBallShotSucces : " + std::to_string(clientBallShotSucces) + ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pBallShot->shottype);

			m_pHost->ToLog(str.c_str());
		}

		pBallShot->success = true; // Ŭ���̾�Ʈ���� True�� ������� ���� ó���� 

		m_pHost->BallNumberSet(pBallShot->ballnumber + 1);

		m_pHost->SetBallOwner(pBallShot->owner);

		//! 20210415 replay - by thinkingpig
		{
			ACTION_TYPE action_type = ConvertShotTypeToActionType(pBallShot->shottype);

			m_pHost->SetCharacterActionOverlap(pBallShot->owner, action_type, pBallShot->skillindex);
		}
		//! end

		CPhysicsHandler* pPhysics = m_pHost->GetPhysicsHandler();

		if (pPhysics != nullptr)
		{
			CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pBallShot->owner);
			SCharacterAction* pCharacterActionInfo = m_pHost->GetCharacterActionInfo(pBallShot->owner);
			if (pCharacter != nullptr && pCharacterActionInfo != nullptr)
			{
				F4PACKET::SPlayerInformationT* pCharacterInformation = pCharacter->GetCharacterInformation();
				if (pCharacterInformation != nullptr)
				{
					//! ���� ���� ���� ����
					DHOST_TYPE_BOOL bHindrancesCheck = true;

					//! ��ų �� �޴� üũ
					DHOST_TYPE_BOOL bSkillCheck = true;


					if (pBallShot->skillindex != SKILL_INDEX::skill_none)
					{
						bSkillCheck = m_pHost->CheckHaveSkill(pCharacterInformation->id, pBallShot->skillindex);
					}

					if (bSkillCheck == false)
					{
						string bSkillCheckStr = "[HACK_CHECK] DOES_NOT_HAVE_SKILL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
							+ ", CharacterID : " + std::to_string(pCharacterInformation->characterid) + ", Skill : " + F4PACKET::EnumNameSKILL_INDEX((F4PACKET::SKILL_INDEX)pBallShot->skillindex)
							+ ", bSkillCheck : " + std::to_string(bSkillCheck);
						m_pHost->ToLog(bSkillCheckStr.c_str());
					}

					DHOST_TYPE_BOOL bSignatureCheck = true;
					bSignatureCheck = m_pHost->CheckDoYouHaveSignature(pCharacterInformation, pBallShot->signaturekey);
					if (bSignatureCheck == false)
					{
						string bSignatureCheckStr = "[HACK_CHECK] DOES_NOT_HAVE_SIGNATURE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID()) + ", bSignatureCheck : " + std::to_string(bSignatureCheck);
						m_pHost->ToLog(bSignatureCheckStr.c_str());
					}

					if (pBallShot->bonussignature > kFLOAT_INIT)
					{
						if (pBallShot->signaturekey == kINT32_INIT)
						{
							bSignatureCheck = false;

							string bSignatureCheckStr = "[HACK_CHECK] DOES_NOT_HAVE_SIGNATURE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
								+ ", bonussignature : " + std::to_string(pBallShot->bonussignature)
								+ ", signaturekey : " + std::to_string(pBallShot->signaturekey);
							m_pHost->ToLog(bSignatureCheckStr.c_str());
						}
					}


					CAbility* pOwnerAbility = pCharacter->GetAbility();

					if (pOwnerAbility != nullptr)
					{
						pBallShot->point = 2;

						F4PACKET::SHOT_TYPE client_shot_type = pBallShot->shottype;

						// ĳ������ �� ��� ��ġ ���Ӽ��ϰ�� ������ ��ġ��, �������� ���� �÷��̾�� ��ġ��
						TB::SVector3 shot_position(kFLOAT_INIT, kFLOAT_INIT, kFLOAT_INIT);

						// ĳ������ ���� �׼��� ���ΰ��
						if ((pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_shot
							|| pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_shotDunk
							|| pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_shotJump
							|| pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_shotLayUp
							|| pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_shotPost
							|| pCharacterActionInfo->actionType == F4PACKET::ACTION_TYPE::action_alleyOopShot) && pCharacterActionInfo->action != nullptr)
						{
							shot_position.mutate_x(pCharacterActionInfo->action->positionlogic().x());
							shot_position.mutate_y(pCharacterActionInfo->action->positionlogic().y());
							shot_position.mutate_z(pCharacterActionInfo->action->positionlogic().z());
						}

						if (pBallShot->skillindex == F4PACKET::SKILL_INDEX::skill_aimShot 
							|| pBallShot->skillindex == F4PACKET::SKILL_INDEX::skill_stepBackThreePointShot 
							|| pBallShot->skillindex == F4PACKET::SKILL_INDEX::sideStepJumper)
						{
							shot_position.mutate_x(pBallShot->positionowner->x());
							shot_position.mutate_y(pBallShot->positionowner->y());
							shot_position.mutate_z(pBallShot->positionowner->z());
						}

						if (pPhysics->Is3PointArea(shot_position))
						{
							pBallShot->point = 3;
							pBallShot->shottype = F4PACKET::SHOT_TYPE::shotType_threePoint;
						}

						if (client_shot_type == F4PACKET::SHOT_TYPE::shotType_threePoint && pBallShot->point == 2)
						{
							pBallShot->shottype = F4PACKET::SHOT_TYPE::shotType_middle;
						}

						// ���� �������� �������� Ȯ��
						bool bSuccess = false;

						// �� ������
						float successRate = kFLOAT_INIT;

						BALL_STATE ballState = BALL_STATE::ballState_none;

						// ������ ��ġ
						auto posowner = *pBallShot->positionowner;

						// ���ƿ� �ð� ����
						m_pHost->SetShotRoomElapsedTime(recv_time - pBallShot->time);

						// ���� ��ġ
						TB::SVector3 rimPos = pPhysics->GetRimPosition();
						rimPos.mutate_y(0.0f);

						// ���Ϳ� �� ������ �Ÿ�
						DHOST_TYPE_FLOAT distanceOwnerRim = VECTOR3_DISTANCE(posowner, rimPos);

						DHOST_TYPE_FLOAT targetReachTime = 0.0f;

						DHOST_TYPE_BOOL bLogShotRate = true;

#ifdef LOCAL_HOST_TEST
						bLogShotRate = m_pHost->GetLogShotRate();
#endif

						DHOST_TYPE_FLOAT shotHindrances = kFLOAT_INIT;

						DHOST_TYPE_FLOAT shotHindrancesPotential = kFLOAT_INIT;

						shot_rate_log += "CharacterID : " + std::to_string(pBallShot->owner) + ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pBallShot->shottype) + ", Distance : " + std::to_string(distanceOwnerRim) + "\n";

						if (data->ballshot()->shotbound() == SHOT_BOUND::success_clean)
						{
							
							// 1. �� Ÿ�Կ� ���� �� ������  **** ���⼭ pBallShot->shotthrow �� �ٲ�, �귿������ 
							successRate = CalcShotTypeToSuccessRate(pCharacterInformation, pBallShot, pOwnerAbility, distanceOwnerRim, ballState);

							if (bLogShotRate)
							{
								//string log_message = "1. CalcShotTypeToSuccessRate successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "1. CalcShotTypeToSuccessRate successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							// 2. �� �� ������� ���� �� ������ ���ʽ�
							successRate += CalcHandednessToSuccessRateBonus(pCharacterInformation, pBallShot, pOwnerAbility);

							if (bLogShotRate)
							{
								//string log_message = "2. CalcHandednessToSuccessRateBonus successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "2. CalcHandednessToSuccessRateBonus successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							// 3. ��ų������� ���� ������
							successRate = CalcSkillToSuccessRateBonus(pCharacterInformation, pBallShot, pOwnerAbility, successRate, pCharacter->CheckSkillForce(pBallShot->skillindex));

							if (bLogShotRate)
							{
								//string log_message = "3. CalcSkillToSuccessRateBonus successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "3. CalcSkillToSuccessRateBonus successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							// �޴� ���ʽ�
							successRate += ApplyMedalEffectToShotSuccessRate(pCharacterInformation, pBallShot);

							if (bLogShotRate)
							{
								//string log_message = "4. ApplyMedalEffectToShotSuccessRate successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "4. ApplyMedalEffectToShotSuccessRate successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							pBallShot->targetreachtime = (CalculateTargetReachTime(m_pHost->GetBalanceTable(), pBallShot, distanceOwnerRim));

							// �� ����
							for (int i = 0; i < pBallShot->hindrances.size(); i++)
							{
								//! �� �������� �ܰ� ���� (���ù��ذ� ������� �ƴ϶� Ŭ�� �ذ����� �� - ��ȭ ���θ� üũ)
								//DHOST_TYPE_INT32 HindrancesFxLevel = kINT32_INIT;
								//shotHindrancesPotential = CalcPotentialToHindrancesBonus(pBallShot->hindrances[i].id(), pBallShot, &HindrancesFxLevel, pBallShot->hindrances[i].actiontype());
								//pBallShot->hindrances[i].mutate_potentialfxlevelbreathlessperimeterdef(HindrancesFxLevel);
								//
								//shotHindrances += (pBallShot->hindrances[i].pointresult() + shotHindrancesPotential);
								//successRate -= (pBallShot->hindrances[i].pointresult() + shotHindrancesPotential);

								//! ���� �ɷ� ����
								EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, pBallShot->hindrances[i].id(), pBallShot->hindrances[i].potentialfxlevelbreathlessperimeterdef(), POTENTIAL_INDEX::breathless_perimeter_def);
								if (check_verify != EACTION_VERIFY::SUCCESS)
								{
									bHindrancesCheck = false;
								}

								shotHindrances += pBallShot->hindrances[i].pointresult();
								successRate -= pBallShot->hindrances[i].pointresult();
							}

							if (bLogShotRate)
							{
								//string log_message = "5. hindrances successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "5. hindrances successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							// ���̽��н��� ��� ������ ���ʽ�, �ñ״���, �нú�, ��Ŭ �극��ũ 
							successRate += pBallShot->bonus;


							DHOST_TYPE_FLOAT fPassiveBonus = pCharacter->GetPassiveBonus(F4PACKET::ACTION_TYPE::action_shot, pBallShot->shottype); // ���ƺ��� ���⼭ ������ by steven.
							successRate += fPassiveBonus; 

							/* ĳ���ͺ� ���� ���ʽ� */
							DHOST_TYPE_FLOAT fBuffBonus = pCharacter->GetBuffBonus(F4PACKET::ACTION_TYPE::action_shot, pBallShot->shottype);
							successRate += fBuffBonus; // ���ʽ��� �ִٸ� �����ؼ� �� ���̴�. 
							pBallShot->successrate += fBuffBonus; // Ŭ���̾�Ʈ �˸��� ( �̰��� �ڿ� successRate �� ���� �ֱ� ������ �ʿ� ���� �� ������? ) 

							/*�̴ϰ��ӿ��� ���� ���ʽ�*/
							fBuffBonus = CPlay_c2s_ballShot::AddMinigameCustomInfo(m_pHost);
							successRate += fBuffBonus; // ���ʽ��� �ִٸ� �����ؼ� �� ���̴�. 
							pBallShot->successrate += fBuffBonus; // Ŭ���̾�Ʈ �˸��� ( �̰��� �ڿ� successRate �� ���� �ֱ� ������ �ʿ� ���� �� ������? ) 

						    // ����Ʈ������ ���������� ���� 
							if (pBallShot->shottype == SHOT_TYPE::shotType_middle || pBallShot->shottype == SHOT_TYPE::shotType_threePoint)
							{
								DHOST_TYPE_FLOAT fGiftBuffBonus = pCharacter->GetGiftBuffBonus(F4PACKET::ACTION_TYPE::action_shot, CCharacterBuff::GiftBuff_JumpShotValue);
								successRate += fGiftBuffBonus; // ���ʽ��� �ִٸ� �����ؼ� �� ���̴�. 
								pBallShot->successrate += fGiftBuffBonus; // Ŭ���̾�Ʈ �˸��� ( �̰��� �ڿ� successRate �� ���� �ֱ� ������ �ʿ� ���� �� ������? ) 
							}

							// ���� �� �нú� ȿ�� �ֱ� 
							if (bLogShotRate)
							{
								//string log_message = "6. nicePass successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());
								shot_rate_log += "6. nicePass successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							// �� ���� ���� ���� ���ʽ�
							successRate += ApplyMedalEffectToShotWideOpen(pCharacterInformation, pBallShot, pOwnerAbility);

							if (bLogShotRate)
							{
								//string log_message = "7. ApplyMedalEffectToShotWideOpen successRate : " + std::to_string(successRate);
								//m_pHost->ToLog(log_message.c_str());

								shot_rate_log += "7. ApplyMedalEffectToShotWideOpen successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							//! ����ɷ� ���ʽ�
							successRate += CalcPotentialToSuccessRateBonus(pCharacterInformation, pBallShot);

							float teamBonus = CalcTeamAttributesShotRateBonus(pCharacterInformation, pBallShot);

							// ���ɷ�ġ ���� 
							successRate += teamBonus; // CalcTeamAttributesShotRateBonus(pCharacterInformation, pBallShot);

							if (bLogShotRate)
							{
								shot_rate_log += "8. PotentialBonus successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4) + "\n";
							}

							//! ��

							float randomSuccess_wrong = abs(RANDOM_FLOAT(1));
							float randomSuccess = abs(Util::GetRandom(1.0f));
							float trainingmode_success = abs(Util::GetRandom(1.0f));

							// �ϸ��ɽ�ũ������ ���� ������� �ɷ��ִ� ��Ȳ�̶�� �� �������� ���ҽ�Ų��.
							if (m_pHost->GetTeamIllegalScreen(pBallShot->owner))
							{
								string log_message = "PreIllegalScreenDebuff successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4);
								m_pHost->SendDebugMessage(log_message);

								successRate -= m_pHost->GetBalanceTable()->GetValue("VIllegalScreen_Penalty_Shooting");

								log_message = "PostIllegalScreenDebuff successRate : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate), 4);
								m_pHost->SendDebugMessage(log_message);
							}

							// ���ù��� Ʃ�丮�� �÷��̾� ���ؿ� ���� �� Ȯ�� ��ȭ�ֱ�.
							if (m_pHost->GetConvertGameMode() == F4PACKET::EGAME_MODE::eGameMode_tutorial_shootingDisturb)
							{
								successRate += 1.0f;
							}

							// ���� ����
							DHOST_TYPE_FLOAT rescue = kFLOAT_INIT;
							// ���� ���� �� ���� ����
							DHOST_TYPE_FLOAT rescue_losing = kFLOAT_INIT;
							// ���� �ʹ� ���� ����
							DHOST_TYPE_FLOAT rescue_early = kFLOAT_INIT;

							// �����ִ� �� ����
							// ��Ƽ��� ��尡 �ƴҰ�쿡�� �� ������ ����
							if (m_pHost->GetModeType() != EMODE_TYPE::CONTINUOUS)
							{
								if (GetEnableRescue(pBallShot->shottype, pOwnerAbility))
								{
									rescue_losing = m_pHost->GetScoreRescue(pCharacterInformation->team, pBallShot);
								}
							}

							// ���� �ʹ� ����
							if (m_pHost->GetEnableRescue_CorrectionEarlyInTheGame(pBallShot, pOwnerAbility))
							{
								rescue_early = m_pHost->GetScoreRescue_CorrectionEarlyInTheGame(pCharacterInformation->team, pBallShot->shottype);
							}

							rescue = rescue_losing + rescue_early;

							decreaseShotSuccessRate = pBallShot->bonussignature;

							if (bLogShotRate)
							{
								shot_rate_log += "9. rescue(rescue_losing + rescue_early) : " + Util::CutOnSpecificDecimalPt(std::to_string(rescue), 4) + ", rescue_losing : " + Util::CutOnSpecificDecimalPt(std::to_string(rescue_losing), 2) + ", rescue_early : " + Util::CutOnSpecificDecimalPt(std::to_string(rescue_early), 4) + "\n";
								m_pHost->SendDebugMessage(shot_rate_log);
							}


							// �Ұ����� ���ýõ� ����
							m_pHost->BadMannerDetectImpossibleShooting(pBallShot->owner, m_pHost->GetShotClock(), successRate + shotHindrances);


							// ������ ���� ���� ����
							if (pBallShot->skillindex == SKILL_INDEX::inYourFace)
								bSuccess = true;
							else
							{
								bSuccess = randomSuccess <= successRate + rescue;

							}


							if (bLogShotRate)
							{
								shot_rate_log += "10. randomSuccess : " + Util::CutOnSpecificDecimalPt(std::to_string(randomSuccess), 4) + ", successRate + rescue : " + Util::CutOnSpecificDecimalPt(std::to_string(successRate + rescue), 4) + "\n";
								m_pHost->SendDebugMessage(shot_rate_log);
							}

							if (bLogShotRate)
							{
								shot_rate_log += "11. TeamBonus : " + Util::CutOnSpecificDecimalPt(std::to_string(teamBonus), 4);
								m_pHost->SendDebugMessage(shot_rate_log);
							}

							if (m_pHost->CheckOption(HOST_FUNC_SHOTFAIL) == true)
							{
								bSuccess = false;
							}

							// �Ϻ� Ʈ���̴� ��忡�� �� Ȯ���� 100%�� ���� // ����ö �߰�
							if (m_pHost->GetConvertGameMode() == F4PACKET::EGAME_MODE::eGameMode_training_jumpshotBlock
								|| m_pHost->GetConvertGameMode() == F4PACKET::EGAME_MODE::eGameMode_training_rimAttackBlock)
							{
								bSuccess = true;
							}

							// �Ϻ� Ʈ���̴� ��忡�� �� Ȯ���� 0%�� ���� // ����ö �߰�
							if (m_pHost->GetConvertGameMode() == F4PACKET::EGAME_MODE::eGameMode_training_rebound)
							{	// ���ξ׼��� �� Ȯ������ �ȹ޵���
								CCharacter* owner = m_pHost->GetCharacterManager()->GetCharacter(m_pHost->GetBallOwner());
								if (owner->GetCharacterStateActionType() != F4PACKET::ACTION_TYPE::action_tipIn)
								{
									bSuccess = false;
								}
							}


							//! ������ �׼ǿ� ���� ������ �˻�
							CCharacter* pCharacterForceBallEventFail = m_pHost->GetCharacterManager()->GetCharacter(pBallShot->owner);
							if (pCharacterForceBallEventFail != nullptr)
							{
								DHOST_TYPE_BALL_NUMBER bCheckBallNo = pCharacterForceBallEventFail->GetCharacterForceBallEventFail();

								if (bCheckBallNo > kBALL_NUMBER_INIT && bCheckBallNo == pBallShot->ballnumber)
								{
									bSuccess = false;

									string str = "[BALL_SHOT] FORCE_SUCCESS_FAIL_SHOT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(userid)
										+ ", Owner : " + std::to_string(pBallShot->owner) + ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pBallShot->shottype)
										+ ", BallNo : " + std::to_string(pBallShot->ballnumber);
									m_pHost->ToLog(str.c_str());

									m_pHost->SetCharacterForceBallEventFail(pBallShot->owner, kBALL_NUMBER_INIT);
								}
							}

							DHOST_TYPE_BOOL bImpossibleAction = false;

							// ���ʽ����� Ư���� �̻��̸� �̻��ѳ��̴�. ���ó��������
							if (pBallShot->bonus > 0.95f)
							{
								bSuccess = false;

								string str = "[HACK_CHECK] OMG FORCE_SUCCESS_FAIL_SHOT_BONUS_TOTAL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid) + ", BallNo : " + std::to_string(pBallShot->ballnumber)
									+ ", bonus : " + std::to_string(pBallShot->bonus)
									+ ", Owner : " + std::to_string(pBallShot->owner) + ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pBallShot->shottype);
								m_pHost->ToLog(str.c_str());

								//m_pHost->KickToTheLobby(userid, EKICK_TYPE::cheat_kick);
								m_pHost->IncreaseHackImpossibleAction(pBallShot->owner);
								bImpossibleAction = true;
							}

							if (pBallShot->shottype != F4PACKET::SHOT_TYPE::shotType_middle && pBallShot->shottype != F4PACKET::SHOT_TYPE::shotType_threePoint)
							{
								pCharacter->SetCheckPassiveValidity(true); // ��Ű �нú� ��Ȳ�� �ƴϸ� , ��Ű�� �����ε� 
							}

							if (!pCharacter->IsValidatePassiveValue(pBallShot->bonuspassive)) // ConvertPacket_play_c2s_playerPassive( SetPacketCharacterPassive ) ���� �нú� üũ�� �̻��� ������ ���� ���� ��Ų�� 
							{
								bSuccess = false;
								F4PACKET::SPlayerInformationT* pSCharacter = pCharacter->GetCharacterInformation();
								if (pSCharacter)
								{
									string str = "[HACK_CHECK] [Passive] OMG Shot Failed RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) 
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(userid) 
										+ ", CharacterResourceID : " + std::to_string(pSCharacter->characterid)
										+ ", PassiveCurrent : " + std::to_string(pCharacter->GetBonusShotPassiveCurrent())
										+ ", Bonuspassive : " + std::to_string(pBallShot->bonuspassive);

									m_pHost->ToLog(str.c_str());
								}
							
							}

							if (pBallShot->bonuspassive > pCharacter->GetBonusShotPassiveMax() + 0.01f )
							{
								F4PACKET::SPlayerInformationT* pSCharacter = pCharacter->GetCharacterInformation();
								if (pSCharacter)
								{
									string str = "[HACK_CHECK] [Passive] OMG BonusPassiveMax Over RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(userid) 
										+ ", CharacterResourceID : " + std::to_string(pSCharacter->characterid)
										+ ", Bonuspassive : " + std::to_string(pBallShot->bonuspassive)
										+ ", BonusShotPassiveMax : " + std::to_string(pCharacter->GetBonusShotPassiveMax());
									m_pHost->ToLog(str.c_str());

									bSuccess = false; // �α׸� ���� ���� 
								}
							}

							//! ���ʽ��� �ƽ� �˼�
							if (bSuccess == true && (pBallShot->bonusanklebreakesuccess > SHOT_BONUS_ANKLE_MAX || pBallShot->bonusnicepass > SHOT_BONUS_NICEPASS_MAX || pBallShot->bonuspassive > SHOT_BONUS_PASSIVE_MAX || pBallShot->bonussignature > SHOT_BONUS_SIGNATURE_MAX))
							{
								
								bSuccess = false;

								F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformationWithUserID(userid);

								if (pCharacter != nullptr)
								{
									string str = "[HACK_CHECK] OMG FORCE_SUCCESS_FAIL_SHOT_BONUS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(userid)
										+ ", BallNo : " + std::to_string(pBallShot->ballnumber)
										+ ", CharID : " + std::to_string(pCharacter->characterid)
										+ ", bonusanklebreakesuccess : " + std::to_string(pBallShot->bonusanklebreakesuccess)
										+ ", bonusnicepass : " + std::to_string(pBallShot->bonusnicepass)
										+ ", bonuspassive : " + std::to_string(pBallShot->bonuspassive)
										+ ", bonussignature : " + std::to_string(pBallShot->bonussignature);

									m_pHost->ToLog(str.c_str());

									m_pHost->IncreaseHackImpossibleAction(pBallShot->owner);
									bImpossibleAction = true;
								}
							}

							DHOST_TYPE_BOOL bCheck_bonus = true;

							DHOST_TYPE_FLOAT bonus_total = pBallShot->bonusanklebreakesuccess + pBallShot->bonusnicepass + pBallShot->bonuspassive + pBallShot->bonussignature;

							DHOST_TYPE_INT32 trunc_bonus_total = (DHOST_TYPE_INT32)truncf(bonus_total * 100.0f);
							DHOST_TYPE_INT32 trunc_bonus = (DHOST_TYPE_INT32)truncf(pBallShot->bonus * 100.0f);

							DHOST_TYPE_INT32 diff_check_bonus = abs(trunc_bonus_total - trunc_bonus);

							//! ���ʽ����� ���̰� 1 �̻��̸� �̻��ѳ�
							if (diff_check_bonus > 1)
							{
								string str = "[HACK_CHECK] HACK_DETECT diff_check_bonus > 1 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", bonus_total : " + std::to_string(bonus_total)
									+ ", bonus : " + std::to_string(pBallShot->bonus);
								m_pHost->ToLog(str.c_str());

								bCheck_bonus = false;
							}

							DHOST_TYPE_BOOL bNicePassCheck = true;
							if (pBallShot->bonusnicepass > kFLOAT_INIT)
							{
								bNicePassCheck = m_pHost->CheckNicePassBonus(pBallShot, pCharacterInformation);
							}


							if ( ( (int)m_pHost->GetModeType() <= (int)EMODE_TYPE::TRAINING ) && (bSpeedHackCheck == false || bSkillCheck == false
								|| bSignatureCheck == false || bNicePassCheck == false || bCheck_bonus == false 
								|| bHindrancesCheck == false) )
							{
								bSuccess = false;

								string str = "[HACK_CHECK] HACK_DETECT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)

									+ ", bSpeedHackCheck : " + std::to_string(bSpeedHackCheck)
									+ ", bSkillCheck : " + std::to_string(bSkillCheck)
									+ ", bSignatureCheck : " + std::to_string(bSignatureCheck)
									+ ", bNicePassCheck : " + std::to_string(bNicePassCheck)
									+ ", bCheck_bonus : " + std::to_string(bCheck_bonus)
									+ ", bHindrancesCheck : " + std::to_string(bHindrancesCheck);
								m_pHost->ToLog(str.c_str());

								bImpossibleAction = true;
							}

							//! Ʈ���̴� ��忡���� ���� �������� (��ŷ�ϴ� ��� �򰥸���)
							if (bImpossibleAction && m_pHost->GetModeType() == EMODE_TYPE::TRAINING)
							{
								bSuccess = randomSuccess <= trainingmode_success;
							}

							// ���� ������ ��
							if (bSuccess == true && clientBallShotSucces)
							{
								// ��ũ��� �麸�� ���¸� �������ش�.
								if (CheckShotTypeDunk(data->ballshot()->shottype()))
								{
									m_pHost->CheckBackBoardState();
								}

								// ��Ÿ�Կ� ���� ���ٿ�� ��������
								pBallShot->shotbound = GetSuccessShotBoundByShotType(pBallShot->bonus, data->ballshot()->shottype());

								// 3�������� Ȯ��
								if (CheckThreePoint(pBallShot->shottype))
								{
									recordType = ECHARACTER_RECORD_TYPE::SUC_3POINT;
								}
							}
							else
							{
								pBallShot->shotbound = GetFailShotBoundByShotType(pBallShot->bonus, data->ballshot()->shottype());
							}

							// 

							// ��ý�Ʈ�� �ִ��� Ȯ��
							if (m_pHost->CheckAssistAtTime(m_pHost->GetBallOwner(), m_pHost->BallNumberGet(), m_pHost->GetGameTime()))
							{
								bAssist = true;
							}

							// ���� ���� �н����̾����� Ȯ��
							if (bAssist == false && m_pHost->CheckIfPrevBallIsPass(m_pHost->BallNumberGet()))
							{
								if (pBallShot->wideopen == true || pBallShot->enabledribble == true || m_pHost->GetPhysicsHandler()->IsPaintZoneArea(*pBallShot->positionowner))
								{
									bAssist = true;
								}
							}

							SBallActionInfo sInfo;
							sInfo.BallNumber = m_pHost->BallNumberGet();
							sInfo.BallOwner = m_pHost->GetBallOwner();
							sInfo.BallOwnerTeam = m_pHost->ConvertToStringTeam(m_pHost->GetCharacterTeam(sInfo.BallOwner));
							sInfo.Ballstate = m_pHost->ConvertToEnumBallState(ballState);
							sInfo.bAssist = bAssist;
							sInfo.fGameTime = m_pHost->GetGameTime();
							sInfo.SkillIndex = (DHOST_TYPE_UINT32)pBallShot->skillindex;
							m_pHost->PushBallActionInfoDeque(sInfo);

							int sectorNumber = kINT32_INIT;

							if (CheckShotTypeDunk(data->ballshot()->shottype()))
							{
								sectorNumber = 0;
							}
							else
							{
								sectorNumber = m_pHost->GetShotSolution()->GetSectorNumber(TB::SVector3(pBallShot->positionball->x(), pBallShot->positionball->y(), pBallShot->positionball->z()), TB::SVector3(rimPos.x(), rimPos.y(), rimPos.z()));
							}

							unsigned int key = m_pHost->GetShotSolution()->GetPointDataKey((F4PACKET::SHOT_THROW)pBallShot->shotthrow, sectorNumber, (F4PACKET::SHOT_BOUND)pBallShot->shotbound);

							int count = m_pHost->GetShotSolution()->GetCount(key);

							if (count > 0)
							{
								pBallShot->index = (rand() % count);
							}
							else
							{
								pBallShot->index = 0;
							}
							
							// ���� !!
							// ���ٿ�� �����ϱ� 
							if (m_pHost->CheckOption(HOST_OPTION_ONLY_SHOT_SOLUTION) == true) // �� �ַ�� �����ϱ�, ���ʸ��ٿ�� Ʃ�丮��
							{
								pBallShot->shotthrow = F4PACKET::SHOT_THROW::high;
								pBallShot->shotbound = SHOT_BOUND::fail_forward_strong_bound;
								pBallShot->index = 0;
							}
																			
							CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

							pGameBallData->ballNumber = m_pHost->BallNumberGet();
							pGameBallData->ballState = ballState;
							pGameBallData->shotType = pBallShot->shottype;
							pGameBallData->owner = pBallShot->owner;
							pGameBallData->point = pBallShot->point;
							pGameBallData->team = pCharacterInformation->team;
							pGameBallData->posStart.fX = pBallShot->positionball->x();
							pGameBallData->posStart.fY = pBallShot->positionball->y();
							pGameBallData->posStart.fZ = pBallShot->positionball->z();
							pGameBallData->timeElapsed = pBallShot->time;

							ECHARACTER_RECORD_TYPE try_shot_record = ECHARACTER_RECORD_TYPE::TRY_2POINT;

							if (CheckThreePoint(pBallShot->shottype))
							{
								try_shot_record = ECHARACTER_RECORD_TYPE::TRY_3POINT;
							}

							m_pHost->UpdateRecord(pBallShot->owner, m_pHost->BallNumberGet(), try_shot_record);
							SendRecordData(pBallShot->owner, (RECORD_TYPE)m_pHost->ConvertToProtobufRecordType(try_shot_record));

							if (data->ballshot()->skillindex() == SKILL_INDEX::skill_tipIn)
							{
								m_pHost->UpdateRecord(pBallShot->owner, m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::REBOUND);
								SendRecordData(pBallShot->owner, (RECORD_TYPE)m_pHost->ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE::REBOUND));
							}

							// ������ ���� �� �̺�Ʈ �ʿ� �� �־��ֳ�??
							if (bSuccess == true && clientBallShotSucces)
							{
								// ���⿡ �־� �־� 
								//pGameBallData->ballEventMap.insert(pair<string, float>("Event_RightGoalIn", pBallShot->targetreachtime() + max(pSolutionTargetData->goalInTime, 0.0f)));
							}
							else
							{
								pGameBallData->ballEventMap.insert(pair<string, float>("Event_FirstRimBound", pBallShot->targetreachtime));
							}
							
							m_pHost->BallControllerGet()->AddShotBallData(pBallShot, pGameBallData, pBallShot->time);

							pBallShot->successrate = (successRate - decreaseShotSuccessRate); // Ŭ���̾�Ʈ�� ������ �α׸� ��� �� 

							// �� ���ظ� �� ĳ���͵��� ������ ����
							if (pBallShot->hindrances.size() > kINT32_INIT)
							{
								std::vector<DHOST_TYPE_CHARACTER_SN> vCharacterSN;

								for (int i = 0; i < pBallShot->hindrances.size(); ++i)
								{
									vCharacterSN.push_back(pBallShot->hindrances[i].id());

									CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pBallShot->hindrances[i].id());
									if (pCharacter)
									{
										pCharacter->OnPlayerShotHindrance(pBallShot->hindrances[i].id());
									}
								}

								m_pHost->PushBallShotHindrance(vCharacterSN);
							}
							else
							{
								m_pHost->ClearShotHindrance();
							}

						} // end of shotclean, Ŭ���̾�Ʈ�� �̷��� ������ �������� üũ�ϴ� ���ΰ�? 
					}
					else
					{
						string str = "[BALL_SHOT] pOwnerAbility is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime());
						m_pHost->ToLog(str.c_str());
					}
				}
				else
				{
					string str = "[BALL_SHOT] pCharacterInformation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime());
					m_pHost->ToLog(str.c_str());
				}
			}
			else
			{
				string str = "[BALL_SHOT] pCharacter is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime());
				m_pHost->ToLog(str.c_str());
			}
		}
		else
		{
			string str = "[BALL_SHOT] pPhysics is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime());
			m_pHost->ToLog(str.c_str());
		}

		pBallShot->ballnumber = m_pHost->BallNumberGet();
		pBallShot->bbstate = m_pHost->GetBackBoardState();
	}
	else
	{
		if (pBallShot->shottype == SHOT_TYPE::shotType_none)
		{
			string str = "[BALL_SHOT] SHOT_TYPE_NONE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", HostBallNumber : " + std::to_string(m_pHost->BallNumberGet()) + ", PakcetBallNumber : " + std::to_string(pBallShot->ballnumber) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(str.c_str());
		}
		else if (bSpeedHackCheck == false)
		{
			string str = "[BALL_SHOT] bSpeedHackCheck is false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", HostBallNumber : " + std::to_string(m_pHost->BallNumberGet()) + ", PakcetBallNumber : " + std::to_string(pBallShot->ballnumber) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(str.c_str());
		}
		else
		{
			string str = "[BALL_SHOT] DIFF_BALL_NO RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", HostBallNumber : " + std::to_string(m_pHost->BallNumberGet()) + ", PakcetBallNumber : " + std::to_string(pBallShot->ballnumber) + ", Owner : " + std::to_string(pBallShot->owner) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(str.c_str());
		}
	}

	//! ��ų� ���� (���ϸ� ���� �տ��� �������ϱ� �ش� ���� �ʱ�ȭ)
	m_pHost->UpdateIntentionalBallHogPlay(kCHARACTER_SN_INIT, m_pHost->GetGameTime(), m_pHost->BallNumberGet());

	CREATE_BUILDER(builder);
	auto offset_ballshot_data = SBallShot::Pack(builder, pBallShot);
	CREATE_FBPACKET(builder, play_s2c_ballShot, message, databuilder);
	databuilder.add_ballshot(offset_ballshot_data);
	STORE_FBPACKET(builder, message, databuilder);
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}





DHOST_TYPE_BOOL CState::Process_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballsimulation() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballsimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;

	F4PACKET::play_c2s_ballRebound_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballRebound_dataT* pBallSimulation = &dataT;

	if (pBallSimulation == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallSimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	DHOST_TYPE_BOOL bResult = pBallSimulation->ballsimulation->success();

	//! ��Ŷ ���� üũ

	//! ������ �׼ǿ� ���� ������ �˻�
	CCharacter* pCharacterForceBallEventFail = m_pHost->GetCharacterManager()->GetCharacter(pBallSimulation->ballsimulation->owner());
	if (pCharacterForceBallEventFail != nullptr)
	{
		DHOST_TYPE_BALL_NUMBER bCheckBallNo = pCharacterForceBallEventFail->GetCharacterForceBallEventFail();

		if (bCheckBallNo > kBALL_NUMBER_INIT && bCheckBallNo == pBallSimulation->ballsimulation->ballnumber())
		{
			string hack_log = "[HACK_CHECK] PLAY_C2S_BALLREBOUND_LAST_ACTION_FAIL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid)
				+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
				+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
				+ ", LastAction : " + F4PACKET::EnumNameACTION_TYPE(pCharacterForceBallEventFail->GetCharacterAction()->actionType);
			m_pHost->ToLog(hack_log.c_str());

			m_pHost->SetCharacterForceBallEventFail(pBallSimulation->ballsimulation->owner(), kBALL_NUMBER_INIT);

			bResult = false;
		}
	}

	SCharacterAction* pAction = m_pHost->GetCharacterActionInfo(pBallSimulation->ballsimulation->owner());

	DHOST_TYPE_BOOL reboundAbleFlag = false;

	if (bResult && pAction != nullptr)
	{
		switch (pBallSimulation->ballsimulation->ballstate())
		{
		case BALL_STATE::ballState_pickRebound:
		{
			// m_pHost->SendSyncInfo(SYNCINFO_TYPE::REBOUND); �̰� ���鼭 �ʿ䰡 ������ 

			/*
			CBallController* pBallController = m_pHost->BallControllerGet();

			if (pBallController)
			{
				if (pBallController->GameBallDataGet() != nullptr)
				{
					if (!pBallController->GameBallDataGet()->checkReboundValid)
					{
						string verify_log = "[PLAY_C2S_BALLREBOUND] unable_to_rebound RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
						m_pHost->ToLog(verify_log.c_str());

						bResult = false;
					}
				}
				else
				{
					string verify_log = "[PLAY_C2S_BALLREBOUND] m_pCurrentBallData_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
						+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
					m_pHost->ToLog(verify_log.c_str());

					bResult = false;
				}
			}
			else
			{
				string verify_log = "[PLAY_C2S_BALLREBOUND] pBallController_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
					+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
				m_pHost->ToLog(verify_log.c_str());

				bResult = false;
			}
			*/
		}
		break;
		case BALL_STATE::ballState_looseRebound:
		case BALL_STATE::ballState_looseBlock:
		{
			reboundAbleFlag = true;
		}
		break;
		case BALL_STATE::ballState_looseSteal:
		{
			reboundAbleFlag = true;
		}
		break;
		case BALL_STATE::ballState_pickSteal:
		{
			if (pAction->actionType == F4PACKET::ACTION_TYPE::action_steal)
			{
				CCharacter* owner = m_pHost->GetCharacterManager()->GetCharacter(m_pHost->GetBallOwner());

				DHOST_TYPE_BOOL checkAction = false;

				if (owner != nullptr)
				{
					switch (owner->GetCharacterStateActionType())
					{
					case F4PACKET::ACTION_TYPE::action_moveDribble:
					case F4PACKET::ACTION_TYPE::action_standDribble:
					case F4PACKET::ACTION_TYPE::action_standPivot:
					case F4PACKET::ACTION_TYPE::action_movePivot:
					case F4PACKET::ACTION_TYPE::action_standTripleThreat:
					{
						checkAction = false;
					}
					break;
					default:
					{
						checkAction = true;
					}
					break;
					}

					if (checkAction == true && m_pHost->BallControllerGet()->GameBallDataGet() != nullptr)
					{
						CBallController* ballController = m_pHost->BallControllerGet();

						// 20221026 ���� ����
						if (ballController != nullptr && pBallSimulation->ballsimulation->skillindex() != F4PACKET::SKILL_INDEX::skill_pokeSteal)
						{
							CBallController::GameBallData* gameBallData = ballController->GameBallDataGet();

							if (gameBallData != nullptr)
							{
								F4PACKET::SPlayerAction* pOwner = m_pHost->GetCharacterStateAction(gameBallData->owner);

								//! 20231121 ��ƿ���� �����ʿ��� �Ÿ� ���� �߰�
								CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pBallSimulation->ballsimulation->owner());
								if (pCharacter != nullptr)
								{
									F4PACKET::SPlayerInformationT* pCharacterInformation = pCharacter->GetCharacterInformation();

									if (pCharacterInformation != nullptr && pCharacterInformation->ability != nullptr)
									{
										float direction = GetYaw(pAction->action->positionlogic().x() - pOwner->positionlogic().x(), pAction->action->positionlogic().z() - pOwner->positionlogic().z());
										float directionDelta = abs(CommonFunction::OptimizeAngle(direction - pOwner->yawlogic()));


										switch (owner->GetCharacterStateActionType())
										{
										case F4PACKET::ACTION_TYPE::action_stand:
										{
											CCharacterState_Stand* characterStateStand = dynamic_cast<CCharacterState_Stand*>(owner->GetCharacterState(EHOST_CHARACTER_STATE::STAND));
											switch (characterStateStand->m_moveMode)
											{
											case F4PACKET::MOVE_MODE::postUp:
												if (directionDelta < 90.0f)
													bResult = false;
												break;
											default:
												if (directionDelta > 130.0f)
													bResult = false;
												break;
											}
										}
										break;
										case F4PACKET::ACTION_TYPE::action_move:
										{
											CCharacterState_Move* characterStateMove = dynamic_cast<CCharacterState_Move*>(owner->GetCharacterState(EHOST_CHARACTER_STATE::MOVE));
											switch (characterStateMove->m_moveMode)
											{
											case F4PACKET::MOVE_MODE::postUp:
												if (directionDelta < 90.0f)
													bResult = false;
												break;
											default:
												if (directionDelta > 130.0f)
													bResult = false;
												break;
											}
										}
										break;
										case F4PACKET::ACTION_TYPE::action_crossOverPostUp:
										{
											if (directionDelta < 90.0f)
												bResult = false;
										}
										break;
										default:
										{
											if (directionDelta > 130.0f)
												bResult = false;
										}
										break;
										}
									}
									else
									{
										string verify_log = "[PLAY_C2S_BALLREBOUND] pCharacterInformation_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
											+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(userid)
											+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
											+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
											+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
										m_pHost->ToLog(verify_log.c_str());

										bResult = false;
									}
								}
								else
								{
									string verify_log = "[PLAY_C2S_BALLREBOUND] pCharacter_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(userid)
										+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
										+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
										+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
									m_pHost->ToLog(verify_log.c_str());

									bResult = false;
								}
							}
						}
					}
					else
					{
						string verify_log = "[PLAY_C2S_BALLREBOUND] action_or_balldata_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
						m_pHost->ToLog(verify_log.c_str());

						bResult = false;
					}

				}
			}
			else if (pAction->actionType == F4PACKET::ACTION_TYPE::shadowSteal)
			{

				DHOST_TYPE_FLOAT distanceToBall = CommonFunction::SVectorDistanceXZ(pBallSimulation->ballsimulation->positionowner(), pBallSimulation->ballsimulation->positionball());

				//! 1. �Ÿ� ���� (�ִ�)
				if (distanceToBall <= m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowSteal_Distance_Max"))
				{
					//! 2. �Ÿ� ���� (�ּ�)
					if (distanceToBall >= m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowSteal_Distance_Min"))
					{
						//! 3. ���� ����
						if (pBallSimulation->ballsimulation->positionball().y() <= m_pHost->GetBalanceTable()->GetValue("VStealCondition_Height"))
						{


							bResult = true;
						}
						else
						{
							string hack_log = "[HACK_CHECK] shadowSteal_unable_to_height RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
								+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
								+ ", BallHeight : " + std::to_string(pBallSimulation->ballsimulation->positionball().y())
								+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
								+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
							m_pHost->ToLog(hack_log.c_str());

							bResult = false;
						}
					}
					else
					{
						string hack_log = "[HACK_CHECK] shadowSteal_unable_to_distance_min RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
							+ ", distanceToBall : " + std::to_string(distanceToBall)
							+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
							+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
						m_pHost->ToLog(hack_log.c_str());

						bResult = false;
					}
				}
				else
				{
					string hack_log = "[HACK_CHECK] shadowSteal_unable_to_distance_max RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
						+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
						+ ", distanceToBall : " + std::to_string(distanceToBall)
						+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
						+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
					m_pHost->ToLog(hack_log.c_str());

					bResult = false;
				}
			}
			else if (pAction->actionType == F4PACKET::ACTION_TYPE::action_intercept)
			{
				if (pBallSimulation->ballsimulation->ballnumber() > kBALL_NUMBER_INIT && m_pHost->GetCharacterBallEventSuccess(pBallSimulation->ballsimulation->owner()) == pBallSimulation->ballsimulation->ballnumber())
				{
					bResult = true;
				}
				else
				{
					string hack_log = "[HACK_CHECK] PLAY_C2S_BALLREBOUND action_intercept_unable RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
						+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
						+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
						+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
					m_pHost->ToLog(hack_log.c_str());

					bResult = false;
				}
			}
			else
			{
				string hack_log = "[PLAY_C2S_BALLREBOUND] ballState_is_not_defined RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
					+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
					+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
					+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
				m_pHost->ToLog(hack_log.c_str());

				bResult = true;
			}
		}
		break;
		default:
		{

		}
		break;
		}


		if (bResult)
		{
			if (m_pHost->BallNumberGet() <= pBallSimulation->ballsimulation->ballnumber())
			{
				if (m_pHost->BallControllerGet()->IsBeforeShotEventTime(pBallSimulation->ballsimulation->time()))
				{
					m_pHost->BallNumberSet(pBallSimulation->ballsimulation->ballnumber() + 1);
					pBallSimulation->ballsimulation->mutate_ballnumber(m_pHost->BallNumberGet());

					m_pHost->SetBallOwner(pBallSimulation->ballsimulation->owner());

					SBallActionInfo sInfo;
					sInfo.BallNumber = m_pHost->BallNumberGet();
					sInfo.BallOwner = m_pHost->GetBallOwner();
					sInfo.BallOwnerTeam = m_pHost->ConvertToStringTeam(m_pHost->GetCharacterTeam(sInfo.BallOwner));
					sInfo.Ballstate = m_pHost->ConvertToEnumBallState(pBallSimulation->ballsimulation->ballstate());
					sInfo.bAssist = false;
					sInfo.fGameTime = m_pHost->GetGameTime();
					sInfo.SkillIndex = (DHOST_TYPE_UINT32)pBallSimulation->ballsimulation->skillindex();

					m_pHost->PushBallActionInfoDeque(sInfo);

					//! ��ĳ ��� ����
					if (pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatch || pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatchPass)
					{
						m_pHost->UpdateRecord(pBallSimulation->ballsimulation->owner(), m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::DIVING_CATCH);

						if (pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatchPass)
						{
							if (pBallSimulation->ballsimulation->target() == pBallSimulation->ballsimulation->owner())
							{
								m_pHost->IncreaseSkillSucCount(pBallSimulation->ballsimulation->owner(), (DHOST_TYPE_UINT32)SKILL_INDEX::skill_divingCatch);
							}
						}
					}

					m_pHost->UpdateRecordBasicTrainingSystem(pBallSimulation->ballsimulation->owner(), sInfo.BallNumber, (F4PACKET::SKILL_INDEX)sInfo.SkillIndex);
					m_pHost->SetCharacterActionOverlap(pBallSimulation->ballsimulation->owner(), ACTION_TYPE::action_pick, SKILL_INDEX::skill_none);
					m_pHost->AddCharacterActionDeque(pBallSimulation->ballsimulation->owner());

					CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

					if (reboundAbleFlag)
					{
						pGameBallData->checkReboundValid = true;
					}

					pGameBallData->ballNumber = m_pHost->BallNumberGet();
					pGameBallData->ballState = pBallSimulation->ballsimulation->ballstate();
					pGameBallData->shotType = SHOT_TYPE::shotType_none;
					pGameBallData->owner = pBallSimulation->ballsimulation->owner();
					pGameBallData->point = 0;
					pGameBallData->team = m_pHost->GetCharacterTeam(pBallSimulation->ballsimulation->owner());
					pGameBallData->ballEventMap.insert(pair<string, float>("Event_ballSimulation", pBallSimulation->ballsimulation->time()));
					m_pHost->BallControllerGet()->AddBallData(data->ballsimulation(), pGameBallData, pBallSimulation->ballsimulation->time());

					bResult = true;
				}
				else
				{
					bResult = false;
				}
			}
			else
			{
				pBallSimulation->ballsimulation->mutate_ballnumber(m_pHost->BallNumberGet());
				bResult = false;
			}
		}
	}
	else
	{
		string verify_log = "[PLAY_C2S_BALLREBOUND] action_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
			+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
		m_pHost->ToLog(verify_log.c_str());

		bResult = false;
	}

	if (bResult == false)
	{
		pBallSimulation->ballsimulation->mutate_success(bResult);
	}


	if (m_pHost->CheckPositionIsNan(&pBallSimulation->ballsimulation->positionball()))
	{
		string log = "[HACK_CHECK] BALLREBOUND Ball is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", RoomID : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid);

		m_pHost->ToLog(log.c_str());


		return false;
	}

#ifdef _DEBUG
	if (m_BallReboundPackets.size() > 0)
	{
		int i = 0;
		for (const auto& packet : m_BallReboundPackets)
		{
			if (i == 0)
			{
				if (packet->sucess)
				{
					if (m_BallReboundPackets.size() > 1)
					{
						string log_message = "Victory in the rebound battle !!, UserID: "
							+ std::to_string(packet->userID)
							+ " Rebound : " + std::to_string(packet->reboundAbility);
							
						m_pHost->SendDebugMessageUserOnly(log_message, packet->userID);
					}
					else
					{
						string log_message = "Rebound Single Success , UserID: " 
							+ std::to_string(packet->userID)
							+ " Rebound : " + std::to_string(packet->reboundAbility);

						m_pHost->SendDebugMessageUserOnly(log_message, packet->userID);
					}
				}
			}
			else
			{
				if (packet->sucess)
				{
					string log_message = "Defeated in the rebound battle, UserID: " 
						+ std::to_string(packet->userID)
						+ " Rebound : " + std::to_string(packet->reboundAbility);

					m_pHost->SendDebugMessageUserOnly(log_message, packet->userID);
				}
			}
			++i;
		}
	}
#endif

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballRebound, message, send_data);
	send_data.add_ballsimulation(pBallSimulation->ballsimulation.get());
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}



DHOST_TYPE_BOOL CState::OnRecvReboundBallSimulation(DHOST_TYPE_USER_ID userid, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const char* pData, DHOST_TYPE_INT32 size, int abillity)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballsimulation() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballsimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;

	F4PACKET::play_c2s_ballRebound_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballRebound_dataT* pBallSimulation = &dataT;

	if (pBallSimulation == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallSimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	DHOST_TYPE_BOOL bResult = pBallSimulation->ballsimulation->success();

	//! ��Ŷ ���� üũ

	///-------------

	DHOST_TYPE_GAME_TIME_F now = m_pHost->GetRoomElapsedTime();

	const DHOST_TYPE_GAME_TIME_F REBOUND_FULL_TIME = 0.2f; // Ŭ���̾�ư ���ٿ�� �ִϸ��̼� Ÿ�� 
	
	DHOST_TYPE_GAME_TIME_F oneWayPing = m_pHost->GetRoomElapsedTime() - pBallSimulation->clienttime;
	if (oneWayPing <= 0)
	{
		// ����ó�� 
		// 
		string invalid_buffer_log2 = "[REBOUND] : Error - if (oneWayPing <= 0)";
		m_pHost->ToLog(invalid_buffer_log2.c_str());
		oneWayPing = 0.1f;
	}
	const float delayWeight = 0.5f;
	DHOST_TYPE_FLOAT canWaitTime = (REBOUND_FULL_TIME - oneWayPing * 2.0f) * delayWeight; // �������� �������� ��ٷ��� �� �ִ� �ð� 
	if (canWaitTime < 0.0f) canWaitTime = 0.0f; // ����ó�� 

	if (!m_IsBallWindowOpen)
	{
		m_IsBallWindowOpen = true;
		m_BallWindowStartTime = now;

		string invalid_buffer_log2 = "[REBOUND] : - WINDOW OPEN- : " + std::to_string(now)
			+ ", m_BallWindowStartTime : " + std::to_string(m_BallWindowStartTime);
		m_pHost->ToLog(invalid_buffer_log2.c_str());

		WINDOW_DURATION = canWaitTime; // �����츦 �� ������ �ð����� �ʱ�ȭ 
	}

	DHOST_TYPE_GAME_TIME_F elapsed = now - m_BallWindowStartTime;

	// �߿��� ���� 
	DHOST_TYPE_GAME_TIME_F leftTimeDurtion = WINDOW_DURATION - elapsed;

	// ���� ���� ������ ���� ������ �� , ���ο� ���� �������� ��ٸ��� �ð��� ������� �� 
	if (canWaitTime < leftTimeDurtion)
	{

		DHOST_TYPE_GAME_TIME_F pre = WINDOW_DURATION; 
		WINDOW_DURATION = elapsed + canWaitTime;

		float gap = (canWaitTime - leftTimeDurtion);

		string invalid_buffer_log = "[REBOUND] Change WINDOW_DURATION UserID : " + std::to_string(userid)
			+ ", Pre DurationTime : " + std::to_string(pre)
			+ ", Changned DurationTime : " + std::to_string(WINDOW_DURATION)
			+ " , gap : " + std::to_string(gap);
		m_pHost->ToLog(invalid_buffer_log.c_str());
	}
	else
	{
		// ���� �ð��� ���� ��ٷ��� �� �ִ� �ð����� �۰� ���� ��� ������ �ȵ�, �״�� ���� 

		float gap = (canWaitTime - leftTimeDurtion);

		string invalid_buffer_log2 = "[REBOUND] Waiting UserID : " + std::to_string(userid)
			+ ", leftTime : " + std::to_string(leftTimeDurtion)
			+ ", canWaitTime : " + std::to_string(canWaitTime)
			+" , gap : " + std::to_string(gap);

		m_pHost->ToLog(invalid_buffer_log2.c_str());

	}

	// �����찡 ������, ���´�. 
	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pBallSimulation->ballsimulation->owner());
	DHOST_TYPE_FLOAT reboundAbillity = 0.0f;
	if (!pCharacter) return false; 

	reboundAbillity = pCharacter->GetAbility()->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_rebound);


	string invalid_buffer_log2 = "[REBOUND] Character RecvReboundPacket UserID : " + std::to_string(userid) 
		+ ", OneWayPing : " + std::to_string(oneWayPing)
	    +", CanWaitTime : " + std::to_string(canWaitTime);
	    +", Abillity : " + std::to_string(reboundAbillity);
	    	m_pHost->ToLog(invalid_buffer_log2.c_str());

	ReboundBallPacket* reboundBallPacket = new ReboundBallPacket(pData, size);

	if (reboundBallPacket->GetData() == nullptr)
	{
		delete reboundBallPacket;
		return false; // ������ �Ľ� ����
	}

	reboundBallPacket->receivedTime = now;
	reboundBallPacket->userID = userid;
	reboundBallPacket->characterID = pBallSimulation->ballsimulation->owner();
	reboundBallPacket->canWaitTime = canWaitTime;
	reboundBallPacket->reboundAbility = reboundAbillity;
	reboundBallPacket->sucess = bResult;

	m_BallReboundPackets.push_back(reboundBallPacket);

	return true;
}

void CState::ProcessReboundBallSimulationPackets()
{
	if (!m_IsBallWindowOpen)
		return;

	DHOST_TYPE_GAME_TIME_F now = m_pHost->GetRoomElapsedTime();
	DHOST_TYPE_GAME_TIME_F elapsed = now - m_BallWindowStartTime;

	if (m_BallReboundPackets.empty())
	{
		m_pHost->ToLog("[REBOUND] : Error - m_BallReboundPackets.empty()");
		m_IsBallWindowOpen = false;
		return;
	}

	if (elapsed < WINDOW_DURATION)
	{
		return;
	}

	m_pHost->ToLog(
		("[REBOUND] Start Decsion !!,  CurrentTime : " + std::to_string(now)
			+ " , Elapsed: " + std::to_string(elapsed)
			+ " , WINDOW_DURATION : " + std::to_string(WINDOW_DURATION)
			+ " , m_BallWindowStartTime : " + std::to_string(m_BallWindowStartTime)).c_str());

	

	// ���ٿ�� ���� ��Ģ 

	ReboundBallPacket* firstPacket1 = m_BallReboundPackets.front();
	if (firstPacket1)
	{
		// 1. ���ٿ�� ��Ŷ�� ���� ���� ���� ���� 
		firstPacket1->fastestRecvePacketPoint = 20.0f;
		firstPacket1->getTotalPoint += firstPacket1->fastestRecvePacketPoint;
	}

	m_BallReboundPackets.sort([](const ReboundBallPacket* a, const ReboundBallPacket* b) {
		if (!a || !b) return false; // �����ϰ� null ���
		return a->reboundAbility > b->reboundAbility;
		});

	ReboundBallPacket* firstPacket2 = m_BallReboundPackets.front();
	if (firstPacket2)
	{
		// 2. ���ٿ�� �ɷ�ġ�� ���� ���� ���� 
		firstPacket2->bigiestReboundAbilityPoint = 10.0f;
		firstPacket2->getTotalPoint += firstPacket2->bigiestReboundAbilityPoint;
	}

	for (ReboundBallPacket* packet : m_BallReboundPackets)
	{
		if (!packet) continue;  // null üũ (����)

		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(packet->characterID);
		if (!pCharacter) continue;

		int teamInndex = pCharacter->GetCharacterInformation()->team;
		int offenseTeam = m_pHost->GetOffenseTeam();

		// 3. ���� ��Ȳ ���� 
		if (teamInndex != offenseTeam) // �������̸� 
		{
			packet->defendingPoint = 5.0f;
			packet->getTotalPoint += packet->defendingPoint;
		}

		// ������ Ȯ�� 
		int scoreDelta = kINT32_INIT;
		if (teamInndex == 0)
			scoreDelta = m_pHost->GetScore(1) - m_pHost->GetScore(0);
		else
			scoreDelta = m_pHost->GetScore(0) - m_pHost->GetScore(1);

		if (scoreDelta >= 7)
		{
			// 4. 7�� �� �̻����� ���� �ִ� ���� ���� 
			packet->score7LoosingPoint = 20.0f;
			packet->getTotalPoint += packet->score7LoosingPoint;
		}
		else
		if (scoreDelta >= 4)
		{
			// 5. 4�� �� �̻�, 6�� �� ���Ϸ� ���� �ִ� ���� ���� 
			packet->score4To6LoosingPoint = 10.0f;
			packet->getTotalPoint += packet->score4To6LoosingPoint;
		}
		else
		if (scoreDelta > 0)
		{
			// 6. 3�� �� ���Ϸ� ���� �ִ� ���� ���� 
			packet->score3LoosingPoint = 5.0f;
			packet->getTotalPoint += packet->score3LoosingPoint;
		}
	}

	float allgetTotalPoints = 0.0f;
	for (ReboundBallPacket* packet : m_BallReboundPackets)
	{
		if (!packet) continue;  // null üũ (����)
		allgetTotalPoints += packet->getTotalPoint;
	}

	if (allgetTotalPoints <= 0) allgetTotalPoints = 1.0f; // ���� ó�� 

	float result = Util::GetRandom(1.0f);
	float cumulative = 0.0f;

	for (ReboundBallPacket* packet : m_BallReboundPackets)
	{
		if (!packet) continue;  

		float ratio = packet->getTotalPoint / allgetTotalPoints;
		cumulative += ratio;

		if (result <= cumulative)
		{
			packet->priority = 1; // ���� ���� 
			break; // ���� �߰� 
		}
	}

	// ���� ����
	m_BallReboundPackets.sort([](const ReboundBallPacket* a, const ReboundBallPacket* b) {
		if (!a || !b) return false; // �����ϰ� null ���
		return a->priority > b->priority;
		});


	for (const ReboundBallPacket* packet : m_BallReboundPackets)
	{
		if (!packet) continue;  // null üũ (����)

		const F4PACKET::play_c2s_ballRebound_data* data = packet->GetData();

		if (data)
		{
			Process_play_c2s_ballRebound(packet->userID, nullptr, packet->GetPacket(), packet->receivedTime);

			float wait = now - packet->receivedTime;

			string log = "[REBOUND] Process_play_c2s_ballRebound  : " + std::to_string(packet->receivedTime)
				+ ", UserID : " + std::to_string(packet->userID)
				+ ", Waited: " + std::to_string(wait)
				+ ", Now: " + std::to_string(now);

			m_pHost->ToLog(log.c_str());
		}
	}

	for (auto* packet : m_BallReboundPackets)
		delete packet;

	m_BallReboundPackets.clear();

	m_IsBallWindowOpen = false;

	string log = "[REBOUND] --- Window Closed !! ----  : " + std::to_string(m_pHost->GetRoomElapsedTime());
	m_pHost->ToLog(log.c_str());
}


// �̰� �۵��Ϸ��� Ŭ���̾�Ʈ���� ballRebound ��Ŷ�� ������ �� 
const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{

	return Process_play_c2s_ballRebound(userid, peer, pPacket, recv_time);
	/*if (!OnRecvReboundBallSimulation(userid, pPacket, 0))
	{
		return Process_play_c2s_ballRebound(userid, peer, pPacket, recv_time);
	}
	*/
	
	//return true;
}



// �̰� �۵��Ϸ��� Ŭ���̾�Ʈ���� ballRebound ��Ŷ�� ������ �� 
const DHOST_TYPE_BOOL CState::ConvertPacketExtend_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const char* pData, int size, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	
	if (!OnRecvReboundBallSimulation(userid, pPacket, pData, size, 0))
	{
		return Process_play_c2s_ballRebound(userid, peer, pPacket, recv_time);
	}
	
	return true;
}


const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_ballSimulation(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballSimulation_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballsimulation() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballsimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	bool retValue = false;

	F4PACKET::play_c2s_ballSimulation_dataT dataT;

	data->UnPackTo(&dataT);

	F4PACKET::play_c2s_ballSimulation_dataT* pBallSimulation = &dataT;

	if (pBallSimulation == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pBallSimulation is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	DHOST_TYPE_BOOL bResult = pBallSimulation->ballsimulation->success();

	//! ��Ŷ ���� üũ

	//! ������ �׼ǿ� ���� ������ �˻�
	CCharacter* pCharacterForceBallEventFail = m_pHost->GetCharacterManager()->GetCharacter(pBallSimulation->ballsimulation->owner());
	if (pCharacterForceBallEventFail != nullptr)
	{
		DHOST_TYPE_BALL_NUMBER bCheckBallNo = pCharacterForceBallEventFail->GetCharacterForceBallEventFail();

		if (bCheckBallNo > kBALL_NUMBER_INIT && bCheckBallNo == pBallSimulation->ballsimulation->ballnumber())
		{
			string hack_log = "[HACK_CHECK] BALL_SIMULATION_LAST_ACTION_FAIL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) 
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid) 
				+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner()) 
				+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
				+ ", LastAction : " + F4PACKET::EnumNameACTION_TYPE(pCharacterForceBallEventFail->GetCharacterAction()->actionType);
			m_pHost->ToLog(hack_log.c_str());

			m_pHost->SetCharacterForceBallEventFail(pBallSimulation->ballsimulation->owner(), kBALL_NUMBER_INIT);

			bResult = false;
		}
	}

	SCharacterAction* pAction = m_pHost->GetCharacterActionInfo(pBallSimulation->ballsimulation->owner());

	DHOST_TYPE_BOOL reboundAbleFlag = false;

	if (bResult && pAction != nullptr)
	{
		switch (pBallSimulation->ballsimulation->ballstate())
		{
			case BALL_STATE::ballState_pickRebound:
			{
				// m_pHost->SendSyncInfo(SYNCINFO_TYPE::REBOUND); �̰� ���鼭 �ʿ䰡 ������ 
				/*
				CBallController* pBallController = m_pHost->BallControllerGet();

				if (pBallController)
				{
					if (pBallController->GameBallDataGet() != nullptr)
					{
						if (!pBallController->GameBallDataGet()->checkReboundValid)
						{
							string verify_log = "[PLAY_C2S_BALLSIMULATION] unable_to_rebound RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
								+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
							m_pHost->ToLog(verify_log.c_str());

							bResult = false;
						}
					}
					else
					{
						string verify_log = "[PLAY_C2S_BALLSIMULATION] m_pCurrentBallData_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
						m_pHost->ToLog(verify_log.c_str());

						bResult = false;
					}
				}
				else
				{
					string verify_log = "[PLAY_C2S_BALLSIMULATION] pBallController_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
						+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
					m_pHost->ToLog(verify_log.c_str());

					bResult = false;
				}
				*/
			}
			break;
			case BALL_STATE::ballState_looseRebound:
			case BALL_STATE::ballState_looseBlock:
			{
				reboundAbleFlag = true;
			}
			break;
			case BALL_STATE::ballState_looseSteal:
			{
				reboundAbleFlag = true;
			}
			break;
			case BALL_STATE::ballState_pickSteal:
			{
				if (pAction->actionType == F4PACKET::ACTION_TYPE::action_steal)
				{
					CCharacter* owner = m_pHost->GetCharacterManager()->GetCharacter(m_pHost->GetBallOwner());

					DHOST_TYPE_BOOL checkAction = false;

					if (owner != nullptr)
					{
						switch (owner->GetCharacterStateActionType())
						{
							case F4PACKET::ACTION_TYPE::action_moveDribble:
							case F4PACKET::ACTION_TYPE::action_standDribble:
							case F4PACKET::ACTION_TYPE::action_standPivot:
							case F4PACKET::ACTION_TYPE::action_movePivot:
							case F4PACKET::ACTION_TYPE::action_standTripleThreat:
							{
								checkAction = false;
							}
							break;
							default:
							{
								checkAction = true;
							}
							break;
						}

						if (checkAction == true && m_pHost->BallControllerGet()->GameBallDataGet() != nullptr)
						{
							CBallController* ballController = m_pHost->BallControllerGet();

							// 20221026 ���� ����
							if (ballController != nullptr && pBallSimulation->ballsimulation->skillindex() != F4PACKET::SKILL_INDEX::skill_pokeSteal)
							{
								CBallController::GameBallData* gameBallData = ballController->GameBallDataGet();

								if (gameBallData != nullptr)
								{
									F4PACKET::SPlayerAction* pOwner = m_pHost->GetCharacterStateAction(gameBallData->owner);

									//! 20231121 ��ƿ���� �����ʿ��� �Ÿ� ���� �߰�
									CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pBallSimulation->ballsimulation->owner());
									if (pCharacter != nullptr)
									{
										F4PACKET::SPlayerInformationT* pCharacterInformation = pCharacter->GetCharacterInformation();

										if (pCharacterInformation != nullptr && pCharacterInformation->ability != nullptr)
										{
											float direction = GetYaw(pAction->action->positionlogic().x() - pOwner->positionlogic().x(), pAction->action->positionlogic().z() - pOwner->positionlogic().z());
											float directionDelta = abs(CommonFunction::OptimizeAngle(direction - pOwner->yawlogic()));


											switch (owner->GetCharacterStateActionType())
											{
												case F4PACKET::ACTION_TYPE::action_stand:
												{
													CCharacterState_Stand* characterStateStand = dynamic_cast<CCharacterState_Stand*>(owner->GetCharacterState(EHOST_CHARACTER_STATE::STAND));
													switch (characterStateStand->m_moveMode)
													{
														case F4PACKET::MOVE_MODE::postUp:
															if (directionDelta < 90.0f)
																bResult = false;
															break;
														default:
															if (directionDelta > 130.0f)
																bResult = false;
															break;
													}
												}
												break;
												case F4PACKET::ACTION_TYPE::action_move:
												{
													CCharacterState_Move* characterStateMove = dynamic_cast<CCharacterState_Move*>(owner->GetCharacterState(EHOST_CHARACTER_STATE::MOVE));
													switch (characterStateMove->m_moveMode)
													{
														case F4PACKET::MOVE_MODE::postUp:
															if (directionDelta < 90.0f)
																bResult = false;
															break;
														default:
															if (directionDelta > 130.0f) 
																bResult = false;
															break;
													}
												}
												break;
												case F4PACKET::ACTION_TYPE::action_crossOverPostUp:
												{
													if (directionDelta < 90.0f)
														bResult = false;
												}
												break;
												default:
												{
													if (directionDelta > 130.0f)
														bResult = false;
												}
												break;
											}
										}
										else
										{
											string verify_log = "[PLAY_C2S_BALLSIMULATION] pCharacterInformation_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
												+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
												+ ", UserID : " + std::to_string(userid)
												+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
												+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
												+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
											m_pHost->ToLog(verify_log.c_str());

											bResult = false;
										}
									}
									else
									{
										string verify_log = "[PLAY_C2S_BALLSIMULATION] pCharacter_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
											+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(userid)
											+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
											+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
											+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
										m_pHost->ToLog(verify_log.c_str());

										bResult = false;
									}
								}
							}
						}
						else
						{
							string verify_log = "[PLAY_C2S_BALLSIMULATION] action_or_balldata_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
								+ ", OwnerActionType : " + F4PACKET::EnumNameACTION_TYPE(owner->GetCharacterStateActionType())
								+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
							m_pHost->ToLog(verify_log.c_str());

							bResult = false;
						}

					}
				}
				else if (pAction->actionType == F4PACKET::ACTION_TYPE::shadowSteal)
				{

					DHOST_TYPE_FLOAT distanceToBall = CommonFunction::SVectorDistanceXZ(pBallSimulation->ballsimulation->positionowner(), pBallSimulation->ballsimulation->positionball());

					//! 1. �Ÿ� ���� (�ִ�)
					if (distanceToBall <= m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowSteal_Distance_Max"))
					{
						//! 2. �Ÿ� ���� (�ּ�)
						if (distanceToBall >= m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowSteal_Distance_Min"))
						{
							//! 3. ���� ����
							if (pBallSimulation->ballsimulation->positionball().y() <= m_pHost->GetBalanceTable()->GetValue("VStealCondition_Height"))
							{
								/*string hack_log = "[SHADOW_STEAL_BALL] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
									+ ", positionownerX : " + std::to_string(pBallSimulation->ballsimulation->positionowner().x())
									+ ", positionownerZ : " + std::to_string(pBallSimulation->ballsimulation->positionowner().z())
									+ ", positionballX : " + std::to_string(pBallSimulation->ballsimulation->positionball().x())
									+ ", positionballZ : " + std::to_string(pBallSimulation->ballsimulation->positionball().z())
									+ ", distanceToBall : " + std::to_string(distanceToBall);
								m_pHost->ToLog(hack_log.c_str());*/

								bResult = true;
							}
							else
							{
								string hack_log = "[HACK_CHECK] shadowSteal_unable_to_height RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
									+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
									+ ", BallHeight : " + std::to_string(pBallSimulation->ballsimulation->positionball().y())
									+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
									+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
								m_pHost->ToLog(hack_log.c_str());

								bResult = false;
							}
						}
						else
						{
							string hack_log = "[HACK_CHECK] shadowSteal_unable_to_distance_min RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
								+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
								+ ", distanceToBall : " + std::to_string(distanceToBall)
								+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
								+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
							m_pHost->ToLog(hack_log.c_str());

							bResult = false;
						}
					}
					else
					{
						string hack_log = "[HACK_CHECK] shadowSteal_unable_to_distance_max RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
							+ ", distanceToBall : " + std::to_string(distanceToBall)
							+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
							+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
						m_pHost->ToLog(hack_log.c_str());

						bResult = false;
					}
				}
				else if (pAction->actionType == F4PACKET::ACTION_TYPE::action_intercept)
				{
					if (pBallSimulation->ballsimulation->ballnumber() > kBALL_NUMBER_INIT && m_pHost->GetCharacterBallEventSuccess(pBallSimulation->ballsimulation->owner()) == pBallSimulation->ballsimulation->ballnumber())
					{
						bResult = true;
					}
					else
					{
						string hack_log = "[HACK_CHECK] action_intercept_unable RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
							+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
							+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
							+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
						m_pHost->ToLog(hack_log.c_str());

						bResult = false;
					}
				}
				else
				{
					string hack_log = "[PLAY_C2S_BALLSIMULATION] ballState_is_not_defined RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
						+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
						+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate())
						+ ", ActionType : " + F4PACKET::EnumNameACTION_TYPE(pAction->actionType);
					m_pHost->ToLog(hack_log.c_str());

					bResult = true;
				}
			}
			break;
			default:
			{
				//! ballState_pickLoose ���� ���� ó���� ���ؼ� �ϴ� ��ŵ

				/*string hack_log = "[PLAY_C2S_BALLSIMULATION] ballState_is_invalid RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(userid)
					+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
					+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber())
					+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallSimulation->ballsimulation->ballstate());
				m_pHost->ToLog(hack_log.c_str());

				bResult = false;*/
			}
			break;
		}
		

		if (bResult)
		{
			if (m_pHost->BallNumberGet() <= pBallSimulation->ballsimulation->ballnumber())
			{
				if (m_pHost->BallControllerGet()->IsBeforeShotEventTime(pBallSimulation->ballsimulation->time()))
				{
					m_pHost->BallNumberSet(pBallSimulation->ballsimulation->ballnumber() + 1);
					pBallSimulation->ballsimulation->mutate_ballnumber(m_pHost->BallNumberGet());

					m_pHost->SetBallOwner(pBallSimulation->ballsimulation->owner());

					SBallActionInfo sInfo;
					sInfo.BallNumber = m_pHost->BallNumberGet();
					sInfo.BallOwner = m_pHost->GetBallOwner();
					sInfo.BallOwnerTeam = m_pHost->ConvertToStringTeam(m_pHost->GetCharacterTeam(sInfo.BallOwner));
					sInfo.Ballstate = m_pHost->ConvertToEnumBallState(pBallSimulation->ballsimulation->ballstate());
					sInfo.bAssist = false;
					sInfo.fGameTime = m_pHost->GetGameTime();
					sInfo.SkillIndex = (DHOST_TYPE_UINT32)pBallSimulation->ballsimulation->skillindex();

					m_pHost->PushBallActionInfoDeque(sInfo);

					//! ��ĳ ��� ����
					if (pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatch || pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatchPass)
					{
						m_pHost->UpdateRecord(pBallSimulation->ballsimulation->owner(), m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::DIVING_CATCH);

						if (pBallSimulation->ballsimulation->skillindex() == F4PACKET::SKILL_INDEX::skill_divingCatchPass)
						{
							if (pBallSimulation->ballsimulation->target() == pBallSimulation->ballsimulation->owner())
							{
								m_pHost->IncreaseSkillSucCount(pBallSimulation->ballsimulation->owner(), (DHOST_TYPE_UINT32)SKILL_INDEX::skill_divingCatch);
							}
						}
					}

					m_pHost->UpdateRecordBasicTrainingSystem(pBallSimulation->ballsimulation->owner(), sInfo.BallNumber, (F4PACKET::SKILL_INDEX)sInfo.SkillIndex);
					m_pHost->SetCharacterActionOverlap(pBallSimulation->ballsimulation->owner(), ACTION_TYPE::action_pick, SKILL_INDEX::skill_none);
					m_pHost->AddCharacterActionDeque(pBallSimulation->ballsimulation->owner());

					CBallController::GameBallData* pGameBallData = new CBallController::GameBallData();

					if (reboundAbleFlag)
					{
						pGameBallData->checkReboundValid = true;
					}

					pGameBallData->ballNumber = m_pHost->BallNumberGet();
					pGameBallData->ballState = pBallSimulation->ballsimulation->ballstate();
					pGameBallData->shotType = SHOT_TYPE::shotType_none;
					pGameBallData->owner = pBallSimulation->ballsimulation->owner();
					pGameBallData->point = 0;
					pGameBallData->team = m_pHost->GetCharacterTeam(pBallSimulation->ballsimulation->owner());
					pGameBallData->ballEventMap.insert(pair<string, float>("Event_ballSimulation", pBallSimulation->ballsimulation->time()));
					m_pHost->BallControllerGet()->AddBallData(data->ballsimulation(), pGameBallData, pBallSimulation->ballsimulation->time());

					bResult = true;
				}
				else
				{
					bResult = false;
				}
			}
			else
			{
				pBallSimulation->ballsimulation->mutate_ballnumber(m_pHost->BallNumberGet());
				bResult = false;
			}
		}
	}
	else
	{
		string verify_log = "[PLAY_C2S_BALLSIMULATION] action_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) 
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(pBallSimulation->ballsimulation->owner())
			+ ", BallNo : " + std::to_string(pBallSimulation->ballsimulation->ballnumber());
		m_pHost->ToLog(verify_log.c_str());

		bResult = false;
	}

	if (bResult == false)
	{
		pBallSimulation->ballsimulation->mutate_success(bResult);
	}

	
	if (m_pHost->CheckPositionIsNan( &pBallSimulation->ballsimulation->positionball()))
	{
		string log = "[HACK_CHECK] BALLSIMULATION Ball is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", RoomID : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid);

		m_pHost->ToLog(log.c_str());


		return false;
	}

	//pBallSimulation->ballsimulation->positionball().x();
	//pBallSimulation->ballsimulation->positionball().y();
	//pBallSimulation->ballsimulation->positionball().z();


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_ballSimulation, message, send_data);
	send_data.add_ballsimulation(pBallSimulation->ballsimulation.get());
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_environmentCat(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_environmentCat_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->positioncat() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER positioncat is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	TB::SVector3 positioncat(data->positioncat()->x(), data->positioncat()->y(), data->positioncat()->z());

	return m_pHost->EnvironmentCatSurprised(data->number(), positioncat);
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_onFireModeInfo(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_onFireModeInfo_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_pHost->UpdateCharacterOnFireModeStatBuff(data->characterid());

	return true;
}


const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerAlleyOopCut(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerAlleyOopCut_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionowner() == nullptr || data->positionball() == nullptr || data->positionballblockevent() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());

		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_alleyOopCut, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerAlleyOopCut;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerAlleyOopCut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_ownerid(data->ownerid());
	send_data.add_positionowner(data->positionowner());
	send_data.add_positionball(data->positionball());
	send_data.add_success(data->success());
	send_data.add_passtargetid(data->passtargetid());
	send_data.add_positionballblockevent(data->positionballblockevent());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerAlleyOopPass(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerAlleyOopPass_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->targetball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());

		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_alleyOopPass, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerAlleyOopPass;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerAlleyOopPass, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_positionball(data->positionball());
	send_data.add_targetball(data->targetball());
	send_data.add_durationarrive(data->durationarrive());
	send_data.add_targetid(data->targetid());
	send_data.add_mirror(data->mirror());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerAlleyOopShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerAlleyOopShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->secondlocalpositionball() == nullptr || data->slidepositiona() == nullptr || data->slidepositionb() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	DHOST_TYPE_INT32 potentialfxlevel = kINT32_INIT;
	DHOST_TYPE_BALL_NUMBER PotentialBloom = kBALL_NUMBER_INIT;
	SHOT_TYPE shotType = SHOT_TYPE::shotType_layUpDriveIn;

	if (data->playeraction()->skillindex() == SKILL_INDEX::skill_selfAlleyOop)
	{
		if (m_pHost->GetAnimationController() != nullptr)
		{
			std::string animName = m_pHost->GetAnimationController()->GetAnimationName(data->animsecondid());

			if (animName.find("DUNK") != string::npos)
			{
				shotType = SHOT_TYPE::shotType_dunkDriveIn;
			}

			SPotentialInfo sInfo;
			DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(data->playeraction()->id(), ACTION_TYPE::action_shot, shotType, data->playeraction()->skillindex(), sInfo);
			if (bPotential)
			{
				//! ������ �ƴٸ�
				if (sInfo.potentialawaken())
				{
					//! �̾ƿ� ���� ��ȭ �ߵ� Ȯ������ ���ٸ� ��ȭ ������ߵ�
					DHOST_TYPE_FLOAT random_value = m_pHost->GetRandomValueAlgorithm(userid, false);

					if (random_value <= sInfo.potentialbloomratevalue())
					{
						PotentialBloom = data->ballnumber();
						potentialfxlevel = (sInfo.potentialbloombufflevel() >= 5 && sInfo.potentialbloomratelevel() >= 5) ? 2 : 1;

						{
							string verify_log = "[POTENTIAL_SHOT_ACTION] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
								+ ", BallNo : " + std::to_string(data->ballnumber())
								+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
								+ ", PotentialLv : " + std::to_string(sInfo.potentiallevel())
								+ ", PotentialValue : " + std::to_string(sInfo.potentialvalue())
								+ ", awaken : " + std::to_string(sInfo.potentialawaken())
								+ ", BloomRateLv : " + std::to_string(sInfo.potentialbloomratelevel())
								+ ", BloomRateValue : " + std::to_string(sInfo.potentialbloomratevalue())
								+ ", BloomBuffLv : " + std::to_string(sInfo.potentialbloombufflevel())
								+ ", BloomBuffValue : " + std::to_string(sInfo.potentialbloombuffvalue())
								+ ", potentialfxlevel : " + std::to_string(potentialfxlevel);
							m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
						}
					}
				}
			}
		}

	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_alleyOopShot, pData, kFLOAT_INIT, shotType, PotentialBloom);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerAlleyOopShot;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerAlleyOopShot, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animfirstid(data->animfirstid());
	send_data.add_animsecondid(data->animsecondid());
	send_data.add_ismirror(data->ismirror());
	send_data.add_positionball(data->positionball());
	send_data.add_successin(data->successin());
	send_data.add_successout(data->successout());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_secondlocalpositionball(data->secondlocalpositionball());
	send_data.add_slidepositiona(data->slidepositiona());
	send_data.add_slidepositionb(data->slidepositionb());
	send_data.add_slideyawb(data->slideyawb());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(potentialfxlevel);
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerBlock(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerBlock_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionowner() == nullptr || data->positionball() == nullptr || data->positionballblockevent() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	else
	{

		if (m_pHost->CheckPositionIsNan(data->positionowner()))
		{
			string invalid_buffer_log = "[HACK_CHECK] Position is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}

		if (m_pHost->CheckPositionIsNan(data->positionball()))
		{
			string invalid_buffer_log = "[HACK_CHECK] playerShot Block BallPosition is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}
	}

	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::persistent_post_def);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_block, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//! �ϳ��� �ɸ��ٸ� ���� ó��
	DHOST_TYPE_BOOL bResult = data->success();
	if (bSpeedHackCheck == false || bResult == false || bVerifySkill == false)
	{
		bResult = false;
		m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());
	}


	if (data->success())
	{
		if (bResult == false)
		{
			string verify_log1 = "[bResult] value RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid)
				+ ", Failed !!! : ";

			m_pHost->ToLog(verify_log1.c_str());
		}
	}

	CCharacter* character = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (character)
	{
		character->OnPlayerBlock(data->playeraction()->id(), 0, bResult);
	}

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerBlock;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerBlock, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_ownerid(data->ownerid());
	send_data.add_positionowner(data->positionowner());
	send_data.add_positionball(data->positionball());
	send_data.add_positionballblockevent(data->positionballblockevent());
	send_data.add_success(bResult);
	send_data.add_mirror(data->mirror());
	send_data.add_strong(data->strong());
	send_data.add_passtargetid(data->passtargetid());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_hostpermit(true);
	send_data.add_normal(bNormal);
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	send_data.add_inputdirection(data->inputdirection());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerBuff(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerBuff_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	SBallActionInfo last_ball_data;
	SBallActionInfo passer_data;

	int ball_number = m_pHost->BallNumberGet();
	int pre_ball_number = ball_number - 1;

	if (m_pHost->FindTargetBallActionInfo(ball_number, last_ball_data))
	{
		if (last_ball_data.BallNumber == ball_number && data->id() == last_ball_data.BallOwner)
		{
			if (m_pHost->FindTargetBallActionInfo(pre_ball_number, passer_data))
			{
				if (passer_data.BallNumber == pre_ball_number && last_ball_data.BallOwnerTeam == passer_data.BallOwnerTeam && (passer_data.Ballstate == EBALL_STATE::PASS || passer_data.Ballstate == EBALL_STATE::PASS_BLOCK || passer_data.Ballstate == EBALL_STATE::PASS_LOOSE || passer_data.Ballstate == EBALL_STATE::PASS_NORMAL || passer_data.Ballstate == EBALL_STATE::PASS_REBOUND))
				{
					if (data->type() == CHARACTER_BUFF_TYPE::NicePass)
					{
						m_pHost->UpdateRecord(passer_data.BallOwner, pre_ball_number, ECHARACTER_RECORD_TYPE::NICE_PASS);
					}
				}
			}
		}
	}
	
	// �н� �޴� �� , ���̽� �н� ������ �ƴ� ��Ȳ������ ������ �Ͽ�, �̰����� ���� ó�� 
	map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();

	for (auto it = characters.begin(); it != characters.end(); ++it)
	{
		CCharacter* character = it->second;
		if (character)
			character->OnPlayerPass(passer_data.BallOwner, last_ball_data.BallOwner, data->type());
	}


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerBuff, message, send_data);
	send_data.add_id(data->id());
	send_data.add_type(data->type());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCeremony(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCeremony_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	static unsigned int ceremonyBallNumber = 0;

	static unsigned int waterBottleCount = 0;

	const SPlayerAction* pData = data->playeraction();

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_ceremony, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//! �� ���÷� ���ų�, ��ũ�ڽ��� ���� �ε������� ����
	if (data->ceremonyid() > 0)
	{
		m_pHost->IncreaseCeremonyCount(data->playeraction()->id(), (DHOST_TYPE_INT32)data->ceremonyid());
	}

	if (data->ceremonytype() >= F4PACKET::CEREMONY_TYPE::ceremonyType_touchJukebox && data->ceremonytype() < F4PACKET::CEREMONY_TYPE::ceremonyType_ownerWin) // ��Ʈ���� NPC���� ������� 
	{
		if (ceremonyBallNumber == m_pHost->BallNumberGet())
		{
			return false;
		}
		ceremonyBallNumber = m_pHost->BallNumberGet();

		/*
		if (data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_touchWaterBottle)
		{
			float random = abs(Util::GetRandom(1.0f));
			if (random < 0.0f)
			{
				m_pHost->GetCharacterManager()->OnCharacterPlayerCeremony(data->playeraction()->id(), pPacket, recv_time);
			}
		}*/
	}

	m_pHost->GetCharacterManager()->OnCharacterPlayerCeremony(data->playeraction()->id(), pPacket, recv_time);

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCeremony;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCeremony, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ceremonytype(data->ceremonytype());
	send_data.add_animid(data->animid());
	send_data.add_playpreviousanimation(data->playpreviousanimation());
	send_data.add_ceremonyid(data->ceremonyid());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCloseOut(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCloseOut_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_closeOut, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCloseOut;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCloseOut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirror(data->ismirror());
	send_data.add_yawlook(data->yawlook());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerChaseContest(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerChaseContest_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_chaseContest, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition serverPosition;
	serverPosition.packetID = PACKET_ID::play_c2s_playerChaseContest;
	serverPosition.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), serverPosition);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerChaseContest, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerHit(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerHit_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->collisionposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER collisionposition is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! 
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCeremony;
	m_pHost->AddServerPostionDeque(data->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerHit, message, send_data);
	send_data.add_id(data->id());
	send_data.add_actiontype(data->actiontype());
	send_data.add_collisionid(data->collisionid());
	send_data.add_collisionposition(data->collisionposition());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCollision(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCollision_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	CCharacter* pTargetCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->targetid());

	if (pCharacter == nullptr || pTargetCharacter == nullptr)
	{
		return false;
	}

	//-----------------------------------------------------------------------------
	ServerCharacterPosition serverPosition;
	serverPosition.packetID = PACKET_ID::play_c2s_playerCollision;
	serverPosition.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), serverPosition);
	//m_pHost->AddServerPostionDeque(data->targetid(), serverPosition);
	//-------------------------------------------------------------------------------------

	F4PACKET::ACTION_TYPE character_action_type = pCharacter->GetCharacterAction()->actionType;
	F4PACKET::ACTION_TYPE target_character_action_type = pTargetCharacter->GetCharacterAction()->actionType;

	// �����ְ� �����϶�
	if (character_action_type == F4PACKET::ACTION_TYPE::action_alleyOopShot || character_action_type == F4PACKET::ACTION_TYPE::action_shotDunk
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shot || character_action_type == F4PACKET::ACTION_TYPE::action_shotJump
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shotLayUp || character_action_type == F4PACKET::ACTION_TYPE::action_shotPost)
	{
		string collision_log = "[play_c2s_playerCollision] off owner return false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(collision_log.c_str());

		return false;
	}

	/*if (character_action_type == F4PACKET::ACTION_TYPE::action_alleyOopShot || character_action_type == F4PACKET::ACTION_TYPE::action_shotDunk
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shot || character_action_type == F4PACKET::ACTION_TYPE::action_shotJump
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shotLayUp || character_action_type == F4PACKET::ACTION_TYPE::action_shotPost)
	{
		if (target_character_action_type == F4PACKET::ACTION_TYPE::action_moveFaceUp || target_character_action_type == F4PACKET::ACTION_TYPE::action_standFaceUp
			|| target_character_action_type == F4PACKET::ACTION_TYPE::action_collision || target_character_action_type == F4PACKET::ACTION_TYPE::action_stand || target_character_action_type == F4PACKET::ACTION_TYPE::action_standDefense)
		{
			string collision_log = "[play_c2s_playerCollision] off owner return false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(collision_log.c_str());

			return false;
		}
	}*/

	// �����ְ� ������ ��
	if (target_character_action_type == F4PACKET::ACTION_TYPE::action_alleyOopShot || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotDunk
		|| target_character_action_type == F4PACKET::ACTION_TYPE::action_shot || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotJump
		|| target_character_action_type == F4PACKET::ACTION_TYPE::action_shotLayUp || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotPost)
	{
		string collision_log = "[play_c2s_playerCollision] def owner return false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(collision_log.c_str());

		return false;
	}

	/*if (character_action_type == F4PACKET::ACTION_TYPE::action_moveFaceUp || character_action_type == F4PACKET::ACTION_TYPE::action_standFaceUp
		|| character_action_type == F4PACKET::ACTION_TYPE::action_collision || character_action_type == F4PACKET::ACTION_TYPE::action_stand || character_action_type == F4PACKET::ACTION_TYPE::action_standDefense)
	{
		if (target_character_action_type == F4PACKET::ACTION_TYPE::action_alleyOopShot || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotDunk
			|| target_character_action_type == F4PACKET::ACTION_TYPE::action_shot || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotJump
			|| target_character_action_type == F4PACKET::ACTION_TYPE::action_shotLayUp || target_character_action_type == F4PACKET::ACTION_TYPE::action_shotPost)
		{
			string collision_log = "[play_c2s_playerCollision] def owner return false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(collision_log.c_str());

			return false;
		}
	}*/

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_collision, pData, kFLOAT_INIT);

	if (data->type() == COLLISION_TYPE::screen)
	{
		m_pHost->SetCharacterActionOverlap(data->playeraction()->id(), ACTION_TYPE::action_screen, data->playeraction()->skillindex());

		// �޴� ����� ���� �ð� ���. 20211214. kin �ۼ�.
		F4PACKET::SPlayerInformationT* pCharacterInformation = m_pHost->FindCharacterInformation(data->playeraction()->id());
			
		if (FindMedalInfo(pCharacterInformation, MEDAL_INDEX::medal_pickAndRollerDunk) >= 0
			|| FindMedalInfo(pCharacterInformation, MEDAL_INDEX::medal_pickAndRollerLayUp) >= 0
			|| FindMedalInfo(pCharacterInformation, MEDAL_INDEX::medal_pickAndPoperShoot) >= 0)
		{
			m_coolTimer->bookMark(CCoolTimer::TYPE::SCREEN_COLLISION);
		}
	}

	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCollision, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_targetid(data->targetid());
	send_data.add_type(data->type());
	send_data.add_mirror(data->mirror());
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCrossOver(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCrossOver_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());
	
	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_crossOverPenetrate, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCrossOver;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCrossOver, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animidmain(data->animidmain());
	send_data.add_animidsub(data->animidsub());
	send_data.add_islefthanded(data->islefthanded());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCutIn(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCutIn_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_cutIn, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCutIn;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCutIn, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_yawdelta(data->yawdelta());
	send_data.add_islefthanded(data->islefthanded());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerDash(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerDash_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerStunIntercept;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-----------------------------------------------------------------------

	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	CCharacter* character = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (character)
	{
		character->OnPlayerDash(data->playeraction()->id());
	}

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::dash, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder);
	CREATE_FBPACKET(builder, play_s2c_playerDash, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_islefthanded(data->islefthanded());
	send_data.add_ismirroranim(data->ismirroranim());
	
	STORE_FBPACKET(builder, message, send_data);
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerVCut(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerVCut_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerVCut;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------

	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_vCut, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerVCut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_yawdelta(data->yawdelta());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerGiveAndGoMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerGiveAndGoMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerGiveAndGoMove;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------

	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::giveAndGo_Move, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerGiveAndGoMove, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerOneDribblePullUpJumperMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerOneDribblePullUpJumperMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerOneDribblePullUpJumperMove;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-----------------------------------------------------------------------
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::oneDribblePullUpJumper_Move, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerOneDribblePullUpJumperMove, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_direction(data->direction());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerEscapeDribble(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerEscapeDribble_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerEscapeDribble;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-----------------------------------------------------------------------


	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::experienced_penetrate);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::escapeDribble, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerEscapeDribble, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_direction(data->direction());
	send_data.add_count(data->count());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerShotInterfere(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerShotInterfere_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerShotInterfere;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-----------------------------------------------------------------------
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::shotInterfere, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerShotInterfere, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_positionball(data->positionball());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_fxDisplayPotential(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_fxDisplayPotential_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_fxDisplayPotential, message, send_data);
	send_data.add_id(data->id());
	send_data.add_potentialindex(data->potentialindex());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerShammgod(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerShammgod_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerShammgod;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-----------------------------------------------------------------------
	
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::shammgod, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerShammgod, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_direction(data->direction());
	send_data.add_lefthand(data->lefthand());
	send_data.add_animid(data->animid());
	send_data.add_penetratecount(data->penetratecount());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerIllegalScreen(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerIllegalScreen_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	DHOST_TYPE_INT32 CharacterID = kINT32_INIT;

	//! �ϸ��� ��ũ�� �ߵ� ���ǿ� ���Ӹ�� �߰�
	if (m_pHost->GetModeType() == EMODE_TYPE::THREE_ON_THREE || m_pHost->GetModeType() == EMODE_TYPE::CUSTOM)
	{
		//! ���� ���� ���°� �����̶�� �ϸ��� ��ũ���� �ߵ����� �ʰ� �Ѵ�.
		if (m_pHost->CheckCurrentBallStateShot() == false)
		{
			if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY && m_pHost->SetTeamIllegalScreen(data->id(), true))
			{
				CharacterID = data->id();
			}
		}
	}

	string str = "play_s2c_playerIllegalScreen data.id : " + std::to_string(data->id()) + ", CharacterID : " + std::to_string(CharacterID);
	m_pHost->SendDebugMessage(str.c_str());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerIllegalScreen, message, send_data);
	send_data.add_id(CharacterID);
	STORE_FBPACKET(builder, message, send_data)

	if (CharacterID != kINT32_INIT)
	{
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
	else
	{
		m_pHost->SendPacket(message, userid);
	}
	
	
	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerDoubleClutch(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerDoubleClutch_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}


	if (data->skillindex() == F4PACKET::SKILL_INDEX::skill_doubleCluch)
	{
		if (m_pHost->GetSecondAnimationReceivePacket(data->id()))
		{
			string invalid_buffer_log = "already receive packet RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->syncedposition() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! 

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerDoubleClutch;
	position.positionlogic = *data->syncedposition();
	m_pHost->AddServerPostionDeque(data->id(), position);
	//-------------------------------------------------------------------------

	m_pHost->OnCharacterMessage(data->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), 1);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerDoubleClutch, message, send_data);
	send_data.add_id(data->id());
	send_data.add_animid(data->animid());
	send_data.add_skillindex(data->skillindex());
	send_data.add_syncedposition(data->syncedposition());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_inputdirection(data->inputdirection());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerDenyDefense(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerDenyDefense_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), recv_time, ACTION_TYPE::action_denyDefense, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerDenyDefense;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerDenyDefense, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_positionball(data->positionball());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerFakeShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerFakeShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_fakeShot, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerFakeShot;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerFakeShot, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_enabledribble(data->enabledribble());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerGoAndCatch(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerGoAndCatch_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_goAndCatch, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition serverPosition;
	serverPosition.packetID = PACKET_ID::play_c2s_playerGoAndCatch;
	serverPosition.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), serverPosition);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerGoAndCatch, message, send_data);
	send_data.add_playeraction(data->playeraction());
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerCatchAndShotMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerCatchAndShotMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::catchAndShot_Move, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerCatchAndShotMove;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerCatchAndShotMove, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerInYourFace(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerInYourFace_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
    
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerInYourFace;
	m_pHost->AddServerPostionDeque(data->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerInYourFace, message, send_data);
	send_data.add_id(data->id());
	send_data.add_targetid(data->targetid());
	send_data.add_animid(data->animid());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerStunIntercept(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStunIntercept_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! 

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerStunIntercept;
	m_pHost->AddServerPostionDeque(data->id(), position);
	//-----------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerStunIntercept, message, send_data);
	send_data.add_id(data->id());
	send_data.add_targetid(data->targetid());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerHandCheck(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerHandCheck_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());
	
	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_handCheck, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerHandCheck;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerHandCheck, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_positionball(data->positionball());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerHookHook(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerHookHook_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());
	
	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_hookHook, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerHandCheck;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerHandCheck, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_positionball(data->positionball());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerHopStep(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerHopStep_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_hopStep, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerHopStep;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerHopStep, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerIntercept(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerIntercept_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_intercept, pData, kFLOAT_INIT);
	
	//! �μ��� �����ߴٸ� �� �̺�Ʈ ����ó��
	if (data->successtype() == 1 || data->successtype() == 2)
	{
		m_pHost->SetCharacterBallEventSuccess(data->playeraction()->id(), data->ballnumber());
	}

	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerIntercept;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerIntercept, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_successtype(data->successtype());
	send_data.add_yawdest(data->yawdest());
	send_data.add_positionball(data->positionball());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerJumpBallTapOut(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerJumpBallTapOut_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_jumpBallTapOut, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	DHOST_TYPE_BOOL bResult = data->success();

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerJumpBallTapOut;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerJumpBallTapOut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_positionball(data->positionball());
	send_data.add_animid(data->animid());
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_success(bResult);
	send_data.add_mirror(data->mirror());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerModificationAbility(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerModificationAbility_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playermodificationabilities() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playermodificationabilities is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	play_c2s_playerModificationAbility_dataT unPackData;
	data->UnPackTo(&unPackData);
	play_c2s_playerModificationAbility_dataT* pData = &unPackData;

	if (pData == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pData is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	CREATE_BUILDER(builder);

	std::vector< flatbuffers::Offset<F4PACKET::SPlayerModificationAbility>> vecModificationAbilities;
	CAbility* pTargetAbility = m_pHost->GetCharacterManager()->GetCharacter(pData->playerid)->GetAbility();

	if (pTargetAbility == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER pTargetAbility is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	for (int i = 0; i < pData->playermodificationabilities.size(); i++)
	{
		F4PACKET::SPlayerModificationAbilityBuilder modificationAbilityBuilder(builder);

		F4PACKET::ABILITY_TYPE abilityType = pData->playermodificationabilities[i]->abilitytype;
		float abilityValue = pData->playermodificationabilities[i]->value;

		// host�� ����.
		pTargetAbility->SetModificationAbility(abilityType, abilityValue);

		// ������ �߰�
		modificationAbilityBuilder.add_abilitytype(abilityType);
		modificationAbilityBuilder.add_value(abilityValue);

		vecModificationAbilities.push_back(modificationAbilityBuilder.Finish());
	}

	auto offsetcountinfo = builder.CreateVector(vecModificationAbilities);

	
	CREATE_FBPACKET(builder, play_s2c_playerModificationAbility, message, databuilder);
	databuilder.add_playerid(pData->playerid);
	databuilder.add_playermodificationabilities(offsetcountinfo);

	STORE_FBPACKET(builder, message, databuilder);

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	else
	{
		const TB::SVector3 client_pos = data->playeraction()->positionlogic();
		if (m_pHost->CheckPositionIsNan(&client_pos))
		{
			string invalid_buffer_log = "[HACK_CHECK] Move Position is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}

		if (isnan(data->speed()) || isinf(data->speed()))
		{
			string invalid_buffer_log = "[HACK_CHECK] Move Speed is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}
	}


	DHOST_TYPE_BOOL bSpeedHackCheck = false;
	DHOST_TYPE_BOOL bNormal = true;
	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter)
	{
		DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(data->playeraction()->positionlogic());
		if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

			if (gap > kCHARACTER_POSITION_CORRECT)
			{
				F4PACKET::SPlayerInformationT* pCharacterInfo = pCharacter->GetCharacterInformation();
				CHostUserInfo* pHostUser = m_pHost->FindUser(pCharacterInfo->userid);

				if (pHostUser)
				{

					if (pHostUser->GetPingAverage() < 0.1f) // �α״� �Ʋ��� 
					{
						if (pHostUser->GetCurFPS() > 60.0f) // �α״� �Ʋ��� 
						{
							string position_log = "[HACK_CHECK] play_c2s_playermove GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
								",   UserID : " + std::to_string(userid)
								+ ", POSITION_CORRECT: " + std::to_string(kCHARACTER_POSITION_CORRECT)
								+ ", Ping: " + std::to_string(pHostUser->GetPingAverage())
								+ ", FPS: " + std::to_string(pHostUser->GetCurFPS())
								+ ", GAP : " + std::to_string(gap);

							m_pHost->ToLog(position_log.c_str());
						}
					}
					
					pHostUser->SpeedHackDetected(SPEEDHACK_CHECK::MOVE);
					bSpeedHackCheck = true;
					pCharacter->SendSeverPositionToClient(data->playeraction()->directioninput());
				}

			}
			else
			if (gap > pCharacter->GetValidMoveRange())
			{
				F4PACKET::SPlayerInformationT* pCharacterInfo = pCharacter->GetCharacterInformation();
				CHostUserInfo* pHostUser = m_pHost->FindUser(pCharacterInfo->userid);

				/* �ϴ� ���������� 
				string position_log = "[HACK_CHECK] play_c2s_playermove GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
					",   UserID : " + std::to_string(userid)
					+ ", VlaidMoveRange: " + std::to_string(validMoveRange)
					+ ", GAP : " + std::to_string(gap);

				m_pHost->ToLog(position_log.c_str());

				pHostUser->SpeedHackDetected(SPEEDHACK_CHECK::MOVE);
				*/

				//pCharacter->SendSeverPositionToClient(data->playeraction()->directioninput());
				//bSpeedHackCheck = true;
			}
		}
	}
	else
	{
		return false;
	}


	TB::SVector3 serverPosition;

	F4PACKET::play_c2s_playerMove_dataT temp_data;
	data->UnPackTo(&temp_data);

	F4PACKET::play_c2s_playerMove_dataT* pTemp_data = &temp_data;
	F4PACKET::SPlayerAction* pAction = nullptr;
	pAction = pTemp_data->playeraction.get();

	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		if (!bSpeedHackCheck)
		{
			bSpeedHackCheck = m_pHost->CheckSpeedHackBothCase(userid,
				data->playeraction()->id(),
				recv_time,
				data->clienttime(),
				pAction->positionlogic(),
				data->speed(),
				serverPosition,
				bNormal);
		}
	}

	const SPlayerAction* pFixedAction = data->playeraction();

	if (pCharacter)
	{
		F4PACKET::PACKET_ID str = pCharacter->GetCurrentAction();

		//if (str == F4PACKET::PACKET_ID::play_c2s_playerFakeShot)// Ʈ���� ������ ���¿����� , ���簡 ������ũ�� �ص� play_c2s_playerFakeShot �̱��� 
		if (str == F4PACKET::PACKET_ID::play_c2s_playerPivot) 
		{
			int preState = (int)pCharacter->GetPreCharacterCurrentState();

			//if (data->runmode() == F4PACKET::MOVE_MODE::dribble && !pCharacter->IsTripleThreat())
			if (data->runmode() == F4PACKET::MOVE_MODE::dribble)
			{
				F4PACKET::SPlayerInformationT* pCharacterInformation = m_pHost->FindCharacterInformation(data->playeraction()->id());
				if (pCharacterInformation)
				{
					F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pCharacterInformation->characterid / 100);

					string position_log = "[HACK_CHECK] FAKESHOT move wrong : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
						+ ", UserID : " + std::to_string(userid)
						+ ", Name : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
						+ ", RoomID : " + m_pHost->GetHostID()
						+ ", data->runmode() : " + std::to_string((int)data->runmode())
						+ ", PreState: " + std::to_string((int)pCharacter->GetPreCharacterCurrentState());

					deque<SCharacterAction> deque = m_pHost->GetCharacterActionDeque(data->playeraction()->id());

					m_pHost->ToLog(position_log.c_str());
				}
			}
		}

		if (data->runmode() == F4PACKET::MOVE_MODE::dribble)
		{
			pCharacter->SetTripleThreat(false);
		}
	}

	DHOST_TYPE_FLOAT speed = data->speed();
	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		CBallController* pBallController = m_pHost->BallControllerGet(); 
		if (pBallController != nullptr)
		{
			if (!pBallController->CheckBallStateShot()) // ���ٿ�� ��Ȳ�� �ƴ� ��쿡���� üũ( ���ص� �������, ���� ���Ŀ��� MOVEMODE �� ofence �� ���� ) 
			{
			}
		}
	}

	if (bSpeedHackCheck)
	{
		//pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			TB::SVector3 server_pos = CommonFunction::ConvertJoVectorToTBVector(pCharacter->GetPosition());		
			//float gap = VECTOR3_DISTANCE(server_pos, client_pos);
			
/*
			string position_log = "[MODIFY_POSITION] play_c2s_playerMove GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
				", GAP : " + std::to_string(gap) + ", PreAction : " + F4PACKET::EnumNameACTION_TYPE(pCharacter->GetCharacterStateActionType()) +
				", Server X : " + std::to_string(server_pos.x()) + ", Server Z : " + std::to_string(server_pos.z()) +
				", Client X : " + std::to_string(pAction->positionlogic().x()) + ", Client Z : " + std::to_string(pAction->positionlogic().z());
			m_pHost->ToLog(position_log.c_str());*/
		
			pAction->mutable_positionlogic().mutate_x(server_pos.x());
			pAction->mutable_positionlogic().mutate_y(server_pos.y());
			pAction->mutable_positionlogic().mutate_z(server_pos.z());
		}
		pFixedAction = pAction;
	}


	//-----------------------
	ServerCharacterPosition position;
	if (data->runmode() == F4PACKET::MOVE_MODE::pivot)
	{
		//string position_log = "[HACK_CHECK] pivot Move";
		//m_pHost->ToLog(position_log.c_str());
		position.packetID = PACKET_ID::play_c2s_playerPivot;
	}
	else
	{
		position.packetID = PACKET_ID::play_c2s_playerMove;
	}

	position.positionlogic = pFixedAction->positionlogic();
	position.speed = speed;  //data->speed();
	position.dirYaw = pFixedAction->directioninput();
	position.clienttime = data->clienttime();
	m_pHost->AddServerPostionDeque(pFixedAction->id(), position);

	//----------------------- 
		
	m_pHost->CheckCharacterPositionSync(pFixedAction, pPacket->GetPacketID());

	// 1. ���⼭ ĳ���� ��ġ�� Ŭ����ġ�� ���ŵǹǷ�
	m_pHost->OnCharacterMessage(pFixedAction->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), pFixedAction->keys(), data->runmode());


	// 2. ���ǵ��ٿ� �ɷȴٸ� ������ġ�� ������, ����
	if (bSpeedHackCheck)
	{
		// ������ġ�� �ٽ� ���� 
		m_pHost->GetCharacterManager()->GetCharacterStateAction(pFixedAction->id())->mutable_positionlogic().mutate_x(pAction->positionlogic().x());
		m_pHost->GetCharacterManager()->GetCharacterStateAction(pFixedAction->id())->mutable_positionlogic().mutate_y(pAction->positionlogic().y());
		m_pHost->GetCharacterManager()->GetCharacterStateAction(pFixedAction->id())->mutable_positionlogic().mutate_z(pAction->positionlogic().z());
	}

	m_pHost->CheckQuestAction(pFixedAction);
	m_pHost->SetCharacterAction(pFixedAction->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_move, pFixedAction, speed /*data->speed()*/);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerMove, message, send_data);
	send_data.add_playeraction(pFixedAction);
	send_data.add_speed(speed /*data->speed()*/);
	send_data.add_runmode(data->runmode());
	send_data.add_lookid(data->lookid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_synctime(data->synctime());
	send_data.add_activemedalagilemovement(data->activemedalagilemovement());
	send_data.add_activemedalagilemovement2(data->activemedalagilemovement2());
	send_data.add_activemedpassionaterebounder(data->activemedpassionaterebounder());
	send_data.add_activemedalvacuum(data->activemedalvacuum());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_normal(bNormal);

	STORE_FBPACKET(builder, message, send_data)

	if (bSpeedHackCheck)
	{
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
	else
	{
		m_pHost->BroadcastPacket(message, userid);
	}
	
	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerReceivePass(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerReceivePass_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	bool result = false;

	// �̹� ���ͼ�Ʈ�� ���ߴٸ� �� �ѹ��� ���������״� �н����ú긦 ���� ó�� ���ش�.
	if (m_pHost->BallNumberGet() == data->ballnumber())
	{
		result = true;
	}

	F4PACKET::SPlayerInformationT* pCharacterInformation = m_pHost->FindCharacterInformation(data->playeraction()->id());
	if (FindMedalInfo(pCharacterInformation, MEDAL_INDEX::scoreSupport) >= 0
		&& result == true)
	{
		m_coolTimer->bookMark(CCoolTimer::TYPE::RECEIVE_PASS);
	}

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_receivePass, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerReceivePass;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerReceivePass, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_balltime(data->balltime());
	send_data.add_animid(data->animid());
	send_data.add_mirror(data->mirror());
	send_data.add_positionball(data->positionball());
	send_data.add_result(result);
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! 
	
	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	
	//! ĳ���� ���� ������Ʈ(��ġ �� �׼�)
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	//! ĳ���� �׼� ����(������)
	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_rebound, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//! ���ٿ�� �Ÿ�üũ
	DHOST_TYPE_BOOL bBoxouted = false;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter)
	{
		DHOST_TYPE_CHARACTER_SN nBoxoutCharacter = 0;
		if (/*data->success() && */ m_pHost->GetCharacterManager()->CheckBoxOutedBy(pCharacter, nBoxoutCharacter))
		{
			bBoxouted = true; // �ڽ��ƿ��� �ɷȴٸ�  //  !! Ŭ�󿡼� ������Ŷ�� �޴� ��� ��ü �����Ǵ��� �� �� �ܿ� ��Ŷ�����̷� ���� �������� �Ǵ��� ��쵵 ���� 

			if (nBoxoutCharacter != 0)
			{
				CCharacter* pBoxOutCharacter = m_pHost->GetCharacterManager()->GetCharacter(nBoxoutCharacter);
				if (pBoxOutCharacter)
				{
					float check_medal_data = m_pHost->CheckDoYouHaveMedal(pBoxOutCharacter->GetCharacterInformation(), F4PACKET::MEDAL_INDEX::paintZoneRuler);
					if (check_medal_data > 0.0f)
					{
						SendMedalUIDisplay(nBoxoutCharacter, F4PACKET::MEDAL_INDEX::paintZoneRuler);
					}
				}
			}
		}
	}

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}
	
	//! �ϳ��� �ɸ��ٸ� ���� ó��
	DHOST_TYPE_BOOL bResult = data->success();
	if (bSpeedHackCheck == false || bResult == false || bVerifySkill == false || bBoxouted == true)
	{
		bResult = false;
		m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());
	}


	// ���ٿ�� ��ũ�� üũ 
	CHostUserInfo* pUser = m_pHost->FindUser(userid);
	if (pUser)
	{
		if(bResult)
		{
			CBallController* pBallController = m_pHost->BallControllerGet();
			if (pBallController != nullptr)
			{
				if (pBallController->CheckBallStateShot())
				{
					if (m_pHost->bTreatDelayMode)
					{
						if (pUser)
						{
							CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
							if (pCharacter)
							{
								F4PACKET::SPlayerInformationT* pCharacterInformation = pCharacter->GetCharacterInformation();
								if (pCharacterInformation->ailevel == kIS_NOT_AI && !pCharacter->IsAICharacter()) // ������, Ʈ����, Ʃ�丮�� ������ �̷��� �߰��� , �������� �ȴٰ� F4PACKET::SPlayerInformationT* pCharacterInformation ailevel �� �������� ���� 
								{
									if (!pUser->IsValidAction())
									{
										// ������
										return false;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerRebound;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerRebound, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_positionball(data->positionball());
	send_data.add_animid(data->animid());
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_mirror(data->mirror());
	send_data.add_success(bResult);
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_hostpermit(true);
	send_data.add_normal(bNormal);
	send_data.add_boxouted(bBoxouted);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPass(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPass_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_pass, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	m_pHost->UpdateRecord(data->playeraction()->id(), m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::PASS);

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPass;
	position.positionlogic = pData->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPass, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_positionball(data->positionball());
	send_data.add_yawdest(data->yawdest());
	send_data.add_targetid(data->targetid());
	send_data.add_mirror(data->mirror());
	send_data.add_passtype(data->passtype());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPassFake(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPassFake_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	int ani_check_resultA = m_pHost->GetAnimationController()->CheckAnimationData(data->fakeanimid());

	if (ani_check_resultA < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->fakeanimid(), ani_check_resultA, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	int ani_check_resultB = m_pHost->GetAnimationController()->CheckAnimationData(data->realanimid());

	if (ani_check_resultB < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->realanimid(), ani_check_resultB, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_pass, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	m_pHost->UpdateRecord(data->playeraction()->id(), m_pHost->BallNumberGet(), ECHARACTER_RECORD_TYPE::PASS);

	CREATE_BUILDER(builder);
	CREATE_FBPACKET(builder, play_s2c_playerPassFake, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_positionball(data->positionball());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_fakeanimid(data->fakeanimid());
	send_data.add_fakeismirroranim(data->fakeismirroranim());
	send_data.add_faketargetposition(data->faketargetposition());
	send_data.add_realanimid(data->realanimid());
	send_data.add_realismirroranim(data->realismirroranim());
	send_data.add_realtargetid(data->realtargetid());
	send_data.add_passtype(data->passtype());
	
	STORE_FBPACKET(builder, message, send_data);
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPassiveDataSet(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPassiveDataSet_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (true)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->idplayer());
		if (pCharacter != nullptr)
		{
			pCharacter->InitialPassiveDataSet(data);
		}
	}
	else
	{
		return false;
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPassive(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPassive_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (true)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->idplayer());
		if (pCharacter != nullptr)
		{
			SCharacterPassive* pCharacterPassive = pCharacter->GetCharacterPassive();
			if (pCharacterPassive != nullptr)
			{
				float before = pCharacterPassive->valueCurr;

				if (m_pHost->SetPacketCharacterPassive(data->idplayer(), data) == false)
				{
					return false; // ��ŷ�� ����̰ų�, ������ �����͸� Ȱ���ϱ� ���ؼ� ��ε� ĳ������ ���ϴ� ���( �̳�, ��Ű, ġ������, ������ �̷��� �ٲ� ) 
				}
				else
				{
					//string log_message = "[PASSIVE] [BeginValidatePassiveValue] ownerID : " + std::to_string(data->idplayer()) + "Before value: " + std::to_string(before)
						//+ " After value: " + std::to_string(pCharacterPassive->valueCurr);
					//m_pHost->ToLog(log_message.c_str());
				}
			}
		}
	}
	else
	{
		return false;
	}

	m_pHost->GetCharacterManager()->OnCharacterPassive(data->idplayer(), pPacket, recv_time);

	m_pHost->OnCharacterMessage(data->idplayer(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), 0);


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);
	send_data.add_idplayer(data->idplayer());
	send_data.add_type(data->type());
	send_data.add_valuecurr(data->valuecurr());
	send_data.add_valuemax(data->valuemax());
	send_data.add_activate(data->activate());
	send_data.add_effecttrigger(data->effecttrigger());
	send_data.add_starttime(data->starttime());
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	
	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPenetrate(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPenetrate_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPenetrate;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	DHOST_TYPE_BOOL bSpeedHackCheck = false;
	DHOST_TYPE_BOOL bNormal = true;

	F4PACKET::play_c2s_playerPenetrate_dataT temp_data;
	data->UnPackTo(&temp_data);
	F4PACKET::play_c2s_playerPenetrate_dataT* pTemp_data = &temp_data;
	F4PACKET::SPlayerAction* pAction = nullptr;
	pAction = pTemp_data->playeraction.get();
	const SPlayerAction* pFixedAction = data->playeraction();

	if (bSpeedHackCheck)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			auto server_pos = pCharacter->GetPosition();	// ������ �˰��ִ� ĳ������ ��ġ�� �ٱ��� 
			auto client_pos = pAction->positionlogic();	    // ��Ŷ���� ���� ĳ������ ��ġ

			pAction->mutable_positionlogic().mutate_x(server_pos.fX);
			pAction->mutable_positionlogic().mutate_y(server_pos.fY);
			pAction->mutable_positionlogic().mutate_z(server_pos.fZ);
		}
		pFixedAction = pAction;
	}

	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	if (bSpeedHackCheck && pFixedAction != nullptr)
	{
		// ������ġ�� �ٽ� ���� 
		m_pHost->SetCharacterStateAction(data->playeraction()->id(), *pFixedAction, ACTION_TYPE::action_penetrate, recv_time);
	}

	float animSpeed = 1.0f;

	if (data->playeraction()->skillindex() == SKILL_INDEX::skill_penetrateSideStep)
	{
		animSpeed = m_pHost->GetBalanceTable()->GetValue("VPenetrateSideStep_Anim_SpeedA") + 
			m_pHost->GetBalanceTable()->GetValue("VPenetrateSideStep_Anim_SpeedB") * m_pHost->GetSkillLevel(data->playeraction()->id(), SKILL_INDEX::skill_penetrateSideStep);

	}
	if (data->playeraction()->skillindex() == SKILL_INDEX::skill_penetrateBetweenTheLegs)
	{
		animSpeed = m_pHost->GetBalanceTable()->GetValue("VBetweenTheLeg_Anim_SpeedA") + 
			m_pHost->GetBalanceTable()->GetValue("VBetweenTheLeg_Anim_SpeedB") * m_pHost->GetSkillLevel(data->playeraction()->id(), SKILL_INDEX::skill_penetrateBetweenTheLegs);
	}
	if (data->playeraction()->skillindex() == SKILL_INDEX::shotHesitation)
	{
		animSpeed = m_pHost->GetBalanceTable()->GetValue("VPenetrateShotHesition_Anim_SpeedA") +
			m_pHost->GetBalanceTable()->GetValue("VPenetrateShotHesition_Anim_SpeedB") * m_pHost->GetSkillLevel(data->playeraction()->id(), SKILL_INDEX::shotHesitation);
	}


	CCharacter* character = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (character)
	{
		character->OnPlayerPenentrate(data->playeraction()->id());
	}

	m_pHost->CheckQuestAction(pFixedAction);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPenetrate, message, send_data);
	send_data.add_playeraction(pFixedAction);
	send_data.add_animid(data->animid());
	send_data.add_animidsub(data->animidsub());
	send_data.add_direction(data->direction());
	send_data.add_count(data->count());
	send_data.add_end(data->end());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_clienttime(data->clienttime());
	send_data.add_normal(bNormal);
	send_data.add_potentialfxlevelflashypenetrate(data->potentialfxlevelflashypenetrate());
	send_data.add_potentialfxlevelexperiencedpenetrate(data->potentialfxlevelexperiencedpenetrate());
	send_data.add_animspeed(animSpeed);
	STORE_FBPACKET(builder, message, send_data)
		
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPenetratePostUp(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPenetratePostUp_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_penetratePostUp, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPenetratePostUp;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPenetratePostUp, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_lefthand(data->lefthand());
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPenetrateReady(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPenetrateReady_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	DHOST_TYPE_BOOL bNormal = true;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPenetrateReady;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	//! ���� �ɷ� ����
	if (data->playeraction()->skillindex() == SKILL_INDEX::skill_none)
	{
		// ������ ����
		EACTION_VERIFY check_verify_flashypenetrate = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevelflashypenetrate(), POTENTIAL_INDEX::flashy_penetrate);
		if (check_verify_flashypenetrate != EACTION_VERIFY::SUCCESS)
		{
			m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

			return true;
		}

		// ����� ���ڵ鸵
		EACTION_VERIFY check_verify_experiencedpenetrate = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevelexperiencedpenetrate(), POTENTIAL_INDEX::experienced_penetrate);
		if (check_verify_experiencedpenetrate != EACTION_VERIFY::SUCCESS)
		{
			m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

			return true;
		}
	}
	

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());
	m_pHost->CheckQuestAction(data->playeraction());
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_penetrateReady, data->playeraction(), kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPenetrateReady, message, send_data);
	send_data.add_playeraction(data->playeraction());
	send_data.add_animid(data->animid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_index(data->index());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevelflashypenetrate(data->potentialfxlevelflashypenetrate());
	send_data.add_potentialfxlevelexperiencedpenetrate(data->potentialfxlevelexperiencedpenetrate());
	send_data.add_clienttime(data->clienttime());
	send_data.add_normal(bNormal);
	
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, userid);


	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPick(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPick_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}


	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	SCharacterAction* pAction = m_pHost->GetCharacterActionInfo(data->playeraction()->id());

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_pick, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());


	DHOST_TYPE_BOOL bResult = data->success();

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPick;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------



	if (bSpeedHackCheck == false || bVerifySkill == false || bResult == false)
	{
		m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());

		string pick_fail_log = "[PICK_FAIL] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(data->playeraction()->id())

			+ ", SpeedHack : " + std::to_string(bSpeedHackCheck)
			+ ", VerifySkill : " + std::to_string(bVerifySkill)
			+ ", ClientSuccess : " + std::to_string(data->success())
			+ ", FinalSuccess : " + std::to_string(bResult)
			+ ", ClientTime : " + std::to_string(data->clienttime())
			+ ", CheckBallEventTime : " + std::to_string(data->checkballeventtime())
			+ ", BallNo : " + std::to_string(data->ballnumber());
		m_pHost->ToLog(pick_fail_log.c_str());
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPick, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_positionball(data->positionball());
	send_data.add_success(bResult);
	send_data.add_targetid(data->targetid());
	send_data.add_corrposition(data->corrposition());
	send_data.add_hostpermit(true);
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_normal(bNormal);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	//m_pHost->BroadcastPacket(message, userid);
		

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPickAndMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPickAndMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_pickAndMove, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPickAndMove;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPickAndMove, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_yawdelta(data->yawdelta());
	send_data.add_speed(data->speed());
	send_data.add_islefthanded(data->islefthanded());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPickAndSlip(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPickAndSlip_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_pickAndMove, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPickAndSlip;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPickAndSlip, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_islefthanded(data->islefthanded());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSyncInfo(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSyncInfo_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (true)
	{
		F4PACKET::SPlayerAction* pAction = m_pHost->GetCharacterManager()->GetCharacterStateAction(data->id());
		if (pAction != nullptr)
		{
			pAction->mutable_positionlogic().mutate_x(data->position()->x());
			pAction->mutable_positionlogic().mutate_y(data->position()->y());
			pAction->mutable_positionlogic().mutate_z(data->position()->z());
		}

		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->id());

		if (pCharacter)
		{
			SPlayerInformationT* pUser = pCharacter->GetCharacterInformation();
			CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);


			if (data->type() == (int)SYNCINFO_TYPE::PICK_DONE)
			{
				pHostUser->SetCurrentRandomKey(data->correct());
			}
			else
			if (data->type() == (int)SYNCINFO_TYPE::REBOUND_DONE)
			{
				pHostUser->SetCurrentRandomKey(data->correct());
			}


			//---------------------------------------------------------
			ServerCharacterPosition position;
			position.packetID = PACKET_ID::play_c2s_playerSyncInfo;
			position.positionlogic = *data->position();
			position.dirYaw = data->yaw();
			position.correct = data->correct();

			// ���� ��ġ ��ǥ �������� ��쿡�� 
			if (data->type() == (int)SYNCINFO_TYPE::NONE)
			{
				m_pHost->AddServerPostionDeque(data->id(), position);
			}
			//-------------------------------------------------------------------		
			
			/*
			F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
			string invalid_buffer_log = "***** playerSyncPosition RoomElapsed: " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
				+ std::to_string(pUser->userid)
				+ ", RoomID : " + m_pHost->GetHostID()
				+ ", UserName : " + pUser->name
				+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
				+ " , Ping : " + std::to_string(pHostUser->GetPingAverage())
				+ " , FPS : " + std::to_string(pHostUser->GetCurFPS());

			m_pHost->ToLog(invalid_buffer_log.c_str());*/
			
		}
	}
	else
	{
		return false;
	}


	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_versionInfo(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_versionInfo_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	string version_log = "[VERSION_CHECK] DISABLED RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
		+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
		+ ", UserID : " + std::to_string(userid);
	m_pHost->ToLog(version_log.c_str());

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPositionCorrect(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPositionCorrect_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->position() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER position is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	F4PACKET::SPlayerAction* pAction = m_pHost->GetCharacterManager()->GetCharacterStateAction(data->id());
	if(pAction != nullptr)
	{
		pAction->mutable_positionlogic().mutate_x(data->position()->x());
		pAction->mutable_positionlogic().mutate_y(data->position()->y());
		pAction->mutable_positionlogic().mutate_z(data->position()->z());
	}

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->id());

	if (pCharacter)
	{
		SPlayerInformationT* pUser = pCharacter->GetCharacterInformation();
		CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

		
		/*
		F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
		string invalid_buffer_log = "@@@ playerPositionCorrect RoomElapsed: " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
			+ std::to_string(pUser->userid)
			+ ", RoomID : " + m_pHost->GetHostID()
			+ ", UserName : " + pUser->name
			+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
			+ " , Ping : " + std::to_string(pHostUser->GetPingAverage())
			+ " , FPS : " + std::to_string(pHostUser->GetCurFPS());

		m_pHost->ToLog(invalid_buffer_log.c_str());*/
	}

	// �÷��� �߿��� ��ġ �������� ���� 
	if (m_pHost->GetCurrentState() != EHOST_STATE::READY && m_pHost->GetCurrentState() != EHOST_STATE::SCORE)
	{
		//string invalid_buffer_log = "@@@ playerPositionCorrect EHOST_STATE::PLAY @@@";
		//m_pHost->ToLog(invalid_buffer_log.c_str());

		//---------------------------------------------------------
		ServerCharacterPosition position;
		position.packetID = PACKET_ID::play_c2s_playerPositionCorrect;
		position.positionlogic = *data->position();
		position.dirYaw = data->yaw();
		position.correct = data->correct();
		m_pHost->AddServerPostionDeque(data->id(), position);
		//-------------------------------------------------------------------

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPositionCorrect, message, send_data);
		send_data.add_id(data->id());
		send_data.add_position(data->position());
		send_data.add_yaw(data->yaw());
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, userid);
	}
	/*
	else
	if (m_pHost->GetCurrentState() == EHOST_STATE::READY)
	{
		string invalid_buffer_log = "@@@ playerPositionCorrect EHOST_STATE::READY @@@";
		m_pHost->ToLog(invalid_buffer_log.c_str()); 
	}
	else
	if (m_pHost->GetCurrentState() == EHOST_STATE::SCORE)
	{
		string invalid_buffer_log = "*** playerPositionCorrect EHOST_STATE::SCORE";
		m_pHost->ToLog(invalid_buffer_log.c_str());
	}
	*/

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPostUpCollision(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPostUpCollision_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->positionplayer() == nullptr || data->positiontarget() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER positionplayer is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CAbility* pOwnerAbility = nullptr;
	CAbility* pDefenderAbility = nullptr;

	CCharacter* pOwnerCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playerid());
	CCharacter* pDefendCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->targetid());

	if (pOwnerCharacter != nullptr && pDefendCharacter != nullptr)
	{
		pOwnerAbility = pOwnerCharacter->GetAbility();
		pDefenderAbility = pDefendCharacter->GetAbility();


		{
			string collision_log = "[play_c2s_playerPostUpCollision] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid)
				+ ", playerid : " + std::to_string(data->playerid()) + ", playeridActiontype : " + F4PACKET::EnumNameACTION_TYPE(pOwnerCharacter->GetCharacterAction()->actionType)
				+ ", targetid : " + std::to_string(data->targetid()) + ", targetActiontype : " + F4PACKET::EnumNameACTION_TYPE(data->targetactiontype());
			m_pHost->ToLog(collision_log.c_str());
		}

		//if (pOwner != nullptr && pDefenser != nullptr)
		if (pOwnerAbility != nullptr && pDefenderAbility != nullptr)
		{
			int result = 0;

			float a = pOwnerAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_strength);
			float b1 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Win_ProbabilityA");
			float c1 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Win_ProbabilityB");
			float d = pDefenderAbility->GetAbility(F4PACKET::ABILITY_TYPE::abilityType_strength);
			float e1 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Win_ProbabilityC");
			float f1 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Win_ProbabilityD");
			float g1 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Win_ProbabilityE");

			float calculated = LOG(a, b1) / c1 - LOG(d, e1) / f1 + g1;

			if (Util::GetRandom(1.0f) <= calculated)
			{
				result = 1;
			}
			else
			{
				float b2 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Draw_ProbabilityA");
				float c2 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Draw_ProbabilityB");
				float e2 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Draw_ProbabilityC");
				float f2 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Draw_ProbabilityD");
				float g2 = m_pHost->GetBalanceTable()->GetValue("VPostUp_BackDown_Draw_ProbabilityE");

				calculated = LOG(a, b2) / c2 - LOG(d, e2) / f2 + g2;

				if (Util::GetRandom(1.0f) <= calculated)
				{
					result = 0;
				}
				else
				{
					result = -1;
				}
			}

			//---------------------------------------------------------
			ServerCharacterPosition position;
			position.packetID = PACKET_ID::play_c2s_playerPostUpCollision;
			position.positionlogic = *data->positionplayer();
			m_pHost->AddServerPostionDeque(data->playerid(), position);
			//-------------------------------------------------------------------

			//---------------------------------------------------------
			position.positionlogic = *data->positiontarget();
			m_pHost->AddServerPostionDeque(data->targetid(), position);
			//-------------------------------------------------------------------


			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerPostUpCollision, message, send_data);
			send_data.add_playerid(data->playerid());
			send_data.add_positionplayer(data->positionplayer());
			send_data.add_targetid(data->targetid());
			send_data.add_positiontarget(data->positiontarget());
			send_data.add_result(result);
			STORE_FBPACKET(builder, message, send_data)

			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

			return true;
		}
		else
		{
			std::string str = "play_c2s_playerPostUpCollision Ability is null RoomElapsedTime : " + std::to_string(recv_time);
			m_pHost->ToLog(str.c_str());
		}
	}
	else
	{
		std::string str = "play_c2s_playerPostUpCollision character is null RoomElapsedTime : " + std::to_string(recv_time);
		m_pHost->ToLog(str.c_str());
	}

	return false;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPostUpPenetrate(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPostUpPenetrate_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	if (data->playeraction()->skillindex() == SKILL_INDEX::skill_postUpSpin)
	{
		EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::flashy_penetrate);
		if (check_verify != EACTION_VERIFY::SUCCESS)
		{
			m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

			return true;
		}
	}
	

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_postUpPenetrate, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());


	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPostUpPenetrate;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPostUpPenetrate, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_mirror(data->mirror());
	send_data.add_lefthand(data->lefthand());
	send_data.add_direction(data->direction());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPostUpReady(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPostUpReady_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::flashy_penetrate);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_postUpReady, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPostUpReady;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPostUpReady, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_enter(data->enter());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerPostUpStepBack(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerPostUpStepBack_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_postUpReady, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerPostUpStepBack;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPostUpStepBack, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerScreen(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerScreen_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_screen, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerScreen;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerScreen, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_mirror(data->mirror());
	send_data.add_readytime(data->readytime());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSpeaking(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSpeaking_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->playerscript() == F4PACKET::PLAYER_SCRIPT::playerScript_passMe)
	{
		m_pHost->UpdateAwayFromKeyboard(data->id(), m_pHost->GetGameTime());
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSpeaking, message, send_data);
	send_data.add_id(data->id());
	send_data.add_playerscript(data->playerscript());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerShakeAndBake(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerShakeAndBake_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::flashy_penetrate);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_shakeAndBake, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerShakeAndBake;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerShakeAndBake, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_lefthand(data->lefthand());
	send_data.add_animid(data->animid());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

// ��ų�� ������ ��Ŷ���� ����, �����ΰ�? by steven 2026-02-10
const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerBehindStepBackJumperShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerBehindStepBackJumperShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	else
	{
		const TB::SVector3 client_pos = data->playeraction()->positionlogic();
		if (m_pHost->CheckPositionIsNan(&client_pos))
		{
			string invalid_buffer_log = "[HACK_CHECK] playerShot Position is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}

		if (m_pHost->CheckPositionIsNan(data->positionball()))
		{
			string invalid_buffer_log = "[HACK_CHECK] playerShot BallPosition is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}
	}


	//! ��Ŷ ���� üũ


	//! ���ο� ���ǵ��� üũ
	DHOST_TYPE_BOOL bNormal = true;

	//! ���� ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	// �ڽ��ƿ� ���� �ʱ�ȭ 
	m_pHost->GetCharacterManager()->CheckBoxOutedClear();

	DHOST_TYPE_BOOL bDistanceToFar = true;


	DHOST_TYPE_FLOAT userPing = 0.0f;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetCharacterStateAction() != nullptr)
	{
		pCharacter->OnPlayerShot(data->playeraction()->id(), data->shottype());

		CHostUserInfo* pUser = m_pHost->FindUser(userid);
		userPing = pUser->GetPingAverage();

		DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(data->playeraction()->positionlogic());
		if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

			if (gap > kCHARACTER_POSITION_CORRECT)
			{
				if (pUser->GetPingAverage() < 0.07f && pUser->GetCurFPS() > 50.0f) // ���� ��翡 ������ �غ��� 
				{
					pUser->SpeedHackDetected(SPEEDHACK_CHECK::SHOT); // IsSpeedhackDectected_High  ���� , ���ϵ� ������ �� 
				}

				string position_log = "[HACK_CHECK] [ACTION_DISTANCE_TO_FAR] playerShot RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", UserID : " + std::to_string(userid)
					+ ", BallNo : " + std::to_string(data->ballnumber())
					+ ", ValidRa: " + std::to_string(validMoveRange)
					+ ", GAP : " + std::to_string(gap)
					+ ", Ping : " + std::to_string(pUser->GetPingAverage())
					+ ", FPS : " + std::to_string(pUser->GetCurFPS());

				m_pHost->ToLog(position_log.c_str());

				pCharacter->SendSeverPositionToClient(data->playeraction()->directioninput());

				bSpeedHackCheck = false; // false �� �ɸ� ��
				bDistanceToFar = false;
			}
			else
				if (gap > validMoveRange)
				{
					CHostUserInfo* pUser = m_pHost->FindUser(userid);
					string position_log = "[HACK_CHECK] [ACTION_DISTANCE_TO_FAR] [LIGHT] playerShot RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
						+ ", UserID : " + std::to_string(userid)
						+ ", BallNo : " + std::to_string(data->ballnumber())
						+ ", ValidRa: " + std::to_string(validMoveRange)
						+ ", GAP : " + std::to_string(gap)
						+ ", Ping : " + std::to_string(pUser->GetPingAverage())
						+ ", FPS : " + std::to_string(pUser->GetCurFPS());

					m_pHost->ToLog(position_log.c_str());
				}
		}

	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}

	//! �ϳ��� �ɸ��ٸ� ���� ó��
	DHOST_TYPE_BOOL bResult = true;
	if (bSpeedHackCheck == false || bVerifySkill == false || bDistanceToFar == false)
	{
		//! �׼ǽ����� �̻��� �����Ȱ��� �������� ������ ����ó���ϰ� �Ѵ�.
		m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());
	}

	const SPlayerAction* pData = data->playeraction();

	m_pHost->CheckQuestAction(pData);

	if (pData != nullptr)
	{
		bool permit = true;

		CPhysicsHandler* physicsHandler = m_pHost->GetPhysicsHandler();

		SVector3 rimPosition = physicsHandler->GetRimPosition();

		float yaw = GetYaw(rimPosition.x() - data->playeraction()->positionlogic().x(), rimPosition.z() - data->playeraction()->positionlogic().z());

		//! ���� �ɷ� ����
		DHOST_TYPE_INT32 potentialfxlevel = kINT32_INIT;
		DHOST_TYPE_BALL_NUMBER PotentialBloom = kBALL_NUMBER_INIT;


		F4PACKET::play_c2s_playerBehindStepBackJumperShot_dataT temp_data;
		data->UnPackTo(&temp_data);
		F4PACKET::SPlayerAction* pAction = nullptr;
		pAction = temp_data.playeraction.get();

		if (!bSpeedHackCheck)//m_pHost->bServerSyncPos)
		{
			pAction->mutable_positionlogic().mutate_x(pCharacter->GetPosition().fX);
			pAction->mutable_positionlogic().mutate_y(pCharacter->GetPosition().fY);
			pAction->mutable_positionlogic().mutate_z(pCharacter->GetPosition().fZ);
		}

		m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_shot, pAction, kFLOAT_INIT, data->shottype(), PotentialBloom);
		m_pHost->AddCharacterActionDeque(data->playeraction()->id());

		//------------------------------------------------------------------
		ServerCharacterPosition position;
		position.packetID = PACKET_ID::play_c2s_playerBehindStepBackJumperShot; // �̰� ���Ǹ� ����
		position.positionlogic = pAction->positionlogic();
		m_pHost->AddServerPostionDeque(pAction->id(), position);
		//-------------------------------------------------------------------------

		// ���ٿ��� ���� ������ �㰡�� �ޱ����� ������ �߿��� �ʱ�ȭ ���� ( 2025-04-05 ) 
		m_pHost->SendSyncInfo(SYNCINFO_TYPE::READY);

		float gap = recv_time - data->clienttime();
		userPing = gap;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerBehindStepBackJumperShot, message, send_data)
		send_data.add_playeraction(pAction);
		send_data.add_ballnumber(data->ballnumber());
		send_data.add_shottype(data->shottype());
		send_data.add_animid(data->animid());
		send_data.add_positionball(data->positionball());
		send_data.add_speed(data->speed());
		send_data.add_enabledribble(data->enabledribble());
		send_data.add_readyanimation(data->readyanimation());
		send_data.add_islefthanded(data->islefthanded());
		send_data.add_localpositionball(data->localpositionball());
		send_data.add_slideposition(data->slideposition());
		send_data.add_ismirroranim(data->ismirroranim());
		send_data.add_hostpermit(true);	// Ŭ�󿡼� �� �׼ǿ� ���� ����ó���� ��� �ϴ� ������ true
		send_data.add_normal(bNormal);
		send_data.add_potentialfxlevel(potentialfxlevel);	// �����ýÿ��� ����Ʈ�� �̸� ������
		send_data.add_ping(userPing);
		STORE_FBPACKET(builder, message, send_data)
			
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		return true;
	}

	return false;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerShot(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerShot_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	else
	{
		const TB::SVector3 client_pos = data->playeraction()->positionlogic();
		if (m_pHost->CheckPositionIsNan(&client_pos))
		{
			string invalid_buffer_log = "[HACK_CHECK] playerShot Position is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}

		if (m_pHost->CheckPositionIsNan(data->positionball()))
		{
			string invalid_buffer_log = "[HACK_CHECK] playerShot BallPosition is nan RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
			m_pHost->ToLog(invalid_buffer_log.c_str());

			return false;
		}
	}


	//! ��Ŷ ���� üũ

	//! ���ο� ���ǵ��� üũ
	DHOST_TYPE_BOOL bNormal = true;

	//! ���� ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	// �ڽ��ƿ� ���� �ʱ�ȭ 
	m_pHost->GetCharacterManager()->CheckBoxOutedClear();

	DHOST_TYPE_BOOL bDistanceToFar = true;


	DHOST_TYPE_FLOAT userPing = 0.0f;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetCharacterStateAction() != nullptr)
	{
		pCharacter->OnPlayerShot(data->playeraction()->id(), data->shottype());

		CHostUserInfo* pUser = m_pHost->FindUser(userid);
		userPing = pUser->GetPingAverage();

		DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(data->playeraction()->positionlogic());
		if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

			if (gap > kCHARACTER_POSITION_CORRECT)
			{
				if (pUser->GetPingAverage() < 0.07f && pUser->GetCurFPS() > 50.0f) // ���� ��翡 ������ �غ��� 
				{
					pUser->SpeedHackDetected(SPEEDHACK_CHECK::SHOT); // IsSpeedhackDectected_High  ���� , ���ϵ� ������ �� 
				}

				string position_log = "[HACK_CHECK] [ACTION_DISTANCE_TO_FAR] playerShot RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", UserID : " + std::to_string(userid)
					+ ", BallNo : " + std::to_string(data->ballnumber())
					+ ", ValidRa: " + std::to_string(validMoveRange)
					+ ", GAP : " + std::to_string(gap)
					+ ", Ping : " + std::to_string(pUser->GetPingAverage())
					+ ", FPS : " + std::to_string(pUser->GetCurFPS());

				m_pHost->ToLog(position_log.c_str());

				pCharacter->SendSeverPositionToClient(data->playeraction()->directioninput());
				
				bSpeedHackCheck = false; // false �� �ɸ� ��
				bDistanceToFar = false;
			}
			else
			if (gap > validMoveRange)
			{
				CHostUserInfo* pUser = m_pHost->FindUser(userid);
				string position_log = "[HACK_CHECK] [ACTION_DISTANCE_TO_FAR] [LIGHT] playerShot RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
					+ ", UserID : " + std::to_string(userid)
					+ ", BallNo : " + std::to_string(data->ballnumber())
					+ ", ValidRa: " + std::to_string(validMoveRange)
					+ ", GAP : " + std::to_string(gap)
					+ ", Ping : " + std::to_string(pUser->GetPingAverage())
					+ ", FPS : " + std::to_string(pUser->GetCurFPS());

				m_pHost->ToLog(position_log.c_str());

				//pCharacter->SendSeverPositionToClient(data->playeraction()->directioninput());
				//bSpeedHackCheck = false; // false �� �ɸ� ��
				//bDistanceToFar = false;

			}
		}

	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}

	//! �ϳ��� �ɸ��ٸ� ���� ó��
	DHOST_TYPE_BOOL bResult = true;
	if (bSpeedHackCheck == false || bVerifySkill == false || bDistanceToFar == false)
	{
		//! �׼ǽ����� �̻��� �����Ȱ��� �������� ������ ����ó���ϰ� �Ѵ�.
		m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());
	}
	
	const SPlayerAction* pData = data->playeraction();

	m_pHost->CheckQuestAction(pData);

	if (pData != nullptr)
	{
		bool permit = true;

		CPhysicsHandler* physicsHandler = m_pHost->GetPhysicsHandler();

		SVector3 rimPosition = physicsHandler->GetRimPosition();

		float yaw = GetYaw(rimPosition.x() - data->playeraction()->positionlogic().x(), rimPosition.z() - data->playeraction()->positionlogic().z());

		//! ���� �ɷ� ����
		DHOST_TYPE_INT32 potentialfxlevel = kINT32_INIT;
		DHOST_TYPE_BALL_NUMBER PotentialBloom = kBALL_NUMBER_INIT;

		//! �������� ��츸 ����ɷ��� ��ȭ �ϴ��� Ȯ��
		if (m_pHost->CheckShotTypeRimAttack(data->shottype()))
		{
			SPotentialInfo sInfo;
			DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(data->playeraction()->id(), ACTION_TYPE::action_shot, data->shottype(), data->playeraction()->skillindex(), sInfo);
			if (bPotential)
			{
				//! ������ �ƴٸ�
				if (sInfo.potentialawaken())
				{
					//! �̾ƿ� ���� ��ȭ �ߵ� Ȯ������ ���ٸ� ��ȭ ������ߵ�
					DHOST_TYPE_FLOAT random_value = m_pHost->GetRandomValueAlgorithm(userid, false);

					if (random_value <= sInfo.potentialbloomratevalue())
					{
						PotentialBloom = data->ballnumber();
						potentialfxlevel = (sInfo.potentialbloombufflevel() >= 5 && sInfo.potentialbloomratelevel() >= 5) ? 2 : 1;

						{
							string verify_log = "[POTENTIAL_SHOT_ACTION] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
								+ ", BallNo : " + std::to_string(data->ballnumber())
								+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
								+ ", PotentialLv : " + std::to_string(sInfo.potentiallevel())
								+ ", PotentialValue : " + std::to_string(sInfo.potentialvalue())
								+ ", awaken : " + std::to_string(sInfo.potentialawaken())
								+ ", BloomRateLv : " + std::to_string(sInfo.potentialbloomratelevel())
								+ ", BloomRateValue : " + std::to_string(sInfo.potentialbloomratevalue())
								+ ", BloomBuffLv : " + std::to_string(sInfo.potentialbloombufflevel())
								+ ", BloomBuffValue : " + std::to_string(sInfo.potentialbloombuffvalue())
								+ ", potentialfxlevel : " + std::to_string(potentialfxlevel);
							m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
						}
					}
				}
			}
		}
		
		F4PACKET::play_c2s_playerShot_dataT temp_data;
		data->UnPackTo(&temp_data);
		F4PACKET::SPlayerAction* pAction = nullptr;
		pAction = temp_data.playeraction.get();

		if (!bSpeedHackCheck)//m_pHost->bServerSyncPos)
		{
			pAction->mutable_positionlogic().mutate_x(pCharacter->GetPosition().fX);
			pAction->mutable_positionlogic().mutate_y(pCharacter->GetPosition().fY);
			pAction->mutable_positionlogic().mutate_z(pCharacter->GetPosition().fZ);
		}

		m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_shot, pAction, kFLOAT_INIT, data->shottype(), PotentialBloom);
		m_pHost->AddCharacterActionDeque(data->playeraction()->id());

		//------------------------------------------------------------------
		ServerCharacterPosition position;
		position.packetID = PACKET_ID::play_c2s_playerShot;
		position.positionlogic = pAction->positionlogic();
		m_pHost->AddServerPostionDeque(pAction->id(), position);
		//-------------------------------------------------------------------------

		// ���ٿ��� ���� ������ �㰡�� �ޱ����� ������ �߿��� �ʱ�ȭ ���� ( 2025-04-05 ) 
		m_pHost->SendSyncInfo(SYNCINFO_TYPE::READY);

		// �� �ִϸ��̼� üũ�ϱ�, �������ε� �������̾��� ������ ������ �־ 
		// 

		/*
		std::string keyStr;
		m_pHost->GetAnimationController()->GetIndexName(data->animid(), keyStr);

		string verify_log2 = "[Animation Name] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", CharacterSN : " + std::to_string(data->playeraction()->id())
			+ ", Inidex : " + std::to_string(data->animid());
			+ ", AniName : " + keyStr;

		m_pHost->ToLog(verify_log2.c_str());


		int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());
		if (ani_check_result < 0)
		{
			F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
			if (pCharacter != nullptr)
			{
				m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
			}
		}
		*/

		float gap = recv_time - data->clienttime();
		userPing = gap;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerShot, message, send_data);
		send_data.add_playeraction(pAction);
		send_data.add_ballnumber(data->ballnumber());
		send_data.add_shottype(data->shottype());
		send_data.add_animid(data->animid());
		send_data.add_positionball(data->positionball());
		send_data.add_speed(data->speed());
		send_data.add_enabledribble(data->enabledribble());
		send_data.add_readyanimation(data->readyanimation());
		send_data.add_islefthanded(data->islefthanded());
		send_data.add_localpositionball(data->localpositionball());
		send_data.add_slideposition(data->slideposition());
		send_data.add_ismirroranim(data->ismirroranim());
		send_data.add_hostpermit(true);	// Ŭ�󿡼� �� �׼ǿ� ���� ����ó���� ��� �ϴ� ������ true
		send_data.add_normal(bNormal);
		send_data.add_potentialfxlevel(potentialfxlevel);	// �����ýÿ��� ����Ʈ�� �̸� ������
		send_data.add_ping(userPing);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);


		
		return true;
	}

	return false;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSlipAndSlide(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSlipAndSlide_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::flashy_penetrate);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_slipAndSlide, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerSlipAndSlide;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSlipAndSlide, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_direction(data->direction());
	send_data.add_count(data->count());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerShadowFaceupDash(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerShadowFaceupDash_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!


	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);

	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_slideStep, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerShadowFaceupDash;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerShadowFaceupDash, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ismirror(data->ismirror());
	send_data.add_animid(data->animid());
	send_data.add_yawlook(data->yawlook());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSlideStep(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSlideStep_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_slideStep, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerSlideStep;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSlideStep, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ismirror(data->ismirror());
	send_data.add_animid(data->animid());
	send_data.add_yawlook(data->yawlook());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSpinMove(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSpinMove_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::flashy_penetrate);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}

	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_spinMove, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerSpinMove;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSpinMove, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_animid(data->animid());
	send_data.add_ismirror(data->ismirror());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_potentialfxlevel(data->potentialfxlevel());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerStand(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStand_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}


	F4PACKET::play_c2s_playerStand_dataT temp_data;
	data->UnPackTo(&temp_data);
	
	F4PACKET::play_c2s_playerStand_dataT* pTemp_data = &temp_data;
	F4PACKET::SPlayerAction* pAction = nullptr;
	pAction = temp_data.playeraction.get();
	
	const SPlayerAction* pFixedAction = data->playeraction();

	DHOST_TYPE_BOOL bSpeedHackCheck = false;
	DHOST_TYPE_BOOL bNormal = true;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter)
	{
		DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(data->playeraction()->positionlogic());
		if(m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
		{
			DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

			CHostUserInfo* pHostUser = m_pHost->FindUser(userid);

			if (gap > kCHARACTER_POSITION_CORRECT) // ����� ���������� �α׸� ��� ���� 
			{
				string position_log = "[HACK_CHECK] play_c2s_playerStand GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
					",   UserID : " + std::to_string(userid)
					+ ", validMoveRange: " + std::to_string(kCHARACTER_POSITION_CORRECT)
					+ ", Ping: " + std::to_string(pHostUser->GetPingAverage())
					+ ", FPS: " + std::to_string(pHostUser->GetCurFPS())
					+ ", GAP : " + std::to_string(gap);

				m_pHost->ToLog(position_log.c_str());
				bSpeedHackCheck = true;
			}
			else
			if (gap > validMoveRange) // ����� ���������� �α׸� ��� ���� 
			{
				bSpeedHackCheck = true;
			}

		}

		if (data->runmode() == F4PACKET::MOVE_MODE::tripleThreat)
		{
			pCharacter->SetTripleThreat(true);
		}
		else
		{
			pCharacter->SetTripleThreat(false);
		}
	}

	if (!bSpeedHackCheck)
	{
	}

	if (bSpeedHackCheck)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			TB::SVector3 server_pos = CommonFunction::ConvertJoVectorToTBVector(pCharacter->GetPosition());
			auto client_pos = pAction->positionlogic();	// ��Ŷ���� ���� ĳ������ ��ġ
			float gap = VECTOR3_DISTANCE(server_pos, client_pos);

			/*string position_log = "[MODIFY_POSITION] play_c2s_playerStand GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", CharacterSN : " + std::to_string(data->playeraction()->id()) +
				", GAP : " + std::to_string(gap) + ", PreAction : " + F4PACKET::EnumNameACTION_TYPE(pCharacter->GetCharacterStateActionType()) +
				", Server X : " + std::to_string(server_pos.x()) + ", Server Z : " + std::to_string(server_pos.z()) +
				", Client X : " + std::to_string(pAction->positionlogic().x()) + ", Client Z : " + std::to_string(pAction->positionlogic().z());
			m_pHost->ToLog(position_log.c_str());*/

			pAction->mutable_positionlogic().mutate_x(server_pos.x());
			pAction->mutable_positionlogic().mutate_y(server_pos.y());
			pAction->mutable_positionlogic().mutate_z(server_pos.z());
		}

		pFixedAction = pAction;
	}

	// 1. ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	// 2. ���⼭ ���� ��ġ�� ���� �� 
	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys(), data->runmode());

	// 3. ���� ��ġ�� ����� �ϹǷ� �ѹ��� 
	if (bSpeedHackCheck)
	{
		m_pHost->GetCharacterManager()->GetCharacterStateAction(data->playeraction()->id())->mutable_positionlogic().mutate_x(pAction->positionlogic().x());
		m_pHost->GetCharacterManager()->GetCharacterStateAction(data->playeraction()->id())->mutable_positionlogic().mutate_y(pAction->positionlogic().y());
		m_pHost->GetCharacterManager()->GetCharacterStateAction(data->playeraction()->id())->mutable_positionlogic().mutate_z(pAction->positionlogic().z());
	}
	// 4. ���� ����
	m_pHost->CheckQuestAction(pFixedAction);
	// 5. �׼� ���� 
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_stand, pFixedAction, kFLOAT_INIT);
	m_pHost->SetCharacterActionOverlapDestYaw(data->playeraction()->id(), ACTION_TYPE::action_stand, data->yawdest());
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//------------------------------------------------------------------------
	ServerCharacterPosition position;
	if (data->runmode() == F4PACKET::MOVE_MODE::pivot)
	{
		//string position_log = "[HACK_CHECK] pivot stand";
		//m_pHost->ToLog(position_log.c_str());
		position.packetID = PACKET_ID::play_c2s_playerPivot;
	}
	else
	{
		position.packetID = PACKET_ID::play_c2s_playerStand;
	}

	position.positionlogic = pFixedAction->positionlogic();
	position.speed = 0.0f;
	position.dirYaw = pFixedAction->yawlogic(); //data->playeraction()->directioninput();
	position.clienttime = data->clienttime();
	m_pHost->AddServerPostionDeque(pFixedAction->id(), position);
	//--------------------------------------------------------------------------


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerStand, message, send_data);
	send_data.add_playeraction(pFixedAction);
	send_data.add_runmode(data->runmode());
	send_data.add_lookid(data->lookid());
	send_data.add_yawdest(data->yawdest());
	send_data.add_lefthand(data->lefthand());
	send_data.add_ismirroranim(data->ismirroranim());
	send_data.add_normal(bNormal);

	STORE_FBPACKET(builder, message, send_data)

	if (bSpeedHackCheck)
	{
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}
	else
	{
		m_pHost->BroadcastPacket(message, userid);
	}

	return true;

}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSteal(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSteal_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!




	//! ��Ŷ ���� üũ

	//! ���ǵ��� üũ
	DHOST_TYPE_BOOL bSpeedHackCheck = true;
	DHOST_TYPE_BOOL bNormal = true;

	//! ���� �ɷ� ����
	EACTION_VERIFY check_verify = m_pHost->VerifyPotentialFxLevel(userid, data->playeraction()->id(), data->potentialfxlevel(), POTENTIAL_INDEX::offguard_steal);
	if (check_verify != EACTION_VERIFY::SUCCESS)
	{
		m_pHost->ForceActionStand(pPacket->GetPacketID(), pPacket, userid, EFORCE_ACTION_STAND_TYPE::VERIFY_POTENTIAL);

		return true;
	}


	STEAL_TYPE client_value = data->stealtype();

	//! ��ų ����üũ
	DHOST_TYPE_BOOL bVerifySkill = true;
	if (data->playeraction()->skillindex() != SKILL_INDEX::skill_none)
	{
		bVerifySkill = m_pHost->CheckHaveSkill(data->playeraction()->id(), data->playeraction()->skillindex());
	}


	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	if (pCharacter != nullptr)
	{
		F4PACKET::play_c2s_playerSteal_dataT temp_data;
		data->UnPackTo(&temp_data);
		F4PACKET::SPlayerAction* pAction = nullptr;
		pAction = temp_data.playeraction.get();
		

		//---------------------------------------------------------------------
		ServerCharacterPosition position;
		position.packetID = PACKET_ID::play_c2s_playerSteal;
		position.positionlogic = pAction->positionlogic();
		m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
		//-------------------------------------------------------------------------


		//! �� �ȿ� ������ ���� �� �ùķ��̼��� �Ͼ�� �ȵ�
		if (client_value == STEAL_TYPE::fail || bSpeedHackCheck == false || bVerifySkill == false)
		{
			client_value = STEAL_TYPE::fail;
			m_pHost->SetCharacterForceBallEventFail(data->playeraction()->id(), data->ballnumber());
		}

		//! ĳ���� ��ġ ����
		m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());

		//! ĳ���� �ǽð� ���� ����
		m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

		const SPlayerAction* pData = pAction;
		m_pHost->CheckQuestAction(pData);

		//! ĳ���� ���� ���� (������)
		m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_steal, pData, kFLOAT_INIT);
		m_pHost->AddCharacterActionDeque(data->playeraction()->id());
		
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerSteal, message, send_data);
		send_data.add_playeraction(pAction);
		send_data.add_ballnumber(data->ballnumber());
		send_data.add_stealtype(client_value);
		send_data.add_animid(data->animid());
		send_data.add_positionball(data->positionball());
		send_data.add_animspeed(data->animspeed());
		send_data.add_ismirroranim(data->ismirroranim());
		send_data.add_hostpermit(true);
		send_data.add_normal(bNormal);
		send_data.add_potentialfxlevel(data->potentialfxlevel());
		send_data.add_ispotentialactionshadowsteal(data->ispotentialactionshadowsteal());
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerStealFail(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStealFail_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerStealFail;
	m_pHost->AddServerPostionDeque(data->stealerid(), position);
	//-------------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerStealFail, message, send_data);
	send_data.add_dribblerid(data->dribblerid());
	send_data.add_stealerid(data->stealerid());
	send_data.add_stealtype(data->stealtype());
	STORE_FBPACKET(builder, message, send_data)
	
	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerStun(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerStun_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	/*{
		string collision_log = "[play_c2s_playerStun] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid)
			+ ", playerID : " + std::to_string(data->playeraction()->id()) + ", StunType : " + F4PACKET::EnumNameSTUN_TYPE(data->stuntype()) + ", causerid : " + std::to_string(data->causerid());
		m_pHost->ToLog(collision_log.c_str());
	}*/

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerStun;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->playeraction()->id());
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!

	if (pCharacter == nullptr)
	{
		return false;
	}

	F4PACKET::ACTION_TYPE character_action_type = pCharacter->GetCharacterAction()->actionType;

	if (character_action_type == F4PACKET::ACTION_TYPE::action_alleyOopShot || character_action_type == F4PACKET::ACTION_TYPE::action_shotDunk
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shot || character_action_type == F4PACKET::ACTION_TYPE::action_shotJump
		|| character_action_type == F4PACKET::ACTION_TYPE::action_shotLayUp || character_action_type == F4PACKET::ACTION_TYPE::action_shotPost)
	{
		CCharacter* pTargetCharacter = m_pHost->GetCharacterManager()->GetCharacter(data->causerid());
		if (pTargetCharacter != nullptr)
		{
			F4PACKET::ACTION_TYPE target_character_action_type = pTargetCharacter->GetCharacterAction()->actionType;
			if ( target_character_action_type == F4PACKET::ACTION_TYPE::action_moveFaceUp 
				|| target_character_action_type == F4PACKET::ACTION_TYPE::action_standFaceUp
				|| target_character_action_type == F4PACKET::ACTION_TYPE::action_moveShadowFaceUp	
				|| target_character_action_type == F4PACKET::ACTION_TYPE::action_standShadowFaceUp
				|| target_character_action_type == F4PACKET::ACTION_TYPE::action_collision)
			{
				string collision_log = "[play_c2s_playerStun] return false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
				m_pHost->ToLog(collision_log.c_str());

				return false;
			}
		}
	}

	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	DHOST_TYPE_FLOAT time = m_pHost->GetGameTime();

	m_pHost->SetCharacterAction(data->playeraction()->id(), time, ACTION_TYPE::action_stun, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	if (data->stuntype() == F4PACKET::STUN_TYPE::stunType_ankleBreakStrong || data->stuntype() == F4PACKET::STUN_TYPE::stunType_ankleBreakWeak)
	{
		m_pHost->CheckCharacterBurstGauge(data->playeraction()->id(), time, (BURST_ACTION_TYPE | BURST_ACTION_TYPE_ANKLE_BREAKED), m_pHost->GetVecScores());

		SBallActionInfo sInfo;
		if (m_pHost->FindTargetBallActionInfo(0, sInfo))
		{
			SCharacterAction* pAction = m_pHost->GetCharacterActionInfo(sInfo.BallOwner);
			if (pAction != nullptr && m_pHost->CheckCharacterActionPenetrateType(pAction->actionType))
			{
				m_pHost->CheckCharacterBurstGauge(sInfo.BallOwner, time, (BURST_ACTION_TYPE | BURST_ACTION_TYPE_ANKLE_BREAK), m_pHost->GetVecScores());
			}
		}

		// ��Ŭ�극��ũ�� �߻���Ų ������ ī��Ʈ ����
		m_pHost->IncreaseActionAnkleBreakeCount(data->causerid());
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerStun, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_stuntype(data->stuntype());
	send_data.add_destyaw(data->destyaw());
	send_data.add_animid(data->animid());
	send_data.add_ismirror(data->ismirror());
	send_data.add_causerid(data->causerid());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSwitchActive(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSwitchActive_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	string log_message = "play_c2s_playerSwitchActive id : " + std::to_string(data->id()) + ", toactive : " + std::to_string(data->toactive()) + ", result : " + std::to_string(data->result());
	m_pHost->ToLog(log_message.c_str());

	bool result = false;

	F4PACKET::SPlayerInformationT* pInfo = m_pHost->FindCharacterInformation(data->id());

	if (pInfo != nullptr)
	{
		DHOST_TYPE_INT32 team = pInfo->team;

		if (data->toactive() == true)
		{
			if (m_pHost->GetSwitchID(team) == 0)
			{
				m_pHost->SetSwitchID(team, data->id());
				result = true;
			}
		}
		else
		{
			if (m_pHost->GetSwitchID(team) == data->id())
			{
				m_pHost->SetSwitchID(team, 0);
				result = true;
			}
		}
	}


	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSwitchActive, message, send_data);
	send_data.add_id(data->id());
	send_data.add_toactive(data->toactive());
	send_data.add_result(result);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSwitchRequest(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSwitchRequest_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_pHost->IncreaseUseSwitchCount(data->id1());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSwitchRequest, message, send_data);
	send_data.add_id1(data->id2());
	send_data.add_id2(data->id1());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerSwitchResponse(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerSwitchResponse_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->response() == true)
	{
		int player1Target = m_pHost->GetMatchInfoTarget(data->id1());
		int player2Target = m_pHost->GetMatchInfoTarget(data->id2());

		m_pHost->ModifyMatchInfo(data->id1(), player2Target);
		m_pHost->ModifyMatchInfo(data->id2(), player1Target);
	}

	F4PACKET::SPlayerInformationT* pInfo = m_pHost->FindCharacterInformation(data->id2());

	if (pInfo != nullptr)
	{
		DHOST_TYPE_INT32 team = pInfo->team;

		m_pHost->SetSwitchID(team, 0);
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerSwitchResponse, message, send_data);
	send_data.add_id1(data->id1());
	send_data.add_id2(data->id2());
	send_data.add_response(data->response());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerEmoji(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerEmoji_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_pHost->IncreaseEmojiCount(data->id(), data->index());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerEmoji, message, send_data);
	send_data.add_id(data->id());
	send_data.add_index(data->index());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerBurstRequest(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerBurstRequest_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	m_pHost->UpdateCharacterBurstGaugeWithPassiveActivation(data->id(), data->requesttype());
	m_pHost->UpdateCharacterBurstGaugeWithMedalActivation(data->id(), data->requesttype());

	return true;
}


const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerTapOut(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerTapOut_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_tapOut, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerTapOut;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerTapOut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_positionball(data->positionball());
	send_data.add_animid(data->animid());
	send_data.add_mirror(data->mirror());
	send_data.add_outside(data->outside());
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_success(data->success());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerTapPass(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerTapPass_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}


	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerTapOut;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------
	// 
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_tapPass, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerTapOut, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_positionball(data->positionball());
	send_data.add_animid(data->animid());
	send_data.add_mirror(data->mirror());
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_success(data->success());
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_slideposition(data->slideposition());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_playerTipIn(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_playerTipIn_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->secondlocalpositionball() == nullptr || data->slidepositiona() == nullptr || data->slidepositionb() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER playeraction is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}


	//---------------------------------------------------------------------
	ServerCharacterPosition position;
	position.packetID = PACKET_ID::play_c2s_playerTipIn;
	position.positionlogic = data->playeraction()->positionlogic();
	m_pHost->AddServerPostionDeque(data->playeraction()->id(), position);
	//-------------------------------------------------------------------------
	// 
	//!
	//! ĳ���� ��ġ ����
	m_pHost->CheckCharacterPositionSync(data->playeraction(), pPacket->GetPacketID());
	//!
	
	int ani_check_result = m_pHost->GetAnimationController()->CheckAnimationData(data->animid());

	if (ani_check_result < 0)
	{
		F4PACKET::SPlayerInformationT* pCharacter = m_pHost->FindCharacterInformation(data->playeraction()->id());
		if (pCharacter != nullptr)
		{
			m_pHost->ToLogInvalidAnimation(pCharacter, data->animid(), ani_check_result, (F4PACKET::PACKET_ID)pPacket->GetPacketID());
		}
	}

	m_pHost->OnCharacterMessage(data->playeraction()->id(), pPacket->GetPacketID(), pPacket, recv_time, m_pHost->GetGameTime(), data->playeraction()->keys());

	const SPlayerAction* pData = data->playeraction();
	m_pHost->CheckQuestAction(pData);
	
	m_pHost->SetCharacterAction(data->playeraction()->id(), m_pHost->GetGameTime(), ACTION_TYPE::action_tipIn, pData, kFLOAT_INIT);
	m_pHost->AddCharacterActionDeque(data->playeraction()->id());


	bool tipinResult = data->success();

	if (data->success()) // by steven 2024-01-09, TipIn �ΰ�� ���ٿ�� ���ɽ��� ���� ���� �����ϴ�
	{
		CBallController* pBallController = m_pHost->BallControllerGet();
		if (pBallController)
		{
			if (pBallController->GameBallDataGet() != nullptr)
			{
				pBallController->GameBallDataGet()->checkReboundValid = true;
				//m_pHost->SendSyncPosition(); // ������ ���� 
			}
			
		}
	}

	/*
	// �������Ϳ��� ������ �Ǵ� ���� �ذ�, ���� ĳ���ʹ� ��ε��ɽ����� ���ϹǷ� Ŭ�󿡼� ���� 
	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		if (data->success()) //m_pHost->GetGameTime() <= 0.0f)
		{
			tipinResult = false; // ��� �ð��� 0 �ΰ�쿡�� ���� ���� 

			string invalid_buffer_log = "[tipinResult] false RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
			m_pHost->ToLog(invalid_buffer_log.c_str());
		}
	}
	*/

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerTipIn, message, send_data);
	send_data.add_playeraction(pData);
	send_data.add_ballnumber(data->ballnumber());
	send_data.add_animid(data->animid());
	send_data.add_ismirror(data->ismirror());
	send_data.add_positionball(data->positionball());
	send_data.add_success(tipinResult);
	send_data.add_localpositionball(data->localpositionball());
	send_data.add_secondlocalpositionball(data->secondlocalpositionball());
	send_data.add_slidepositiona(data->slidepositiona());
	send_data.add_slidepositionb(data->slidepositionb());
	send_data.add_slideyawb(data->slideyawb());
	send_data.add_ismirroranim(data->ismirroranim());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, userid);

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_uiDisplayMedal(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_uiDisplayMedal_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	SendMedalUIDisplay(data->id(), data->medalindex());

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_play_c2s_greatDefense(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_greatDefense_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->defensefeedbacktype() == F4PACKET::DEFENSEFEEDBACK_TYPE::GreatDefense)
	{
		// �ְ��� �����϶� ī��Ʈ
		m_pHost->IncreaseActionGreatDefenseCount(data->id());
	}

	return true;
}

void CState::SendMedalUIDisplay(int32_t id, MEDAL_INDEX medalIndex)
{
	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_uiDisplayMedal, message, send_data);
	send_data.add_id(id);
	send_data.add_medalindex(medalIndex);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_s2s_clientAliveCheck(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_s2s_clientAliveCheck_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	CHostUserInfo* pUser = m_pHost->FindUser(userid);

	if (pUser != nullptr)
	{
		DHOST_TYPE_FLOAT diff =  pUser->ProcessAlivePacket(m_pHost->GetRoomElapsedTime());

		//pUser->PushClientElapsedTimeMap(data->idx(), data->clienttime());
		pUser->PushClientElapsedTimeWithSystemTime(data->clienttime(), data->s2cwaylatency());

		if (diff > kPING_LATENCY_200)
		{
			string log_message = "[ALIVE_CHECK] LATENCY_DELAY RoomElapsedTime : " + std::to_string(recv_time) + ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid) + ", Latency : " + std::to_string(diff);
			m_pHost->ToLog(log_message.c_str());

			m_pHost->IncreaseLatencyCount(pUser->GetUserID(), diff);
		}		
	}
	else
	{
		string log_message = "[ALIVE_CHECK] Cannot Find CHostUserInfo RoomElapsedTime : " + std::to_string(recv_time)
			+ " RoomID : " + m_pHost->GetHostID()
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(log_message.c_str());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_s2s_systemTimeCheck(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_s2s_systemTimeCheck_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!


	// ���⼭ ���� �� ���� �õ� ��ü , 1. ������ Ŭ���̾�Ʈ�� ���� �� ������ ������ �޾ƾ� , ���� ��� �����õ尡 �������� 
	CHostUserInfo* pUser = m_pHost->FindUser(userid);

	if (pUser != nullptr)
	{
		if (pUser->GetSendRandomSeed() == data->systemtime())
		{
			pUser->SetRandomSeedIndex(data->idx());
			pUser->SetRandomSeed(data->systemtime());
		}
		else
		{
			if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
			{
				string invalid_buffer_log = "[HACK_CHECK] RandomSeed is Different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
				m_pHost->ToLog(invalid_buffer_log.c_str());
			}
		}
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_test(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_test_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!


	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_blackBox(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_blackBox_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER snrnsisj is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->memorytampercount() == -50) // Ŭ���̾�Ʈ üũ ���ǵ��� 
	{
		string hack_log = "[HACK_CHECK] BlackBox_SpeedHack_ClientCheck RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
			+ ", ClientTime : " + std::to_string(data->timescale())
			+ ", illegalcount : " + std::to_string(data->illegalcount())
			+ ", memorytampercount : " + std::to_string(data->memorytampercount())
			+ ", Gap : " + data->stringinfo()->str()
			+ ", Ping : " + std::to_string(m_pHost->FindUser(userid)->GetPingAverage());

		m_pHost->ToLog(hack_log.c_str());
		F4PACKET::SPlayerInformationT* pInfo = m_pHost->FindCharacterInformationWithUserIDAndIsNotAi(userid);
		if (pInfo != nullptr)
		{
			m_pHost->SetSpeedHackClientjudgmentCount(pInfo->id, data->memorytampercount());
		}

		//[CompositeClock]
	}
	else
	if (data->memorytampercount() == -200) // ActionMove Ŭ���̾�Ʈ ���ǵ���üũ 
	{
		CHostUserInfo* pUser = m_pHost->FindUser(userid);
		if (pUser)
		{
			string hack_log = "[HACK_CHECK] [CompositeClock] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
				+ ", ClientTime : " + std::to_string(data->timescale())
				+ ", illegalcount : " + std::to_string(data->illegalcount())
				+ ", memorytampercount : " + std::to_string(data->memorytampercount())
				+ ", Info : " + data->stringinfo()->str()
				+ ", Ping : " + std::to_string(m_pHost->FindUser(userid)->GetPingAverage())
				+ ", FPS : " + std::to_string(data->addinfo());

			m_pHost->ToLog(hack_log.c_str());
		}
	}
	else
	if (data->memorytampercount() == -100) // ActionMove Ŭ���̾�Ʈ ���ǵ���üũ 
	{
		F4PACKET::SPlayerInformationT* pInfo = m_pHost->FindCharacterInformationWithUserIDAndIsNotAi(userid);
		if (pInfo != nullptr)
		{
			m_pHost->SetSpeedHackClientjudgmentCount(pInfo->id, data->memorytampercount());
		}

		CHostUserInfo* pUser = m_pHost->FindUser(userid);
		if (pUser)
		{
			string hack_log = "[HACK_CHECK] BlackBox_SpeedHack RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
				+ ", ClientTime : " + std::to_string(data->timescale())
				+ ", illegalcount : " + std::to_string(data->illegalcount())
				+ ", memorytampercount : " + std::to_string(data->memorytampercount())
				+ ", Gap : " + data->stringinfo()->str()
				+ ", Ping : " + std::to_string(m_pHost->FindUser(userid)->GetPingAverage())
				+ ", FPS : " + std::to_string(data->addinfo());

			m_pHost->ToLog(hack_log.c_str());

			pUser->SpeedHackDetected(SPEEDHACK_CHECK::MOVE);
		}
	}
	else
	if (data->memorytampercount() == (int)ABNORMAL::Data_Quit) // �޸� ���� �����ڿ��� 
	{
		string hack_log = "[HACK_CHECK] BlackBox_MemoryTemper RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
			+ ", timescale : " + std::to_string(data->timescale())
			+ ", illegalcount : " + std::to_string(data->illegalcount())
			+ ", memorytampercount : " + std::to_string(data->memorytampercount())
			+ ",  callStack : " + data->stringinfo()->str();

		m_pHost->ToLog(hack_log.c_str());
	}
	else
	if (data->memorytampercount() == (int)ABNORMAL::Data_implicit) // �޸� ����, �Ϲ��� ����ȯ���� ȣ�� 
	{
		string hack_log = "[HACK_CHECK] BlackBox_MemoryTemper RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
			+ ", timescale : " + std::to_string(data->timescale())
			+ ", illegalcount : " + std::to_string(data->illegalcount())
			+ ", memorytampercount : " + std::to_string(data->memorytampercount())
			+ ",  callStack : " + data->stringinfo()->str();

		m_pHost->ToLog(hack_log.c_str());
	}
	else
	if (data->memorytampercount() == (int)ABNORMAL::Data_ReboundAble)
	{
		CHostUserInfo* pUser = m_pHost->FindUser(userid);
		if (pUser)
		{
			pUser->REBOUNDABLETIME = data->timescale();
			pUser->REBOUNDABLETIME_CLIENT = recv_time;


			CBallController* pBallController = m_pHost->BallControllerGet();
			if (pBallController != nullptr && pBallController->GameBallDataGet() != nullptr)
			{

				if (pBallController->GameBallDataGet()->checkReboudEnableTime <= 0.0f) // reboundable �̺�Ʈ ���� ���� ���� ��� 
				{

					string position_log = "[HACK_CHECK] [SPEED] REBOUNDABLETIME GameTime: " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(userid)
						+", Ping: " + std::to_string(pUser->GetPingAverage())
						+ ", FPS: " + std::to_string(pUser->GetCurFPS());

					m_pHost->ToLog(position_log.c_str());

				}
				else
				{

				}

			}

		}
	}
	else
	{
		string hack_log = "[HACK_CHECK] BlackBox_MemoryTemper RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)pPacket->GetPacketID())
			+ ", timescale : " + std::to_string(data->timescale())
			+ ", illegalcount : " + std::to_string(data->illegalcount())
			+ ", memorytampercount : " + std::to_string(data->memorytampercount())
			+ ",  callStack : " + data->stringinfo()->str();

		m_pHost->ToLog(hack_log.c_str());
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_wkawofur(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_wkawofur_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	DHOST_TYPE_FLOAT server_potential_value_sum = m_pHost->GetCharacterPotentialValueSum(data->id());

	DHOST_TYPE_FLOAT client_potential_value_sum = data->gkq();

	DHOST_TYPE_FLOAT diff = std::abs(server_potential_value_sum - client_potential_value_sum);
	if (diff > kCALC_CLIENT_SERVER_GAP)
	{
		string verify_log = "[HACK_CHECK] server_client_potential_value_sum_are_different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(data->id())
			+ ", Client : " + std::to_string(client_potential_value_sum)
			+ ", Server : " + std::to_string(server_potential_value_sum)
			+ ", Diff : " + std::to_string(diff);
		m_pHost->ToLog(verify_log.c_str());

		m_pHost->KickToTheLobby(userid, EKICK_TYPE::cheat_kick);
	}

	return true;
}

const DHOST_TYPE_BOOL CState::ConvertPacket_system_c2s_skinChange(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<system_c2s_skinChange_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! ��Ŷ ������ ���� (��� ��Ŷ�� �߰��� ��)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	int randomValue = Util::GetRandomMinToMax(0, 9);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, system_s2c_skinChange, message, send_data);
	send_data.add_playerid(data->playerid());
	send_data.add_randomvalue(randomValue);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	return true;
}