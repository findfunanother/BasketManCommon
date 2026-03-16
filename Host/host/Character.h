#pragma once
#include <deque>
#include <map>
#include "HostCommonFunction.h"
#include "Ability.h"
#include "Util.h"
#include "FileLog.h"
#include "../../GameCore/CAnimationController.h"

#include "CharacterBuff.h"
#include "CharacterMovement.h"
#include "CharacterAction.h"

//#include "Host.h"


//class CCharacterBuff;

class CCharacterState;
class CCharacterManager;
class CBallController;
class CHost;
class CDataManagerBalance;

struct SCharacterAction
{
	DHOST_TYPE_GAME_TIME_F actionTime;
	F4PACKET::ACTION_TYPE actionType;	    // 대분류 (슛, 리바, 블락, 스틸등)
	F4PACKET::ACTION_TYPE preActionType;	// 대분류 (슛, 리바, 블락, 스틸등)
	F4PACKET::ACTION_TYPE preActionTypeAgo;	// 대분류 (슛, 리바, 블락, 스틸등)
	F4PACKET::SPlayerAction* action;
	DHOST_TYPE_FLOAT speed;
	F4PACKET::SHOT_TYPE shotType;
	DHOST_TYPE_BOOL shotIsLeftHand;
	DHOST_TYPE_BALL_NUMBER potentialBloom;	// 개화 여부
};

struct SCharacterPassive
{
	F4PACKET::CHARACTER_PASSIVE_TYPE type;
	DHOST_TYPE_FLOAT valueCurr;
	DHOST_TYPE_FLOAT valueMax;
	DHOST_TYPE_BOOL activate;
	DHOST_TYPE_BOOL effectTrigger;
	DHOST_TYPE_FLOAT startTime;
};


class CCharacter : public CCharacterBuff, public CCharacterMovement, public CCharacterAction
{
public:
	CCharacter(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber = kUINT32_INIT);
	virtual ~CCharacter();

	void SetCharacterUserID(DHOST_TYPE_USER_ID value);

	void Initialize(F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber);
	void InitCharacterAction();

	F4PACKET::SPlayerInformationT* GetCharacterInformation();

	void SetCharacterAction(DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType, F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_BALL_NUMBER potentialBloom);
	void SetCharacterActionOverlap(F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value);
	void SetCharacterActionOverlapDestYaw(F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw);

	SCharacterAction* GetCharacterAction();
	F4PACKET::SHOT_TYPE GetCharacterActionShotType();
	DHOST_TYPE_BALL_NUMBER GetCharacterActionPotentialBloom();

	void SetCharacterRestriction(EMOVEMENT_RESTRICTION value);
	EMOVEMENT_RESTRICTION GetCharacterRestriction();

	const DHOST_TYPE_INT32 GetActionIDX() const { return m_ActionIDX; }
	void SetActionIDX(DHOST_TYPE_INT32 value) { m_ActionIDX = value; }

	const DHOST_TYPE_FLOAT GetFocusPoint() const { return m_FocusPoint; }
	void SetFocusPoint(DHOST_TYPE_FLOAT value) { m_FocusPoint = value; }

	DHOST_TYPE_BOOL GetSwitchToAi() { return m_CharacterRecord->SwitchToAi; }
	void SetSwitchToAi(DHOST_TYPE_BOOL value) { m_CharacterRecord->SwitchToAi = value; }

	DHOST_TYPE_UINT32 GetSpeedHackLevel() { return m_CharacterRecord->SpeedHackLevel; }
	void SetSpeedHackLevel(DHOST_TYPE_UINT32 value) { m_CharacterRecord->SpeedHackLevel = value; }

	DHOST_TYPE_FLOAT GetSpeedHackTimeAccumulate() { return m_CharacterRecord->SpeedHackTimeAccumulate; }
	void SetSpeedHackTimeAccumulate(DHOST_TYPE_FLOAT value) { m_CharacterRecord->SpeedHackTimeAccumulate = value; }

	ECHARACTER_KICK_TYPE GetCharacterKickType() { return m_CharacterRecord->CharacterKickType; }
	void SetCharacterKickType(ECHARACTER_KICK_TYPE value) { m_CharacterRecord->CharacterKickType = value; }

