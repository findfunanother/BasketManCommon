#define _CRT_SECURE_NO_WARNINGS

#include "VerifyManager.h"
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
#include "XorShift.h"


CVerifyManager::CVerifyManager(CHost* host) : m_pHost(host)
{
	addBlockPotentialBuffDebug = 0.0f;
	subBlockPotentialBuffDebug = 0.0f;
}

CVerifyManager::~CVerifyManager()
{

}

DHOST_TYPE_BOOL CVerifyManager::CheckShotTypeRimAttack(SHOT_TYPE value)
{
	DHOST_TYPE_BOOL result = false;

	switch (value)
	{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
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


DHOST_TYPE_FLOAT CVerifyManager::CorrectionEarlyInTheGame(DHOST_TYPE_CHARACTER_SN id, ECORRECTION_EARLY_TYPE value)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(id);
	if (pCharacterInfo != nullptr)
	{
		//! ���Ӹ�� ����
		if (m_pHost->GetModeType() == EMODE_TYPE::THREE_ON_THREE)
		{
			//! �Ϲ��� / ��ŷ�� ����
			if (m_pHost->GetIsNormalGame() == true)
			{
				//! �ʹ� ���� ����
				if ((m_pHost->GetGameTimeInit() - m_pHost->GetGameTime()) < m_pHost->GetBalanceTable()->GetValue("VEarlyRescue_Time"))
				{
					DHOST_TYPE_FLOAT step1Score = m_pHost->GetBalanceTable()->GetValue("VEarlyRescue1Score");
					DHOST_TYPE_FLOAT step2Score = m_pHost->GetBalanceTable()->GetValue("VEarlyRescue2Score");
					DHOST_TYPE_INT32 opponentTeam = m_pHost->GetIndexOpponentTeam(pCharacterInfo->team);
					DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(pCharacterInfo->team) - m_pHost->GetScore(opponentTeam);
					DHOST_TYPE_UINT32 absScoreDelta = abs(scoreDelta);

					switch (value)
					{
						case ECORRECTION_EARLY_TYPE::NICE_PASS:
						{
							if (scoreDelta < kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VNicePassProbability_EarlyRescue_Penalty2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VNicePassProbability_EarlyRescue_Penalty1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::BLOCK:
						{
							if (scoreDelta < kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_EarlyRescue_Penalty2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_EarlyRescue_Penalty1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::STEAL:
						{
							if (scoreDelta < kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VStealCondition_Probability_EarlyRescue_Penalty2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VStealCondition_Probability_EarlyRescue_Penalty1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::INTERCEPT:
						{
							if (scoreDelta < kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VIntercept_Success_EarlyRescue_Penalty2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VIntercept_Success_EarlyRescue_Penalty1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::BAD_RECEIVE_LOOSE_BALL:
						{
							if (scoreDelta > kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VBadReceiveLooseBallProbability_EarlyRescue_Benefit2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VBadReceiveLooseBallProbability_EarlyRescue_Benefit1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::PENETRATE_PIVOT:
						{
							if (scoreDelta > kINT32_INIT)
							{
								break;
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VCollision_Penetrate_FaceUp_Probability_EarlyRescue_Benefit2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VCollision_Penetrate_FaceUp_Probability_EarlyRescue_Benefit1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::WIDE_OPEN_MID_RANGE_SHOT:
						{
							if (scoreDelta > kINT32_INIT)
							{
								break;
							}

							if (pCharacterInfo != nullptr && m_pHost->GetCharacterManager() != nullptr)
							{
								CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pCharacterInfo->id);
								if (pCharacter->GetAbility() != nullptr)
								{
									if (pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_midRangeShot) < 40)
									{
										break;
									}
								}
							}
							
							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShot_EarlyRescue_Benefit2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_MidRangeShot_EarlyRescue_Benefit1");
							}
							else
							{

							}
						}
						break;
						case ECORRECTION_EARLY_TYPE::WIDE_OPEN_THREE_POINT_SHOT:
						{
							if (scoreDelta > kINT32_INIT)
							{
								break;
							}

							if (pCharacterInfo != nullptr && m_pHost->GetCharacterManager() != nullptr)
							{
								CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pCharacterInfo->id);
								if (pCharacter->GetAbility() != nullptr)
								{
									if (pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_threePointShot) < 40)
									{
										break;
									}
								}
							}

							if (absScoreDelta >= step2Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShot_EarlyRescue_Benefit2");
							}
							else if (absScoreDelta >= step1Score)
							{
								result = m_pHost->GetBalanceTable()->GetValue("VWideOpenBonus_ThreePointShot_EarlyRescue_Benefit1");
							}
							else
							{

							}
						}
						break;
					}
				}
			}
		}
	}

	return result;
}

//! ĳ���� �ε��� �������� (� ĳ��������)
F4PACKET::ECHARACTER_INDEX CVerifyManager::GetCharacterIndex(SPlayerInformationT* pInfo)
{
	F4PACKET::ECHARACTER_INDEX result = F4PACKET::ECHARACTER_INDEX::NONE;

	if (pInfo != nullptr && pInfo->characterid > kUINT32_INIT)
	{
		result = (F4PACKET::ECHARACTER_INDEX)(pInfo->characterid / 100);
	}

	return result;
}


//! ���� ȣȯ ��ų���� Ȯ��
DHOST_TYPE_BOOL	CVerifyManager::CheckHaveSkill(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value, DHOST_TYPE_BOOL bLogCheck)
{
	DHOST_TYPE_BOOL result = false;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(id);
	if (pCharacterInfo != nullptr)
	{
		DHOST_TYPE_UINT32 param_convert_uint = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_divide = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_remainder = kUINT32_INIT;

		param_convert_uint = (DHOST_TYPE_UINT32)value;

		if (param_convert_uint > kUINT32_INIT)
		{
			param_divide = param_convert_uint / kSKILL_INDEX_DIVIDE;
			param_remainder = param_convert_uint % kSKILL_INDEX_DIVIDE;	// �������� 1 �̻��� ��� ���� ��ų

			for (int i = 0; i < pCharacterInfo->skills.size(); ++i)
			{
				DHOST_TYPE_UINT32 own_convert_uint = (DHOST_TYPE_UINT32)pCharacterInfo->skills[i].index();

				if (own_convert_uint > kUINT32_INIT)
				{
					DHOST_TYPE_UINT32 own_divide = own_convert_uint / kSKILL_INDEX_DIVIDE;

					if (param_divide == own_divide)
					{
						DHOST_TYPE_UINT32 own_remainder = own_convert_uint % kSKILL_INDEX_DIVIDE;	// �������� 1 �̻��� ��� ���� ��ų

						if (param_remainder >= kSKILL_INDEX_DIVIDE_REMAINDER)
						{
							if (param_remainder == own_remainder)
							{
								result = true;
							}
						}
						else
						{
							result = true;
						}
						//break;
					}
				}
			}
		}

		if (result == false && bLogCheck == true)
		{
			string verify_log = "[HACK_CHECK] DOES_NOT_HAVE_SKILL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid) + ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(value);
			m_pHost->ToLog(verify_log.c_str());
		}
	}

	return result;
}

DHOST_TYPE_FLOAT CVerifyManager::GetSkillLevel(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(id);
	if (pCharacterInfo != nullptr)
	{
		DHOST_TYPE_UINT32 param_convert_uint = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_divide = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_remainder = kUINT32_INIT;

		param_convert_uint = (DHOST_TYPE_UINT32)value;

		if (param_convert_uint > kUINT32_INIT)
		{
			param_divide = param_convert_uint / kSKILL_INDEX_DIVIDE;
			param_remainder = param_convert_uint % kSKILL_INDEX_DIVIDE;	// �������� 1 �̻��� ��� ���� ��ų

			for (int i = 0; i < pCharacterInfo->skills.size(); ++i)
			{
				DHOST_TYPE_UINT32 own_convert_uint = (DHOST_TYPE_UINT32)pCharacterInfo->skills[i].index();

				if (own_convert_uint > kUINT32_INIT)
				{
					DHOST_TYPE_UINT32 own_divide = own_convert_uint / kSKILL_INDEX_DIVIDE;

					if (param_divide == own_divide)
					{
						result = pCharacterInfo->skills[i].level() * kTYPE_CAST_TO_FLOAT;
						break;
					}
				}
			}
		}
	}

	return result;
}

DHOST_TYPE_INT32 CVerifyManager::GetMedalValue(SPlayerInformationT* pInfo, MEDAL_INDEX value)
{
	DHOST_TYPE_INT32 result = kINT32_INIT;

	if (pInfo != nullptr)
	{
		for (int i = 0; i < pInfo->medals.size(); ++i)
		{
			if (pInfo->medals[i].index() == value)
			{
				result = pInfo->medals[i].value();
				break;
			}
		}
	}

	return result;
}


DHOST_TYPE_FLOAT CVerifyManager::GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE type, CCharacter* pCharacter, DHOST_TYPE_FLOAT param, DHOST_TYPE_FLOAT paramSecond)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr && pCharacter->GetCharacterInformation() != nullptr)
	{
		switch (type)
		{
			case EPLAYER_VARIABLE_TYPE::BOXOUT_ANGLE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_AngleA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_boxOut);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_AngleB");

				result = a + b * c;
			}
			break;

			case EPLAYER_VARIABLE_TYPE::BOXOUT_RADIUS:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_RadiusA");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_boxOut);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_RadiusB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_RadiusC");

				result =  a * b + c * d - e;

			}
			break;

			case EPLAYER_VARIABLE_TYPE::ENHANCED_BOXOUT_ANGLE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Enhanced_Disturb_AngleA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::enhancedBoxOut);  
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Enhanced_Disturb_AngleB");

				result =  a + b * c;

			}
			break;

			case EPLAYER_VARIABLE_TYPE::ENHANCED_BOXOUT_RADIUS:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Enhanced_Disturb_RadiusA");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::enhancedBoxOut);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Enhanced_Disturb_RadiusB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Enhanced_Disturb_RadiusC");

				result = a * b + c * d + e;


			}
			break;

			case EPLAYER_VARIABLE_TYPE::MOVE_DEFENSE:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_speed);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VDefenseMove_Run_SpeedA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VDefenseMove_Run_SpeedB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VDefenseMove_Run_SpeedC");

				try
				{
					result = LOG(a, b) / c - d;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;


			case EPLAYER_VARIABLE_TYPE::MOVE_DRIBBLE:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_speed);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_SpeedA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_SpeedB");
				DHOST_TYPE_FLOAT d = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_dribble);
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_SpeedC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_SpeedD");
				DHOST_TYPE_FLOAT g = m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_SpeedE");

				try
				{
					result = LOG(a, b) / c + LOG(d, e) / f - g;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;

			case EPLAYER_VARIABLE_TYPE::REBOUND_RADIUS:
			{
				DHOST_TYPE_FLOAT a = param;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VReboundRadiusA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VReboundRadiusB");
				DHOST_TYPE_FLOAT d = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_speed);
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VReboundRadiusC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VReboundRadiusD");
				DHOST_TYPE_FLOAT g = m_pHost->GetBalanceTable()->GetValue("VReboundRadiusE");

				try
				{
					result = LOG(a, b) / c + LOG(d, e) / f - g;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::REBOUND_RADIUS_POWER:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_RadiusA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_powerRebound);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_RadiusB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_RadiusC");
				DHOST_TYPE_FLOAT e = param;

				try
				{
					result = e * a + b * c + d;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::REBOUND_HEIGHT:
			{
				DHOST_TYPE_FLOAT a = param;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VReboundHeightA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VReboundHeightB");
				DHOST_TYPE_FLOAT d = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VReboundHeightC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VReboundHeightD");
				DHOST_TYPE_FLOAT g = m_pHost->GetBalanceTable()->GetValue("VReboundHeightE");

				try
				{
					result = LOG(a, b) / c + LOG(d, e) / f - g;
					DHOST_TYPE_FLOAT fMin = m_pHost->GetBalanceTable()->GetValue("VReboundHeight_Min");
					DHOST_TYPE_FLOAT fMax = m_pHost->GetBalanceTable()->GetValue("VReboundHeight_Max");

					result = CLAMP(result, fMin, fMax);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::REBOUND_HEIGHT_POWER:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_HeightA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_powerRebound);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_HeightB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VPowerRebound_Condition_HeightC");
				DHOST_TYPE_FLOAT e = param;

				try
				{
					result = e * a + b * c + d;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::STEAL_ANIM_SPEED:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VStealAnimSpeedA");
				DHOST_TYPE_FLOAT b = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_steal);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VStealAnimSpeedB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VStealAnimSpeedMin");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VStealAnimSpeedMax");

				try
				{
					result = CLAMP(a * b + c, d, e);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::STEAL_RADIUS:	// ��ƿ ����
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VStealCondition_RadiusA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_steal);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VStealCondition_RadiusB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VStealCondition_RadiusC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VStealCondition_RadiusD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_DUNK:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_DistanceB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_DistanceD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_LAYUP:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_DistanceB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_DistanceD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_POST_SHOT:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_DistanceB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_DistanceD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_FADEAWAY:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FadeAway_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FadeAway_DistanceB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FadeAway_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FadeAway_DistanceD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_JUMP_SHOT:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_DistanceB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_DistanceD");

				try
				{
					result = (a * b) + (LOG(c, d) / e - f);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_HEIGHT:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetCharacterInformation()->height;
				DHOST_TYPE_FLOAT b = pCharacter->GetCharacterInformation()->winspan;
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightA");
				DHOST_TYPE_FLOAT d = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightB");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightC");
				DHOST_TYPE_FLOAT g = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightD");
				DHOST_TYPE_FLOAT h = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);
				DHOST_TYPE_FLOAT i = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightE");
				DHOST_TYPE_FLOAT j = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightF");
				DHOST_TYPE_FLOAT k = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_HeightG");

				try
				{
					result = ((a + b) * c) + (LOG(d, e) / f - g) + (LOG(h, i) / j - k);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_postDefense);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_RimProtect_AngleA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_RimProtect_AngleB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_RimProtect_AngleC");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_RimProtect_AngleD");

				try
				{
					result = expf(a / (b * c)) * d + e;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::POPUP_CATCH_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_RangeA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_RangeB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_popUpCatch);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::DIVING_CATCH_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_DistanceA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_DistanceB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_divingCatchPass);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::DIVING_CATCH_PASS_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_DistanceA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_DistanceB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_divingCatch);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::PICK_UP_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_speed);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VPickUpBall_RangeA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VPickUpBall_RangeB");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VPickUpBall_RangeC");

				try
				{
					result = LOG(a, b) / c + d;

					if (isnan(result))
					{
						string verify_log = "[HACK_CHECK] PICK_UP_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", CharacterSN : " + std::to_string(pCharacter->GetCharacterInformation()->id)
							+ ", a : " + std::to_string(a)
							+ ", b : " + std::to_string(b)
							+ ", c : " + std::to_string(c)
							+ ", d : " + std::to_string(d);
						m_pHost->ToLog(verify_log.c_str());
					}
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_PROBABILITY_DUNK:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_DunkA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_DunkB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_chasedownBlock);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_CATCH_PROBABILITY_DUNK:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_DunkA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_DunkB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_chasedownCatch);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_PROBABILITY_LAYUP:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_LayUpA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_LayUpB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_chasedownBlock);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_CATCH_PROBABILITY_LAYUP:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_LayUpA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_LayUpB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_chasedownCatch);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_DUNK:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT b = paramSecond;
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_DunkA");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_DunkB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_DunkC");
				DHOST_TYPE_FLOAT f = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_strength);
				DHOST_TYPE_FLOAT g = param;
				DHOST_TYPE_FLOAT h = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_DunkD");

				try
				{
					result = (expf((a - b) / (c * d)) + e ) + ((f - g) * h);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_LAYUP:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT b = paramSecond;
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayupA");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayupB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayupC");

				try
				{
					result = expf((a - b) / (c * d)) + e;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT b = paramSecond;
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_JumpShotA");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_JumpShotB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_JumpShotC");

				try
				{
					result = expf((a - b) / (c * d)) + e;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP_SHOT_RAINBOW_SHOT:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_JumpShot_RainbowShotA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_JumpShot_RainbowShotB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_rainbowShot);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_POST:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_block);
				DHOST_TYPE_FLOAT b = paramSecond;
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShotA");
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShotB");
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShotC");

				try
				{
					result = expf((a - b) / (c * d)) + e;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_AND_CATCH_DUNK_PROBABILITY:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VDunkBlocklAndCatch_ProbabilityA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_blockCatch);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VDunkBlocklAndCatch_ProbabilityB");

				try
				{
					result = -a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_AND_CATCH_PROBABILITY:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlockAndCatch_ProbabilityA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_blockCatch);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VBlockAndCatch_ProbabilityB");

				try
				{
					result = -a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::BLOCK_POWER_BLOCK_PROBABILITY:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_PowerBlockA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_PowerBlockB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_powerBlock);

				DHOST_TYPE_FLOAT skillForceValue = 0.0f;
				if (pCharacter->CheckSkillForce(SKILL_INDEX::skill_powerBlock))
				{
					skillForceValue = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_PowerBlock_Upgrade");
				}

				try
				{
					result = -a + b * c + skillForceValue;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::SNATCH_BLOCK_BLOCK_PROBABILITY:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShotA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShotB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_snatchBlock);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::DOUBLE_HANDS_UP_BLOCK_PROBABILITY:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_DoubleHandsUpBlockA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_DoubleHandsUpBlockB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_doubleHandsUp);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::HOP_STEP_SHOT_RANGE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VHopStepShot_RangeA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_hopStep);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VHopStepShot_RangeB");

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::SPIN_MOVE_SHOT_RANGE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_RangeA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_spinMove);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_RangeB");

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_DistanceA");
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_DistanceB");
				DHOST_TYPE_FLOAT d = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_layUp);
				DHOST_TYPE_FLOAT e = m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_DistanceC");
				DHOST_TYPE_FLOAT f = m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_DistanceD");
				DHOST_TYPE_FLOAT g = m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_DistanceE");

				try
				{
					result = (LOG(a, b) / c) + (LOG(d, e) / f) + g;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::FINGER_ROLL_DISTANCE:
			{
				DHOST_TYPE_FLOAT a = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_DistanceA");
				DHOST_TYPE_FLOAT c = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_layUp);
				DHOST_TYPE_FLOAT d = m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_DistanceB");
				DHOST_TYPE_FLOAT e = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);

				try
				{
					result = a + (b * c) + (d * e);
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::SHAKE_AND_BAKE_LAYUP_RANGE:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VShakeAndBakeLayUp_RangeA");
				DHOST_TYPE_FLOAT b = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_shakeAndBake);
				DHOST_TYPE_FLOAT c = m_pHost->GetBalanceTable()->GetValue("VShakeAndBakeLayUp_RangeB");

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::EURO_STEP_LAYUP_DISTANCE_MAX:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayUp_Distance_MaxA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayUp_Distance_MaxB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_euroStepLayUp);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::QUICK_LAYUP_DISTANCE_MAX:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VQuickLayUp_Distance_MaxA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VQuickLayUp_Distance_MaxB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::quickLayUp);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			case EPLAYER_VARIABLE_TYPE::SHOT_FLOATER_DISTANCE_MAX:
			{
				DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VFloater_Distance_MaxA");
				DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VFloater_Distance_MaxB");
				DHOST_TYPE_FLOAT c = GetSkillLevel(pCharacter->GetCharacterInformation()->id, SKILL_INDEX::skill_floater);

				try
				{
					result = a + b * c;
				}
				catch (std::exception& e)
				{
					m_pHost->ToLog(e.what());
				}
			}
			break;
			
			
			default:
				break;
		}
	}

	return result;
}

DHOST_TYPE_FLOAT CVerifyManager::GetRandomValueAlgorithm(DHOST_TYPE_USER_ID userid, DHOST_TYPE_BOOL xorShift, DHOST_TYPE_INT32 randCount)
{
	//! ��ó�� ���������� ����ϴ°� �޸��� Ʈ������ �˰������ �����̰�, Ŭ�� �����ִ°� Ȯ���� ���� XorShift �˰����� ���°ɷ� (�׷��� Ŭ���� ���� ������ Ȯ���� �� ����)
	//! �Ϲ������� �޸��� �˰������� �� ���� ������ �����Ѱ� ��
	//! Ŭ��� ���� �����⸦ ������� XorShift �� ���°ǵ� ����� ������ ���� ����̶� ĳ���͸��� ������ ������ �� ���� ���߿� �ʿ��ϴٸ� ĳ������ �ٲ����
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	if (xorShift)
	{
		DHOST_TYPE_UINT32 seed = m_pHost->GetRandomSeed(userid);
		if (seed > kUINT32_INIT)
		{
			XorShifts::XorShift constRandom = XorShifts::XorShift(seed);

			if (randCount > kINT32_INIT)
			{
				for (int i = 0; i < randCount; ++i)
				{
					result = constRandom.randFloat();
				}
			}
			else
			{
				result = constRandom.randFloat();
			}
		}
	}
	else
	{
		result = Util::GetRandom(1.0f);
	}

	return result;
}

