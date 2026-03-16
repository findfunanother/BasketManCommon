#include "Character.h"
//! CharacterState
#include "CharacterState_Move.h"
#include "CharacterState_Stand.h"
#include "CharacterState_Rebound.h"
#include "CharacterState_Block.h"
#include "CharacterState_Pick.h"
#include "CharacterState_Shot.h"
#include "CharacterState_Pass.h"
#include "CharacterState_FakeShot.h"
#include "Host.h"
#include "DataManagerBalance.h"

//! End
//! 



CCharacter::CCharacter(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber)
	: m_bCheckPassiveValidity(true)
{
	m_pHost = pHost;

	CCharacterMovement::m_pHost = m_pHost;
	CCharacterMovement::m_pCharacter = this;
	CCharacterBuff::m_pHost = m_pHost;
	CCharacterBuff::m_pCharacter = this;
	CCharacterAction::m_pHost = m_pHost;
	CCharacterAction::m_pCharacter = this;


	m_pBalanceData = pBalanceData;
	m_pOwnerAnimationController = pAniInfo;
	m_pOwnerBallController = pBallInfo;
	m_Information = pInfo;
	m_PlayerNumber = playerNumber;

	m_Restriction = EMOVEMENT_RESTRICTION::NONE;
	m_CharacterCurrentState = EHOST_CHARACTER_STATE::NONE;
	m_PreCharacterCurrentState = EHOST_CHARACTER_STATE::NONE;

	m_Action = new SCharacterAction();
	m_Action->action = new F4PACKET::SPlayerAction();
	m_CharacterRecord = new SCharacterRecord();
	m_ability = new CAbility(pInfo);
	
	m_CharacterPassive = new SCharacterPassive();
	m_CharacterPassive->valueMax = SHOT_BONUS_PASSIVE_MAX; // 
	m_CharacterPassive->valueCurr = 0.0f;

	//! 20220623 CharacterState by thinkingpig
	m_CharacterStateAction = new F4PACKET::SPlayerAction();
	m_CharacterStateActionType = F4PACKET::ACTION_TYPE::action_none;
	m_CharacterStateActionTime = kTIME_F_INIT;
	
	m_CharacterStateAnimationIndex = kINT32_INIT;
	m_AwayFromKeyboardGameTime = kFLOAT_INIT;

	m_MoveMode = F4PACKET::MOVE_MODE::normal;
	m_PreMoveMode = F4PACKET::MOVE_MODE::normal;
	m_DisconnectTime = kFLOAT_INIT;
	m_DiconnectComebackTime = kFLOAT_INIT;


	m_ForceBallEventFail = kBALL_NUMBER_INIT;

	m_CharacterPotentialValueSum = kFLOAT_INIT;

	InitCharacterStateActionPosition();
	InitCharacterStateStartAnimationPosition();
	//! end

	InitCharacterPotentialInfo();

	InitCharacterAction();
	InitCharacterRecord();
	InitCharacterPassive();

	m_CharacterActionDeque.clear();
	m_SereverPositionDeque.clear();
	SetActionIDX(kINT32_INIT);
	SetFocusPoint(kFLOAT_INIT);

	SetCharacterBurstOnFireModeStep(EBURST_ON_FIRE_MODE_STEP::NONE);
	SetBurstGauge(kFLOAT_INIT);
	SetBurstActionSubtractTime(kFLOAT_INIT);
	SetBurstActionReleaseTriggerTime(kFLOAT_INIT);


	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::NONE, new CCharacterState(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::MOVE, new CCharacterState_Move(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::STAND, new CCharacterState_Stand(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::REBOUND, new CCharacterState_Rebound(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::BLOCK, new CCharacterState_Block(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::PICK, new CCharacterState_Pick(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::SHOT, new CCharacterState_Shot(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::FAKESHOT, new CCharacterState_FakeShot(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::PASS, new CCharacterState_Pass(this)));

	ChangeCharacterState(EHOST_CHARACTER_STATE::STAND, kTIME_F_INIT);

	m_bPreActionTapOut = false;
	m_bPreActionMoved = false;
	m_fTapOutElapsedTime = kFLOAT_INIT;

	m_bAI = false;

	m_AutoPlayWatingTime = m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_NOTHING");

	if (m_Information->autoplayuser > 0)
	{
		m_AutoPlayWatingTime = 3.0f;
	}
	/*else
	if (m_Information->teamcontrolluser && ( m_pHost->GetModeType() == EMODE_TYPE::TRIO || m_pHost->GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED ))
	{
		m_AutoPlayWatingTime = 3.0f;
	}*/
	else
	{
		m_AutoPlayWatingTime = m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_NOTHING");
	}
}

CCharacter::~CCharacter()
{
	if (m_ability)
	{
		SAFE_DELETE(m_ability);
	}
	m_ability = nullptr;

	if (m_CharacterStateAction)
	{
		SAFE_DELETE(m_CharacterStateAction);
	}
	m_CharacterStateAction = nullptr;

	if (m_CharacterRecord)
	{
		SAFE_DELETE(m_CharacterRecord);
	}
	m_CharacterRecord = nullptr;

	if (m_CharacterPassive)
	{
		SAFE_DELETE(m_CharacterPassive);
	}
	m_CharacterPassive = nullptr;

	if (m_Action)
	{
		if (m_Action->action)
		{
			SAFE_DELETE(m_Action->action);
			m_Action->action = nullptr;
		}
		SAFE_DELETE(m_Action);
	}
	m_Action = nullptr;

	if (m_Information)
	{
		SAFE_DELETE(m_Information);
	}
	m_Information = nullptr;

	if (false == m_CharacterState.empty())
	{
		for (auto& it : m_CharacterState)
		{
			auto pState = it.second;

			SAFE_DELETE(pState);
		}
		m_CharacterState.clear();
	}
}

// 이거 안쓰나 ? 
void CCharacter::Initialize(F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber)
{
	m_pOwnerAnimationController = pAniInfo;
	m_pOwnerBallController = pBallInfo;
	m_Information = pInfo;
	m_Restriction = EMOVEMENT_RESTRICTION::NONE;
	m_CharacterCurrentState = EHOST_CHARACTER_STATE::NONE;
	m_PreCharacterCurrentState = EHOST_CHARACTER_STATE::NONE;

	m_Action = new SCharacterAction();
	m_Action->action = new F4PACKET::SPlayerAction();
	m_CharacterRecord = new SCharacterRecord();
	m_ability = new CAbility(pInfo);
	m_CharacterPassive = new SCharacterPassive();

	//! 20220623 CharacterState by thinkingpig
	m_CharacterStateAction = new F4PACKET::SPlayerAction();
	m_CharacterStateActionType = F4PACKET::ACTION_TYPE::action_none;
	m_CharacterStateActionTime = kTIME_F_INIT;

	m_CharacterStateAnimationIndex = kINT32_INIT;
	m_AwayFromKeyboardGameTime = kFLOAT_INIT;
	m_MoveMode = F4PACKET::MOVE_MODE::normal;

	m_DisconnectTime = kFLOAT_INIT;
	m_DiconnectComebackTime = kFLOAT_INIT;

	m_PlayerNumber = playerNumber;

	m_ForceBallEventFail = kBALL_NUMBER_INIT;

	m_CharacterPotentialValueSum = kFLOAT_INIT;

	InitCharacterStateActionPosition();
	InitCharacterStateStartAnimationPosition();
	//! end

	InitCharacterPotentialInfo();

	InitCharacterAction();
	InitCharacterRecord();
	InitCharacterPassive();

	m_CharacterActionDeque.clear();
	m_SereverPositionDeque.clear();
	SetActionIDX(kINT32_INIT);
	SetFocusPoint(kFLOAT_INIT);

	SetCharacterBurstOnFireModeStep(EBURST_ON_FIRE_MODE_STEP::NONE);
	SetBurstGauge(kFLOAT_INIT);
	SetBurstActionSubtractTime(kFLOAT_INIT);
	SetBurstActionReleaseTriggerTime(kFLOAT_INIT);


	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::NONE, new CCharacterState(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::MOVE, new CCharacterState_Move(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::STAND, new CCharacterState_Stand(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::REBOUND, new CCharacterState_Rebound(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::BLOCK, new CCharacterState_Block(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::PICK, new CCharacterState_Pick(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::SHOT, new CCharacterState_Shot(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::FAKESHOT, new CCharacterState_FakeShot(this)));
	m_CharacterState.insert(std::map<EHOST_CHARACTER_STATE, CCharacterState*>::value_type(EHOST_CHARACTER_STATE::PASS, new CCharacterState_Pass(this)));

	ChangeCharacterState(EHOST_CHARACTER_STATE::STAND, kTIME_F_INIT);
}

void CCharacter::InitCharacterAction()
{
	m_Action->actionTime = kFLOAT_INIT;
	m_Action->actionType = F4PACKET::ACTION_TYPE::action_none;
	m_Action->preActionType = F4PACKET::ACTION_TYPE::action_none;
	m_Action->preActionTypeAgo = F4PACKET::ACTION_TYPE::action_none;
	m_Action->action;
	m_Action->speed = kFLOAT_INIT;
	m_Action->shotType = F4PACKET::SHOT_TYPE::shotType_none;
	m_Action->shotIsLeftHand = false;
	m_Action->potentialBloom = kBALL_NUMBER_INIT;
}

F4PACKET::SPlayerInformationT* CCharacter::GetCharacterInformation()
{
	/*
	size_t total_size = 0;

	// 각 필드 크기 계산
	total_size += sizeof(m_Information->userid);              // uint32_t
	total_size += sizeof(m_Information->characterid);         // uint32_t
	total_size += sizeof(m_Information->id);                  // int32_t
	total_size += sizeof(m_Information->role);                // Enum
	total_size += sizeof(m_Information->team);                // int32_t
	total_size += sizeof(m_Information->radius);              // float
	total_size += sizeof(m_Information->height);              // float
	total_size += sizeof(m_Information->winspan);             // float
	total_size += sizeof(m_Information->gender);              // uint32_t
	total_size += sizeof(m_Information->passivetype);         // Enum
	total_size += sizeof(m_Information->elo);                 // uint32_t
	total_size += sizeof(m_Information->burstvalue);          // uint32_t

	// 문자열과 벡터는 실제 데이터 크기를 추가 계산해야 함
	total_size += m_Information->name.capacity();             // 문자열 메모리 크기
	total_size += m_Information->handedness.capacity();       // 문자열 메모리 크기
	total_size += m_Information->skills.capacity() * sizeof(F4PACKET::SSkillInfo);  // 벡터 크기
	total_size += m_Information->medals.capacity() * sizeof(F4PACKET::SMedalInfo);  // 벡터 크기
	total_size += m_Information->emojis.capacity() * sizeof(F4PACKET::SEmojiInfo);  // 벡터 크기
	total_size += m_Information->potentials.capacity() * sizeof(F4PACKET::SPotentialInfo);  // 벡터 크기
	total_size += m_Information->signatures.capacity() * sizeof(int32_t);            // 벡터 크기

	// 고유 포인터로 된 객체도 포함
	if (m_Information->ability) {
		total_size += sizeof(*(m_Information->ability));       // SPlayerAbilityT의 크기
	}
	if (m_Information->abilitymodification) {
		total_size += sizeof(*(m_Information->abilitymodification));
	}

	*/

	return m_Information;
}

void CCharacter::SetCharacterAction(DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType, F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_BALL_NUMBER potentialBloom)
{
	m_Action->actionTime = actionTime;
	m_Action->preActionTypeAgo = m_Action->preActionType;
	m_Action->preActionType = m_Action->actionType;
	m_Action->actionType = actionType;
	m_Action->shotType = shotType;
	m_Action->potentialBloom = potentialBloom;

	if (pInfo != nullptr)
	{
		if (actionType == F4PACKET::ACTION_TYPE::action_move)
		{
			pInfo->mutate_yawlogic(pInfo->directioninput());
		}

		m_Action->action->mutable_positionlogic().mutate_x(pInfo->positionlogic().x());
		m_Action->action->mutable_positionlogic().mutate_y(pInfo->positionlogic().y());
		m_Action->action->mutable_positionlogic().mutate_z(pInfo->positionlogic().z());

		m_Action->action->mutate_collisionpriority(pInfo->collisionpriority());
		m_Action->action->mutate_directioninput(pInfo->directioninput());
		m_Action->action->mutate_id(pInfo->id());
		m_Action->action->mutate_keys(pInfo->keys());
		m_Action->action->mutate_number(pInfo->number());
		m_Action->action->mutate_skillindex(pInfo->skillindex());
		m_Action->action->mutate_yawlogic(pInfo->yawlogic());

		m_Action->speed = speed;
	}
}

void CCharacter::SetCharacterActionOverlap(F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value)
{
	m_Action->actionType = actionType;
	m_Action->action->mutate_skillindex(value);
}

void CCharacter::SetCharacterActionOverlapDestYaw(F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw)
{
	if (m_Action->actionType == (F4PACKET::ACTION_TYPE)actionType)
	{
		m_Action->action->mutate_yawlogic(DestYaw);
	}
}

SCharacterAction* CCharacter::GetCharacterAction()
{
	return m_Action;
}

F4PACKET::SHOT_TYPE CCharacter::GetCharacterActionShotType()
{
	return m_Action->shotType;
}

DHOST_TYPE_BALL_NUMBER CCharacter::GetCharacterActionPotentialBloom()
{
	return m_Action->potentialBloom;
}

void CCharacter::SetCharacterRestriction(EMOVEMENT_RESTRICTION value)
{
	m_Restriction = value;
	//m_SereverPositionDeque.clear();
}

EMOVEMENT_RESTRICTION CCharacter::GetCharacterRestriction()
{
	return m_Restriction;
}

void CCharacter::InitCharacterRecord()
{
	// ĳ���� ��� ���� �ʱ�ȭ
	m_CharacterRecord->UserID = m_Information->userid;
	m_CharacterRecord->CharacterSN = m_Information->id;;
	m_CharacterRecord->CharacterID = m_Information->characterid;
	m_CharacterRecord->SwitchToAi = false;
	m_CharacterRecord->SpeedHackLevel = kUINT32_INIT;
	m_CharacterRecord->SpeedHackTimeAccumulate = kFLOAT_INIT;
	m_CharacterRecord->AwayFromKeyboardTimeAccumulate = kFLOAT_INIT;
	m_CharacterRecord->DisconnectTimeAccumulate = kFLOAT_INIT;
	m_CharacterRecord->CharacterKickType = ECHARACTER_KICK_TYPE::NONE;
	m_CharacterRecord->LatencyExceed100 = kUINT32_INIT;
	m_CharacterRecord->LatencyExceed200 = kUINT32_INIT;
	m_CharacterRecord->AvgPingLatency = kFLOAT_INIT;
	m_CharacterRecord->UseJoystick = false;
	m_CharacterRecord->UserName = m_Information->name;
	m_CharacterRecord->Team = std::to_string(m_Information->team);
	m_CharacterRecord->TotalScore = kUINT16_INIT;
	m_CharacterRecord->RegularTotalScore = kUINT16_INIT;
	m_CharacterRecord->Try2PointCount = kUINT16_INIT;
	m_CharacterRecord->RegularTry2PointCount = kUINT16_INIT;
	m_CharacterRecord->Suc2Pointcount = kUINT16_INIT;
	m_CharacterRecord->RegularSuc2Pointcount = kUINT16_INIT;
	m_CharacterRecord->Try3PointCount = kUINT16_INIT;
	m_CharacterRecord->RegularTry3PointCount = kUINT16_INIT;
	m_CharacterRecord->Suc3Pointcount = kUINT16_INIT;
	m_CharacterRecord->RegularSuc3Pointcount = kUINT16_INIT;
	m_CharacterRecord->AssistCount = kUINT16_INIT;
	m_CharacterRecord->RegularAssistCount = kUINT16_INIT;
	m_CharacterRecord->ReboundCount = kUINT16_INIT;
	m_CharacterRecord->RegularReboundCount = kUINT16_INIT;
	m_CharacterRecord->BlockCount = kUINT16_INIT;
	m_CharacterRecord->RegularBlockCount = kUINT16_INIT;
	m_CharacterRecord->StealCount = kUINT16_INIT;
	m_CharacterRecord->RegularStealCount = kUINT16_INIT;
	m_CharacterRecord->LooseBallCount = kUINT16_INIT;
	m_CharacterRecord->RegularLooseBallCount = kUINT16_INIT;
	m_CharacterRecord->TurnOverCount = kUINT16_INIT;
	m_CharacterRecord->RegularTurnOverCount = kUINT16_INIT;
	m_CharacterRecord->PassCount = kUINT16_INIT;
	m_CharacterRecord->NicePassCount = kUINT16_INIT;
	m_CharacterRecord->DivingCatchCount = kUINT16_INIT;
	m_CharacterRecord->OverPassCount = kUINT16_INIT;
	m_CharacterRecord->JumpShot2PointCount = kINT32_INIT;
	m_CharacterRecord->RegularJumpShot2PointCount = kINT32_INIT;
	m_CharacterRecord->MVPScore = kUINT16_INIT;
	m_CharacterRecord->vShootInfo.clear();
	m_CharacterRecord->mapBadMannerInfo.clear();
	m_CharacterRecord->mapEmojiInfo.clear();
	m_CharacterRecord->mapCeremonyInfo.clear();
	m_CharacterRecord->mapSKillSucInfo.clear();
	m_CharacterRecord->UseSkillCount = kUINT16_INIT;
	m_CharacterRecord->UseSignatureCount = kUINT16_INIT;
	m_CharacterRecord->UseQuickChatCount = kUINT16_INIT;
	m_CharacterRecord->UseSwitchCount = kUINT16_INIT;
	m_CharacterRecord->ActionGreatDefenseCount = kUINT16_INIT;
	m_CharacterRecord->ActionLayUpGoalCount = kUINT16_INIT;
	m_CharacterRecord->ActionDunkGoalCount = kUINT16_INIT;
	m_CharacterRecord->ActionAnkleBreakeCount = kUINT16_INIT;
	m_CharacterRecord->IllegalScreenCount = kUINT16_INIT;
	m_CharacterRecord->NotSamePositionCount = kINT32_INIT;
	m_CharacterRecord->SpeedHackClientjudgment = kINT32_INIT;
	m_CharacterRecord->HackImpossibleAction = kINT32_INIT;
	m_CharacterRecord->MemoryTamperClientjudgment = kINT32_INIT;
	m_CharacterRecord->PacketTamperCount = kINT32_INIT;
	m_CharacterRecord->RegularNicePassCount = kUINT16_INIT;
	m_CharacterRecord->RegularActionGreatDefenseCount = kUINT16_INIT;
	m_CharacterRecord->RegularActionLayUpGoalCount = kUINT16_INIT;
	m_CharacterRecord->RegularActionDunkGoalCount = kUINT16_INIT;
	m_CharacterRecord->RegularActionAnkleBreakeCount = kUINT16_INIT;
	m_CharacterRecord->SurrenderType = kINT32_INIT;
}

void CCharacter::SetCharacterUserID(DHOST_TYPE_USER_ID value)
{
	m_CharacterRecord->UserID = value;
}

DHOST_TYPE_UINT16 CCharacter::GetCharacterRecordCountWithType(F4PACKET::RECORD_TYPE Type)
{
	switch (Type)
	{
		case F4PACKET::RECORD_TYPE::PointTwo:
		{
			return m_CharacterRecord->Suc2Pointcount;
		}
		break;
		case F4PACKET::RECORD_TYPE::PointThree:
		{
			return m_CharacterRecord->Suc3Pointcount;
		}
		break;
		case F4PACKET::RECORD_TYPE::Assist:
		{
			return m_CharacterRecord->AssistCount;
		}
		break;
		case F4PACKET::RECORD_TYPE::Rebound:
		{
			return m_CharacterRecord->ReboundCount;
		}
		break;
		case F4PACKET::RECORD_TYPE::Block:
		{
			return m_CharacterRecord->BlockCount;
		}
		break;
		case F4PACKET::RECORD_TYPE::Steal:
		{
			return m_CharacterRecord->StealCount;
		}
		break;
		case F4PACKET::RECORD_TYPE::LooseBall:
		{
			return m_CharacterRecord->LooseBallCount;
		}
		break;
		default:
		{

		} break;
	}

	return kUINT16_INIT;
}

void CCharacter::UpdateCharacterRecord(ECHARACTER_RECORD_TYPE Type, DHOST_TYPE_BOOL bOverTime)
{
	switch (Type)
	{
		break;
		case ECHARACTER_RECORD_TYPE::TRY_2POINT:
		{
			m_CharacterRecord->Try2PointCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::SUC_2POINT:
		{
			m_CharacterRecord->Suc2Pointcount += 1;
			m_CharacterRecord->TotalScore += 2;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TRY_3POINT:
		{
			m_CharacterRecord->Try3PointCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::SUC_3POINT:
		{
			m_CharacterRecord->Suc3Pointcount += 1;
			m_CharacterRecord->TotalScore += 3;
		}
		break;
		case ECHARACTER_RECORD_TYPE::ASSIST:
		{
			m_CharacterRecord->AssistCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::REBOUND:
		{
			m_CharacterRecord->ReboundCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::BLOCK:
		{
			m_CharacterRecord->BlockCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::STEAL:
		{
			m_CharacterRecord->StealCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::LOOSE_BALL:
		{
			m_CharacterRecord->LooseBallCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TURN_OVER:
		{
			m_CharacterRecord->TurnOverCount += 1;
			//OnTurnOver();
		}
		break;
		case ECHARACTER_RECORD_TYPE::PASS:
		{
			m_CharacterRecord->PassCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::NICE_PASS:
		{
			m_CharacterRecord->NicePassCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::DIVING_CATCH:
		{
			m_CharacterRecord->DivingCatchCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::OVER_PASS:
		{
			m_CharacterRecord->OverPassCount += 1;
		}
		break;
		case ECHARACTER_RECORD_TYPE::JUMP_SHOT_2_POINT_COUNT:
		{
			m_CharacterRecord->JumpShot2PointCount += 1;
		}
		break;
		default:
		{

		}
		break;
	}
}

void CCharacter::AddCharacterActionDeque()
{
	SCharacterAction sInfo;
	sInfo.action = m_Action->action;
	sInfo.actionTime = m_Action->actionTime;
	sInfo.actionType = m_Action->actionType;
	sInfo.speed = m_Action->speed;

	m_CharacterActionDeque.push_back(sInfo);
}

void CCharacter::SetCharacterReadyPosition(F4PACKET::ACTION_TYPE actionType, TB::SVector3 position)
{
	F4PACKET::ACTION_TYPE set_action_type = F4PACKET::ACTION_TYPE::action_stand;

	switch (actionType)
	{
		case F4PACKET::ACTION_TYPE::action_standDribble:
		case F4PACKET::ACTION_TYPE::action_standDefense:
		case F4PACKET::ACTION_TYPE::action_standOffense:
		case F4PACKET::ACTION_TYPE::action_standNormal:
			set_action_type = F4PACKET::ACTION_TYPE::action_stand;
			break;
		default:
			set_action_type = actionType;
			break;
	}

	m_Action->actionType = set_action_type;
	m_Action->action->mutable_positionlogic() = position;

	//! ��������� �� ��ġ ����ȭ
	m_CharacterStateActionType = set_action_type;
	m_CharacterStateAction->mutable_positionlogic() = position;


	// 리뉴얼된 서버 포지션도 여기 해줘야 한다.
	m_ServerPosition = position;
	
}

void CCharacter::AddCharacterShootInfo(float x, float z, uint32_t zone, bool goal)
{
	SCharacterRecordShootInfo sInfo;
	sInfo.PositionX = x;
	sInfo.PositionZ = z;
	sInfo.Zone = zone;
	sInfo.Goal = goal;

	m_CharacterRecord->vShootInfo.push_back(sInfo);
}

DHOST_TYPE_BOOL CCharacter::CheckActionTypeStand(F4PACKET::ACTION_TYPE actionType)
{
	switch (actionType)
	{
		case F4PACKET::ACTION_TYPE::action_stand:
		case F4PACKET::ACTION_TYPE::action_standBoxOut:
		case F4PACKET::ACTION_TYPE::action_standDefense:
		case F4PACKET::ACTION_TYPE::action_standDenyDefense:
		case F4PACKET::ACTION_TYPE::action_standDribble:
		case F4PACKET::ACTION_TYPE::action_standFaceUp:
		case F4PACKET::ACTION_TYPE::action_standHandsUp:
		case F4PACKET::ACTION_TYPE::action_standIntercept:
		case F4PACKET::ACTION_TYPE::action_standNormal:
		case F4PACKET::ACTION_TYPE::action_standOffense:
		case F4PACKET::ACTION_TYPE::action_standPivot:
		case F4PACKET::ACTION_TYPE::action_standPostUp:
		case F4PACKET::ACTION_TYPE::action_standReceive:
		case F4PACKET::ACTION_TYPE::action_standTripleThreat:
			return true;
		default:
			return false;
	}
	return false;
}

DHOST_TYPE_BOOL CCharacter::CheckActionTypeMove(F4PACKET::ACTION_TYPE actionType)
{
	switch (actionType)
	{
	case F4PACKET::ACTION_TYPE::action_crossOverPenetrate:
	case F4PACKET::ACTION_TYPE::action_crossOverPostUp:
	case F4PACKET::ACTION_TYPE::action_crossOverPenetrateBetweenTheLegs:
	case F4PACKET::ACTION_TYPE::action_fakeShot:
	case F4PACKET::ACTION_TYPE::action_hopStep:

	case F4PACKET::ACTION_TYPE::action_move:

	case F4PACKET::ACTION_TYPE::action_moveDribble:
	case F4PACKET::ACTION_TYPE::action_moveDenyDefense:
	case F4PACKET::ACTION_TYPE::action_moveFaceUp:
	case F4PACKET::ACTION_TYPE::action_moveHandsUp:
	case F4PACKET::ACTION_TYPE::action_moveIntercept:

	case F4PACKET::ACTION_TYPE::action_movePostUp:
	case F4PACKET::ACTION_TYPE::action_penetrate:

	case F4PACKET::ACTION_TYPE::action_penetratePostUp:
	case F4PACKET::ACTION_TYPE::action_penetrateReady:
	case F4PACKET::ACTION_TYPE::action_postUpPenetrate:
	case F4PACKET::ACTION_TYPE::action_postUpReady:

	case F4PACKET::ACTION_TYPE::action_shakeAndBake:
	case F4PACKET::ACTION_TYPE::action_slipAndSlide:
	case F4PACKET::ACTION_TYPE::action_spinMove:
	case F4PACKET::ACTION_TYPE::shammgod:
	case F4PACKET::ACTION_TYPE::escapeDribble:
		return true;
	default:
		return false;
	}

	return false;
}


CCharacterState* CCharacter::GetCharacterState(EHOST_CHARACTER_STATE value)
{
	return m_CharacterState[value];
}

void CCharacter::ChangeCharacterState(EHOST_CHARACTER_STATE value, DHOST_TYPE_GAME_TIME_F time, void* pData)
{
	//if (m_CharacterCurrentState != value || (m_CharacterCurrentState == EHOST_CHARACTER_STATE::MOVE && value == EHOST_CHARACTER_STATE::MOVE))
	if (m_CharacterCurrentState != value)
	{
		m_CharacterState[m_CharacterCurrentState]->OnExit();
		m_PreCharacterCurrentState = m_CharacterCurrentState;
		m_CharacterCurrentState = value;
		m_CharacterState[m_CharacterCurrentState]->OnEnter(pData);
	}
}

void CCharacter::UpdateCharacterState(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	SetCurrentBallPosition(ballPos);

	m_CharacterState[m_CharacterCurrentState]->OnUpdate(gameTime, timeDelta, elapsedTime, ballPos);

	UpdateCharacter(timeDelta);

	// tencity 메달 3단계가 리바운드가 안되는 버그 수정, 이 부분을 장착한 캐릭터로 옮길 것( 골라 )
	if (m_bPreActionTapOut)
	{
		m_fTapOutElapsedTime += timeDelta;
		if (m_fTapOutElapsedTime > 2.5f) // 랙시간 고려
		{
			m_bPreActionTapOut = false;
			m_fTapOutElapsedTime = kFLOAT_INIT;
		}
	}
}

void CCharacter::OnCharacterMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, F4PACKET::MOVE_MODE moveMode)
{
	if (GetMoveMode() == F4PACKET::MOVE_MODE::boxOut || GetMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
	{
		if ((F4PACKET::PACKET_ID)packetID != F4PACKET::PACKET_ID::play_c2s_playerStand && (F4PACKET::PACKET_ID)packetID != F4PACKET::PACKET_ID::play_c2s_playerMove)
		{
			//string _log = "*** OnCharacterMessage OnMessage **** ";
			//GetHost()->ToLog(_log.c_str());
			RemoveBoxOutedCharacterAll();
		}
	}


	SetMoveMode(moveMode);
	SetCharacterForceBallEventFail(kBALL_NUMBER_INIT);
	SetCharacterBallEventSuccess(kBALL_NUMBER_INIT);


	if (m_CharacterCurrentState == EHOST_CHARACTER_STATE::FAKESHOT) // 피봇 상태일때 이동 패킷은 무시 , 현재 상태 유지 
	{
		if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerMove) // || (F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerStand)
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerMove_data>*)pData;
			auto* data = pPacket->GetData();

			/*
			if (data->runmode() != F4PACKET::MOVE_MODE::pivot)
			{
				return;
			}
			*/
			return; // FakeShot 일때는 스테이트 변환이 없음 
		}
		else
			if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerStand)
			{
				auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerStand_data>*)pData;
				auto* data = pPacket->GetData();
				/*
				if (data->runmode() != F4PACKET::MOVE_MODE::pivot)
				{
					return;
				}
				*/

				return;
			}

	}


	if (m_Action->actionType == F4PACKET::ACTION_TYPE::action_shot) // 슛 페이크 일때 이동 패킷은 무시 , 현재 상태 유지 
	{
		if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerFakeShot)
		{
			/*
			if (m_Action->preActionType == F4PACKET::ACTION_TYPE::action_move || (m_Action->preActionTypeAgo == F4PACKET::ACTION_TYPE::action_move))
			{
				ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); // 피봇 상태이다
				m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);
				return;

			}
			*/

			if (m_bPreActionMoved)
			{
				ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); // 피봇 상태이다 
				m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);
				return;

			}

		}
	}

	// case F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake:
	// case F4PACKET::PACKET_ID::play_c2s_playerHopStep:
	// skill_penetrateSideStep
	// case F4PACKET::PACKET_ID::play_c2s_playerSpinMove:
	/* 이것들은 피봇상태가 된다.
		130010	홉스텝
		130070	쉐이크앤베이크
		130100	돌파 중 사이드스텝
		130110	스핀무브*/


	if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate
		|| (F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake
		|| (F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerHopStep
		|| (F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerSpinMove
		|| (F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerPenetrate) // 이것을 하면 피봇 상태가 된다 
	{

		if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerPenetrate)
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrate_data>*)pData;
			auto* data = pPacket->GetData();

			if (data->playeraction()->skillindex() == F4PACKET::SKILL_INDEX::skill_penetrateSideStep)
			{
				m_bPreActionMoved = true;
				ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); // 피봇 상태이다 
				m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);
				return;
			}
		}
		else
		if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate)
		{
			auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpPenetrate_data>*)pData;
			auto* data = pPacket->GetData();

			if (data->playeraction()->skillindex() != F4PACKET::SKILL_INDEX::skill_postUpPenetrate && 
				data->playeraction()->skillindex() != F4PACKET::SKILL_INDEX::skill_postUpSpin)
			{
				m_bPreActionMoved = true;
				ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); // 피봇 상태이다 
				m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);

				return;
			}
		}
		else
		{
			m_bPreActionMoved = true;
			ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); // 피봇 상태이다 
			m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);

			return;
		}
	}



	// tencity 메달 3단계가 리바운드가 안되는 버그 수정 
	if ((F4PACKET::PACKET_ID)packetID == F4PACKET::PACKET_ID::play_c2s_playerTapOut)
	{
		auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerTapOut_data>*)pData;
		if (pPacket != nullptr)
		{	
			auto* data = pPacket->GetData();
			if (data->success())
			{
				m_bPreActionTapOut = true;
				m_fTapOutElapsedTime = kFLOAT_INIT;
			}
		}
	}

	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerStand:
			ChangeCharacterState(EHOST_CHARACTER_STATE::STAND, time, pData); 
			break;

		case F4PACKET::PACKET_ID::play_c2s_playerRebound:
		case F4PACKET::PACKET_ID::play_c2s_playerTapOut:
		case F4PACKET::PACKET_ID::play_c2s_playerTapPass:
		case F4PACKET::PACKET_ID::play_c2s_playerJumpBallTapOut:
			m_bPreActionMoved = false;
			ChangeCharacterState(EHOST_CHARACTER_STATE::REBOUND, time, pData); 
			break;

		case F4PACKET::PACKET_ID::play_c2s_playerBlock:
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopCut:
			m_bPreActionMoved = false;
			ChangeCharacterState(EHOST_CHARACTER_STATE::BLOCK, time, pData); 
			break;

		case F4PACKET::PACKET_ID::play_c2s_playerMove:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrate:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateReady:
		case F4PACKET::PACKET_ID::play_c2s_playerDash:
		case F4PACKET::PACKET_ID::play_c2s_playerCutIn:
		case F4PACKET::PACKET_ID::play_c2s_playerVCut:
		case F4PACKET::PACKET_ID::play_c2s_playerHopStep:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetratePostUp:
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndMove:
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndSlip:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpReady:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpStepBack:
		case F4PACKET::PACKET_ID::play_c2s_playerScreen:
		case F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake:
		case F4PACKET::PACKET_ID::play_c2s_playerSlideStep:
		case F4PACKET::PACKET_ID::play_c2s_playerSpinMove:
		case F4PACKET::PACKET_ID::play_c2s_playerSlipAndSlide:
		case F4PACKET::PACKET_ID::play_c2s_playerStun:
		case F4PACKET::PACKET_ID::play_c2s_playerCollision:
		case F4PACKET::PACKET_ID::play_c2s_playerCloseOut:
		case F4PACKET::PACKET_ID::play_c2s_playerChaseContest:
		case F4PACKET::PACKET_ID::play_c2s_playerGoAndCatch:
		case F4PACKET::PACKET_ID::play_c2s_playerCatchAndShotMove:
		case F4PACKET::PACKET_ID::play_c2s_playerCrossOver:
		case F4PACKET::PACKET_ID::play_c2s_playerHandCheck:
		case F4PACKET::PACKET_ID::play_c2s_playerHookHook:
			m_bPreActionMoved = true;
			ChangeCharacterState(EHOST_CHARACTER_STATE::MOVE, time, pData); 
			break;

		case F4PACKET::PACKET_ID::play_c2s_playerPick:
		case F4PACKET::PACKET_ID::play_c2s_playerIntercept:
		case F4PACKET::PACKET_ID::play_c2s_playerReceivePass:
		case F4PACKET::PACKET_ID::play_c2s_playerSteal:
			m_bPreActionMoved = false;
			ChangeCharacterState(EHOST_CHARACTER_STATE::PICK, time, pData); 
			break;

		//case F4PACKET::PACKET_ID::play_c2s_playerFakeShot:
			//ChangeCharacterState(EHOST_CHARACTER_STATE::FAKESHOT, time, pData); break;
		case F4PACKET::PACKET_ID::play_c2s_playerShot:
		case F4PACKET::PACKET_ID::play_c2s_playerDoubleClutch:
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopShot:
		case F4PACKET::PACKET_ID::play_c2s_playerTipIn:
		case F4PACKET::PACKET_ID::play_c2s_playerBehindStepBackJumperShot:
			ChangeCharacterState(EHOST_CHARACTER_STATE::SHOT, time, pData); 
			break;

		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopPass:
		case F4PACKET::PACKET_ID::play_c2s_playerPass:
			m_bPreActionMoved = false;
			ChangeCharacterState(EHOST_CHARACTER_STATE::PASS, time, pData); 
			break;

		default:
			m_bPreActionMoved = false;
			ChangeCharacterState(EHOST_CHARACTER_STATE::NONE, time, pData); break;
	}

	m_CharacterState[m_CharacterCurrentState]->OnMessage(packetID, pData, time);
}

DHOST_TYPE_UINT32 CCharacter::PushAndCheckBurstConsecutiveScore(DHOST_TYPE_GAME_TIME_F time)
{
	DHOST_TYPE_UINT32 result_value = kUINT32_INIT;

	if (m_vecBurstConsecutiveScore.empty())
	{
		m_vecBurstConsecutiveScore.push_back(time);
	}
	else
	{
		DHOST_TYPE_GAME_TIME_F diff_time = m_vecBurstConsecutiveScore.front() - time;

		if (m_vecBurstConsecutiveScore.size() == 1)
		{
			if (diff_time < 30.f)
			{
				result_value = 1;
			}
		}
		else
		{
			if (diff_time < 40.f)
			{
				result_value = 2;
			}
		}

		m_vecBurstConsecutiveScore.clear();
		m_vecBurstConsecutiveScore.push_back(time);
	}

	return result_value;
}

DHOST_TYPE_UINT32 CCharacter::PushAndCheckBurstConsecutiveSteal(DHOST_TYPE_GAME_TIME_F time)
{
	DHOST_TYPE_UINT32 result_value = kUINT32_INIT;

	if (m_vecBurstConsecutiveSteal.empty())
	{
		m_vecBurstConsecutiveSteal.push_back(time);
	}
	else
	{
		DHOST_TYPE_GAME_TIME_F diff_time = m_vecBurstConsecutiveSteal.front() - time;

		if (m_vecBurstConsecutiveSteal.size() == 1)
		{
			if (diff_time < 30.f)
			{
				result_value = 1;
			}
		}
		else
		{
			if (diff_time < 40.f)
			{
				result_value = 2;
			}
		}
		m_vecBurstConsecutiveSteal.clear();
		m_vecBurstConsecutiveSteal.push_back(time);
	}

	return result_value;
}

DHOST_TYPE_UINT32 CCharacter::PushAndCheckBurstConsecutiveBlock(DHOST_TYPE_GAME_TIME_F time)
{
	DHOST_TYPE_UINT32 result_value = kUINT32_INIT;

	if (m_vecBurstConsecutiveBlock.empty())
	{
		m_vecBurstConsecutiveBlock.push_back(time);
	}
	else
	{
		DHOST_TYPE_GAME_TIME_F diff_time = m_vecBurstConsecutiveBlock.front() - time;

		if (m_vecBurstConsecutiveBlock.size() == 1)
		{
			if (diff_time < 30.f)
			{
				result_value = 1;
			}
		}
		else
		{
			if (diff_time < 40.f)
			{
				result_value = 2;
			}
		}
		m_vecBurstConsecutiveBlock.clear();
		m_vecBurstConsecutiveBlock.push_back(time);
	}

	return result_value;
}

void CCharacter::ClearBurstConsecutiveScore()
{
	m_vecBurstConsecutiveScore.clear();
}

void CCharacter::ClearBurstConsecutiveSteal()
{
	m_vecBurstConsecutiveSteal.clear();
}

void CCharacter::ClearBurstConsecutiveBlock()
{
	m_vecBurstConsecutiveBlock.clear();
}

CAbility* CCharacter::GetAbility()
{
	return m_ability;
}

CAnimationController* CCharacter::GetAnimationController()
{
	return m_pOwnerAnimationController;
}

DHOST_TYPE_BOOL CCharacter::IsLeftHand()
{
	DHOST_TYPE_BOOL result = false;

	if (m_Information->handedness == "Left")
	{
		result = true;
	}

	return result;
}

//! 20220623 CharacterState by thinkingpig
void CCharacter::InitCharacterStateStartAnimationPosition()
{
	m_CharacterStateStartAnimationPosition.fX = kFLOAT_INIT;
	m_CharacterStateStartAnimationPosition.fY = kFLOAT_INIT;
	m_CharacterStateStartAnimationPosition.fZ = kFLOAT_INIT;
}

void CCharacter::InitCharacterStateMoveDirection()
{
}

void CCharacter::InitCharacterStateActionPosition()
{
	m_CharacterStateAction->mutable_positionlogic().mutate_x(kFLOAT_INIT);
	m_CharacterStateAction->mutable_positionlogic().mutate_y(kFLOAT_INIT);
	m_CharacterStateAction->mutable_positionlogic().mutate_z(kFLOAT_INIT);
}

F4PACKET::SPlayerAction* CCharacter::GetCharacterStateAction()
{
	return m_CharacterStateAction;
}

void CCharacter::SetCharacterStateAction(F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time)
{
	// ��Ŷ �ޱ����� ������ �˰��ִ� ĳ���� ���� ����
	if (actionType == F4PACKET::ACTION_TYPE::action_jumpBallTapOut)
	{
		// ���������� ���� Ŭ�� ĳ������ ��ġ�� �����ؼ� �ֱ⶧���� ���� ���¿� ���� ������ �����Ƿ� ��Ŷ������ ĳ���Ϳ� �ٷ� �����Ѵ�.
		SetStartYaw(sInfo.yawlogic());
		SetStartPosition(&sInfo);
	}
	else if (actionType == F4PACKET::ACTION_TYPE::action_shot || actionType == F4PACKET::ACTION_TYPE::action_alleyOopShot || actionType == F4PACKET::ACTION_TYPE::action_tipIn)
	{
		// ���� �׻� ��븦 ���⶧���� �䰪�� ��Ŷ�� �ִ°� �״�� ����
		SetStartYaw(sInfo.yawlogic());
		SetStartPosition(m_CharacterStateAction);
	}
	else
	{
		SetStartYaw(m_CharacterStateAction->yawlogic());
		SetStartPosition(m_CharacterStateAction);
	}
	
	// ��Ŷ ������ ĳ���Ϳ� ����
	SetCharacterStateActionPosition(sInfo.positionlogic().x(), sInfo.positionlogic().y(), sInfo.positionlogic().z(), "CCharacterState::SetSPlayerAction");

	m_CharacterStateAction->mutate_collisionpriority(sInfo.collisionpriority());
	m_CharacterStateAction->mutate_directioninput(sInfo.directioninput());
	m_CharacterStateAction->mutate_id(sInfo.id());
	m_CharacterStateAction->mutate_keys(sInfo.keys());
	m_CharacterStateAction->mutate_number(sInfo.number());
	m_CharacterStateAction->mutate_skillindex(sInfo.skillindex());
	m_CharacterStateAction->mutate_yawlogic(sInfo.yawlogic());

	if (actionType == F4PACKET::ACTION_TYPE::action_move)
	{
		m_CharacterStateAction->mutate_yawlogic(sInfo.directioninput());
	}

	m_CharacterStateActionTime = time;
	m_CharacterStateActionType = actionType;
}

void CCharacter::SetCharacterStateActionPosition(float x, float y, float z, string callPos)
{
	// Ŭ�� ��Ŷ���� �������� ĳ������ ��ġ�� ȣ��Ʈ���� ������ ĳ���� ��ġ���� �Ÿ�
	//float diff_distance = CommonFunction::SVectorDistanceXZ(m_CharacterStateAction->mutable_positionlogic(), x, z);
	
	m_CharacterStateAction->mutable_positionlogic().mutate_x(x);
	m_CharacterStateAction->mutable_positionlogic().mutate_y(y);
	m_CharacterStateAction->mutable_positionlogic().mutate_z(z);
}

F4PACKET::ACTION_TYPE CCharacter::GetCharacterStateActionType()
{
	return m_CharacterStateActionType;
}

void CCharacter::SetCharacterStateActionType(F4PACKET::ACTION_TYPE value)
{
	m_CharacterStateActionType = value;
}

DHOST_TYPE_GAME_TIME_F CCharacter::GetCharacterStateActionTime()
{
	return m_CharacterStateActionTime;
}

void CCharacter::SetCharacterStateActionTime(DHOST_TYPE_GAME_TIME_F value)
{
	m_CharacterStateActionTime = value;
}

DHOST_TYPE_INT32 CCharacter::GetCharacterStateAnimationIndex()
{
	return m_CharacterStateAnimationIndex;
}

void CCharacter::SetCharacterStateAnimationIndex(DHOST_TYPE_INT32 value)
{
	m_CharacterStateAnimationIndex = value;
}

JOVECTOR3& CCharacter::GetCharacterStateStartAnimationPosition()
{
	return m_CharacterStateStartAnimationPosition;
}

void CCharacter::SetCharacterStateStartAnimationPosition(const TB::SVector3& pPos)
{
	m_CharacterStateStartAnimationPosition.fX = pPos.x();
	m_CharacterStateStartAnimationPosition.fY = pPos.y();
	m_CharacterStateStartAnimationPosition.fZ = pPos.z();
}

CBallController* CCharacter::GetBallController()
{
	return m_pOwnerBallController;
}

JOVECTOR3 CCharacter::GetCurrentBallPosition()
{
	return m_CurrentBallPosition;
}

void CCharacter::SetCurrentBallPosition(JOVECTOR3 value)
{
	m_CurrentBallPosition.fX = value.fX;
	m_CurrentBallPosition.fY = value.fY;
	m_CurrentBallPosition.fZ = value.fZ;
}

void CCharacter::IncreaseLatencyCount(DHOST_TYPE_FLOAT value)
{
	m_CharacterRecord->LatencyExceed100 += 1;

	if (value >= kPING_LATENCY_200)
	{
		m_CharacterRecord->LatencyExceed200 += 1;
	}
}

void CCharacter::SetAvgPingLatency(DHOST_TYPE_FLOAT value)
{
	m_CharacterRecord->AvgPingLatency = value;
}

void CCharacter::IncreaseBadMannerCount(EBAD_MANNER_TYPE type)
{
	if (m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS || m_pHost->GetModeType() == EMODE_TYPE::THREE_ON_THREE 
		|| m_pHost->GetModeType() == EMODE_TYPE::CONTINUOUS || m_pHost->GetModeType() == EMODE_TYPE::TWO_ON_TWO 
		|| m_pHost->GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
	{
		if (!m_Information->autoplayuser)
		{
			std::map<EBAD_MANNER_TYPE, DHOST_TYPE_UINT16>::iterator it = m_CharacterRecord->mapBadMannerInfo.find(type);

			if (it == m_CharacterRecord->mapBadMannerInfo.end())
			{
				m_CharacterRecord->mapBadMannerInfo.insert(std::pair<EBAD_MANNER_TYPE, DHOST_TYPE_UINT16>(type, 1));
			}
			else
			{
				it->second++;
			}
		}
	}
}

DHOST_TYPE_UINT16 CCharacter::GetBadMannerTypeCount(EBAD_MANNER_TYPE type)
{
	DHOST_TYPE_UINT16 result = kUINT16_INIT;

	std::map<EBAD_MANNER_TYPE, DHOST_TYPE_UINT16>::iterator it = m_CharacterRecord->mapBadMannerInfo.find(type);

	if (it != m_CharacterRecord->mapBadMannerInfo.end())
	{
		result = it->second;
	}

	return result;
}

void CCharacter::IncreaseEmojiCount(DHOST_TYPE_INT32 value)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_INT32>::iterator it = m_CharacterRecord->mapEmojiInfo.find(value);

	if (it == m_CharacterRecord->mapEmojiInfo.end())
	{
		m_CharacterRecord->mapEmojiInfo.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_INT32>(value, 1));
	}
	else
	{
		it->second++;
	}
}

void CCharacter::IncreaseCeremonyCount(DHOST_TYPE_INT32 value)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_INT32>::iterator it = m_CharacterRecord->mapCeremonyInfo.find(value);

	if (it == m_CharacterRecord->mapCeremonyInfo.end())
	{
		m_CharacterRecord->mapCeremonyInfo.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_INT32>(value, 1));
	}
	else
	{
		it->second++;
	}
}

void CCharacter::IncreaseSkillSucCount(DHOST_TYPE_UINT32 value)
{
	std::map<DHOST_TYPE_UINT32, DHOST_TYPE_INT32>::iterator it = m_CharacterRecord->mapSKillSucInfo.find(value);

	if (it == m_CharacterRecord->mapSKillSucInfo.end())
	{
		m_CharacterRecord->mapSKillSucInfo.insert(std::pair<DHOST_TYPE_UINT32, DHOST_TYPE_INT32>(value, 1));
	}
	else
	{
		it->second++;
	}
}

DHOST_TYPE_FLOAT CCharacter::GetAwayFromKeyboardGameTime()
{
	return m_AwayFromKeyboardGameTime;
}

void CCharacter::SetAwayFromKeyboardGameTime(DHOST_TYPE_FLOAT value)
{
	m_AwayFromKeyboardGameTime = value;
}

void CCharacter::UpdateAwayFromKeyboardTimeAccumulate(DHOST_TYPE_FLOAT value)
{
	m_CharacterRecord->AwayFromKeyboardTimeAccumulate += value;
}

void CCharacter::IncreaseUseSkillCount()
{
	++m_CharacterRecord->UseSkillCount;
}

void CCharacter::IncreaseUseSignatureCount()
{
	++m_CharacterRecord->UseSignatureCount;
}

void CCharacter::IncreaseUseQuickChatCount()
{
	++m_CharacterRecord->UseQuickChatCount;
}

void CCharacter::IncreaseUseSwitchCount()
{
	++m_CharacterRecord->UseSwitchCount;
}

void CCharacter::IncreaseActionGreatDefenseCount()
{
	++m_CharacterRecord->ActionGreatDefenseCount;
}

void CCharacter::IncreaseActionLayUpGoalCount()
{
	++m_CharacterRecord->ActionLayUpGoalCount;
}

void CCharacter::IncreaseActionDunkGoalCount()
{
	++m_CharacterRecord->ActionDunkGoalCount;
}

void CCharacter::IncreaseActionAnkleBreakeCount()
{
	++m_CharacterRecord->ActionAnkleBreakeCount;
}

void CCharacter::IncreaseIllegalScreenCount()
{
	++m_CharacterRecord->IllegalScreenCount;
}

F4PACKET::MOVE_MODE CCharacter::GetMoveMode()
{
	return m_MoveMode;;
}

F4PACKET::MOVE_MODE CCharacter::GetPreMoveMode()
{
	return m_PreMoveMode;;
}

void CCharacter::SetMoveMode(F4PACKET::MOVE_MODE value)
{
	m_PreMoveMode = m_MoveMode;
	m_MoveMode = value;
}

void CCharacter::SetPreMoveMode(F4PACKET::MOVE_MODE value)
{
	m_PreMoveMode = value;
}

void CCharacter::UseJoystick()
{
	m_CharacterRecord->UseJoystick = true;
}

void CCharacter::InitCharacterDisConnectTimeInfo()
{
	m_DisconnectTime = kFLOAT_INIT;
	m_DiconnectComebackTime = kFLOAT_INIT;
}

void CCharacter::CalcCharacterDisconnectTimeAccumulate()
{
	if (m_DisconnectTime > kFLOAT_INIT)
	{
		m_CharacterRecord->DisconnectTimeAccumulate += (m_DiconnectComebackTime - m_DisconnectTime);
	}

	InitCharacterDisConnectTimeInfo();
}

DHOST_TYPE_FLOAT CCharacter::GetCharacterDisconnectTime()
{
	return m_DisconnectTime;
}

void CCharacter::SetCharacterDisconnectTime(DHOST_TYPE_FLOAT value)
{
	m_DisconnectTime = value;
}

DHOST_TYPE_FLOAT CCharacter::GetCharacterDiconnectComebackTime()
{
	return m_DiconnectComebackTime;
}

void CCharacter::SetCharacterDiconnectComebackTime(DHOST_TYPE_FLOAT value)
{
	m_DiconnectComebackTime = value;

	CalcCharacterDisconnectTimeAccumulate();
}

DHOST_TYPE_FLOAT CCharacter::GetStartYaw()
{
	return m_StartYaw;
}

void CCharacter::SetStartYaw(DHOST_TYPE_FLOAT value)
{
	m_StartYaw = value;
}

JOVECTOR3& CCharacter::GetStartPosition()
{
	return m_StartPosition;
}
void CCharacter::SetStartPosition(F4PACKET::SPlayerAction* pInfo)
{
	if (pInfo != nullptr)
	{
		m_StartPosition.fX = pInfo->positionlogic().x();
		m_StartPosition.fY = pInfo->positionlogic().y();
		m_StartPosition.fZ = pInfo->positionlogic().z();
	}
}

DHOST_TYPE_BOOL CCharacter::GetSecondAnimationReceivePacket()
{
	if (m_CharacterCurrentState == EHOST_CHARACTER_STATE::SHOT && m_CharacterState[m_CharacterCurrentState] != nullptr)
	{
		return m_CharacterState[m_CharacterCurrentState]->GetSecondAnimationReceivePacket();
	}

	return false;
}

DHOST_TYPE_BOOL CCharacter::GetShotIsLeftHanded()
{
	return m_Action->shotIsLeftHand;
}

void CCharacter::SetShotIsLeftHanded(DHOST_TYPE_BOOL value)
{
	m_Action->shotIsLeftHand = value;
}

void CCharacter::IncreaseNotSamePositionCount()
{
	++m_CharacterRecord->NotSamePositionCount;
}

DHOST_TYPE_UINT32 CCharacter::GetPlayerNumber()
{
	return m_PlayerNumber;
}

void CCharacter::SetSpeedHackClientjudgmentCount(DHOST_TYPE_INT32 value)
{
	m_CharacterRecord->SpeedHackClientjudgment = value;
}

void CCharacter::IncreaseHackImpossibleAction()
{
	++m_CharacterRecord->HackImpossibleAction;
}

void CCharacter::SetMemoryTamperClientjudgmentCount(DHOST_TYPE_INT32 value)
{
	m_CharacterRecord->MemoryTamperClientjudgment = value;
}

void CCharacter::IncreasePacketTamper()
{
	++m_CharacterRecord->PacketTamperCount;
}

DHOST_TYPE_BALL_NUMBER CCharacter::GetCharacterForceBallEventFail()
{
	return m_ForceBallEventFail;
}

void CCharacter::SetCharacterForceBallEventFail(DHOST_TYPE_BALL_NUMBER value)
{
	m_ForceBallEventFail = value;
}

void CCharacter::SetCharacterBallEventSuccess(DHOST_TYPE_BALL_NUMBER value)
{
	m_BallEventSuccess = value;
}

void CCharacter::InitCharacterPassive()
{
	if (m_CharacterPassive != nullptr)
	{
		m_CharacterPassive->type = F4PACKET::CHARACTER_PASSIVE_TYPE::none;
		m_CharacterPassive->valueCurr = kFLOAT_INIT;
		m_CharacterPassive->valueMax = kFLOAT_INIT;
		m_CharacterPassive->activate = false;
		m_CharacterPassive->effectTrigger = false;
	}
}

DHOST_TYPE_BOOL CCharacter::InitialPassiveDataSet(const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo)
{
	return true;
}

DHOST_TYPE_BOOL CCharacter::SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	if (BeginValidatePassiveValue(pInfo->valuecurr(), pInfo->valuemax()))
	{
		if (pInfo != nullptr)
		{
			m_CharacterPassive->type = pInfo->type();
			m_CharacterPassive->valueCurr = pInfo->valuecurr();
			m_CharacterPassive->valueMax = pInfo->valuemax();
			m_CharacterPassive->activate = pInfo->activate();
			m_CharacterPassive->effectTrigger = pInfo->effecttrigger();
			m_CharacterPassive->startTime = pInfo->starttime();

		}

		return true;
	}

	return false;
}