	void InitCharacterRecord();
	SCharacterRecord* GetCharacterRecord() const { return m_CharacterRecord; }
	DHOST_TYPE_UINT16 GetCharacterRecordCountWithType(F4PACKET::RECORD_TYPE Type);
	void UpdateCharacterRecord(ECHARACTER_RECORD_TYPE Type = ECHARACTER_RECORD_TYPE::NONE, DHOST_TYPE_BOOL bOverTime = false);

	void AddCharacterActionDeque();
	std::deque<SCharacterAction> GetCharacterActionDeque() { return m_CharacterActionDeque; }

	void SetCharacterReadyPosition(F4PACKET::ACTION_TYPE actionType, TB::SVector3 position);

	void AddCharacterShootInfo(float x, float z, uint32_t zone, bool goal);

	DHOST_TYPE_BOOL CheckActionTypeStand(F4PACKET::ACTION_TYPE actionType);
	DHOST_TYPE_BOOL CheckActionTypeMove(F4PACKET::ACTION_TYPE actionType);

	CCharacterState* GetCharacterState(EHOST_CHARACTER_STATE characterStateType);
	EHOST_CHARACTER_STATE GetCharacterCurrentState() const { return m_CharacterCurrentState; }
	EHOST_CHARACTER_STATE GetPreCharacterCurrentState() const { return m_PreCharacterCurrentState; }

	void SetCharacterCurrentState(EHOST_CHARACTER_STATE value) { m_CharacterCurrentState = value; }
	void ChangeCharacterState(EHOST_CHARACTER_STATE value, DHOST_TYPE_GAME_TIME_F time, void* pData = nullptr);
	void UpdateCharacterState(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	void OnCharacterMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, F4PACKET::MOVE_MODE moveMode);

	const EBURST_ON_FIRE_MODE_STEP GetCharacterBurstOnFireModeStep() const { return m_BurstOnFireModeStep; }
	void SetCharacterBurstOnFireModeStep(EBURST_ON_FIRE_MODE_STEP value) { m_BurstOnFireModeStep = value; }
	const DHOST_TYPE_FLOAT GetCharacterBurstGauge() const { return m_BurstGauge; }
	void SetBurstGauge(DHOST_TYPE_FLOAT value) { m_BurstGauge = value; }
	const DHOST_TYPE_FLOAT GetBurstActionSubtractTime() const { return m_BurstActionSubtractTime; }
	void SetBurstActionSubtractTime(DHOST_TYPE_FLOAT value) { m_BurstActionSubtractTime = value; }
	const DHOST_TYPE_FLOAT GetBurstActionReleaseTriggerTime() const { return m_BurstActionReleaseTriggerTime; }
	void SetBurstActionReleaseTriggerTime(DHOST_TYPE_FLOAT value) { m_BurstActionReleaseTriggerTime = value; }

	DHOST_TYPE_UINT32 PushAndCheckBurstConsecutiveScore(DHOST_TYPE_GAME_TIME_F time);
	DHOST_TYPE_UINT32 PushAndCheckBurstConsecutiveSteal(DHOST_TYPE_GAME_TIME_F time);
	DHOST_TYPE_UINT32 PushAndCheckBurstConsecutiveBlock(DHOST_TYPE_GAME_TIME_F time);
	void ClearBurstConsecutiveScore();
	void ClearBurstConsecutiveSteal();
	void ClearBurstConsecutiveBlock();
	
	CAbility* GetAbility();

	CAnimationController* GetAnimationController();

	DHOST_TYPE_BOOL IsLeftHand();
	
	//! 20220623 CharacterState by thinkingpig
	void InitCharacterStateStartAnimationPosition();
	void InitCharacterStateMoveDirection();
	void InitCharacterStateActionPosition();

	F4PACKET::SPlayerAction* GetCharacterStateAction();
	void SetCharacterStateAction(F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time);

	void SetCharacterStateActionPosition(float x, float y, float z, string callPos = "");

	F4PACKET::ACTION_TYPE GetCharacterStateActionType();
	void SetCharacterStateActionType(F4PACKET::ACTION_TYPE value);

	DHOST_TYPE_GAME_TIME_F GetCharacterStateActionTime();
	void SetCharacterStateActionTime(DHOST_TYPE_GAME_TIME_F value);