DHOST_TYPE_FLOAT CVerifyManager::GetBlockTeamAttributeBonus(const F4PACKET::play_c2s_playerBlock_data* pInfo)
{

	F4PACKET::SPlayerInformationT* pDefencePlayer = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
	F4PACKET::SPlayerInformationT* pAttackPlayer = m_pHost->FindCharacterInformation(pInfo->ownerid());

	F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
	SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());

	DHOST_TYPE_FLOAT returnValue = kFLOAT_INIT;

	DHOST_TYPE_FLOAT a = 0.0f;
	DHOST_TYPE_FLOAT b = 0.0f;
	DHOST_TYPE_FLOAT c = 1.0f;
	DHOST_TYPE_FLOAT d = 0.0f;
	DHOST_TYPE_FLOAT e = 0.0f;


	// �� �� �� ���̶� ����Ʈ�� ������ �ƴϸ� 
	if (!pAttackPlayer->teamcontrolluser || !pDefencePlayer->teamcontrolluser) return 0.0f;

	if (true)
	{
		switch (ownerShotType)
		{
		case SHOT_TYPE::shotType_layUpPost:
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:

			a = pAttackPlayer->teamattributesinfo->finishingatpost
				- (pDefencePlayer->teamattributesinfo->postdefense* m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_Layup_Offset1")
					+ pDefencePlayer->teamattributesinfo->shotcontesting * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_Layup_Offset2"));

			b = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_LayupA");
			c = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_LayupB");
			d = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_LayupC");
			e = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_LayupD");

			break;

		case SHOT_TYPE::shotType_dunkPost:
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:

			a = pAttackPlayer->teamattributesinfo->finishingatpost
				- (pDefencePlayer->teamattributesinfo->postdefense * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_Dunk_Offset1")
					+ pDefencePlayer->teamattributesinfo->shotcontesting * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_Dunk_Offset2"));

			b = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_DunkA");
			c = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_DunkB");
			d = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_DunkC");
			e = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_DunkD");

			break;

		case SHOT_TYPE::shotType_post:
		case SHOT_TYPE::shotType_postUnstready:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_closePostUnstready:
			a = pAttackPlayer->teamattributesinfo->finishingatpost
				- (pDefencePlayer->teamattributesinfo->postdefense * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShot_Offset1")
					+ pDefencePlayer->teamattributesinfo->shotcontesting * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShot_Offset2"));

			b = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShotA");
			c = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShotB");
			d = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShotC");
			e = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_PostShotD");

			break;

		case SHOT_TYPE::shotType_middle:
		case SHOT_TYPE::shotType_middleUnstready:
		case SHOT_TYPE::shotType_threePoint: // 
			a = pAttackPlayer->teamattributesinfo->perimetershooting
				- (pDefencePlayer->teamattributesinfo->perimeterdefense * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShot_Offset1")
					+ pDefencePlayer->teamattributesinfo->shotcontesting * m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShot_Offset2"));

			b = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShotA");
			c = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShotB");
			d = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShotC");
			e = m_pHost->GetBalanceTable()->GetValue("VTvT_BlockCondition_Probability_JumpShotD");

			break;

		default:
			return returnValue = 0.0f;
		}

	}

	try
	{
		if (a > 0.0f)
		{
			returnValue = (SafeLog(a, b) / c + d)*e;
		}
		else
		if (a < 0.0f)
		{
			a = abs(a);
			returnValue = (SafeLog(a, b) / c + d)*e;
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

// Ȯ�� ��� �ϴ� �� 
// 1. ���� �� ���� 
// 2. �޴�( �Ϻδ� Ŭ�󿡼� �������� �͵� ���� ) 
// 3. ���� Ȯ�� 
// 4. �ʹ� ���� 
// 5. ���� �� ���� 
// 
// **�� ���ʽ�( �̰��� Ŭ�󿡼� ) 
// �нú�, ��Ŭ�극��ũ, �ϸ����� Ŭ�󿡼� 

DHOST_TYPE_FLOAT CVerifyManager::GetBlockSuccessRate(const F4PACKET::play_c2s_playerBlock_data* pInfo)
{
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_FLOAT rescue = kFLOAT_INIT; // ���� �� ���� 

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
		if (pCharacterInfo != nullptr)
		{
			F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
			CCharacter* pOwnerCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->ownerid());
			if (pOwnerAction != nullptr && pOwnerCharacter != nullptr && pOwnerCharacter->GetAbility() != nullptr)
			{
				DHOST_TYPE_FLOAT handBonus = kFLOAT_INIT;
				DHOST_TYPE_INT32 medalValue = kINT32_INIT;
				
				SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());
				SKILL_INDEX skillIndex = pInfo->playeraction()->skillindex();

				TB::SVector3 positionRim;
				positionRim.mutate_x(RIM_POS_X);
				positionRim.mutate_y(kFLOAT_INIT);
				positionRim.mutate_z(RIM_POS_Z());

				TB::SVector3 positionBall;
				positionBall.mutate_x(pInfo->positionball()->x());
				positionBall.mutate_y(kFLOAT_INIT);
				positionBall.mutate_z(pInfo->positionball()->z());

				TB::SVector3 axis;
				axis.mutate_x(kFLOAT_INIT);
				axis.mutate_y(1.0f);
				axis.mutate_z(kFLOAT_INIT);

				TB::SVector3 directionBall;
				directionBall.mutate_x(kFLOAT_INIT);
				directionBall.mutate_y(kFLOAT_INIT);
				directionBall.mutate_z(kFLOAT_INIT);
				directionBall = CommonFunction::SVectorSub(positionBall, pInfo->playeraction()->positionlogic());

				DHOST_TYPE_FLOAT directionBallYaw = CommonFunction::SVectorSubYaw(positionBall, pInfo->playeraction()->positionlogic());

				TB::SVector3 shotDirection;
				shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());

				TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

				DHOST_TYPE_FLOAT angleDefenseTest = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection);
				DHOST_TYPE_FLOAT angleDefense = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

				DHOST_TYPE_FLOAT absAngleDefense = abs(angleDefense);

				if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownBlock)
				{
					DHOST_TYPE_FLOAT angle = m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle");

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockAngle);

					if (medalValue > kINT32_INIT)
					{
						angle += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle_Medal_ShadowBlock") * medalValue;
					}

					if (absAngleDefense < angle)
					{
						skillIndex = SKILL_INDEX::skill_none;
					}
				}

				switch (ownerShotType)
				{
					case SHOT_TYPE::shotType_dunkDriveIn:
					case SHOT_TYPE::shotType_dunkFingerRoll:
					case SHOT_TYPE::shotType_dunkPost:
					case SHOT_TYPE::shotType_layUpDriveIn:
					case SHOT_TYPE::shotType_layUpFingerRoll:
					case SHOT_TYPE::shotType_layUpPost:
					{
						// �� ���ʽ� 
						//! Ŭ������ �޾ƿ;ߵ� �������� �� �� ����

					}
					break;
					default:
					{
						if (skillIndex == SKILL_INDEX::skill_chasedownBlock || skillIndex == SKILL_INDEX::skill_chasedownCatch)
						{
							skillIndex = SKILL_INDEX::skill_none;
						}
					}
					break;
				}

				if (skillIndex == SKILL_INDEX::skill_chasedownBlock || skillIndex == SKILL_INDEX::skill_chasedownCatch)
				{
					switch (ownerShotType)
					{
						case SHOT_TYPE::shotType_dunkDriveIn:
						case SHOT_TYPE::shotType_dunkFingerRoll:
						case SHOT_TYPE::shotType_dunkPost:
						{
							successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_PROBABILITY_DUNK, pCharacter);

							if (ownerShotType == SHOT_TYPE::shotType_dunkPost)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Post");
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockProbability);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_Medal_ShadowBlock") * medalValue;
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::shadowBlock);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_Medal_ShadowBlock") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::highlightFilm);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_HighlightFilm") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Monster") * medalValue;
							}

							//! ü�̽��ٿ� ���� ĳġ ��ų�� �ִٸ�
							DHOST_TYPE_FLOAT catchProbability = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_CATCH_PROBABILITY_DUNK, pCharacter);

							if (catchProbability > kFLOAT_INIT)
							{
								medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowCatch);

								if (medalValue > kINT32_INIT)
								{
									catchProbability += m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_Medal_ShadowBlock") * medalValue;
								}

								if (Util::GetRandom(1.0f) <= catchProbability)
								{
									skillIndex = SKILL_INDEX::skill_chasedownCatch;
								}
							}
						}
						break;
						case SHOT_TYPE::shotType_layUpDriveIn:
						case SHOT_TYPE::shotType_layUpFingerRoll:
						case SHOT_TYPE::shotType_layUpPost:
						{
							successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_PROBABILITY_LAYUP, pCharacter);

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockProbability);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_Medal_ShadowBlock") * medalValue;
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::shadowBlock);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VChaseDownBlock_Probability_Medal_ShadowBlock") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::delicateTouch);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayUp_DelicateTouch") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
							}

							//! ü�̽��ٿ� ���� ĳġ ��ų�� �ִٸ�
							DHOST_TYPE_FLOAT catchProbability = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CHASE_DOWN_CATCH_PROBABILITY_LAYUP, pCharacter);

							if (catchProbability > kFLOAT_INIT)
							{
								medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowCatch);

								if (medalValue > kINT32_INIT)
								{
									catchProbability += m_pHost->GetBalanceTable()->GetValue("VChaseDownCatch_Probability_Medal_ShadowBlock") * medalValue;
								}

								if (Util::GetRandom(1.0f) <= catchProbability)
								{
									skillIndex = SKILL_INDEX::skill_chasedownCatch;
								}
							}
						}
						break;
						default:
						{
							successRate = kFLOAT_INIT;
						}
						break;
					}
				}//! ü�̽��ٿ�
				else
				{
					switch (ownerShotType)
					{
						case SHOT_TYPE::shotType_dunkDriveIn:
						case SHOT_TYPE::shotType_dunkFingerRoll:
						case SHOT_TYPE::shotType_dunkPost:
						{
							switch (skillIndex)
							{
								case SKILL_INDEX::skill_blockCatch:
								{
									if (CheckHaveSkill(pCharacterInfo->id, SKILL_INDEX::skill_blockCatchDunk, false))
									{
										successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_DUNK, pCharacter, pOwnerCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_strength));
										//! �� ��ġ�� Ŭ���ڵ�� �нú갡 ������ (��Ŷ�� �нú� �������� ���� �޾Ƽ� ���ؾߵ�)
										// successRate -= owner.PassiveManager.GetEffect(Passive.EFFECT.blockResistance_dunk);
										skillIndex = SKILL_INDEX::skill_blockCatchDunk;
									}
									else
									{
										successRate = kFLOAT_INIT;
									}
								}
								break;
								case SKILL_INDEX::skill_stretchBlock:
								{
									successRate = kFLOAT_INIT;
								}
								break;
								default:
								{
									successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_DUNK, pCharacter, pOwnerCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_strength));
									//! �� ��ġ�� Ŭ���ڵ�� �нú갡 ������ (��Ŷ�� �нú� �������� ���� �޾Ƽ� ���ؾߵ�)
									// successRate -= owner.PassiveManager.GetEffect(Passive.EFFECT.blockResistance_dunk);
								}
								break;
							}

							if (ownerShotType == SHOT_TYPE::shotType_dunkPost)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Post");
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::highlightFilm);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_HighlightFilm") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeDunk);

							if (medalValue > kINT32_INIT)
							{
								successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Medal_Fortitude") * medalValue;
							}

							//! medal_putBackAttackerDunk ���⿡ ��Ÿ�� ����Ǵ°� �־ Ŭ�󿡼� ���� �޾ƾߵ�
							/*if (owner.HaveMedal(MEDAL_INDEX.medal_putBackAttackerDunk) == true && owner.ActionManager.CoolTimer.IsPassed(CoolTimer.TYPE.rebound_offense, 4.0f) == false)
							{
								successRate -= managerBalance.GetValue("BlockCondition_Probability_Dunk_Medal_PutBackAttacker_Dunk") * owner.GetMedalValue(MEDAL_INDEX.medal_putBackAttackerDunk);
							}*/

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_mismatchChefRimAttack);

							if (medalValue > kINT32_INIT && (pOwnerCharacter->GetCharacterInformation()->height > pCharacterInfo->height))
							{
								successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_MismatchChef") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Monster") * medalValue;
							}
						}
						break;
						case SHOT_TYPE::shotType_layUpDriveIn:
						case SHOT_TYPE::shotType_layUpFingerRoll:
						case SHOT_TYPE::shotType_layUpPost:
						{
							successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_LAYUP, pCharacter);
							//! �нú갪 �����ؾߵ�
							//! successRate -= owner.PassiveManager.GetEffect(Passive.EFFECT.blockResistance_layUp);

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeLayUp);

							if (medalValue > kINT32_INIT)
							{
								successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayUp_Medal_Fortitude") * medalValue;
							}

							//! medal_putBackAttackerDunk ���⿡ ��Ÿ�� ����Ǵ°� �־ Ŭ�󿡼� ���� �޾ƾߵ�
							/*
							* if (owner.HaveMedal(MEDAL_INDEX.medal_putBackAttackerLayUp) == true && owner.ActionManager.CoolTimer.IsPassed(CoolTimer.TYPE.rebound_offense, 4.0f) == false)
								{
									successRate -= managerBalance.GetValue("BlockCondition_Probability_LayUp_Medal_PutBackAttacker_LayUp") * owner.GetMedalValue(MEDAL_INDEX.medal_putBackAttackerLayUp);
								}
							*/

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_mismatchChefRimAttack);

							if (medalValue > kINT32_INIT && (pOwnerCharacter->GetCharacterInformation()->height > pCharacterInfo->height))
							{
								successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_MismatchChef") * medalValue;
							}

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::delicateTouch);

							if (medalValue > kINT32_INIT)
							{
								successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayUp_DelicateTouch") * medalValue;
							}
						}
						break;
						case SHOT_TYPE::shotType_middle:
						case SHOT_TYPE::shotType_middleUnstready:
						case SHOT_TYPE::shotType_threePoint:
						{
							TB::SVector3 velocityBall;
							velocityBall.mutate_x(kFLOAT_INIT);
							velocityBall.mutate_y(kFLOAT_INIT);
							velocityBall.mutate_z(kFLOAT_INIT);

							if (m_pHost->BallControllerGet() != nullptr)
							{
								DHOST_TYPE_FLOAT timeDelay = m_pHost->GetRoomElapsedTime() - pInfo->clienttime();
								DHOST_TYPE_FLOAT predictTime = kCLIENT_BLOCK_EVENT_TIME - timeDelay;

								velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(predictTime));

								if (velocityBall.y() >= kFLOAT_INIT)
								{
									successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP, pCharacter);
								}
								else
								{

									string verify_log = "[BLOCK_SUCCESS_RATE] velocityBall is invalid RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", velocityBallX : " + std::to_string(velocityBall.x())
										+ ", velocityBallY : " + std::to_string(velocityBall.y())
										+ ", velocityBallZ : " + std::to_string(velocityBall.z());
									m_pHost->ToLog(verify_log.c_str());
								}

								if (ownerShotType == SHOT_TYPE::shotType_threePoint)
								{
									medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeThreePoint);

									if (medalValue > kINT32_INIT)
									{
										successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_ThreePointShot_Medal_Fortitude") * medalValue;
									}
								}
								else
								{
									medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeMid);

									if (medalValue > kINT32_INIT)
									{
										successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_MidRangeShot_Medal_Fortitude") * medalValue;
									}
								}

								if (pOwnerAction->skillindex() == SKILL_INDEX::skill_rainbowShot)
								{
									successRate -= GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP_SHOT_RAINBOW_SHOT, pOwnerCharacter);
								}
							}
							else
							{
								string verify_log = "[BLOCK_SUCCESS_RATE] BallControllerGet is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex());
								m_pHost->ToLog(verify_log.c_str());
							}
						}
						break;
						case SHOT_TYPE::shotType_closePostUnstready:
						case SHOT_TYPE::shotType_hook:
						case SHOT_TYPE::shotType_post:
						case SHOT_TYPE::shotType_postUnstready:
						{
							TB::SVector3 velocityBall;
							velocityBall.mutate_x(kFLOAT_INIT);
							velocityBall.mutate_y(kFLOAT_INIT);
							velocityBall.mutate_z(kFLOAT_INIT);

							if (m_pHost->BallControllerGet() != nullptr)
							{
								DHOST_TYPE_FLOAT timeDelay = m_pHost->GetRoomElapsedTime() - pInfo->clienttime();
								DHOST_TYPE_FLOAT predictTime = kCLIENT_BLOCK_EVENT_TIME - timeDelay;

								velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(predictTime));

								if (velocityBall.y() >= kFLOAT_INIT)
								{
									successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_POST, pCharacter);
								}
								else
								{
									string verify_log = "[BLOCK_SUCCESS_RATE] velocityBall is invalid RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", velocityBallX : " + std::to_string(velocityBall.x())
										+ ", velocityBallY : " + std::to_string(velocityBall.y())
										+ ", velocityBallZ : " + std::to_string(velocityBall.z());
									m_pHost->ToLog(verify_log.c_str());
								}

								medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudePost);

								if (medalValue > kINT32_INIT)
								{
									successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShot_Medal_Fortitude") * medalValue;
								}
							}
							else
							{
								string verify_log = "[BLOCK_SUCCESS_RATE] BallControllerGet is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex());
								m_pHost->ToLog(verify_log.c_str());
							}
						}
						break;
					}
				}//! ü�̽��ٿ��� �ƴѰ�

				if (pOwnerAction->skillindex() == SKILL_INDEX::tomahawkDunk)
				{
					DHOST_TYPE_INT32 animIndex = pOwnerCharacter->GetCharacterStateAnimationIndex();

					std::string animName = "";

					if (m_pHost->GetAnimationController() != nullptr)
					{
						animName = m_pHost->GetAnimationController()->GetAnimationName(animIndex);

						if (animName.find("BACK") != string::npos)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Tomahawk");
						}
					}

					// �丶ȣũ ��ų������ �۾� 
					float subValue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Tomahawk_LevelA") + m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Tomahawk_LevelB")
						* GetSkillLevel(pInfo->ownerid(), SKILL_INDEX::tomahawkDunk);

					successRate -= subValue;
				}

				switch (ownerShotType)
				{
					case SHOT_TYPE::shotType_dunkFingerRoll:
					case SHOT_TYPE::shotType_layUpFingerRoll:
					{
						DHOST_TYPE_FLOAT a = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_FingerRollLayUpA");
						DHOST_TYPE_FLOAT b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_FingerRollLayUpB");
						DHOST_TYPE_FLOAT c = kFLOAT_INIT;

						if (ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll)
						{
							c = pOwnerCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_dunk);
						}
						else
						{
							c = pOwnerCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_layUp);
						}

						successRate -= a + b * c;
					}
					break;
					default:
					{

					}
					break;
				}

				if (skillIndex == SKILL_INDEX::skill_stretchBlock)
				{
					successRate = successRate - m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_StretchBlockA")
						+ m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_StretchBlockB")
						* GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::skill_stretchBlock);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_stretchBlock);

					if (medalValue > kINT32_INIT)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_StretchBlock_Medal") * medalValue;
					}

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::stretchBlock);

					if (medalValue > kINT32_INIT)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_StretchBlock_Medal") * medalValue;
					}
				}

				if (skillIndex == SKILL_INDEX::spikeBlock)
				{
					successRate = successRate + m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_SpikeBlockA")
						+ m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_SpikeBlockB")
						* GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::spikeBlock);
				}

				successRate += handBonus;

				medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_aerialAcrobat);

				//! ��� ���� ����ߴ��� ��Ȯ���� �ʱ� ������ Ŭ�󿡼� �޾ƿ;ߵ�
				/*if (medalValue > kINT32_INIT)
				{
					if (((m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == true) && angleDefense >= kFLOAT_INIT) || ((m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == false) && angleDefense < kFLOAT_INIT))
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_AerialAcrobat") * medalValue;
					}
				}*/

				//! ��� ���� ����ߴ��� ��Ȯ���� �ʱ� ������ Ŭ�󿡼� �޾ƿ;ߵ�
				/*medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_leftHandedWizard);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == true)
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_LeftHandedWizard") * medalValue;
					}
				}

				medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_rightHandedWizard);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == false)
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_RightHandedWizard") * medalValue;
					}
				}*/

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_hotStartBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == false)
					{
						if (m_pHost->GetGameTimeInit() - m_pHost->GetGameTime() < 60.0f)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_HotStart_Block") * medalValue;
						}
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_stableFinishBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == false)
					{
						if (m_pHost->GetGameTime() < 60.0f)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_StableFinish_Block") * medalValue;
						}
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_overTimeRulerBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == true)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_OverTimeRuler_Block") * medalValue;
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_competitiveSpiritBlock);

				if (medalValue > kINT32_INIT)
				{
					DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(pOwnerCharacter->GetCharacterInformation()->team) - m_pHost->GetScore(pCharacterInfo->team);

					if (scoreDelta > kINT32_INIT)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_CompetitiveSpirit_Block") * medalValue;
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_rimProtector);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_RimProtector_Block") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder1);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder1") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder2);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder2") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::almightyDefender);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_AlmightyDefender") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::notInMyHouse);

				if (medalValue > kINT32_INIT)
				{
					switch (ownerShotType)
					{
						case SHOT_TYPE::shotType_dunkDriveIn:
						case SHOT_TYPE::shotType_dunkFingerRoll:
						case SHOT_TYPE::shotType_dunkPost:
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Medal_NotInMyHouse") * medalValue;
						}
						break;
						default:
						{

						}
						break;
					}	
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::guardian);

				if (medalValue > kINT32_INIT)
				{
					switch (ownerShotType)
					{
						case SHOT_TYPE::shotType_dunkDriveIn:
						case SHOT_TYPE::shotType_dunkFingerRoll:
						case SHOT_TYPE::shotType_dunkPost:
						case SHOT_TYPE::shotType_layUpDriveIn:
						case SHOT_TYPE::shotType_layUpFingerRoll:
						case SHOT_TYPE::shotType_layUpPost:
						case SHOT_TYPE::shotType_post:
						case SHOT_TYPE::shotType_hook:
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_Guardian") * medalValue;
						}
						break;
						default:
						{

						}
						break;
					}
				}

				switch (skillIndex)
				{
					case SKILL_INDEX::skill_blockCatchDunk:
					{
						successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_AND_CATCH_DUNK_PROBABILITY, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goldenGlove);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockAndCatch_Probability_Medal_GoldGlove") * medalValue;
						}
					}
					break;
					case SKILL_INDEX::skill_blockCatch:
					{
						successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_AND_CATCH_PROBABILITY, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goldenGlove);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockAndCatch_Probability_Medal_GoldGlove") * medalValue;
						}
					}
					break;
					case SKILL_INDEX::skill_powerBlock:
					{
						successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_POWER_BLOCK_PROBABILITY, pCharacter);

						if (ownerShotType == SHOT_TYPE::shotType_dunkDriveIn || ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll || ownerShotType == SHOT_TYPE::shotType_dunkPost)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_PowerBlock_Bonus");
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_spikeBlock);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Probability_PowerBlock_Medal") * medalValue;
						}
					}
					break;
				}

				//! �нú� ȿ�� �߰��ؾߵ�
				/*
				if (ownerActionShot is not null)
                successRate += player.PassiveManager.GetEffect(Passive.EFFECT.defense_blockSuccessRate);
				*/

				//! �ñ״�ó ȿ�� �߰��ؾߵ�
				/*
				if (ownerActionShot is not null)
				{
					DataAnimation.CATEGORY signatureCategory = ownerActionShot.ShotType switch
					{
						SHOT_TYPE.shotType_layUpPost => DataAnimation.CATEGORY.postLayUp,
						SHOT_TYPE.shotType_layUpDriveIn => DataAnimation.CATEGORY.drivingLayUp,
						SHOT_TYPE.shotType_layUpFingerRoll => DataAnimation.CATEGORY.fingerRollLayUp,
						SHOT_TYPE.shotType_dunkPost => DataAnimation.CATEGORY.postDunk,
						SHOT_TYPE.shotType_dunkDriveIn => DataAnimation.CATEGORY.drivingDunk,
						SHOT_TYPE.shotType_dunkFingerRoll => DataAnimation.CATEGORY.fingerRollDunk,
						_ => DataAnimation.CATEGORY.none,
					};

					int signatureCurrentIndex = owner.SignatureManager.GetSignatureIndex(signatureCategory, ownerActionShot.GetAnimID);
					if (signatureCurrentIndex > 0)
					{
						float signatureBonusValue = managerBalance.GetValue("Signature_Benefit_RimAttack");
						successRate += signatureBonusValue;
					}
				}
				*/

				//! ��Ŭ�극��ũ ȿ�� �߰��ؾߵ�
				/*
				if (ownerActionShot is not null)
				{
					if (owner.ActionManager.CoolTimer.withinTime(CoolTimer.TYPE.ankleBreakeSuccess, 2))
					{
						switch (ownerActionShot.ShotType)
						{
							case SHOT_TYPE.shotType_dunkPost:
							case SHOT_TYPE.shotType_dunkDriveIn:
							case SHOT_TYPE.shotType_dunkFingerRoll:
							case SHOT_TYPE.shotType_layUpPost:
							case SHOT_TYPE.shotType_layUpDriveIn:
							case SHOT_TYPE.shotType_layUpFingerRoll:
								successRate += managerBalance.GetValue("BlockCondition_Probability_AnkleBonus");
								break;
						}
					}
				}
				*/

				//! ���� �ʹ� ����
				successRate += CorrectionEarlyInTheGame(pCharacterInfo->id, ECORRECTION_EARLY_TYPE::BLOCK);

				//! �������� ������ ���� ���� (�ϸ��� ��ũ��) - Ŭ�󿡼� �����;ߵ�
				/*
				if (ownerActionShot is not null)
				{
					switch (ownerActionShot.ShotType)
					{
						case SHOT_TYPE.shotType_dunkPost:
						case SHOT_TYPE.shotType_dunkDriveIn:
						case SHOT_TYPE.shotType_dunkFingerRoll:
						case SHOT_TYPE.shotType_layUpPost:
						case SHOT_TYPE.shotType_layUpDriveIn:
						case SHOT_TYPE.shotType_layUpFingerRoll:
							{
								successRate += owner.immoralDetector.GetDataValue.rimAttackBlockProbability;
							}
							break;
					}
				}
				*/

				//! ������ ����
				switch (m_pHost->GetModeType())
				{
					case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
					case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
						break;
					default:
					{
						DHOST_TYPE_INT32 oppnentTeam = m_pHost->GetIndexOpponentTeam(pCharacterInfo->team);
						DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(oppnentTeam) - m_pHost->GetScore(pCharacterInfo->team);

						if (scoreDelta >= m_pHost->GetGameManager()->GetRescue2ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue2");
						}
						else if (scoreDelta >= m_pHost->GetGameManager()->GetRescue1ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue1");
						}
					}
					break;
				}

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Pre RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}
				

				//! �������� ���̾�, �ı����� ��ũ ��ȭ�� �ߵ����� �� ���� ������ ���� (�������� �� ��ȭ�ƴ��� Ȯ��)
				float re =  GetBlockSuccessRateWithRimAttacktPotential(pInfo->ownerid(), ownerShotType, pOwnerAction->skillindex(), pInfo->ballnumber());
				successRate -= re; // GetBlockSuccessRateWithRimAttacktPotential(pInfo->ownerid(), ownerShotType, pOwnerAction->skillindex(), pInfo->ballnumber());
				subBlockPotentialBuffDebug = re;

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Post RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}


				{
					string verify_log = "[POTENTIAL_PERSISTENT_POST_DEF] Pre RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}

				//! �����
				DHOST_TYPE_FLOAT potential_bonus = VerifyBlockPotential(pCharacterInfo, ownerShotType, skillIndex, pInfo->potentialfxlevel());
				addBlockPotentialBuffDebug = potential_bonus;

				if (potential_bonus < kFLOAT_INIT)
				{
					successRate = kFLOAT_INIT;
				}
				else
				{
					successRate += potential_bonus;
				}

				{
					string verify_log = "[POTENTIAL_PERSISTENT_POST_DEF] Post RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}
			}
			else
			{
				string verify_log = "GetBlockSuccessRate_pOwnerAction_pOwnerCharacter_pOwnerCharacter->GetAbility_is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id());
				m_pHost->ToLog(verify_log.c_str());
			}
		}
		else
		{
			string verify_log = "GetBlockSuccessRate_pCharacterInfo_is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id());
			m_pHost->ToLog(verify_log.c_str());
		}
	}
	else
	{
		string verify_log = "GetBlockSuccessRate_pCharacter_pCharacter->GetAbility()_is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id());
		m_pHost->ToLog(verify_log.c_str());
	}

	return successRate + rescue;
}