void CCharacter::InitCharacterPotentialInfo()
{
	m_mapPotentialInfo.clear();

	if (m_Information != nullptr)
	{
		for(int i = 0; i < m_Information->potentials.size(); ++i)
		{
			m_mapPotentialInfo.insert(std::pair<F4PACKET::POTENTIAL_INDEX, F4PACKET::SPotentialInfo>(m_Information->potentials[i].potentialindex(), m_Information->potentials[i]));

			m_CharacterPotentialValueSum += m_Information->potentials[i].potentialvalue();
			m_CharacterPotentialValueSum += m_Information->potentials[i].potentialbloomratevalue();
			m_CharacterPotentialValueSum += m_Information->potentials[i].potentialbloombuffvalue();
		}
	}
}

DHOST_TYPE_BOOL CCharacter::GetCharacterPotentialInfo(F4PACKET::POTENTIAL_INDEX value, F4PACKET::SPotentialInfo& sInfo)
{
	DHOST_TYPE_BOOL result = false;

	std::map<F4PACKET::POTENTIAL_INDEX, F4PACKET::SPotentialInfo>::iterator iter = m_mapPotentialInfo.find(value);

	if (iter != m_mapPotentialInfo.end())
	{
		sInfo = iter->second;
		result = true;
	}

	return result;
}