	DHOST_TYPE_INT32 GetCharacterStateAnimationIndex();
	void SetCharacterStateAnimationIndex(DHOST_TYPE_INT32 value);

	JOVECTOR3& GetCharacterStateStartAnimationPosition();
	void SetCharacterStateStartAnimationPosition(const TB::SVector3& pPos);

	CBallController* GetBallController();

	JOVECTOR3 GetCurrentBallPosition();
	void SetCurrentBallPosition(JOVECTOR3 value);
	//! end

	void IncreaseLatencyCount(DHOST_TYPE_FLOAT value);

	void SetAvgPingLatency(DHOST_TYPE_FLOAT value);

	void IncreaseBadMannerCount(EBAD_MANNER_TYPE type);
	DHOST_TYPE_UINT16 GetBadMannerTypeCount(EBAD_MANNER_TYPE type);

	void IncreaseEmojiCount(DHOST_TYPE_INT32 value);
	void IncreaseCeremonyCount(DHOST_TYPE_INT32 value);
	void IncreaseSkillSucCount(DHOST_TYPE_UINT32 value);

	DHOST_TYPE_FLOAT GetAwayFromKeyboardGameTime();
	void SetAwayFromKeyboardGameTime(DHOST_TYPE_FLOAT value);

	void UpdateAwayFromKeyboardTimeAccumulate(DHOST_TYPE_FLOAT value);

	void IncreaseUseSkillCount();
	void IncreaseUseSignatureCount();
	void IncreaseUseQuickChatCount();
	void IncreaseUseSwitchCount();

	void IncreaseActionGreatDefenseCount();
	void IncreaseActionLayUpGoalCount();
	void IncreaseActionDunkGoalCount();
	void IncreaseActionAnkleBreakeCount();

	void IncreaseIllegalScreenCount();

	F4PACKET::MOVE_MODE GetMoveMode();
	F4PACKET::MOVE_MODE GetPreMoveMode();
	void SetPreMoveMode(F4PACKET::MOVE_MODE value);
	void SetMoveMode(F4PACKET::MOVE_MODE value);

	void UseJoystick();

	void InitCharacterDisConnectTimeInfo();
	void CalcCharacterDisconnectTimeAccumulate();

	DHOST_TYPE_FLOAT GetCharacterDisconnectTime();
	void SetCharacterDisconnectTime(DHOST_TYPE_FLOAT value);

	DHOST_TYPE_FLOAT GetCharacterDiconnectComebackTime();
	void SetCharacterDiconnectComebackTime(DHOST_TYPE_FLOAT value);

	DHOST_TYPE_FLOAT GetStartYaw();
	void SetStartYaw(DHOST_TYPE_FLOAT value);

	JOVECTOR3& GetStartPosition();
	void SetStartPosition(F4PACKET::SPlayerAction* pInfo);

	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket();

	DHOST_TYPE_BOOL GetShotIsLeftHanded();
	void SetShotIsLeftHanded(DHOST_TYPE_BOOL value);

	void IncreaseNotSamePositionCount();

	DHOST_TYPE_UINT32 GetPlayerNumber();

	void SetSpeedHackClientjudgmentCount(DHOST_TYPE_INT32 value);

	void IncreaseHackImpossibleAction();

	void SetMemoryTamperClientjudgmentCount(DHOST_TYPE_INT32 value);

	void IncreasePacketTamper();

	DHOST_TYPE_BALL_NUMBER GetCharacterForceBallEventFail();
	void SetCharacterForceBallEventFail(DHOST_TYPE_BALL_NUMBER value);

	DHOST_TYPE_BALL_NUMBER GetCharacterBallEventSuccess() const { return m_BallEventSuccess; }
	void SetCharacterBallEventSuccess(DHOST_TYPE_BALL_NUMBER value);

	//! passive
	void InitCharacterPassive();