DHOST_TYPE_FLOAT CVerifyManager::GetBlockSuccessRateSnatch(const F4PACKET::play_c2s_playerBlock_data* pInfo)
{
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_FLOAT rescue = kFLOAT_INIT;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
		if (pCharacterInfo != nullptr)
		{
			F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
			CCharacter* pOwnerCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->ownerid());
			if (pOwnerAction != nullptr && pOwnerCharacter != nullptr && pOwnerCharacter->GetAbility() != nullptr)
			{
				DHOST_TYPE_FLOAT handBonus = kFLOAT_INIT;
				DHOST_TYPE_INT32 medalValue = kINT32_INIT;

				SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());
				SKILL_INDEX skillIndex = pInfo->playeraction()->skillindex();

				TB::SVector3 positionRim;
				positionRim.mutate_x(RIM_POS_X);
				positionRim.mutate_y(kFLOAT_INIT);
				positionRim.mutate_z(RIM_POS_Z());

				TB::SVector3 positionBall;
				positionBall.mutate_x(pInfo->positionball()->x());
				positionBall.mutate_y(kFLOAT_INIT);
				positionBall.mutate_z(pInfo->positionball()->z());

				TB::SVector3 axis;
				axis.mutate_x(kFLOAT_INIT);
				axis.mutate_y(1.0f);
				axis.mutate_z(kFLOAT_INIT);

				TB::SVector3 directionBall;
				directionBall.mutate_x(kFLOAT_INIT);
				directionBall.mutate_y(kFLOAT_INIT);
				directionBall.mutate_z(kFLOAT_INIT);
				directionBall = CommonFunction::SVectorSub(positionBall, pInfo->playeraction()->positionlogic());

				DHOST_TYPE_FLOAT directionBallYaw = CommonFunction::SVectorSubYaw(positionBall, pInfo->playeraction()->positionlogic());

				TB::SVector3 shotDirection;
				shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());

				TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

				DHOST_TYPE_FLOAT angleDefenseTest = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection);
				DHOST_TYPE_FLOAT angleDefense = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

				DHOST_TYPE_FLOAT absAngleDefense = abs(angleDefense);

				//! �� �׼ǿ� ���� ���� Ȯ�� ����
				switch (ownerShotType)
				{
					case SHOT_TYPE::shotType_middle:
					case SHOT_TYPE::shotType_middleUnstready:
					case SHOT_TYPE::shotType_threePoint:
					{
						successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP, pCharacter, kFLOAT_INIT);
					}
					break;
					case SHOT_TYPE::shotType_closePostUnstready:
					case SHOT_TYPE::shotType_hook:
					case SHOT_TYPE::shotType_post:
					case SHOT_TYPE::shotType_postUnstready:
					{
						successRate = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_POST, pCharacter, kFLOAT_INIT);
					}
					break;
					default:
					{

					}
					break;
				}

				//! �޴��� ���� ���� Ȯ�� ����
				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder1);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder1") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder2);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder2") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::almightyDefender);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_AlmightyDefender") * medalValue;
				}

				//! �нú꿡 ���� Ȯ�� ���� (Ŭ�󿡼� �����;ߵ�)
				/*
				successRate += player.PassiveManager.GetEffect(Passive.EFFECT.defense_blockSuccessRate);
				*/

				//! �ñ״�ó�� ���� ȿ�� (Ŭ�󿡼� �����;ߵ�)
				/*
				DataAnimation.CATEGORY signatureCategory = ownerActionShot.ShotType switch
                {
                    SHOT_TYPE.shotType_layUpPost => DataAnimation.CATEGORY.postLayUp,
                    SHOT_TYPE.shotType_layUpDriveIn => DataAnimation.CATEGORY.drivingLayUp,
                    SHOT_TYPE.shotType_layUpFingerRoll => DataAnimation.CATEGORY.fingerRollLayUp,
                    SHOT_TYPE.shotType_dunkPost => DataAnimation.CATEGORY.postDunk,
                    SHOT_TYPE.shotType_dunkDriveIn => DataAnimation.CATEGORY.drivingDunk,
                    SHOT_TYPE.shotType_dunkFingerRoll => DataAnimation.CATEGORY.fingerRollDunk,
                    _ => DataAnimation.CATEGORY.none,
                };

                int signatureCurrentIndex = owner.SignatureManager.GetSignatureIndex(signatureCategory, ownerActionShot.GetAnimID);
                if (signatureCurrentIndex > 0)
                {
                    float signatureBonusValue = managerBalance.GetValue("Signature_Benefit_RimAttack");
                    successRate += signatureBonusValue;
                }
				*/

				//! ������ ����
				switch (m_pHost->GetModeType())
				{
					case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
					case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
						break;
					default:
					{
						DHOST_TYPE_INT32 oppnentTeam = m_pHost->GetIndexOpponentTeam(pCharacterInfo->team);
						DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(oppnentTeam) - m_pHost->GetScore(pCharacterInfo->team);

						if (scoreDelta >= m_pHost->GetGameManager()->GetRescue2ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue2");
						}
						else if (scoreDelta >= m_pHost->GetGameManager()->GetRescue1ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue1");
						}
					}
					break;
				}

				//! ���� �ʹ� ����
				successRate += CorrectionEarlyInTheGame(pCharacterInfo->id, ECORRECTION_EARLY_TYPE::BLOCK);

				//! ��ų ȿ�� ����
				successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::SNATCH_BLOCK_BLOCK_PROBABILITY, pCharacter, kFLOAT_INIT);

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Pre RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);

					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}


				//! �������� ���̾�, �ı����� ��ũ ��ȭ�� �ߵ����� �� ���� ������ ���� (�������� �� ��ȭ�ƴ��� Ȯ��)
				//! 				
				float re = GetBlockSuccessRateWithRimAttacktPotential(pInfo->ownerid(), ownerShotType, pOwnerAction->skillindex(), pInfo->ballnumber());
				successRate -= re; // GetBlockSuccessRateWithRimAttacktPotential(pInfo->ownerid(), ownerShotType, pOwnerAction->skillindex(), pInfo->ballnumber());
				subBlockPotentialBuffDebug = re;

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Post RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}

				//! �����
				DHOST_TYPE_FLOAT potential_bonus = VerifyBlockPotential(pCharacterInfo, ownerShotType, skillIndex, pInfo->potentialfxlevel());
				addBlockPotentialBuffDebug = potential_bonus;
					 
				if (potential_bonus < kFLOAT_INIT)
				{
					successRate = kFLOAT_INIT;
				}
				else
				{
					successRate += potential_bonus;
				}
			}
		}
	}

	return successRate + rescue;
}