DHOST_TYPE_BOOL  CCharacter::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{
	m_bCheckPassiveValidity = true; // 오버라이드 되지 않은 하위 캐릭터가 호출되지 않은 상태에서는 이게 호출됨 
	return m_bCheckPassiveValidity;
}

DHOST_TYPE_BOOL  CCharacter::IsValidatePassiveValue(DHOST_TYPE_FLOAT valueCurr)
{
	DHOST_TYPE_BOOL re = m_bCheckPassiveValidity;
	return re;
}

void CCharacter::EndValidatePassiveValue()
{ 
	m_bCheckPassiveValidity = true; 
}

// 1. 슛은 서버에서만 적용 
// 2. 블럭은 클라와 서버가 같이 적용 
// 3. 나머지는 클라만 적용 
// 처음 부터 구조를 그렇게 만들어 통일하기 어렵다 ㅜㅜ 

void CCharacter::OnCharacterGiftGive(SGiveGiftInfo* pGiftInfo)
{
	float duration = 0.0f;
	float buffValue = 0.0f;

	DHOST_TYPE_BOOL result = false;

	std::string strKey = "None";

	for (std::map<std::string, SAddAbility>::iterator it = pGiftInfo->Attrs.begin(); it != pGiftInfo->Attrs.end(); ++it)
	{
		strKey = it->first;
		const SAddAbility& abilityInfo = it->second;

		if (strKey == CCharacterBuff::GiftBuff_JumpShotValue)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_ShootDefensePerimeterValue)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_AnkleBreakValue_Increase)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_AnkleBreakValue_Resistance)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_BlockValue_RimAttack_Increase)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_BlockValue_RimAttack_Resistance)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_StealValue)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}
		else
		if (strKey == CCharacterBuff::GiftBuff_NicePassValue)
		{
			// 처리 로직
			buffValue = (float)(abilityInfo.AddValue / 100.0f);
			duration = (float)abilityInfo.Times;
			result = true;
			break;
		}

	}

	//string strKey = ;

	if (result)
	{
		F4PACKET::ACTION_TYPE actioType;
		actioType = F4PACKET::ACTION_TYPE::action_shot;

		SCharacterBuff buff(0, GetCharacterInformation()->id,		// 버프가 되는 캐릭터 
			F4PACKET::CHARACTER_BUFF_TYPE::Buff_Common,			
			actioType,
			DHOST_TYPE_BOOL(true),
			duration,
			buffValue, m_Information->specialcharacterlevel,
			strKey);

		GenerateBuff(buff);


		string log_message = "GenerateBuff !! ";
		m_pHost->ToLog(log_message.c_str());
	}
}