	virtual DHOST_TYPE_BOOL SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo);
	virtual DHOST_TYPE_BOOL InitialPassiveDataSet(const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo);

	SCharacterPassive* GetCharacterPassive() const { return m_CharacterPassive; }

	//! potential
	void InitCharacterPotentialInfo();
	DHOST_TYPE_BOOL GetCharacterPotentialInfo(F4PACKET::POTENTIAL_INDEX value, F4PACKET::SPotentialInfo& sInfo);

	DHOST_TYPE_FLOAT GetCharacterPotentialValueSum() const { return m_CharacterPotentialValueSum; }

	DHOST_TYPE_FLOAT GetAutoPlayWatingTime() { return m_AutoPlayWatingTime; }
	
protected:
	DHOST_TYPE_FLOAT m_AutoPlayWatingTime;

	CAnimationController* m_pOwnerAnimationController;

	F4PACKET::SPlayerInformationT* m_Information;

	SCharacterAction* m_Action;

	EMOVEMENT_RESTRICTION m_Restriction;

	CAbility* m_ability;

	DHOST_TYPE_INT32 m_ActionIDX;	// 리플레이 포인트 정산시 캐릭터의 몇번째 액션까지 검수했는지 확인하는 변수
	DHOST_TYPE_FLOAT m_FocusPoint;

	SCharacterRecord* m_CharacterRecord;
	std::deque<SCharacterAction> m_CharacterActionDeque;

	EHOST_CHARACTER_STATE m_CharacterCurrentState;
	EHOST_CHARACTER_STATE m_PreCharacterCurrentState;

	std::map<EHOST_CHARACTER_STATE, CCharacterState*> m_CharacterState;

	EBURST_ON_FIRE_MODE_STEP m_BurstOnFireModeStep;
	DHOST_TYPE_FLOAT m_BurstGauge;
	DHOST_TYPE_FLOAT m_BurstActionSubtractTime;	// 온 파이어모드 발동 시작 시간
	DHOST_TYPE_FLOAT m_BurstActionReleaseTriggerTime;	// 온 파이어모드 해제 트리거 시간
	std::vector<DHOST_TYPE_GAME_TIME_F> m_vecBurstConsecutiveScore;	// 버스트 야투 득점시간
	std::vector<DHOST_TYPE_GAME_TIME_F> m_vecBurstConsecutiveSteal;	// 버스트 스틸 시간
	std::vector<DHOST_TYPE_GAME_TIME_F> m_vecBurstConsecutiveBlock;	// 버스트 블럭 시간

	//! 20220623 CharacterState by thinkingpig
	F4PACKET::SPlayerAction* m_CharacterStateAction;

	F4PACKET::ACTION_TYPE m_CharacterStateActionType;


	DHOST_TYPE_GAME_TIME_F m_CharacterStateActionTime;
	DHOST_TYPE_INT32 m_CharacterStateAnimationIndex;
	JOVECTOR3 m_CharacterStateStartAnimationPosition;

	CBallController* m_pOwnerBallController;
	JOVECTOR3 m_CurrentBallPosition;
	//! end

	DHOST_TYPE_FLOAT m_AwayFromKeyboardGameTime;

	DHOST_TYPE_FLOAT m_DisconnectTime;
	DHOST_TYPE_FLOAT m_DiconnectComebackTime;

	F4PACKET::MOVE_MODE m_MoveMode;
	F4PACKET::MOVE_MODE m_PreMoveMode;

	DHOST_TYPE_FLOAT m_StartYaw;
	JOVECTOR3 m_StartPosition;

	DHOST_TYPE_UINT32 m_PlayerNumber;

	DHOST_TYPE_BALL_NUMBER m_ForceBallEventFail;
	DHOST_TYPE_BALL_NUMBER m_BallEventSuccess;

	std::map<F4PACKET::POTENTIAL_INDEX, F4PACKET::SPotentialInfo> m_mapPotentialInfo;

	DHOST_TYPE_FLOAT m_CharacterPotentialValueSum;

	// About Passive 
public :
	CHost* GetHost() { return m_pHost; }

protected:
	CHost* m_pHost;
	CDataManagerBalance* m_pBalanceData;
	SCharacterPassive* m_CharacterPassive;
	DHOST_TYPE_BOOL  m_bCheckPassiveValidity;

private:
	SGiveGiftInfo* pGiftInfo;

private:
	DHOST_TYPE_BOOL  m_bPreActionMoved;
	DHOST_TYPE_BOOL  m_bPreActionTapOut;
	DHOST_TYPE_FLOAT m_fTapOutElapsedTime;
	DHOST_TYPE_BOOL  m_bAI;