DHOST_TYPE_FLOAT CVerifyManager::GetBlockSuccessRateDoubleHandsUp(const F4PACKET::play_c2s_playerBlock_data* pInfo)
{
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_FLOAT rescue = kFLOAT_INIT;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
		if (pCharacterInfo != nullptr)
		{
			F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
			CCharacter* pOwnerCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->ownerid());
			if (pOwnerAction != nullptr && pOwnerCharacter != nullptr && pOwnerCharacter->GetAbility() != nullptr)
			{
				DHOST_TYPE_FLOAT handBonus = kFLOAT_INIT;
				DHOST_TYPE_INT32 medalValue = kINT32_INIT;

				SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());
				SKILL_INDEX skillIndex = pInfo->playeraction()->skillindex();

				TB::SVector3 positionRim;
				positionRim.mutate_x(RIM_POS_X);
				positionRim.mutate_y(kFLOAT_INIT);
				positionRim.mutate_z(RIM_POS_Z());

				TB::SVector3 positionBall;
				positionBall.mutate_x(pInfo->positionball()->x());
				positionBall.mutate_y(kFLOAT_INIT);
				positionBall.mutate_z(pInfo->positionball()->z());

				TB::SVector3 axis;
				axis.mutate_x(kFLOAT_INIT);
				axis.mutate_y(1.0f);
				axis.mutate_z(kFLOAT_INIT);

				TB::SVector3 directionBall;
				directionBall.mutate_x(kFLOAT_INIT);
				directionBall.mutate_y(kFLOAT_INIT);
				directionBall.mutate_z(kFLOAT_INIT);
				directionBall = CommonFunction::SVectorSub(positionBall, pInfo->playeraction()->positionlogic());

				DHOST_TYPE_FLOAT directionBallYaw = CommonFunction::SVectorSubYaw(positionBall, pInfo->playeraction()->positionlogic());

				TB::SVector3 shotDirection;
				shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());

				TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

				DHOST_TYPE_FLOAT angleDefenseTest = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection);
				DHOST_TYPE_FLOAT angleDefense = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

				DHOST_TYPE_FLOAT absAngleDefense = abs(angleDefense);

				//! �޴��� ���� ���� Ȯ�� ����
				//medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_aerialAcrobat);

				//! ��� ���� ����ߴ��� ��Ȯ���� �ʱ� ������ Ŭ�󿡼� �޾ƿ;ߵ�
				/*if (medalValue > kINT32_INIT)
				{
					if (((m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == true) && angleDefense >= kFLOAT_INIT) || ((m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == false) && angleDefense < kFLOAT_INIT))
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_AerialAcrobat") * medalValue;
					}
				}*/

				//! ��� ���� ����ߴ��� ��Ȯ���� �ʱ� ������ Ŭ�󿡼� �޾ƿ;ߵ�
				/*medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_leftHandedWizard);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == true)
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_LeftHandedWizard") * medalValue;
					}
				}

				medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_rightHandedWizard);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetShotIsLeftHanded(pInfo->ownerid()) == false)
					{
						successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_RightHandedWizard") * medalValue;
					}
				}*/

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_hotStartBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == false)
					{
						if (m_pHost->GetGameTimeInit() - m_pHost->GetGameTime() < 60.0f)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_HotStart_Block") * medalValue;
						}
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_stableFinishBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == false)
					{
						if (m_pHost->GetGameTime() < 60.0f)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_StableFinish_Block") * medalValue;
						}
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_overTimeRulerBlock);

				if (medalValue > kINT32_INIT)
				{
					if (m_pHost->GetOverTime() == true)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_OverTimeRuler_Block") * medalValue;
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_competitiveSpiritBlock);

				if (medalValue > kINT32_INIT)
				{
					DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(pOwnerCharacter->GetCharacterInformation()->team) - m_pHost->GetScore(pCharacterInfo->team);

					if (scoreDelta > kINT32_INIT)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_CompetitiveSpirit_Block") * medalValue;
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_clutchPlayerBlock);

				if (medalValue > kINT32_INIT)
				{
					DHOST_TYPE_INT32 scoreDelta = abs(m_pHost->GetScore(pOwnerCharacter->GetCharacterInformation()->team) - m_pHost->GetScore(pCharacterInfo->team));

					if (scoreDelta <= 3)
					{
						successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_ClutchPlayer_Block") * medalValue;
					}
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_rimProtector);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_RimProtector_Block") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder1);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder1") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::defensiveAllRounder2);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_DefensiveAllRounder2") * medalValue;
				}

				medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::almightyDefender);

				if (medalValue > kINT32_INIT)
				{
					successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_AlmightyDefender") * medalValue;
				}

				//! ��ų:����������� ���� ȿ�� ����
				successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DOUBLE_HANDS_UP_BLOCK_PROBABILITY, pCharacter);

				//! �нú꿡 ���� ���� Ȯ�� ���� (Ŭ�󿡼� �����;� ��)
				/*
				retValue += blocker.PassiveManager.GetEffect(Passive.EFFECT.defense_blockSuccessRate);
				*/

				//! �� �׼ǿ� ���� ���� Ȯ�� ����
				switch (ownerShotType)
				{
					case SHOT_TYPE::shotType_dunkDriveIn:
					case SHOT_TYPE::shotType_dunkFingerRoll:
					case SHOT_TYPE::shotType_dunkPost:
					{
						successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_DUNK, pCharacter, pOwnerCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_strength), kFLOAT_INIT);
						// �нú갪 �����;ߵ�
						/*
						retValue -= owner.PassiveManager.GetEffect(Passive.EFFECT.blockResistance_dunk);
						*/


						if (ownerShotType == SHOT_TYPE::shotType_dunkPost)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Post");
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeDunk);

						if (medalValue > kINT32_INIT)
						{
							successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_Medal_Fortitude") * medalValue;
						}

						//! medal_putBackAttackerDunk ���⿡ ��Ÿ�� ����Ǵ°� �־ Ŭ�󿡼� ���� �޾ƾߵ�
						/*if (owner.HaveMedal(MEDAL_INDEX.medal_putBackAttackerDunk) == true && owner.ActionManager.CoolTimer.IsPassed(CoolTimer.TYPE.rebound_offense, 4.0f) == false)
						{
							successRate -= managerBalance.GetValue("BlockCondition_Probability_Dunk_Medal_PutBackAttacker_Dunk") * owner.GetMedalValue(MEDAL_INDEX.medal_putBackAttackerDunk);
						}*/

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_mismatchChefRimAttack);

						if (medalValue > kINT32_INIT && (pOwnerCharacter->GetCharacterInformation()->height > pCharacterInfo->height))
						{
							successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_MismatchChef") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::highlightFilm);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Dunk_HighlightFilm") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Monster") * medalValue;
						}
					}
					break;
					case SHOT_TYPE::shotType_layUpDriveIn:
					case SHOT_TYPE::shotType_layUpFingerRoll:
					case SHOT_TYPE::shotType_layUpPost:
					{
						successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_LAYUP, pCharacter);
						//! �нú갪 �����ؾߵ�
						//! successRate -= owner.PassiveManager.GetEffect(Passive.EFFECT.blockResistance_layUp);

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeLayUp);

						if (medalValue > kINT32_INIT)
						{
							successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayUp_Medal_Fortitude") * medalValue;
						}

						//! medal_putBackAttackerLayUp ���⿡ ��Ÿ�� ����Ǵ°� �־ Ŭ�󿡼� ���� �޾ƾߵ�
						/*
						* if (owner.HaveMedal(MEDAL_INDEX.medal_putBackAttackerLayUp) == true && owner.ActionManager.CoolTimer.IsPassed(CoolTimer.TYPE.rebound_offense, 4.0f) == false)
							{
								successRate -= managerBalance.GetValue("BlockCondition_Probability_LayUp_Medal_PutBackAttacker_LayUp") * owner.GetMedalValue(MEDAL_INDEX.medal_putBackAttackerLayUp);
							}
						*/

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_mismatchChefRimAttack);

						if (medalValue > kINT32_INIT && (pOwnerCharacter->GetCharacterInformation()->height > pCharacterInfo->height))
						{
							successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Medal_MismatchChef") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::fortitude2);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Fortitude2") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::delicateTouch);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_LayUp_DelicateTouch") * medalValue;
						}

						medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::aerialAcrobat);

						if (medalValue > kINT32_INIT)
						{
							successRate += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_RimAttack_AerialAcrobat") * medalValue;
						}
					}
					break;
					case SHOT_TYPE::shotType_middle:
					case SHOT_TYPE::shotType_middleUnstready:
					case SHOT_TYPE::shotType_threePoint:
					{
						TB::SVector3 velocityBall;
						velocityBall.mutate_x(kFLOAT_INIT);
						velocityBall.mutate_y(kFLOAT_INIT);
						velocityBall.mutate_z(kFLOAT_INIT);

						if (m_pHost->BallControllerGet() != nullptr)
						{
							//! ��������� ������ �� �ƿ� ������ ����ͼ� ���ν�Ƽ���� ���� �� ����
							/*DHOST_TYPE_FLOAT timeDelay = m_pHost->GetRoomElapsedTime() - pInfo->clienttime();
							DHOST_TYPE_FLOAT predictTime = 0.34f - timeDelay;

							velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(predictTime));

							if (velocityBall.y() >= kFLOAT_INIT)
							{
								successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP, pCharacter);
							}*/

							successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP, pCharacter);

							if (ownerShotType == SHOT_TYPE::shotType_threePoint)
							{
								medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeThreePoint);

								if (medalValue > kINT32_INIT)
								{
									successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_ThreePointShot_Medal_Fortitude") * medalValue;
								}
							}
							else
							{
								medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudeMid);

								if (medalValue > kINT32_INIT)
								{
									successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_MidRangeShot_Medal_Fortitude") * medalValue;
								}
							}

							if (pOwnerAction->skillindex() == SKILL_INDEX::skill_rainbowShot)
							{
								successRate -= GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_JUMP_SHOT_RAINBOW_SHOT, pOwnerCharacter);
							}
						}
					}
					break;
					case SHOT_TYPE::shotType_closePostUnstready:
					case SHOT_TYPE::shotType_hook:
					case SHOT_TYPE::shotType_post:
					case SHOT_TYPE::shotType_postUnstready:
					{
						TB::SVector3 velocityBall;
						velocityBall.mutate_x(kFLOAT_INIT);
						velocityBall.mutate_y(kFLOAT_INIT);
						velocityBall.mutate_z(kFLOAT_INIT);

						if (m_pHost->BallControllerGet() != nullptr)
						{
							//! ��������� ������ �� �ƿ� ������ ����ͼ� ���ν�Ƽ���� ���� �� ����
							/*DHOST_TYPE_FLOAT timeDelay = m_pHost->GetRoomElapsedTime() - pInfo->clienttime();
							DHOST_TYPE_FLOAT predictTime = 0.34f - timeDelay;

							velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(predictTime));

							if (velocityBall.y() >= kFLOAT_INIT)
							{
								successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_POST, pCharacter);
							}*/

							successRate += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_PROBABILITY_POST, pCharacter);

							medalValue = GetMedalValue(pOwnerCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fortitudePost);

							if (medalValue > kINT32_INIT)
							{
								successRate -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_PostShot_Medal_Fortitude") * medalValue;
							}
						}
					}
					break;
				}

				//! �ñ״�ó ȿ�� �߰��ؾߵ�
				/*
				if (ownerActionShot is not null)
				{
					DataAnimation.CATEGORY signatureCategory = ownerActionShot.ShotType switch
					{
						SHOT_TYPE.shotType_layUpPost => DataAnimation.CATEGORY.postLayUp,
						SHOT_TYPE.shotType_layUpDriveIn => DataAnimation.CATEGORY.drivingLayUp,
						SHOT_TYPE.shotType_layUpFingerRoll => DataAnimation.CATEGORY.fingerRollLayUp,
						SHOT_TYPE.shotType_dunkPost => DataAnimation.CATEGORY.postDunk,
						SHOT_TYPE.shotType_dunkDriveIn => DataAnimation.CATEGORY.drivingDunk,
						SHOT_TYPE.shotType_dunkFingerRoll => DataAnimation.CATEGORY.fingerRollDunk,
						_ => DataAnimation.CATEGORY.none,
					};

					int signatureCurrentIndex = owner.SignatureManager.GetSignatureIndex(signatureCategory, ownerActionShot.GetAnimID);
					if (signatureCurrentIndex > 0)
					{
						float signatureBonusValue = managerBalance.GetValue("Signature_Benefit_RimAttack");
						successRate += signatureBonusValue;
					}
				}
				*/

				//! ������ ����
				switch (m_pHost->GetModeType())
				{
					case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
					case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
						break;
					default:
					{
						DHOST_TYPE_INT32 oppnentTeam = m_pHost->GetIndexOpponentTeam(pCharacterInfo->team);
						DHOST_TYPE_INT32 scoreDelta = m_pHost->GetScore(oppnentTeam) - m_pHost->GetScore(pCharacterInfo->team);

						if (scoreDelta >= m_pHost->GetGameManager()->GetRescue2ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue2");
						}
						else if (scoreDelta >= m_pHost->GetGameManager()->GetRescue1ScoreExtend())
						{
							rescue = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Probability_Rescue1");
						}
					}
					break;
				}

				//! ���� �ʹ� ����
				successRate += CorrectionEarlyInTheGame(pCharacterInfo->id, ECORRECTION_EARLY_TYPE::BLOCK);

				//! �������� ������ ���� ���� (�ϸ��� ��ũ��) - Ŭ�󿡼� �����;ߵ�
				/*
				if (ownerActionShot is not null)
				{
					switch (ownerActionShot.ShotType)
					{
						case SHOT_TYPE.shotType_dunkPost:
						case SHOT_TYPE.shotType_dunkDriveIn:
						case SHOT_TYPE.shotType_dunkFingerRoll:
						case SHOT_TYPE.shotType_layUpPost:
						case SHOT_TYPE.shotType_layUpDriveIn:
						case SHOT_TYPE.shotType_layUpFingerRoll:
							{
								successRate += owner.immoralDetector.GetDataValue.rimAttackBlockProbability;
							}
							break;
					}
				}
				*/

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Pre RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}


				//! �������� ���̾�, �ı����� ��ũ ��ȭ�� �ߵ����� �� ���� ������ ���� (�������� �� ��ȭ�ƴ��� Ȯ��)
				successRate -= GetBlockSuccessRateWithRimAttacktPotential(pInfo->ownerid(), ownerShotType, pOwnerAction->skillindex(), pInfo->ballnumber());

				{
					string verify_log = "[POTENTIAL_RIM_ATTACK] Post RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", CharacterSN : " + std::to_string(pInfo->playeraction()->id())
						+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pOwnerAction->skillindex())
						+ ", successRate : " + std::to_string(successRate);
					m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
				}
			}
		}
	}

	return successRate + rescue;
}

DHOST_TYPE_FLOAT CVerifyManager::GetBlockSuccessRateWithRimAttacktPotential(DHOST_TYPE_CHARACTER_SN owner, SHOT_TYPE ownerShotType, SKILL_INDEX skillIndex, DHOST_TYPE_BALL_NUMBER ballNo)
{
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;

	SPlayerInformationT* pInfo = m_pHost->FindCharacterInformation(owner);

	if (pInfo != nullptr)
	{
		if (m_pHost->CheckShotTypeRimAttack(ownerShotType))
		{
			SPotentialInfo sInfo;
			DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(owner, ACTION_TYPE::action_shot, ownerShotType, skillIndex, sInfo);
			if (bPotential)
			{
				//! �Ϲ� ���� �ɷ�
				successRate = sInfo.potentialvalue();

				//! 2. ���� ���� Ȯ��
				if (sInfo.potentialawaken())
				{
					//! �������� �� ��ȭ�ƴ��� Ȯ��
					DHOST_TYPE_BALL_NUMBER ball_number = m_pHost->GetCharacterActionPotentialBloom(owner);
					if (ball_number > kBALL_NUMBER_INIT && ball_number == ballNo)
					{
						successRate += sInfo.potentialbloombuffvalue();
					}
				}
			}
		}	
	}

	return successRate;
}

DHOST_TYPE_FLOAT CVerifyManager::VerifyBlockPotential(F4PACKET::SPlayerInformationT* pCharacterInfo, SHOT_TYPE ownerShotType, SKILL_INDEX skillIndex, DHOST_TYPE_INT32 fxLevel)
{
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;

	if (pCharacterInfo != nullptr)
	{
		SPotentialInfo sInfo;
		DHOST_TYPE_BOOL bPotential = m_pHost->GetPotentialInfo(pCharacterInfo->id, ACTION_TYPE::action_block, ownerShotType, skillIndex, sInfo);
		if (bPotential)
		{
			successRate += sInfo.potentialvalue();

			if (fxLevel > kINT32_INIT)
			{
				if (sInfo.potentialawaken())
				{
					successRate += sInfo.potentialbloombuffvalue();
				}
			}
		}
	}
	
	return successRate;
}

EACTION_VERIFY CVerifyManager::VerifyBlockDoubleHandUp(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid)
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());

	F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());

	SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());


	TB::SVector3 positionCharacter = pInfo->playeraction()->positionlogic();
	TB::SVector3 positionBall = *pInfo->positionball();
	DHOST_TYPE_FLOAT heightBall = positionBall.y();

	TB::SVector3 velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_BLOCK_EVENT_TIME));
	DHOST_TYPE_FLOAT distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionball());

	DHOST_TYPE_INT32 medalValue = kINT32_INIT;
	DHOST_TYPE_FLOAT radiusMin = kFLOAT_INIT;
	DHOST_TYPE_FLOAT radiusMax = kFLOAT_INIT;
	DHOST_TYPE_FLOAT height = kFLOAT_INIT;
	DHOST_TYPE_FLOAT angle = kFLOAT_INIT;
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_UINT32 seed = m_pHost->GetRandomSeed(userid);


	velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_DOUBLE_HANDS_UP_BLOCK_EVENT_TIME));

	height = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_HEIGHT, pCharacter);

	switch (ownerShotType)
	{
	case SHOT_TYPE::shotType_dunkDriveIn:
	case SHOT_TYPE::shotType_dunkFingerRoll:
	case SHOT_TYPE::shotType_dunkPost:
	{
		radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_DUNK, pCharacter);
		angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle");
		angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);

		radiusMax = kFLOAT_INIT;	//! ��������� ������ ��ũ�� ������
	}
	break;
	case SHOT_TYPE::shotType_layUpDriveIn:
	case SHOT_TYPE::shotType_layUpFingerRoll:
	case SHOT_TYPE::shotType_layUpPost:
	{
		radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_LAYUP, pCharacter);
		angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_Angle");
		angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);
	}
	break;
	case SHOT_TYPE::shotType_post:
	case SHOT_TYPE::shotType_hook:
	case SHOT_TYPE::shotType_postUnstready:
	case SHOT_TYPE::shotType_closePostUnstready:
	{
		radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_POST_SHOT, pCharacter);
		angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_Angle");
	}
	break;
	default:
	{
		if (pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway || pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway3)
		{
			radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_FADEAWAY, pCharacter);
		}
		else
		{
			radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_JUMP_SHOT, pCharacter);
		}

		angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_Angle");
	}
	break;
	}

	//! Ŭ�󿡼� ����� ���� ���� �Ÿ��� �������� ����� ���� ���� �Ÿ��� ������ 0.1m �̳��� ��� (����Ʈ ������ ������ ���� �� ����)
	//! bonusa �� Ŭ�󿡼� ����� ���� ���� �Ÿ�, bonusb �� Ŭ�󿡼� ����� ĳ���Ϳ� ������ �Ÿ�
	DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - radiusMax);
	if (diff > kCALC_CLIENT_SERVER_GAP)
	{
		string verify_log = "[GAP_IN_CALC] BLOCK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
			+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
			+ ", Client : " + std::to_string(pInfo->bonusa())
			+ ", Server : " + std::to_string(radiusMax);
		m_pHost->ToLog(verify_log.c_str());
	}

	if (diff < kBURST_BUFFER_DISTANCE)
	{
		//! 1. �Ÿ� ����
		if (distance <= (radiusMax + diff))
		{
			//! 2. ���� ����
			TB::SVector3 positionRim;
			positionRim.mutate_x(RIM_POS_X);
			positionRim.mutate_y(kFLOAT_INIT);
			positionRim.mutate_z(RIM_POS_Z());

			TB::SVector3 axis;
			axis.mutate_x(kFLOAT_INIT);
			axis.mutate_y(1.0f);
			axis.mutate_z(kFLOAT_INIT);

			TB::SVector3 shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());
			TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

			DHOST_TYPE_FLOAT angleDefense = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

			DHOST_TYPE_FLOAT absAngleDefense = abs(angleDefense);

			if (angle > absAngleDefense)
			{
				//! 3. ���� ����
				if (heightBall <= height)
				{
					successRate = GetBlockSuccessRateDoubleHandsUp(pInfo) + pInfo->bonusd();

					DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc());
					if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP)
					{
						if (seed != kUINT64_INIT)
						{
							//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
							//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

							DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

							if (randomValue <= successRate)
							{
								result = EACTION_VERIFY::SUCCESS;
							}
							else
							{
								string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", seed : " + std::to_string(seed)
									+ ", randomValue : " + std::to_string(randomValue);
								m_pHost->ToLog(verify_log.c_str());

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
						else
						{
							string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{

						if (successRate > pInfo->bonusc()) // �̰� ũ�� ���� �޶� ��ŷ�̶�� ���� ���� 
						{
							DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

							if (randomValue <= successRate)
							{
								result = EACTION_VERIFY::SUCCESS;
							}
							else
							{
								string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", seed : " + std::to_string(seed)
									+ ", randomValue : " + std::to_string(randomValue);
								m_pHost->ToLog(verify_log.c_str());

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
						else
						{
							string verify_log = "[HACK_CHECK] Probability_is_different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
								+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
								+ ", ServerSuccessRate : " + std::to_string(successRate);
							m_pHost->ToLog(verify_log.c_str());

							result = EACTION_VERIFY::FAIL;
						}
					}
				}
				else
				{
					string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
						+ ", heightBall : " + std::to_string(heightBall)
						+ ", height : " + std::to_string(height);
					m_pHost->ToLog(verify_log.c_str());
				}
			}
			else
			{
				string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
					+ ", angle : " + std::to_string(angle)
					+ ", absAngleDefense : " + std::to_string(absAngleDefense);
				m_pHost->ToLog(verify_log.c_str());
			}
		}
		else
		{
			string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
				+ ", radiusMin : " + std::to_string(radiusMin)
				+ ", radiusMax : " + std::to_string(radiusMax)
				+ ", bonusa : " + std::to_string(pInfo->bonusa())
				+ ", bonusb : " + std::to_string(pInfo->bonusb())
				+ ", diff : " + std::to_string(diff)
				+ ", distance : " + std::to_string(distance);
			m_pHost->ToLog(verify_log.c_str());

			result = EACTION_VERIFY::HACK_FAIL;
		}
	}
	else
	{
		string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_RADIUS_DIFFERENT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(pCharacterInfo->userid)
			+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
			+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
			+ ", ServerRadius : " + std::to_string(radiusMax)
			+ ", ClientRadius : " + std::to_string(pInfo->bonusa())
			+ ", distance : " + std::to_string(distance);
		m_pHost->ToLog(verify_log.c_str());

		m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);

		result = EACTION_VERIFY::HACK_FAIL;
	}

	return result;

}

EACTION_VERIFY CVerifyManager::VerifyBlockSnatchBlock(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid)
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	//SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());


	TB::SVector3 positionCharacter = pInfo->playeraction()->positionlogic();
	TB::SVector3 positionBall = *pInfo->positionball();
	DHOST_TYPE_FLOAT heightBall = positionBall.y();

	TB::SVector3 velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_BLOCK_EVENT_TIME));
	DHOST_TYPE_FLOAT distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionball());

	DHOST_TYPE_INT32 medalValue = kINT32_INIT;
	DHOST_TYPE_FLOAT radiusMin = kFLOAT_INIT;
	DHOST_TYPE_FLOAT radiusMax = kFLOAT_INIT;
	DHOST_TYPE_FLOAT height = kFLOAT_INIT;
	DHOST_TYPE_FLOAT angle = kFLOAT_INIT;
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_UINT32 seed = m_pHost->GetRandomSeed(userid);

	height = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_HEIGHT, pCharacter);

	height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_SnatchBlock");

	//! 1. ��������
	if (heightBall <= height)
	{
		//! ����ġ ������ �� �̺�Ʈ Ÿ���� �ٸ�
		velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_SNATCH_BLOCK_EVENT_TIME));

		//! 2. ���� ���� ���� ���� (����ġ ������ ��츸 ���������� ���ν�Ƽ���� 0�̶� ����� - ���� ��Ŷ�� ���� �ȿͼ� ������ �ƴ�)
		if (velocityBall.y() >= kFLOAT_INIT)
		{
			TB::SVector3 rimPos;
			rimPos.mutate_x(RIM_POS_X);
			rimPos.mutate_y(RIM_POS_Y);
			rimPos.mutate_z(RIM_POS_Z());

			TB::SVector3 directionBallToRim = CommonFunction::SVectorSub(rimPos, *pInfo->positionball());

			DHOST_TYPE_FLOAT dot = CommonFunction::SVectorDot(directionBallToRim, velocityBall);
			if (dot >= kFLOAT_INIT)
			{
				DHOST_TYPE_FLOAT magnitude = CommonFunction::SVectorDistance(directionBallToRim);
				if (magnitude >= RIM_HALF_DISTANCE)
				{
					successRate = GetBlockSuccessRateSnatch(pInfo) + pInfo->bonusd();

					DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc());

					if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP)
					{
						if (seed != kUINT64_INIT)
						{
							//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
							//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

							DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

							if (randomValue <= successRate)
							{
								result = EACTION_VERIFY::SUCCESS;
							}
							else
							{
								string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", seed : " + std::to_string(seed)
									+ ", randomValue : " + std::to_string(randomValue);
								m_pHost->ToLog(verify_log.c_str());

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
						else
						{
							string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{

						if (successRate > pInfo->bonusc()) // ������ ū ���� ������ 
						{
							DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

							if (randomValue <= successRate)
							{
								result = EACTION_VERIFY::SUCCESS;
							}
							else
							{
								string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", seed : " + std::to_string(seed)
									+ ", randomValue : " + std::to_string(randomValue);
								m_pHost->ToLog(verify_log.c_str());

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
						else
						{
							string verify_log = "[HACK_CHECK] Probability_is_different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
								+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
								+ ", ServerSuccessRate : " + std::to_string(successRate);
							m_pHost->ToLog(verify_log.c_str());

							result = EACTION_VERIFY::FAIL;
						}
					}
				}
				else
				{
					string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_BALL_POS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
						+ ", magnitude : " + std::to_string(magnitude);
					m_pHost->ToLog(verify_log.c_str());
				}
			}
			else
			{
				string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_VECTOR_DOT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
					+ ", dot : " + std::to_string(dot);
				m_pHost->ToLog(verify_log.c_str());
			}
		}
		else
		{
			string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_BALL_VELOCITY RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
				+ ", velocityBall.y() : " + std::to_string(velocityBall.y());
			m_pHost->ToLog(verify_log.c_str());
		}
	}
	else
	{
		string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(pCharacterInfo->userid)
			+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
			+ ", heightBall : " + std::to_string(heightBall)
			+ ", height : " + std::to_string(height);
		m_pHost->ToLog(verify_log.c_str());
	}

	return result;
}

EACTION_VERIFY CVerifyManager::VerifyBlockShadowBlock(const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid)
{
	SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());

	//! 1. ��Ÿ�� ����
	if (ownerShotType == SHOT_TYPE::shotType_middle || ownerShotType == SHOT_TYPE::shotType_middleUnstready || ownerShotType == SHOT_TYPE::shotType_threePoint)
	{
		//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
		string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_SHOT_TYPE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
			+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType);
		m_pHost->ToLog(verify_log.c_str());

		return EACTION_VERIFY::HACK_FAIL;
	}

	//! 2. ���� ����
	{
		TB::SVector3 positionRim;
		positionRim.mutate_x(RIM_POS_X);
		positionRim.mutate_y(kFLOAT_INIT);
		positionRim.mutate_z(RIM_POS_Z());

		TB::SVector3 axis;
		axis.mutate_x(kFLOAT_INIT);
		axis.mutate_y(1.0f);
		axis.mutate_z(kFLOAT_INIT);

		TB::SVector3 shotDirection;

		shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());

		TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

		DHOST_TYPE_FLOAT defenseAngle = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

		DHOST_TYPE_FLOAT defenseAngleAbs = abs(defenseAngle);

		DHOST_TYPE_FLOAT angle = m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowBlock_Angle");

		if (defenseAngleAbs < angle)
		{
			//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
			string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", defenseAngleAbs : " + std::to_string(defenseAngleAbs)
				+ ", angle : " + std::to_string(angle);
			m_pHost->ToLog(verify_log.c_str());

			return EACTION_VERIFY::HACK_FAIL;
		}
	}

	//! 3. �Ÿ� ����
	DHOST_TYPE_FLOAT directionBall = CommonFunction::SVectorDistanceXZ(*pInfo->positionball(), pInfo->playeraction()->positionlogic());
	if (directionBall > m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowBlock_Distance"))
	{
		//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
		string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
			+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
			+ ", UserID : " + std::to_string(userid)
			+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
			+ ", directionBall : " + std::to_string(directionBall);
		m_pHost->ToLog(verify_log.c_str());

		return EACTION_VERIFY::HACK_FAIL;
	}

	return EACTION_VERIFY::SUCCESS;
}