void CCharacter::OnCharacterPlayerCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto pPacket = (CFlatBufPacket<F4PACKET::play_c2s_playerCeremony_data>*)pData;
	auto* data = pPacket->GetData();

	if (data->ceremonytype() == F4PACKET::CEREMONY_TYPE::ceremonyType_touchWaterBottle)
	{
		float random = abs(Util::GetRandom(1.0f));
		if (random < 0.0f)
		{
			DHOST_TYPE_FLOAT duration = 14.0f;  //m_pBalanceData->GetValue("VPassive_Lia_Time");

			SCharacterBuff buff(m_Information->id, GetCharacterInformation()->id, // 버프가 되는 캐릭터 
				F4PACKET::CHARACTER_BUFF_TYPE::WaterDrink,
				F4PACKET::ACTION_TYPE::action_shot,
				DHOST_TYPE_BOOL(true),
				duration,
				0.20f, false);

			CCharacterBuff::GenerateBuff(buff);
		}
	}
}

void CCharacter::UpdateCharacter(DHOST_TYPE_FLOAT timeDelta)
{
	CCharacterMovement::Update(timeDelta);

	CCharacterBuff::ProcessTerminateBuff(timeDelta);
}


DHOST_TYPE_BOOL CCharacter::CheckSkillForce(SKILL_INDEX value)
{
	DHOST_TYPE_BOOL result = false;

	F4PACKET::SPlayerInformationT* pCharacterInfo = m_Information;
	if (pCharacterInfo != nullptr)
	{
		DHOST_TYPE_UINT32 param_convert_uint = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_divide = kUINT32_INIT;
		DHOST_TYPE_UINT32 param_remainder = kUINT32_INIT;

		param_convert_uint = (DHOST_TYPE_UINT32)value;

		if (param_convert_uint > kUINT32_INIT)
		{
			param_divide = param_convert_uint / kSKILL_INDEX_DIVIDE;
			param_remainder = param_convert_uint % kSKILL_INDEX_DIVIDE;	// 나머지가 1 이상인 경우 상위 스킬

			for (int i = 0; i < pCharacterInfo->skills.size(); ++i)
			{
				DHOST_TYPE_UINT32 own_convert_uint = (DHOST_TYPE_UINT32)pCharacterInfo->skills[i].index();

				if (own_convert_uint > kUINT32_INIT)
				{
					DHOST_TYPE_UINT32 own_divide = own_convert_uint / kSKILL_INDEX_DIVIDE;

					if (param_divide == own_divide)
					{
						result = pCharacterInfo->skills[i].forceawaken();
						break;
					}
				}
			}
		}
	}

	return result;
}

/*
DHOST_TYPE_BOOL CCharacter::VerifyPassiveValue()
{
	
	m_CharacterPassive->type = pInfo->type();
	m_CharacterPassive->valueCurr = pInfo->valuecurr();
	m_CharacterPassive->valueMax = pInfo->valuemax();
	m_CharacterPassive->activate = pInfo->activate();
	m_CharacterPassive->effectTrigger = pInfo->effecttrigger();
	m_CharacterPassive->startTime = pInfo->starttime();
	
}
*/
//! end