public:
	void SetAI(DHOST_TYPE_BOOL bAi) { m_bAI = bAi; }
	DHOST_TYPE_BOOL IsAICharacter() { return m_bAI; }

	void SetActionTapOut(DHOST_TYPE_BOOL flag) { m_bPreActionTapOut = flag; }
	DHOST_TYPE_BOOL GetActionTapOut() { return m_bPreActionTapOut; }

public:

	DHOST_TYPE_BOOL CheckSkillForce(F4PACKET::SKILL_INDEX value);
	/*DHOST_TYPE_INT32 m_nCheckCnt;
	DHOST_TYPE_BOOL CheckSpeed(DHOST_TYPE_FLOAT elapsedCurTime, TB::SVector3 curPos, DHOST_TYPE_FLOAT speed, DHOST_TYPE_FLOAT& outExpectDistance, DHOST_TYPE_FLOAT& outDistance, DHOST_TYPE_FLOAT& outGap);
	void ClearMoveCnt();
	TB::SVector3 GetCurPos() { return m_MoveCurPos; }
	*/

public:
	void OnCharacterGiftGive(SGiveGiftInfo* pGiftInfo);
	virtual void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) {}
	virtual void OnCharacterPlayerCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time);
	virtual void OnCharacterPlayerCoupleCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time) {}

	virtual void OnGoalResult(DHOST_TYPE_CHARACTER_SN CharacterSN, F4PACKET::SHOT_TYPE value, DHOST_TYPE_BOOL result) {} // 오카포 때 만듦
	virtual void OnGoalFailed(std::vector<DHOST_TYPE_CHARACTER_SN> vBallShotHindrance) {} // 플로리스 때 만듦
	virtual void OnTurnOver(DHOST_TYPE_CHARACTER_SN turnoverCauser, DHOST_TYPE_CHARACTER_SN turnoverTarget) {}
	virtual void OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime) {}
	
	virtual void OnPlayerShot(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SHOT_TYPE value) {}
	virtual void OnPlayerShotHindrance(DHOST_TYPE_CHARACTER_SN characterSN) {}
	virtual void OnPlayerPass(DHOST_TYPE_CHARACTER_SN SendCharacterSN, DHOST_TYPE_CHARACTER_SN RecvCharacterSN, F4PACKET::CHARACTER_BUFF_TYPE buffType) {}
	virtual void OnPlayerBlock(DHOST_TYPE_CHARACTER_SN blockCharacterSN, DHOST_TYPE_CHARACTER_SN blockedCharacterSN, DHOST_TYPE_BOOL bSuccess) {}
	virtual void OnPlayerPenentrate(DHOST_TYPE_CHARACTER_SN characterSN) {}
	virtual void OnPlayerDash(DHOST_TYPE_CHARACTER_SN characterSN) {}

	virtual void SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) {}
	virtual DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) { return 0.0f; }

	virtual DHOST_TYPE_FLOAT GetBonusShotPassiveCurrent() { return m_CharacterPassive->valueCurr; }  // 오버라이딩이 안되어 있다면 기본 보너스 패시브 
	virtual DHOST_TYPE_FLOAT GetBonusShotPassiveMax()
	{ 
		return m_CharacterPassive->valueMax; 
	
	}              // 오버라이딩이 안되어 있다면 기본 보너스 패시브 맥스 

	virtual DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax); // 1. 패시브 데이터 검증 시작 , c2s_PlayerPassive 에서 체크( 모든 액션의 시작 전에 판단해야 함 ) 
	virtual DHOST_TYPE_BOOL  IsValidatePassiveValue(DHOST_TYPE_FLOAT valueCurr);     // 2. 확인이 되면 노골이 되거나 다른 것을 못하게 함 
	//virtual void             EndValidatePassiveValue();   // 3. 끝내는 함수 변수 초기화 

	template<typename... Args>
	void EndValidatePassive(Args... args) { EndValidatePassiveValue(args...); }

protected:

	virtual void UpdateCharacter(DHOST_TYPE_FLOAT timeDelta);
	virtual void EndValidatePassiveValue();
	virtual void EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType) {}
};