EACTION_VERIFY CVerifyManager::VerifyBlockDefault(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid)
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
	F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
	SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());

	TB::SVector3 positionCharacter = pInfo->playeraction()->positionlogic();
	TB::SVector3 positionBall = *pInfo->positionball();
	DHOST_TYPE_FLOAT heightBall = positionBall.y();

	TB::SVector3 velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_BLOCK_EVENT_TIME));
	DHOST_TYPE_FLOAT distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionball());

	DHOST_TYPE_INT32 medalValue = kINT32_INIT;
	DHOST_TYPE_FLOAT radiusMin = kFLOAT_INIT;
	DHOST_TYPE_FLOAT radiusMax = kFLOAT_INIT;
	DHOST_TYPE_FLOAT height = kFLOAT_INIT;
	DHOST_TYPE_FLOAT angle = kFLOAT_INIT;
	DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
	DHOST_TYPE_UINT32 seed = m_pHost->GetRandomSeed(userid);

	velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_DOUBLE_HANDS_UP_BLOCK_EVENT_TIME));

	height = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_HEIGHT, pCharacter);

	DHOST_TYPE_BOOL bRimAttack = false;

	if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownBlock || pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownCatch)
	{
		height = 100.0f;	// ü���� ���� ���̸� ���� �ʴ´�.
		switch (ownerShotType)
		{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			radiusMax = m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance");

			medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockDistance);

			if (medalValue > kINT32_INIT)
			{
				radiusMax += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance_Medal_ShadowBlock") * medalValue;
			}

			medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::shadowBlock);

			if (medalValue > kINT32_INIT)
			{
				radiusMax += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance_Medal_ShadowBlock") * medalValue;
			}

			distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

			angle = m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle");

			medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockAngle);

			if (medalValue > kINT32_INIT)
			{
				angle += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle_Medal_ShadowBlock") * medalValue;
			}
		}
		break;
		default:
		{
			radiusMax = -1.0f;	// �������� �ƴ� ������ ä���� �Ÿ� ���ǿ� ������� �����ؾ� ��
		}
		break;
		}
	}
	else // ����Ʈ ������ ��� 
	{
		switch (ownerShotType)
		{
		case SHOT_TYPE::shotType_dunkDriveIn:
		case SHOT_TYPE::shotType_dunkFingerRoll:
		case SHOT_TYPE::shotType_dunkPost:
		{
			radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_DUNK, pCharacter);

			angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle");

			if (pOwnerAction->skillindex() == SKILL_INDEX::skill_popUpDunk)
			{
				float a = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle");
				float b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle_Level");

				//angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle"); ���� �� 
				float skilllevel = GetSkillLevel(pInfo->ownerid(), SKILL_INDEX::skill_popUpDunk);
				float addAngle = a - skilllevel * b;
				angle += addAngle;

				radiusMax += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Distance");
			}

			bRimAttack = true;
		}
		break;
		case SHOT_TYPE::shotType_layUpDriveIn:
		case SHOT_TYPE::shotType_layUpFingerRoll:
		case SHOT_TYPE::shotType_layUpPost:
		{
			radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_LAYUP, pCharacter);

			angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_Angle");

			bRimAttack = true;
		}
		break;
		case SHOT_TYPE::shotType_post:
		case SHOT_TYPE::shotType_hook:
		case SHOT_TYPE::shotType_postUnstready:
		case SHOT_TYPE::shotType_closePostUnstready:
		{
			radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_POST_SHOT, pCharacter);
			angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_Angle");
		}
		break;
		default:
		{
			if (pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway || pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway3)
			{
				radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_FADEAWAY, pCharacter);
			}
			else
			{
				radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_JUMP_SHOT, pCharacter);
			}

			angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_Angle");
		}
		break;
		}

		if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_stretchBlock)
		{
			radiusMin = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlock_Min");
			radiusMax += -m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlockA") + m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlockB") * GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::skill_stretchBlock);

			height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_StretchBlock");

			angle -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Angle_StretchBlock");

			if (bRimAttack)
			{
				radiusMax = kFLOAT_INIT;	// ��Ʈ��ġ ������ ������ ���� ������ �������
			}
		}

		if (pInfo->playeraction()->skillindex() == SKILL_INDEX::spikeBlock)
		{
			radiusMin = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlock_Min");
			radiusMax += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlockA") + m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlockB") * GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::spikeBlock);

			height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_SpikeBlock");

			angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Angle_SpikeBlock");

			if (ownerShotType == SHOT_TYPE::shotType_dunkPost
				|| ownerShotType == SHOT_TYPE::shotType_dunkDriveIn
				|| ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll)
			{
				radiusMax = kFLOAT_INIT;	// ������ũ ������ ��ũ ���� ������ �������
			}
		}

		angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);
	}

	if (ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll || ownerShotType == SHOT_TYPE::shotType_layUpFingerRoll)
	{
		angle -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FingerRollLayup_Angle");
	}
	else if (ownerShotType == SHOT_TYPE::shotType_dunkPost)
	{
		angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle_Post");
	}

	medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::notInMyHouse);

	if (medalValue > kINT32_INIT)
	{
		angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle_Medal_NotInMyHouse") * medalValue;
	}

	//! radiusMax ���� ������ ���� ä���� �ϴµ� ���� �������� �ƴҶ��ۿ� ����.
	if (radiusMax > kFLOAT_INIT)
	{
		//! Ŭ�󿡼� ����� ���� ���� �Ÿ��� �������� ����� ���� ���� �Ÿ��� ������ 0.1m �̳��� ��� (����Ʈ ������ ������ ���� �� ����)
		DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - radiusMax);
		if (diff > kCALC_CLIENT_SERVER_GAP)
		{
			string verify_log = "[GAP_IN_CALC] BLOCK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
				+ ", Client : " + std::to_string(pInfo->bonusa())
				+ ", Server : " + std::to_string(radiusMax);
			m_pHost->ToLog(verify_log.c_str());
		}

		if (diff < kBURST_BUFFER_DISTANCE)
		{
			//! 1. �Ÿ� ����
			if (radiusMin < distance && distance <= (radiusMax + diff))
			{
				//! 2. ���� ����
				if (heightBall <= height)
				{
					//! 3. ���� ����
					TB::SVector3 positionRim;
					positionRim.mutate_x(RIM_POS_X);
					positionRim.mutate_y(kFLOAT_INIT);
					positionRim.mutate_z(RIM_POS_Z());

					TB::SVector3 axis;
					axis.mutate_x(kFLOAT_INIT);
					axis.mutate_y(1.0f);
					axis.mutate_z(kFLOAT_INIT);

					TB::SVector3 standardDirection;

					if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownBlock || pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownCatch)
					{
						standardDirection = CommonFunction::SVectorSub(*pInfo->positionowner(), positionRim);
					}
					else
					{
						standardDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());
					}

					TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

					DHOST_TYPE_FLOAT defenseAngle = CommonFunction::SVectorSignedAngle(standardDirection, defenseDirection, axis);

					DHOST_TYPE_FLOAT defenseAngleAbs = abs(defenseAngle);

					if (defenseAngleAbs < angle)
					{
						//! ç���� �����̳� Ʈ���̴� ������ ��� ���Ǹ� ������ ������ ����� �����ʰ� ������ �������� ��
						if (m_pHost->GetModeType() == EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK ||
							m_pHost->GetModeType() == EMODE_TYPE::TRAINING_JUMPSHOTBLOCK ||
							m_pHost->GetModeType() == EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK || 
							m_pHost->GetModeType() == EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER )
						{
							result = EACTION_VERIFY::SUCCESS;
						}
						else
						{

							DHOST_TYPE_FLOAT susscessServerRate = GetBlockSuccessRate(pInfo);
							DHOST_TYPE_FLOAT teamAttributeRate = GetBlockTeamAttributeBonus(pInfo);
							DHOST_TYPE_FLOAT giftBuffBonus = 0.0f; 
							DHOST_TYPE_FLOAT giftBlockResBonus = 0.0f;

							if (bRimAttack)
							{
								giftBuffBonus = pCharacter->GetGiftBuffBonus(F4PACKET::ACTION_TYPE::action_block, CCharacterBuff::GiftBuff_BlockValue_RimAttack_Increase);

								CCharacter* pOwnerCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->ownerid());
								if (pOwnerCharacter)
								{
									giftBlockResBonus = pOwnerCharacter->GetGiftBuffBonus(F4PACKET::ACTION_TYPE::action_block, CCharacterBuff::GiftBuff_BlockValue_RimAttack_Resistance);
								}
							}

							successRate = GetBlockSuccessRate(pInfo) + teamAttributeRate + giftBuffBonus + pInfo->bonusd() - giftBlockResBonus; // ������ ��� ���ϴ� ���� Ŭ���̾�Ʈ�� ������ 
/*
							string verify_log1 = "[BLOCK] value RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Total successRate : " + std::to_string(successRate)
								+ ", Seed Number : " + std::to_string(seed)
								+ ", GiftBuffBonus : " + std::to_string(giftBuffBonus)
								+ ", GiftBlockResBonus : " + std::to_string(giftBlockResBonus)
								+ ", GetBlockSuccessRate : " + std::to_string(susscessServerRate)
								+ ", teamAttributeRate : " + std::to_string(teamAttributeRate)
								+ ", pInfo->bonusd() : " + std::to_string(pInfo->bonusd())
								+ ", pInfo->bonusc() : " + std::to_string(pInfo->bonusc())
								+ ", addbuffDebug : " + std::to_string(addBlockPotentialBuffDebug)
								+ ", subBlockPotentialBuffDebug : " + std::to_string(subBlockPotentialBuffDebug); 

							m_pHost->ToLog(verify_log1.c_str());*/
							

							DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc()); // pInfo->bonusc() :: Ŭ���̾�Ʈ�� ���� Ȯ���̴�. 

							if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP) // ���ٸ� 
							{
								if (seed != kUINT64_INIT)
								{
									//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
									//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

									DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

									if (randomValue <= successRate)
									{
										result = EACTION_VERIFY::SUCCESS;
									}
									else
									{
										string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
											+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(userid)
											+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
											+ ", seed : " + std::to_string(seed)
											+ ", randomValue : " + std::to_string(randomValue);
										m_pHost->ToLog(verify_log.c_str());

										result = EACTION_VERIFY::HACK_FAIL;
									}
								}
								else
								{
									string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
									m_pHost->ToLog(verify_log.c_str());
								}
							}
							else // ������ Ŭ���̾�Ʈ�� Ȯ���� �ٸ� ��� 
							{
								if (successRate > pInfo->bonusc()) // �ٸ����� ������ ū ���� ���̶�� ���� �����
								{
									DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);
									if (randomValue <= successRate)
									{
										result = EACTION_VERIFY::SUCCESS;
									}
									else
									{
										string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
											+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(userid)
											+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
											+ ", seed : " + std::to_string(seed)
											+ ", randomValue : " + std::to_string(randomValue);
										m_pHost->ToLog(verify_log.c_str());

										result = EACTION_VERIFY::HACK_FAIL;
									}
								}
								else
								{
									string verify_log = "[HACK_CHECK] Probability_is_different BlockSkillNone RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
										+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
										+ ", CharacterID : " + std::to_string(pCharacterInfo->characterid)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", AddPoten : " + std::to_string(addBlockPotentialBuffDebug)
										+ ", SubPoten : " + std::to_string(subBlockPotentialBuffDebug)
										+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
										+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
										+ ", ServerSuccessRate : " + std::to_string(successRate);

									m_pHost->ToLog(verify_log.c_str());

									result = EACTION_VERIFY::FAIL;
								}
							}
						}
					}
					else
					{
						string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", defenseAngleAbs : " + std::to_string(defenseAngleAbs)
							+ ", angle : " + std::to_string(angle);
						m_pHost->ToLog(verify_log.c_str());
					}
				}
				else
				{
					string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
						+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
						+ ", heightBall : " + std::to_string(heightBall)
						+ ", height : " + std::to_string(height);
					m_pHost->ToLog(verify_log.c_str());
				}
			}
			else
			{
				string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
					+ ", radiusMin : " + std::to_string(radiusMin)
					+ ", radiusMax : " + std::to_string(radiusMax)
					+ ", bonusa : " + std::to_string(pInfo->bonusa())
					+ ", bonusb : " + std::to_string(pInfo->bonusb())
					+ ", diff : " + std::to_string(diff)
					+ ", distance : " + std::to_string(distance);
				m_pHost->ToLog(verify_log.c_str());

				result = EACTION_VERIFY::HACK_FAIL;
			}
		}
		else
		{
			string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_RADIUS_DIFFERENT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
				+ ", ServerRadius : " + std::to_string(radiusMax)
				+ ", ClientRadius : " + std::to_string(pInfo->bonusa())
				+ ", distance : " + std::to_string(distance);
			m_pHost->ToLog(verify_log.c_str());

			m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);

			result = EACTION_VERIFY::HACK_FAIL;
		}
	}

	return result;
}



/*
// Legacy VerifyBlock implementation removed.
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	if (pInfo != nullptr)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
		if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
		{
			F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
			if (pCharacterInfo != nullptr)
			{
				F4PACKET::SPlayerAction* pOwnerAction = m_pHost->GetCharacterAction(pInfo->ownerid());
				if (pOwnerAction != nullptr)
				{
					SHOT_TYPE ownerShotType = m_pHost->GetCharacterActionShotType(pInfo->ownerid());

					//! ������ ���� ĵ�� �ƴٸ� (Ŭ��� ���� ������ �� ĵ���� �ƴ��� �� �� ����)
					if (ownerShotType == SHOT_TYPE::shotType_none)
					{
						string verify_log = "[VERIFY_ACTION_FAIL] ownerShotType_is_none RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
							+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType);
						m_pHost->ToLog(verify_log.c_str());

						return EACTION_VERIFY::FAIL;
					}

					TB::SVector3 positionCharacter = pInfo->playeraction()->positionlogic();
					TB::SVector3 positionBall = *pInfo->positionball();
					DHOST_TYPE_FLOAT heightBall = positionBall.y();

					TB::SVector3 velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_BLOCK_EVENT_TIME));

					DHOST_TYPE_FLOAT distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionball());



					DHOST_TYPE_INT32 medalValue = kINT32_INIT;
					DHOST_TYPE_FLOAT radiusMin = kFLOAT_INIT;
					DHOST_TYPE_FLOAT radiusMax = kFLOAT_INIT;
					DHOST_TYPE_FLOAT height = kFLOAT_INIT;
					DHOST_TYPE_FLOAT angle = kFLOAT_INIT;
					DHOST_TYPE_FLOAT successRate = kFLOAT_INIT;
					DHOST_TYPE_UINT32 seed = m_pHost->GetRandomSeed(userid);

					//! ����ɷ� ������ �������� Ȯ�� (��ų�� �ƴ�)
					std::string ani_name = m_pHost->GetAnimationController()->GetAnimationName(pInfo->animid());

					if (ani_name == "HIDDEN_SHADOWBLOCK_01")
					{
						//! 1. ��Ÿ�� ����
						if (ownerShotType == SHOT_TYPE::shotType_middle || ownerShotType == SHOT_TYPE::shotType_middleUnstready || ownerShotType == SHOT_TYPE::shotType_threePoint)
						{
							//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
							string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_SHOT_TYPE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(ownerShotType);
							m_pHost->ToLog(verify_log.c_str());

							return EACTION_VERIFY::HACK_FAIL;
						}

						//! 2. ���� ����
						{
							TB::SVector3 positionRim;
							positionRim.mutate_x(RIM_POS_X);
							positionRim.mutate_y(kFLOAT_INIT);
							positionRim.mutate_z(RIM_POS_Z());

							TB::SVector3 axis;
							axis.mutate_x(kFLOAT_INIT);
							axis.mutate_y(1.0f);
							axis.mutate_z(kFLOAT_INIT);

							TB::SVector3 shotDirection;

							shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());

							TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

							DHOST_TYPE_FLOAT defenseAngle = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

							DHOST_TYPE_FLOAT defenseAngleAbs = abs(defenseAngle);

							angle = m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowBlock_Angle");

							if (defenseAngleAbs < angle)
							{
								//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
								string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
									+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", defenseAngleAbs : " + std::to_string(defenseAngleAbs)
									+ ", angle : " + std::to_string(angle);
								m_pHost->ToLog(verify_log.c_str());

								return EACTION_VERIFY::HACK_FAIL;
							}
						}

						//! 3. �Ÿ� ����
						DHOST_TYPE_FLOAT directionBall = CommonFunction::SVectorDistanceXZ(*pInfo->positionball(), pInfo->playeraction()->positionlogic());
						if (directionBall > m_pHost->GetBalanceTable()->GetValue("VHidden_ShadowBlock_Distance"))
						{
							//! �Լ� �߰����� �����ϴ� �ڵ�� �����ؾ� ������.....�̰� ��¿ ��.....
							string verify_log = "[HACK_CHECK] SHADOW_BLOCK_CANT_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", directionBall : " + std::to_string(directionBall);
							m_pHost->ToLog(verify_log.c_str());

							return EACTION_VERIFY::HACK_FAIL;
						}

						return EACTION_VERIFY::SUCCESS;
					}

					height = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_HEIGHT, pCharacter);

					switch (pInfo->playeraction()->skillindex())
					{
					case SKILL_INDEX::skill_snatchBlock:
					{
						height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_SnatchBlock");

						//! 1. ��������
						if (heightBall <= height)
						{
							//! ����ġ ������ �� �̺�Ʈ Ÿ���� �ٸ�
							velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_SNATCH_BLOCK_EVENT_TIME));

							//! 2. ���� ���� ���� ���� (����ġ ������ ��츸 ���������� ���ν�Ƽ���� 0�̶� ����� - ���� ��Ŷ�� ���� �ȿͼ� ������ �ƴ�)
							if (velocityBall.y() >= kFLOAT_INIT)
							{
								TB::SVector3 rimPos;
								rimPos.mutate_x(RIM_POS_X);
								rimPos.mutate_y(RIM_POS_Y);
								rimPos.mutate_z(RIM_POS_Z());

								TB::SVector3 directionBallToRim = CommonFunction::SVectorSub(rimPos, *pInfo->positionball());

								DHOST_TYPE_FLOAT dot = CommonFunction::SVectorDot(directionBallToRim, velocityBall);
								if (dot >= kFLOAT_INIT)
								{
									DHOST_TYPE_FLOAT magnitude = CommonFunction::SVectorDistance(directionBallToRim);
									if (magnitude >= RIM_HALF_DISTANCE)
									{
										successRate = GetBlockSuccessRateSnatch(pInfo) + pInfo->bonusd();

										DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc());
										if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP)
										{
											if (seed != kUINT64_INIT)
											{
												//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
												//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

												DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

												if (randomValue <= successRate)
												{
													result = EACTION_VERIFY::SUCCESS;
												}
												else
												{
													string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
														+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
														+ ", UserID : " + std::to_string(userid)
														+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
														+ ", seed : " + std::to_string(seed)
														+ ", randomValue : " + std::to_string(randomValue);
													m_pHost->ToLog(verify_log.c_str());

													result = EACTION_VERIFY::HACK_FAIL;
												}
											}
											else
											{
												string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
													+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
													+ ", UserID : " + std::to_string(pCharacterInfo->userid)
													+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
												m_pHost->ToLog(verify_log.c_str());
											}
										}
										else
										{

											if (successRate > pInfo->bonusc()) // ������ ū ���� ������ 
											{
												DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

												if (randomValue <= successRate)
												{
													result = EACTION_VERIFY::SUCCESS;
												}
												else
												{
													string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
														+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
														+ ", UserID : " + std::to_string(userid)
														+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
														+ ", seed : " + std::to_string(seed)
														+ ", randomValue : " + std::to_string(randomValue);
													m_pHost->ToLog(verify_log.c_str());

													result = EACTION_VERIFY::HACK_FAIL;
												}
											}
											else
											{
												string verify_log = "[HACK_CHECK] Probability_is_different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
													+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
													+ ", UserID : " + std::to_string(pCharacterInfo->userid)
													+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
													+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
													+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
													+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
													+ ", ServerSuccessRate : " + std::to_string(successRate);
												m_pHost->ToLog(verify_log.c_str());

												result = EACTION_VERIFY::FAIL;
											}
										}

										break;
									}
									else
									{
										string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_BALL_POS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(pCharacterInfo->userid)
											+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
											+ ", magnitude : " + std::to_string(magnitude);
										m_pHost->ToLog(verify_log.c_str());
									}
								}
								else
								{
									string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_VECTOR_DOT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", dot : " + std::to_string(dot);
									m_pHost->ToLog(verify_log.c_str());
								}
							}
							else
							{
								string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_BALL_VELOCITY RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
									+ ", velocityBall.y() : " + std::to_string(velocityBall.y());
								m_pHost->ToLog(verify_log.c_str());
							}
						}
						else
						{
							string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", heightBall : " + std::to_string(heightBall)
								+ ", height : " + std::to_string(height);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					break;
					case SKILL_INDEX::skill_doubleHandsUp:
					{
						velocityBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallVelocity(kCLIENT_DOUBLE_HANDS_UP_BLOCK_EVENT_TIME));

						switch (ownerShotType)
						{
						case SHOT_TYPE::shotType_dunkDriveIn:
						case SHOT_TYPE::shotType_dunkFingerRoll:
						case SHOT_TYPE::shotType_dunkPost:
						{
							radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_DUNK, pCharacter);
							angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle");
							angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);

							radiusMax = kFLOAT_INIT;	//! ��������� ������ ��ũ�� ������
						}
						break;
						case SHOT_TYPE::shotType_layUpDriveIn:
						case SHOT_TYPE::shotType_layUpFingerRoll:
						case SHOT_TYPE::shotType_layUpPost:
						{
							radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_LAYUP, pCharacter);
							angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_Angle");
							angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);
						}
						break;
						case SHOT_TYPE::shotType_post:
						case SHOT_TYPE::shotType_hook:
						case SHOT_TYPE::shotType_postUnstready:
						case SHOT_TYPE::shotType_closePostUnstready:
						{
							radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_POST_SHOT, pCharacter);
							angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_Angle");
						}
						break;
						default:
						{
							if (pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway || pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway3)
							{
								radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_FADEAWAY, pCharacter);
							}
							else
							{
								radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_JUMP_SHOT, pCharacter);
							}

							angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_Angle");
						}
						break;
						}

						//! Ŭ�󿡼� ����� ���� ���� �Ÿ��� �������� ����� ���� ���� �Ÿ��� ������ 0.1m �̳��� ��� (����Ʈ ������ ������ ���� �� ����)
						//! bonusa �� Ŭ�󿡼� ����� ���� ���� �Ÿ�, bonusb �� Ŭ�󿡼� ����� ĳ���Ϳ� ������ �Ÿ�
						DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - radiusMax);
						if (diff > kCALC_CLIENT_SERVER_GAP)
						{
							string verify_log = "[GAP_IN_CALC] BLOCK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", Client : " + std::to_string(pInfo->bonusa())
								+ ", Server : " + std::to_string(radiusMax);
							m_pHost->ToLog(verify_log.c_str());
						}

						if (diff < kBURST_BUFFER_DISTANCE)
						{
							//! 1. �Ÿ� ����
							if (distance <= (radiusMax + diff))
							{
								//! 2. ���� ����
								TB::SVector3 positionRim;
								positionRim.mutate_x(RIM_POS_X);
								positionRim.mutate_y(kFLOAT_INIT);
								positionRim.mutate_z(RIM_POS_Z());

								TB::SVector3 axis;
								axis.mutate_x(kFLOAT_INIT);
								axis.mutate_y(1.0f);
								axis.mutate_z(kFLOAT_INIT);

								TB::SVector3 shotDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());
								TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

								DHOST_TYPE_FLOAT angleDefense = CommonFunction::SVectorSignedAngle(shotDirection, defenseDirection, axis);

								DHOST_TYPE_FLOAT absAngleDefense = abs(angleDefense);

								if (angle > absAngleDefense)
								{
									//! 3. ���� ����
									if (heightBall <= height)
									{
										successRate = GetBlockSuccessRateDoubleHandsUp(pInfo) + pInfo->bonusd();

										DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc());
										if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP)
										{
											if (seed != kUINT64_INIT)
											{
												//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
												//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

												DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

												if (randomValue <= successRate)
												{
													result = EACTION_VERIFY::SUCCESS;
												}
												else
												{
													string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
														+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
														+ ", UserID : " + std::to_string(userid)
														+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
														+ ", seed : " + std::to_string(seed)
														+ ", randomValue : " + std::to_string(randomValue);
													m_pHost->ToLog(verify_log.c_str());

													result = EACTION_VERIFY::HACK_FAIL;
												}
											}
											else
											{
												string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
													+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
													+ ", UserID : " + std::to_string(pCharacterInfo->userid)
													+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
												m_pHost->ToLog(verify_log.c_str());
											}
										}
										else
										{

											if (successRate > pInfo->bonusc()) // �̰� ũ�� ���� �޶� ��ŷ�̶�� ���� ���� 
											{
												DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

												if (randomValue <= successRate)
												{
													result = EACTION_VERIFY::SUCCESS;
												}
												else
												{
													string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
														+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
														+ ", UserID : " + std::to_string(userid)
														+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
														+ ", seed : " + std::to_string(seed)
														+ ", randomValue : " + std::to_string(randomValue);
													m_pHost->ToLog(verify_log.c_str());

													result = EACTION_VERIFY::HACK_FAIL;
												}
											}
											else
											{
												string verify_log = "[HACK_CHECK] Probability_is_different RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
													+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
													+ ", UserID : " + std::to_string(pCharacterInfo->userid)
													+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
													+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
													+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
													+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
													+ ", ServerSuccessRate : " + std::to_string(successRate);
												m_pHost->ToLog(verify_log.c_str());

												result = EACTION_VERIFY::FAIL;
											}
										}

										break;
									}
									else
									{
										string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(pCharacterInfo->userid)
											+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
											+ ", heightBall : " + std::to_string(heightBall)
											+ ", height : " + std::to_string(height);
										m_pHost->ToLog(verify_log.c_str());
									}
								}
								else
								{
									string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", angle : " + std::to_string(angle)
										+ ", absAngleDefense : " + std::to_string(absAngleDefense);
									m_pHost->ToLog(verify_log.c_str());
								}
							}
							else
							{
								string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
									+ ", radiusMin : " + std::to_string(radiusMin)
									+ ", radiusMax : " + std::to_string(radiusMax)
									+ ", bonusa : " + std::to_string(pInfo->bonusa())
									+ ", bonusb : " + std::to_string(pInfo->bonusb())
									+ ", diff : " + std::to_string(diff)
									+ ", distance : " + std::to_string(distance);
								m_pHost->ToLog(verify_log.c_str());

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
						else
						{
							string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_RADIUS_DIFFERENT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", ServerRadius : " + std::to_string(radiusMax)
								+ ", ClientRadius : " + std::to_string(pInfo->bonusa())
								+ ", distance : " + std::to_string(distance);
							m_pHost->ToLog(verify_log.c_str());

							m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);

							result = EACTION_VERIFY::HACK_FAIL;
						}
					}
					break;
					default:
					{
						if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownBlock || pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownCatch)
						{
							height = 100.0f;	// ü���� ���� ���̸� ���� �ʴ´�.
							switch (ownerShotType)
							{
							case SHOT_TYPE::shotType_dunkDriveIn:
							case SHOT_TYPE::shotType_dunkFingerRoll:
							case SHOT_TYPE::shotType_dunkPost:
							case SHOT_TYPE::shotType_layUpDriveIn:
							case SHOT_TYPE::shotType_layUpFingerRoll:
							case SHOT_TYPE::shotType_layUpPost:
							{
								radiusMax = m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance");

								medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockDistance);

								if (medalValue > kINT32_INIT)
								{
									radiusMax += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance_Medal_ShadowBlock") * medalValue;
								}

								medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::shadowBlock);

								if (medalValue > kINT32_INIT)
								{
									radiusMax += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Distance_Medal_ShadowBlock") * medalValue;
								}

								distance = CommonFunction::SVectorDistanceXZ(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

								angle = m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle");

								medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_shadowBlockAngle);

								if (medalValue > kINT32_INIT)
								{
									angle += m_pHost->GetBalanceTable()->GetValue("VChaseDown_Condition_Angle_Medal_ShadowBlock") * medalValue;
								}
							}
							break;
							default:
							{
								radiusMax = -1.0f;	// �������� �ƴ� ������ ä���� �Ÿ� ���ǿ� ������� �����ؾ� ��
							}
							break;
							}
						}
						else // ����Ʈ ������ ��� 
						{
							DHOST_TYPE_BOOL bRimAttack = false;
							switch (ownerShotType)
							{
							case SHOT_TYPE::shotType_dunkDriveIn:
							case SHOT_TYPE::shotType_dunkFingerRoll:
							case SHOT_TYPE::shotType_dunkPost:
							{
								radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_DUNK, pCharacter);

								angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle");

								if (pOwnerAction->skillindex() == SKILL_INDEX::skill_popUpDunk)
								{
									float a = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle");
									float b = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle_Level");

									//angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Angle"); ���� �� 
									float skilllevel = GetSkillLevel(pInfo->ownerid(), SKILL_INDEX::skill_popUpDunk);
									float addAngle = a - skilllevel * b;
									angle += addAngle;

									radiusMax += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PopUpDunk_Distance");
								}

								bRimAttack = true;
							}
							break;
							case SHOT_TYPE::shotType_layUpDriveIn:
							case SHOT_TYPE::shotType_layUpFingerRoll:
							case SHOT_TYPE::shotType_layUpPost:
							{
								radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_LAYUP, pCharacter);

								angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Layup_Angle");

								bRimAttack = true;
							}
							break;
							case SHOT_TYPE::shotType_post:
							case SHOT_TYPE::shotType_hook:
							case SHOT_TYPE::shotType_postUnstready:
							case SHOT_TYPE::shotType_closePostUnstready:
							{
								radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_POST_SHOT, pCharacter);
								angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_PostShot_Angle");
							}
							break;
							default:
							{
								if (pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway || pOwnerAction->skillindex() == SKILL_INDEX::skill_fadeAway3)
								{
									radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_FADEAWAY, pCharacter);
								}
								else
								{
									radiusMax = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_RADIUS_JUMP_SHOT, pCharacter);
								}

								angle = m_pHost->GetBalanceTable()->GetValue("VBlockCondition_JumpShot_Angle");
							}
							break;
							}

							if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_stretchBlock)
							{
								radiusMin = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlock_Min");
								radiusMax += -m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlockA") + m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_StretchBlockB") * GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::skill_stretchBlock);

								height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_StretchBlock");

								angle -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Angle_StretchBlock");

								if (bRimAttack)
								{
									radiusMax = kFLOAT_INIT;	// ��Ʈ��ġ ������ ������ ���� ������ �������
								}
							}

							if (pInfo->playeraction()->skillindex() == SKILL_INDEX::spikeBlock)
							{
								radiusMin = m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlock_Min");
								radiusMax += m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlockA") + m_pHost->GetBalanceTable()->GetValue("VBlocklCondition_Distance_SpikeBlockB") * GetSkillLevel(pCharacterInfo->id, SKILL_INDEX::spikeBlock);

								height += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Height_SpikeBlock");

								angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Angle_SpikeBlock");

								if (ownerShotType == SHOT_TYPE::shotType_dunkPost
									|| ownerShotType == SHOT_TYPE::shotType_dunkDriveIn
									|| ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll)
								{
									radiusMax = kFLOAT_INIT;	// ������ũ ������ ��ũ ���� ������ �������
								}
							}

							angle += GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BLOCK_CONDITION_RIM_PROTECT_ANGLE, pCharacter);
						}

						if (ownerShotType == SHOT_TYPE::shotType_dunkFingerRoll || ownerShotType == SHOT_TYPE::shotType_layUpFingerRoll)
						{
							angle -= m_pHost->GetBalanceTable()->GetValue("VBlockCondition_FingerRollLayup_Angle");
						}
						else if (ownerShotType == SHOT_TYPE::shotType_dunkPost)
						{
							angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle_Post");
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::notInMyHouse);

						if (medalValue > kINT32_INIT)
						{
							angle += m_pHost->GetBalanceTable()->GetValue("VBlockCondition_Dunk_Angle_Medal_NotInMyHouse") * medalValue;
						}

						//! radiusMax ���� ������ ���� ä���� �ϴµ� ���� �������� �ƴҶ��ۿ� ����.
						if (radiusMax > kFLOAT_INIT)
						{
							//! Ŭ�󿡼� ����� ���� ���� �Ÿ��� �������� ����� ���� ���� �Ÿ��� ������ 0.1m �̳��� ��� (����Ʈ ������ ������ ���� �� ����)
							DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - radiusMax);
							if (diff > kCALC_CLIENT_SERVER_GAP)
							{
								string verify_log = "[GAP_IN_CALC] BLOCK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
									+ ", Client : " + std::to_string(pInfo->bonusa())
									+ ", Server : " + std::to_string(radiusMax);
								m_pHost->ToLog(verify_log.c_str());
							}

							if (diff < kBURST_BUFFER_DISTANCE)
							{
								//! 1. �Ÿ� ����
								if (radiusMin < distance && distance <= (radiusMax + diff))
								{
									//! 2. ���� ����
									if (heightBall <= height)
									{
										//! 3. ���� ����
										TB::SVector3 positionRim;
										positionRim.mutate_x(RIM_POS_X);
										positionRim.mutate_y(kFLOAT_INIT);
										positionRim.mutate_z(RIM_POS_Z());

										TB::SVector3 axis;
										axis.mutate_x(kFLOAT_INIT);
										axis.mutate_y(1.0f);
										axis.mutate_z(kFLOAT_INIT);

										TB::SVector3 standardDirection;

										if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownBlock || pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_chasedownCatch)
										{
											standardDirection = CommonFunction::SVectorSub(*pInfo->positionowner(), positionRim);
										}
										else
										{
											standardDirection = CommonFunction::SVectorSub(positionRim, *pInfo->positionowner());
										}

										TB::SVector3 defenseDirection = CommonFunction::SVectorSub(pInfo->playeraction()->positionlogic(), *pInfo->positionowner());

										DHOST_TYPE_FLOAT defenseAngle = CommonFunction::SVectorSignedAngle(standardDirection, defenseDirection, axis);

										DHOST_TYPE_FLOAT defenseAngleAbs = abs(defenseAngle);

										if (defenseAngleAbs < angle)
										{
											//! ç���� �����̳� Ʈ���̴� ������ ��� ������ ������ ������ ����� �����ʰ� ������ �������� ��
											if (m_pHost->GetModeType() == EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK ||
												m_pHost->GetModeType() == EMODE_TYPE::TRAINING_JUMPSHOTBLOCK ||
												m_pHost->GetModeType() == EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK)
											{
												result = EACTION_VERIFY::SUCCESS;
											}
											else
											{

												float susscessServerRate = GetBlockSuccessRate(pInfo);

												successRate = GetBlockSuccessRate(pInfo) + pInfo->bonusd(); // pInfo->bonusd() ������ ��� ���ϴ� ���� Ŭ���̾�Ʈ�� �����ذ� ���� 

	

												DHOST_TYPE_FLOAT diffSuccessRate = abs(successRate - pInfo->bonusc()); // pInfo->bonusc() Ŭ���̾�Ʈ�� ���� Ȯ���̴�. 

												if (diffSuccessRate < kCALC_CLIENT_SERVER_GAP) // ���ٸ� 
												{
													if (seed != kUINT64_INIT)
													{
														//XorShifts::XorShift constRandom = XorShifts::XorShift(seed);
														//DHOST_TYPE_FLOAT randomValue = constRandom.randFloat();

														DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);

														if (randomValue <= successRate)
														{
															result = EACTION_VERIFY::SUCCESS;
														}
														else
														{
															string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
																+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
																+ ", UserID : " + std::to_string(userid)
																+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
																+ ", seed : " + std::to_string(seed)
																+ ", randomValue : " + std::to_string(randomValue);
															m_pHost->ToLog(verify_log.c_str());

															result = EACTION_VERIFY::HACK_FAIL;
														}
													}
													else
													{
														string verify_log = "[BLOCK_SUCCESS_RATE] Random_seed_is_null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
															+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
															+ ", UserID : " + std::to_string(pCharacterInfo->userid)
															+ ", CharacterSN : " + std::to_string(pCharacterInfo->id);
														m_pHost->ToLog(verify_log.c_str());
													}
												}
												else // ������ Ŭ���̾�Ʈ�� Ȯ���� �ٸ� ��� 
												{
													if (successRate > pInfo->bonusc()) // �ٸ����� ������ ū ���� ���̶�� ���� �����
													{
														DHOST_TYPE_FLOAT randomValue = GetRandomValueAlgorithm(userid, true);
														if (randomValue <= successRate)
														{
															result = EACTION_VERIFY::SUCCESS;
														}
														else
														{
															string verify_log = "[HACK_CHECK] randomValue_is_differant RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
																+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
																+ ", UserID : " + std::to_string(userid)
																+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
																+ ", seed : " + std::to_string(seed)
																+ ", randomValue : " + std::to_string(randomValue);
															m_pHost->ToLog(verify_log.c_str());

															result = EACTION_VERIFY::HACK_FAIL;
														}
													}
													else
													{
														string verify_log = "[HACK_CHECK] Probability_is_different BlockSkillNone RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
															+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
															+ ", UserID : " + std::to_string(pCharacterInfo->userid)
															+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
															+ ", CharacterID : " + std::to_string(pCharacterInfo->characterid)
															+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
															+ ", AddPoten : " + std::to_string(addBlockPotentialBuffDebug)
															+ ", SubPoten : " + std::to_string(subBlockPotentialBuffDebug)
															+ ", ClientSuccessRate : " + std::to_string(pInfo->bonusc())
															+ ", ClientSuccessRateEx : " + std::to_string(pInfo->bonusd())
															+ ", ServerSuccessRate : " + std::to_string(successRate);

														m_pHost->ToLog(verify_log.c_str());

														result = EACTION_VERIFY::FAIL;
													}
												}
											}
										}
										else
										{
											string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_ANGLE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
												+ ", UserID : " + std::to_string(pCharacterInfo->userid)
												+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
												+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
												+ ", defenseAngleAbs : " + std::to_string(defenseAngleAbs)
												+ ", angle : " + std::to_string(angle);
											m_pHost->ToLog(verify_log.c_str());
										}
									}
									else
									{
										string verify_log = "[VERIFY_ACTION_FAIL] BLOCK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
											+ ", UserID : " + std::to_string(pCharacterInfo->userid)
											+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
											+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
											+ ", heightBall : " + std::to_string(heightBall)
											+ ", height : " + std::to_string(height);
										m_pHost->ToLog(verify_log.c_str());
									}
								}
								else
								{
									string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										+ ", radiusMin : " + std::to_string(radiusMin)
										+ ", radiusMax : " + std::to_string(radiusMax)
										+ ", bonusa : " + std::to_string(pInfo->bonusa())
										+ ", bonusb : " + std::to_string(pInfo->bonusb())
										+ ", diff : " + std::to_string(diff)
										+ ", distance : " + std::to_string(distance);
									m_pHost->ToLog(verify_log.c_str());

									result = EACTION_VERIFY::HACK_FAIL;
								}
							}
							else
							{
								string verify_log = "[HACK_CHECK] BLOCK_BECAUSE_RADIUS_DIFFERENT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
									+ ", ServerRadius : " + std::to_string(radiusMax)
									+ ", ClientRadius : " + std::to_string(pInfo->bonusa())
									+ ", distance : " + std::to_string(distance);
								m_pHost->ToLog(verify_log.c_str());

								m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);

								result = EACTION_VERIFY::HACK_FAIL;
							}
						}
					}

					break;
					}
				}
			}
		}
	}

	return result;
}

*/
DHOST_TYPE_BOOL CVerifyManager::VerifyCommonPosition(CCharacter* pCharacter, TB::SVector3 position, string taglog )
{
	DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(position);

	string position_log; 

	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

		if (gap > kCHARACTER_POSITION_CORRECT)
		{

			F4PACKET::SPlayerInformationT* pCharacterInfo = pCharacter->GetCharacterInformation();
			CHostUserInfo* pHostUser = m_pHost->FindUser(pCharacterInfo->userid);

			position_log = taglog
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", validMoveRange: " + std::to_string(kCHARACTER_POSITION_CORRECT)
				+ ", GAP : " + std::to_string(gap);
				+ ", Ping: " + std::to_string(pHostUser->GetPingAverage())
				+ ", FPS: " + std::to_string(pHostUser->GetCurFPS());


			m_pHost->ToLog(position_log.c_str());

			return true;
		}
		else
		if (gap > validMoveRange) // ����� ���������� �α׸� ��� ���� 
		{
			/*
			F4PACKET::SPlayerInformationT* pCharacterInfo = pCharacter->GetCharacterInformation();
			CHostUserInfo* pHostUser = m_pHost->FindUser(pCharacterInfo->userid);

			position_log = taglog + " [ONLYLOG] "
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", validMoveRange: " + std::to_string(validMoveRange)
				+ ", GAP : " + std::to_string(gap);
				+" , Ping: " + std::to_string(pHostUser->GetPingAverage())
				+ ", FPS: " + std::to_string(pHostUser->GetCurFPS());

			m_pHost->ToLog(position_log.c_str());
			*/

			return false; // �α׸� ���� 
		}
	}

	return false;
}

DHOST_TYPE_INT32 CVerifyManager::VerifyPickMove(const F4PACKET::play_c2s_playerPick_data* pInfo)
{
	DHOST_TYPE_INT32 result = 0;

	if (pInfo != nullptr)
	{
		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
		if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
		{

			DHOST_TYPE_FLOAT gap = pCharacter->GetGapServerPosition(pInfo->playeraction()->positionlogic());
			if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
			{
				DHOST_TYPE_FLOAT  validMoveRange = pCharacter->GetValidMoveRange();

				if (gap > kCHARACTER_POSITION_CORRECT) //validMoveRange)
				{
					F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
					CHostUserInfo* pUser = m_pHost->FindUser(pCharacterInfo->userid);

					if (pUser->GetPingAverage() < 0.07f && pUser->GetCurFPS() > 50.0f) // ���� ��翡 ������ �غ��� 
					{
						pUser->SpeedHackDetected(SPEEDHACK_CHECK::PICK); // IsSpeedhackDectected_High  ���� , ���ϵ� ������ �� 

						string position_log = "[HACK_CHECK] [VerifyPickMove] playerpick RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", GAP : " + std::to_string(gap)
							+ ", Ping : " + std::to_string(pUser->GetPingAverage())
							+ ", FPS : " + std::to_string(pUser->GetCurFPS());

						m_pHost->ToLog(position_log.c_str());


						return -1;

					}
				}
				else
				if (gap > validMoveRange)
				{
					/*
					F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
					CHostUserInfo* pUser = m_pHost->FindUser(pCharacterInfo->userid);

					string position_log = "[HACK_CHECK] [VerifyPickMove] [Light] playerpick RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", GAP : " + std::to_string(gap)
						+ ", Ping : " + std::to_string(pUser->GetPingAverage())
						+ ", FPS : " + std::to_string(pUser->GetCurFPS());

					m_pHost->ToLog(position_log.c_str());
					*/
				}

			}
		}
	}

	return 1; // ���� 
}

DHOST_TYPE_INT32 CVerifyManager::VerifyPickBall(const F4PACKET::play_c2s_playerPick_data* pInfo)
{
	DHOST_TYPE_INT32 result = 0; // 0 : ����, 1: ���� -1: forcestand �� ���� 

	if (pInfo != nullptr)
	{
		//! ���ε� ���� �������� �� ���� �˻縦 ���� �ʴ´�.
		if (m_pHost->GetBallNumberGoalIn() == pInfo->ballnumber())
		{
			return true;
		}

		CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
		if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
		{
			F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
			if (pCharacterInfo != nullptr)
			{
				TB::SVector3 positionCharacter = pInfo->playeraction()->positionlogic();
				TB::SVector3 positionBall = *pInfo->positionball();
				DHOST_TYPE_FLOAT distanceToBall = CommonFunction::SVectorDistanceXZ(positionCharacter, positionBall);

				/*
				* 1. �������� �ùķ��̼� �Ǵ� ���� Ŭ���̾�Ʈ�� ����ġ ���̸� ��
				* 2. Ư�� ������ ũ�� ���� ó���� �Ѵ�( ���̰ų� ��ŷ���� ���� ) by steven
				* 3. ���ٿ� ���ǰ� �����ϰ� ���( �������� ������� ���� ��� �ùķ��̼� �Ǵ��� �˾����� �ٸ��� �н��� ���� �ʴ� ���� �߰�)
				*/
				CBallController* pBallController = m_pHost->BallControllerGet();
				if (pBallController != nullptr)
				{
					if (pBallController->GameBallDataGet() != nullptr && pBallController->CheckBallStateEnablePick())
					{
						DHOST_TYPE_FLOAT timeDelay = m_pHost->GetRoomElapsedTime() - pInfo->clienttime();

						float evnetTime = pInfo->checkballeventtime();

						DHOST_TYPE_FLOAT predictTime = evnetTime - timeDelay; // ���ð��� ����� �� 

						//DHOST_TYPE_FLOAT predictTime = pInfo->checkballeventtime() + timeDelay;

						TB::SVector3 positionServerBall = CommonFunction::ConvertJoVectorToTBVector(m_pHost->BallControllerGet()->GetBallPosition(predictTime));
						DHOST_TYPE_FLOAT distanceServerCheckBall = CommonFunction::SVectorDistanceXZ(positionCharacter, positionServerBall);
						DHOST_TYPE_FLOAT heightServerBall = positionServerBall.y();
						DHOST_TYPE_FLOAT gap = abs(distanceToBall - distanceServerCheckBall);

						//! ��ĳ�� ��� �ϴ� �α� ��
						if (pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_divingCatch || pInfo->playeraction()->skillindex() == SKILL_INDEX::skill_divingCatchPass)
						{
							/*
							string verify_log = "[VERIFY_PICK] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
								+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", PositionBallX : " + std::to_string(positionBall.x())
								+ ", PositionBallY : " + std::to_string(positionBall.y())
								+ ", PositionBallZ : " + std::to_string(positionBall.z())
								+ ", ServerBallX : " + std::to_string(positionServerBall.x())
								+ ", ServerBallY : " + std::to_string(positionServerBall.y())
								+ ", ServerBallZ : " + std::to_string(positionServerBall.z())
								+ ", PositionCharacterX : " + std::to_string(positionCharacter.x())
								+ ", PositionCharacterY : " + std::to_string(positionCharacter.y())
								+ ", PositionCharacterZ : " + std::to_string(positionCharacter.z())
								+ ", distanceToBall : " + std::to_string(distanceToBall);
							m_pHost->ToLog(verify_log.c_str());
							*/
						}

						if (gap > 1.5f)
						{
							if (m_pHost->CheckSingleMode()) return true;

							CHostUserInfo* pUser = m_pHost->FindUser(pCharacterInfo->userid);

							if (pUser)
							{
								string verify_log = "[HACK_CHECK] OMG PICK_BY_DISTANCE_SERVERBALL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
									+ ", UserID : " + std::to_string(pCharacterInfo->userid)
									+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
									+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
									//+ ", distanceToBall : " + std::to_string(distanceToBall)
									//+ ", distanceServerCheckBall : " + std::to_string(distanceServerCheckBall)
									+ ", timeDelay : " + std::to_string(timeDelay)
									+ ", predictTime : " + std::to_string(predictTime)
									+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallController->GameBallDataGet()->ballState)
									+ ", Ping : " + std::to_string(pUser->GetPingAverage())
									+ ", FPS : " + std::to_string(pUser->GetCurFPS())
									+ ", gap : " + std::to_string(gap);


								m_pHost->ToLog(verify_log.c_str());

								if (pUser)
								{
									if (pUser->GetPingAverage() < 0.07f)
									{
										pUser->SpeedHackDetected(SPEEDHACK_CHECK::PICK);
									}
								}
							}

							return result;
						}

						gap = abs(positionBall.y() - heightServerBall);

						if (gap > 1.5f)
						{
							if (m_pHost->CheckSingleMode()) return true;

							CHostUserInfo* pUser = m_pHost->FindUser(pCharacterInfo->userid);

							if (pUser)
							{
								if (pInfo->playeraction()->skillindex() != SKILL_INDEX::skill_divingCatch && pInfo->playeraction()->skillindex() != SKILL_INDEX::skill_divingCatchPass)
								{
									string verify_log = "[HACK_CHECK] OMG PICK_BY_HEIGHT_SERVERBALL RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
										+ ", UserID : " + std::to_string(pCharacterInfo->userid)
										+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
										+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
										//+ ", positionBall.y() : " + std::to_string(positionBall.y())
										//+ ", heightServerBall.Y() : " + std::to_string(heightServerBall)
										+ ", timeDelay : " + std::to_string(timeDelay)
										+ ", predictTime : " + std::to_string(predictTime)
										+ ", BallState : " + F4PACKET::EnumNameBALL_STATE(pBallController->GameBallDataGet()->ballState)
										+ ", Ping : " + std::to_string(pUser->GetPingAverage())
										+ ", FPS : " + std::to_string(pUser->GetCurFPS())
										+ ", gap : " + std::to_string(gap);

									m_pHost->ToLog(verify_log.c_str());

									if (pUser->GetPingAverage() < 0.07f)
									{
										pUser->SpeedHackDetected(SPEEDHACK_CHECK::PICK);
									}

									return result;
								}
							}
						}
					}
				}// end of server ball check

				DHOST_TYPE_INT32 medalValue = kINT32_INIT;
				DHOST_TYPE_FLOAT distanceEnable = kFLOAT_INIT;
				DHOST_TYPE_FLOAT heightEnable = kFLOAT_INIT;

				switch (pInfo->playeraction()->skillindex())
				{
				case SKILL_INDEX::skill_popUpCatch:
				{
					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::popUp);

					// �޴� �˾��� �ִٸ�
					DHOST_TYPE_FLOAT distanceMax = kFLOAT_INIT;
					distanceMax = m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_Ball_Distance_Max");
					if (medalValue > kINT32_INIT)
					{
						distanceMax += m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_Range_Medal") * medalValue;
					}
					//! 1. �Ÿ� ����
					if (distanceToBall <= distanceMax)
					{
						distanceEnable = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::POPUP_CATCH_DISTANCE, pCharacter);

						// �޴� �˾��� �ִٸ�
						if (medalValue > kINT32_INIT)
						{
							distanceEnable += m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_Range_Medal") * medalValue;
						}

						DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - distanceEnable);
						if (diff > kCALC_CLIENT_SERVER_GAP)
						{
							string verify_log = "[GAP_IN_CALC] PICK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", Client : " + std::to_string(pInfo->bonusa())
								+ ", Server : " + std::to_string(distanceEnable);
							m_pHost->ToLog(verify_log.c_str());
						}

						if (distanceToBall <= distanceEnable)
						{
							heightEnable = m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_Ball_Height");
							// �޴� �˾��� �ִٸ�
							if (medalValue > kINT32_INIT)
							{
								heightEnable += m_pHost->GetBalanceTable()->GetValue("VPopUpCatch_Ball_Height_Medal") * medalValue;
							}

							//! 2. ���� ����
							if (positionBall.y() <= heightEnable)
							{
								result = true;
							}
						}
						else
						{
							string verify_log = "[HACK_CHECK] PICK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", distanceToBall : " + std::to_string(distanceToBall)
								+ ", distanceEnable : " + std::to_string(distanceEnable);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{
						string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", distanceToBall : " + std::to_string(distanceToBall)
							+ ", distanceEnable : " + std::to_string(distanceEnable);
						m_pHost->ToLog(verify_log.c_str());

						m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);
					}
				}
				break;
				case SKILL_INDEX::skill_divingCatch:
				{
					if (CheckHaveSkill(pCharacterInfo->id, SKILL_INDEX::skill_divingCatchPass, false))
					{
						distanceEnable = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DIVING_CATCH_PASS_DISTANCE, pCharacter);
					}
					else
					{
						distanceEnable = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DIVING_CATCH_DISTANCE, pCharacter);
					}

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goodDivingDistance);

					if (medalValue > kINT32_INIT)
					{
						distanceEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Distance_Medal_GoodDiving") * medalValue;
					}

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::diver);

					if (medalValue > kINT32_INIT)
					{
						distanceEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Distance_Medal_Diver") * medalValue;
					}

					DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - distanceEnable);
					if (diff > kCALC_CLIENT_SERVER_GAP)
					{
						string verify_log = "[GAP_IN_CALC] PICK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", Client : " + std::to_string(pInfo->bonusa())
							+ ", Server : " + std::to_string(distanceEnable);
						m_pHost->ToLog(verify_log.c_str());
					}

					//! 1. �Ÿ� ����
					if (distanceToBall <= distanceEnable)
					{
						//! 2. ���� ����
						heightEnable = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_HeightWaist");

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goodDivingHeight);

						if (medalValue > kINT32_INIT)
						{
							heightEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Height_Medal_GoodDiving") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::diver);

						if (medalValue > kINT32_INIT)
						{
							heightEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Height_Medal_Diver") * medalValue;
						}

						if (positionBall.y() <= heightEnable)
						{
							result = true;
						}
						else
						{
							string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", positionBall.y() : " + std::to_string(positionBall.y())
								+ ", heightEnable : " + std::to_string(heightEnable);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{
						string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", distanceToBall : " + std::to_string(distanceToBall)
							+ ", distanceEnable : " + std::to_string(distanceEnable);
						m_pHost->ToLog(verify_log.c_str());

						return -1;
					}
				}
				break;
				case SKILL_INDEX::skill_divingCatchPass:
				{
					distanceEnable = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DIVING_CATCH_PASS_DISTANCE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goodDivingDistance);

					if (medalValue > kINT32_INIT)
					{
						distanceEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Distance_Medal_GoodDiving") * medalValue;
					}

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::diver);

					if (medalValue > kINT32_INIT)
					{
						distanceEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Distance_Medal_Diver") * medalValue;
					}

					DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - distanceEnable);
					if (diff > kCALC_CLIENT_SERVER_GAP)
					{
						string verify_log = "[GAP_IN_CALC] PICK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", Client : " + std::to_string(pInfo->bonusa())
							+ ", Server : " + std::to_string(distanceEnable);
						m_pHost->ToLog(verify_log.c_str());
					}

					//! 1. �Ÿ� ����
					if (distanceToBall <= distanceEnable)
					{
						//! 2. ���� ����
						heightEnable = m_pHost->GetBalanceTable()->GetValue("VDivingCatch_HeightWaist");

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_goodDivingHeight);

						if (medalValue > kINT32_INIT)
						{
							heightEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Height_Medal_GoodDiving") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::diver);

						if (medalValue > kINT32_INIT)
						{
							heightEnable += m_pHost->GetBalanceTable()->GetValue("VDivingCatch_Height_Medal_Diver") * medalValue;
						}

						if (positionBall.y() <= heightEnable)
						{
							result = true;
						}
						else
						{
							string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", positionBall.y() : " + std::to_string(positionBall.y())
								+ ", heightEnable : " + std::to_string(heightEnable);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{
						string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", distanceToBall : " + std::to_string(distanceToBall)
							+ ", distanceEnable : " + std::to_string(distanceEnable);
						m_pHost->ToLog(verify_log.c_str());
					}
				}
				break;
				default: // �Ϲ� �� 
				{
					//! 1. ���� ����
					if (positionBall.y() < 2.0f)
					{
						if (pBallController != nullptr)
						{
							if (pBallController->CheckBallStateShot())
							{
								if (m_pHost->bTreatDelayMode)
								{
									CHostUserInfo* pUser = m_pHost->FindUser(pCharacterInfo->userid);
									if (pUser)
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

						distanceEnable = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::PICK_UP_DISTANCE, pCharacter);

						DHOST_TYPE_FLOAT diff = abs(pInfo->bonusa() - distanceEnable);
						if (diff > kCALC_CLIENT_SERVER_GAP)
						{
							string verify_log = "[GAP_IN_CALC] PICK_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", Client : " + std::to_string(pInfo->bonusa())
								+ ", Server : " + std::to_string(distanceEnable);
							m_pHost->ToLog(verify_log.c_str());
						}

						//! 2. �Ÿ� ����
						if (distanceToBall <= distanceEnable)
						{
							result = true;
						}
						else
						{
							string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_DISTANCE RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
								+ ", UserID : " + std::to_string(pCharacterInfo->userid)
								+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
								+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
								+ ", distanceToBall : " + std::to_string(distanceToBall)
								+ ", distanceEnable : " + std::to_string(distanceEnable);
							m_pHost->ToLog(verify_log.c_str());
						}
					}
					else
					{
						string verify_log = "[HACK_CHECK] OMG PICK_BECAUSE_HEIGHT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
							+ ", UserID : " + std::to_string(pCharacterInfo->userid)
							+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
							+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
							+ ", positionBall.y() : " + std::to_string(positionBall.y());
						m_pHost->ToLog(verify_log.c_str());

						m_pHost->IncreaseHackImpossibleAction(pCharacterInfo->id);
					}
				}
				break;
				}
			}
		}
	}

	return result;

}

DHOST_TYPE_BOOL CVerifyManager::CheckMove(const F4PACKET::play_c2s_playerMove_data* pInfo)
{
	DHOST_TYPE_BOOL result = false;
	DHOST_TYPE_BOOL bAlreayCheck = false;
	DHOST_TYPE_INT32 medalValue = kINT32_INIT;

	DHOST_TYPE_FLOAT serverSpeed = 0.0f;
	DHOST_TYPE_FLOAT clientSpeed = 0.0f;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());

	// �̵� �ӵ� �÷��ִ� ���̽� 
	// 1. �޴� 
	// 30058 : passionateRebounder( ���ٿ�� ��Ȳ ) 
	// 30012 : agileMovement ( ���ٿ�� ��Ȳ ) 
	// 30086 : agileMovement2 ( ���ٿ�� ��Ȳ )
	// 30089 : vacuum ( ���ٿ�� ��Ȳ ) 
	// 30068 : Chaser 
	// 30001 : ���� ��Ȳ�� (��������, �������� �Ѵ�) �̵� �ӵ� ����
	// 30002 : ���� ��Ȳ �� �̵� �ӵ� ����
	// 30003 : ���� ��Ȳ�� �̵� �ӵ� ����(�� ����)
	// 30009 : offTheBallMove 

	// 2. ġġ �нú�� �̵��ӵ��� ������Ŵ 
	// 
	// 3. ����Ʈ �ɷ�ġ�� ������ �̵��ӵ��� ���� ( ���������� �ɷ�ġ�� üũ�ϹǷ� �̰��� �� �۵��ϴ� ������ ���̳� ����ȭ �ð��� ���� �ɸ��� �� ���� ) 
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		if (pInfo->runmode() == F4PACKET::MOVE_MODE::defense)
		{ 
			serverSpeed = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::MOVE_DEFENSE, pCharacter, 0.0f, 0.0f);

			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VDefenseMove_Run_Speed_Tag") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_energeticDefender);
			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VRun_Speed_Defense_Medal_EnergeticDefender1") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::energeticDefender1);
			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VRun_Speed_Defense_Medal_EnergeticDefender2") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::energeticDefender2);
			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VRun_Speed_Defense_Medal_Chaser") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::chaser);

			DHOST_TYPE_FLOAT gap = 0.0f;

			clientSpeed = pInfo->speed();

			if (serverSpeed + 0.001f < clientSpeed)
			{
				F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());

				gap = pInfo->speed() - serverSpeed;

				F4PACKET::ECHARACTER_INDEX index = (F4PACKET::ECHARACTER_INDEX)(pCharacter->GetCharacterInformation()->characterid / 100);

				if (F4PACKET::ECHARACTER_INDEX::Qiqi != index) // ġġ�� �нú�� �̵��ӵ��� �������Ƿ� ���� 
				{
					string verify_log = "[HACK_CHECK] MOVESPEED DEFENSE IS OVER RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())

						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
						+ ", Name : " + F4PACKET::EnumNameECHARACTER_INDEX(index)
						+ ", GAP : " + std::to_string(gap);

					m_pHost->ToLog(verify_log.c_str());
				}

				return false;
			}
		}
		else
		if (pInfo->runmode() == F4PACKET::MOVE_MODE::dribble)
		{
			serverSpeed = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::MOVE_DRIBBLE, pCharacter, 0.0f, 0.0f);
			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VFaceUpDribble_Run_Speed_Tag") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_quickness);
			serverSpeed += m_pHost->GetBalanceTable()->GetValue("VRun_Speed_Attack_Medal_Quickness") * GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::quickness);

			
			DHOST_TYPE_FLOAT gap = 0.0f;

			clientSpeed = pInfo->speed();

			if (serverSpeed + 0.001f < clientSpeed )
			{
				F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());

				gap = pInfo->speed() - serverSpeed;

				F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pCharacter->GetCharacterInformation()->characterid / 100);

				string verify_log = "[HACK_CHECK] MOVESPEED DRIBBLE IS OVER RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", Name : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
					+", GAP : " + std::to_string(gap);

				m_pHost->ToLog(verify_log.c_str());

				return false;
			}
		}
		else
		{

		}
	}

	return true;
}


DHOST_TYPE_BOOL CVerifyManager::CheckShotDunk(const F4PACKET::play_c2s_playerShot_data* pInfo)
{
	DHOST_TYPE_BOOL result = false;
	DHOST_TYPE_BOOL bAlreayCheck = false;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
		if (pCharacterInfo != nullptr)
		{
			TB::SVector3 positionRim;
			positionRim.mutate_x(RIM_POS_X);
			positionRim.mutate_y(kFLOAT_INIT);
			positionRim.mutate_z(RIM_POS_Z());

			TB::SVector3 axis;
			axis.mutate_x(kFLOAT_INIT);
			axis.mutate_y(1.0f);
			axis.mutate_z(kFLOAT_INIT);

			TB::SVector3 directionRim;
			directionRim.mutate_x(kFLOAT_INIT);
			directionRim.mutate_y(kFLOAT_INIT);
			directionRim.mutate_z(kFLOAT_INIT);
			directionRim = CommonFunction::SVectorSub(positionRim, pInfo->playeraction()->positionlogic());

			DHOST_TYPE_FLOAT distanceRim = CommonFunction::SVectorDistance(directionRim);

			DHOST_TYPE_FLOAT usableDistance = kFLOAT_INIT;
			DHOST_TYPE_INT32 medalValue = kINT32_INIT;
			DHOST_TYPE_FLOAT directionRimToPlayer = kFLOAT_INIT;

			DHOST_TYPE_FLOAT abilityLayUp = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_layUp);
			DHOST_TYPE_FLOAT abilityVertical = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);

			switch (pInfo->playeraction()->skillindex())
			{
				case SKILL_INDEX::skill_hopStep:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::HOP_STEP_SHOT_RANGE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::hopStepFinisher);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VHopStepShot_Range_Medal_HopStepFinisher") * medalValue;
					}

					if (distanceRim <= usableDistance)
					{
						//! ���� ����
						directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
						if (directionRimToPlayer <= m_pHost->GetBalanceTable()->GetValue("VHopStepShot_Angle"))
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::skill_spinMove:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::SPIN_MOVE_SHOT_RANGE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_spinMoveFinisher);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_Range_Medal_SpinMoveFinisher") * medalValue;
					}

					if (distanceRim <= usableDistance)
					{
						result = true;
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::skill_closeReverseDunk:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");

					if (distanceRim <= usableDistance)
					{
						//! ���� ����
						directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
						if (directionRimToPlayer > m_pHost->GetBalanceTable()->GetValue("VCloseReverseLayup_Angle"))
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::skill_reverseDunk:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
					}

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
					}
					
					if (distanceRim < usableDistance)
					{
						//! ���� ����
						directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
						if (directionRimToPlayer > m_pHost->GetBalanceTable()->GetValue("VReverseLayup_Angle"))
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::euroStepDunk:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayUp_Distance_Min");

					if (distanceRim >= usableDistance)
					{
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::EURO_STEP_LAYUP_DISTANCE_MAX, pCharacter);

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::tomahawkDunk:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");

					if (distanceRim > usableDistance)
					{
						if (abilityLayUp < 60 && (abilityLayUp < 50 || abilityVertical < 50))
						{
							usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::FINGER_ROLL_DISTANCE, pCharacter);

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::airWalk);

							if (medalValue > kINT32_INIT)
							{
								usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_AirWalk") * medalValue;
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

							if (medalValue > kINT32_INIT)
							{
								usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

							if (medalValue > kINT32_INIT)
							{
								usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_Monster") * medalValue;
							}
						}
						else
						{
							usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

							if (medalValue > kINT32_INIT)
							{
								usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
							}

							medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

							if (medalValue > kINT32_INIT)
							{
								usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
							}
						}
						
						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::inYourFace:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");

					if (distanceRim >= usableDistance)
					{
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::FINGER_ROLL_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::airWalk);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_AirWalk") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_Monster") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::skill_popUpDunk:
				{
					//! ���� �Ÿ�
					if ((distanceRim < m_pHost->GetBalanceTable()->GetValue("VPopUpDunk_Near_Distance")) || (distanceRim < m_pHost->GetBalanceTable()->GetValue("VPopUpDunk_Back_Distance")))
					{
						result = true;
					}

					bAlreayCheck = true;
				}
				break;
				case SKILL_INDEX::skill_putback:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VPutback_Range");

					if (distanceRim <= usableDistance)
					{
						result = true;
					}

					bAlreayCheck = true;
				}
				break;
				default:
				{

				}
				break;
			}

			if (bAlreayCheck == false)
			{
				switch (pInfo->shottype())
				{
					case SHOT_TYPE::shotType_dunkFingerRoll:
					{
						//! �ɷ�ġ ����
						if (abilityLayUp < 60 && (abilityLayUp < 50 || abilityVertical < 50))
						{
							break;
						}

						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						//! �ּ� ���� �Ÿ�
						if (distanceRim < usableDistance)
						{
							break;
						}

						//! �ִ� ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::FINGER_ROLL_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::airWalk);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_AirWalk") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_Monster") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
					break;
					case SHOT_TYPE::shotType_dunkPost:
					{
						//! ���� �Ÿ�
						usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");
						if (distanceRim <= usableDistance)
						{
							result = true;
						}
					}
					break;
					case SHOT_TYPE::shotType_dunkDriveIn:
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						//! �ּ� ���� �Ÿ�
						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
					break;
					default:
					{

					}
					break;
				}
			}

			if (result == false)
			{
				string verify_log = "[CHECK_SHOT_RIM_ATTACK] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pInfo->shottype())
					+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
					+ ", distanceRim : " + std::to_string(distanceRim)
					+ ", directionRimToPlayer : " + std::to_string(directionRimToPlayer);
				m_pHost->ToLog(verify_log.c_str());
			}
		}
	}

	

	return result;
}

DHOST_TYPE_BOOL CVerifyManager::CheckShotLayUp(const F4PACKET::play_c2s_playerShot_data* pInfo)
{
	DHOST_TYPE_BOOL result = false;

	CCharacter* pCharacter = m_pHost->GetCharacterManager()->GetCharacter(pInfo->playeraction()->id());
	if (pCharacter != nullptr && pCharacter->GetAbility() != nullptr)
	{
		F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(pInfo->playeraction()->id());
		if (pCharacterInfo != nullptr)
		{
			TB::SVector3 positionRim;
			positionRim.mutate_x(RIM_POS_X);
			positionRim.mutate_y(kFLOAT_INIT);
			positionRim.mutate_z(RIM_POS_Z());

			TB::SVector3 axis;
			axis.mutate_x(kFLOAT_INIT);
			axis.mutate_y(1.0f);
			axis.mutate_z(kFLOAT_INIT);

			TB::SVector3 directionRim;
			directionRim.mutate_x(kFLOAT_INIT);
			directionRim.mutate_y(kFLOAT_INIT);
			directionRim.mutate_z(kFLOAT_INIT);
			directionRim = CommonFunction::SVectorSub(positionRim, pInfo->playeraction()->positionlogic());

			DHOST_TYPE_FLOAT distanceRim = CommonFunction::SVectorDistance(directionRim);

			DHOST_TYPE_FLOAT usableDistance = kFLOAT_INIT;
			DHOST_TYPE_INT32 medalValue = kINT32_INIT;
			DHOST_TYPE_FLOAT directionRimToPlayer = kFLOAT_INIT;

			DHOST_TYPE_FLOAT abilityLayUp = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_layUp);
			DHOST_TYPE_FLOAT abilityVertical = pCharacter->GetAbility()->GetAbility(ABILITY_TYPE::abilityType_vertical);

			switch (pInfo->playeraction()->skillindex())
			{
				case SKILL_INDEX::hookShotFakeLayUp:
					result = true;
					break;

				case SKILL_INDEX::skill_hopStep:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::HOP_STEP_SHOT_RANGE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::hopStepFinisher);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VHopStepShot_Range_Medal_HopStepFinisher") * medalValue;
					}

					if (distanceRim <= usableDistance)
					{
						//! ���� ����
						directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
						if (directionRimToPlayer <= m_pHost->GetBalanceTable()->GetValue("VHopStepShot_Angle"))
						{
							result = true;
						}
					}
				}
				break;
				case SKILL_INDEX::skill_shakeAndBake:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::SHAKE_AND_BAKE_LAYUP_RANGE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_fancyBaker);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VShakeAndBakeLayUp_Range_Medal_FancyBaker") * medalValue;
					}

					if (distanceRim <= usableDistance)
					{
						result = true;
					}
				}
				break;
				case SKILL_INDEX::skill_spinMove:
				{
					//! ���� �Ÿ�
					usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::SPIN_MOVE_SHOT_RANGE, pCharacter);

					medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_spinMoveFinisher);

					if (medalValue > kINT32_INIT)
					{
						usableDistance += m_pHost->GetBalanceTable()->GetValue("VSpinMoveShot_Range_Medal_SpinMoveFinisher") * medalValue;
					}

					if (distanceRim <= usableDistance)
					{
						result = true;
					}
				}
				break;
				case SKILL_INDEX::skill_closeReverseDunk:
				case SKILL_INDEX::skill_closeReverseLayUp:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");

					if (distanceRim <= usableDistance)
					{
						result = true;
					}
				}
				break;
				case SKILL_INDEX::skill_reverseDunk:
				case SKILL_INDEX::skill_reverseLayUp:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							//! ���� ����
							directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
							if (directionRimToPlayer > m_pHost->GetBalanceTable()->GetValue("VReverseLayup_Angle"))
							{
								result = true;
							}
						}
					}
				}
				break;
				case SKILL_INDEX::skill_jellyLayUp:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							//! ���� ����
							directionRimToPlayer = abs(CommonFunction::ToYaw(directionRim));
							if (directionRimToPlayer <= m_pHost->GetBalanceTable()->GetValue("VJellyLayup_Angle") * 0.5f)
							{
								result = true;
							}
						}
					}
				}
				break;
				case SKILL_INDEX::euroStepDunk:
				case SKILL_INDEX::skill_euroStepLayUp:
				{
					//! ���� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VEuroStepLayUp_Distance_Min");

					if (distanceRim >= usableDistance)
					{
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::EURO_STEP_LAYUP_DISTANCE_MAX, pCharacter);

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
				}
				break;
				case SKILL_INDEX::quickLayUp:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VQuickLayUp_Distance_Min");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::QUICK_LAYUP_DISTANCE_MAX, pCharacter);

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
				}
				break;
				case SKILL_INDEX::skill_scoopShot:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
				}
				break;
				case SKILL_INDEX::skill_floater:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VFloater_Distance_Min");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::SHOT_FLOATER_DISTANCE_MAX, pCharacter);

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
				}
				break;
				case SKILL_INDEX::passFake:
				{
					//! // ��� ���� �ּ� �Ÿ�
					usableDistance = m_pHost->GetBalanceTable()->GetValue("VPassFake_LayUp_Distance_Min");
					if (distanceRim >= usableDistance)
					{
						//! ���� �Ÿ�
						usableDistance = m_pHost->GetBalanceTable()->GetValue("VPassFake_LayUp_Distance_Max");

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
				}
				break;
				default:
				{

				}
				break;
			}

			if (result == false)
			{
				switch (pInfo->shottype())
				{
					case SHOT_TYPE::shotType_layUpPost:
					{
						//! ���� �Ÿ�
						//! ���� ���°� ����Ʈ���̶�� �Ÿ��� �����ؾߵ�
						//! ����Ʈ���� �������¶�� MOVE_MODE::normal �� �Ǽ� ������ �� �� ����. �׷��� �ִϸ��̼����� ��������...
						if (m_pHost->GetAnimationController() != nullptr)
						{
							std::string animName = m_pHost->GetAnimationController()->GetAnimationName(pInfo->animid());

							if (animName.find("POSTUPLAYUP") != string::npos)
							{
								usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);
							}
						}

						if (usableDistance  == kFLOAT_INIT)
						{
							if (pCharacter->GetMoveMode() == MOVE_MODE::postUp)
							{
								usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);
							}
							else
							{
								usableDistance = m_pHost->GetBalanceTable()->GetValue("VJumpShot_DistanceMin");
							}
						}

						if (distanceRim <= usableDistance)
						{
							result = true;
						}
					}
					break;
					case SHOT_TYPE::shotType_layUpDriveIn:
					{
						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						//! �ּ� ���� �Ÿ�
						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
					break;
					case SHOT_TYPE::shotType_layUpFingerRoll:
					{
						//! �ɷ�ġ ����
						if (abilityLayUp < 60 && (abilityLayUp < 50 || abilityVertical < 50))
						{
							break;
						}

						//! ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::DRIVING_LAYUP_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::medal_flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Tag") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						//! �ּ� ���� �Ÿ�
						if (distanceRim < usableDistance)
						{
							break;
						}

						//! �ִ� ���� �Ÿ�
						usableDistance = GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::FINGER_ROLL_DISTANCE, pCharacter);

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::airWalk);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_AirWalk") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::flyHigh);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VDrivingLayUpCondition_Distance_Medal") * medalValue;
						}

						medalValue = GetMedalValue(pCharacter->GetCharacterInformation(), MEDAL_INDEX::monster);

						if (medalValue > kINT32_INIT)
						{
							usableDistance += m_pHost->GetBalanceTable()->GetValue("VFingerRollLayUpCondition_Distance_Medal_Monster") * medalValue;
						}

						if (distanceRim < usableDistance)
						{
							result = true;
						}
					}
					break;
					default:
					{

					}
					break;
				}
			}

			if (result == false)
			{
				string verify_log = "[CHECK_SHOT_RIM_ATTACK] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", ShotType : " + F4PACKET::EnumNameSHOT_TYPE(pInfo->shottype())
					+ ", Skill : " + F4PACKET::EnumNameSKILL_INDEX(pInfo->playeraction()->skillindex())
					+ ", distanceRim : " + std::to_string(distanceRim)
					+ ", directionRimToPlayer : " + std::to_string(directionRimToPlayer);
				m_pHost->ToLog(verify_log.c_str());
			}
		}
	}

	return result;
}

EACTION_VERIFY CVerifyManager::VerifyPotentialFxLevel(DHOST_TYPE_USER_ID userid, DHOST_TYPE_CHARACTER_SN id, DHOST_TYPE_INT32 fxlevel, POTENTIAL_INDEX value)
{
	EACTION_VERIFY result = EACTION_VERIFY::FAIL;

	SPotentialInfo sInfo;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_pHost->FindCharacterInformation(id);
	if (pCharacterInfo != nullptr)
	{
		DHOST_TYPE_BOOL bPotential = m_pHost->GetCharacterPotentialInfo(id, value, sInfo);

		if (bPotential && sInfo.potentialawaken())
		{
			DHOST_TYPE_FLOAT random_value = GetRandomValueAlgorithm(userid, true);

			{
				string verify_log = "[POTENTIAL_RATE] RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
					+ ", random_value : " + std::to_string(random_value);
				m_pHost->ToLog(verify_log.c_str(), LOG_TYPE::LOG_POTENTIAL);
			}

			if (random_value <= sInfo.potentialbloomratevalue())
			{
				DHOST_TYPE_INT32 effect_level = (sInfo.potentialbloombufflevel() >= 5 && sInfo.potentialbloomratelevel() >= 5) ? 2 : 1;

				if (fxlevel == effect_level)
				{
					result = EACTION_VERIFY::SUCCESS;
				}
				else
				{
					//! ��ȭ ���� ������ �ٸ���? ��ģ���̴�
					string verify_log = "[HACK_CHECK] FX_LEVEL_DIFFERENT RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
						+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
						+ ", UserID : " + std::to_string(pCharacterInfo->userid)
						+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
						+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
						+ ", ClientLevel : " + std::to_string(fxlevel)
						+ ", ServerLevel : " + std::to_string(effect_level);
					m_pHost->ToLog(verify_log.c_str());

					result = EACTION_VERIFY::HACK_FAIL;
				}
			}
			else
			{
				//! ��ȭ�� ���ϴ� Ȯ���ε� ��ȭ�� �ߴ�? ��ģ���̴�
				string verify_log = "[HACK_CHECK] CAN'T_POTENTIAL_BLOOM RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
					+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
					+ ", UserID : " + std::to_string(pCharacterInfo->userid)
					+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
					+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
					+ ", BloomRateValue : " + std::to_string(sInfo.potentialbloomratevalue())
					+ ", RandomValue : " + std::to_string(random_value);
				m_pHost->ToLog(verify_log.c_str());

				result = EACTION_VERIFY::HACK_FAIL;
			}
		}
		else
		{
			//! ������ ���ߴµ� �����ߴٰ� ��ȭ ���ʽ��� ���´ٰ�? ��ģ���̴�!!
			string verify_log = "[HACK_CHECK] NO_AWAKEN RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime())
				+ ", GameTime : " + std::to_string(m_pHost->GetGameTime())
				+ ", UserID : " + std::to_string(pCharacterInfo->userid)
				+ ", CharacterSN : " + std::to_string(pCharacterInfo->id)
				+ ", Potential : " + F4PACKET::EnumNamePOTENTIAL_INDEX(sInfo.potentialindex())
				+ ", PotentialAwaken : " + std::to_string(sInfo.potentialawaken());
			m_pHost->ToLog(verify_log.c_str());

			result = EACTION_VERIFY::HACK_FAIL;
		}
	}

	return result;
}


void CVerifyManager::VerifyLog(LOG_TYPE logType)
{
	switch (logType)
	{
	case LOG_TYPE::LOG_SPEEDHACK:

		break;

	case LOG_TYPE::LOG_POTENTIAL:



		break;
	default:
		break;
	}
}