#pragma once
#include <map>
#include <exception>
#include "Character.h"
#include "HeaderDefine.h"
#include "HostTypeDefine.h"
#include "AnimationInformation_generated.h"
#include "MannerManager.h"

using namespace std;
using namespace TB;
using namespace F4PACKET;

class CDataManagerBalance;
struct SCharacterAction;
class CCharacterState;
class CAnimationController;
class CBallController;
struct SCharacterBuff;

typedef std::function<void(uint32_t, uint16_t, uint16_t)> BadMannerCallback;
typedef std::function<void()> OnFireModeCallback;

class DECLSPEC CCharacterManager
{
public:

	CCharacterManager();
	~CCharacterManager();

	void CharacterManagerInit(CHost* pHost, CDataManagerBalance* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo);

	DHOST_TYPE_BOOL CreateCharacter(F4PACKET::SPlayerInformationT*, DHOST_TYPE_UINT32 playerNumber = kUINT32_INIT);
	DHOST_TYPE_BOOL CreateCharacter(F4PACKET::SPlayerInformationT* pInfo, DHOST_TYPE_UINT32 playerNumber, F4PACKET::ECHARACTER_INDEX index);

	DHOST_TYPE_BOOL AddCharacterMap(CCharacter* pInfo);
	DHOST_TYPE_BOOL AddTeamCharacterMap(CCharacter* pInfo);
	DHOST_TYPE_BOOL GetTeamCharacterMap(DHOST_TYPE_INT32 teamIndex);
	
	void SetCharacterUserID(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_USER_ID value);

	void SortCharacters();
	void SortCharactersWithRole();

	static bool Compare(CCharacter* a, CCharacter* b);
	static bool CompareWithRole(CCharacter* a, CCharacter* b);
	static bool CompareWithRecord(const std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_UINT16>& a, const std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_UINT16>& b);

	DHOST_TYPE_UINT32 GetRoleCount(DHOST_TYPE_INT32 team, F4PACKET::ROLE role);

	void SetCharacterMoveLock(DHOST_TYPE_CHARACTER_SN characterSN, EMOVEMENT_RESTRICTION value);
	void SetCharacterMoveUnLock();

	CCharacter* CheckCollision(CCharacter* pInfo);
	TB::SVector3 CorrectionCollision(CCharacter* pInfo1, CCharacter* pInfo2);

	CCharacter* CheckCollisionEx(CCharacter* pInfo);
	TB::SVector3 CorrectionCollisionEx(CCharacter* pInfo1, CCharacter* pInfo2);

	void CharacterManagerUpdate(DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos);
	void CharacterManagerLateUpdate(DHOST_TYPE_FLOAT timeDelta);
	void CharacterManagerLateUpdateEx(DHOST_TYPE_FLOAT timeDelta);

	void ClearCharacters();

	void MixCharacterTeamInRole();

	CCharacter* GetCharacter(DHOST_TYPE_CHARACTER_SN characterSN);
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*> GetCharacters();

	F4PACKET::SPlayerInformationT* GetCharacterInformation(DHOST_TYPE_CHARACTER_SN characterSN);

	void GetCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);
	void GetHomeCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);
	void GetAwayCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);

	const TB::SVector3 GetCharacterPosition(DHOST_TYPE_CHARACTER_SN characterSN);

	F4PACKET::SPlayerAction* GetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN);
	F4PACKET::SPlayerAction* GetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN);

	F4PACKET::SHOT_TYPE GetCharacterActionShotType(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BALL_NUMBER GetCharacterActionPotentialBloom(DHOST_TYPE_CHARACTER_SN characterSN);

	SCharacterAction* GetCharacterActionInfo(DHOST_TYPE_CHARACTER_SN characterSN);

	DHOST_TYPE_INT32 GetCharacterTeam(DHOST_TYPE_CHARACTER_SN characterSN);
	CCharacter* GetCharacterTeamWithFirstPlayerNumber(DHOST_TYPE_CHARACTER_SN characterSN);

	void SetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType, F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_BALL_NUMBER potentialBloom);
	void SetCharacterActionOverlap(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value);
	void SetCharacterActionOverlapDestYaw(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw);
	void AddCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN);
	void AddServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN, ServerCharacterPosition position);
	std::deque<SCharacterAction> GetCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN);
	std::deque<ServerCharacterPosition> GetServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN);

	void GenerateCharacterBuff(DHOST_TYPE_CHARACTER_SN characterSN, SCharacterBuff characterBuff);

	size_t GetTeamCharacterSize(DHOST_TYPE_INT32 indexTeam);
	DHOST_TYPE_CHARACTER_SN GetCharacterSN(DHOST_TYPE_INT32 indexTeam, DHOST_TYPE_INT32 indexCharacter);
	DHOST_TYPE_INT32 GetIndexOpponentTeam(DHOST_TYPE_INT32 indexTeam);

	void SetCharacterReadyPosition(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, TB::SVector3 position);

	void CalcFocusPoint();

	void GetTeamsActive(DHOST_TYPE_INT32 &team1, DHOST_TYPE_INT32 &team2);
	void SetTeamsActive(DHOST_TYPE_INT32 team1, DHOST_TYPE_INT32 team2);

	DHOST_TYPE_BOOL GetSwitchToAi(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetSwitchToAi(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);

	DHOST_TYPE_UINT32 GetSpeedHackLevel(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetSpeedHackLevel(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value);

	DHOST_TYPE_FLOAT GetSpeedHackTimeAccumulate(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetSpeedHackTimeAccumulate(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);

	ECHARACTER_KICK_TYPE GetCharacterKickType(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetCharacterKickType(DHOST_TYPE_CHARACTER_SN characterSN, ECHARACTER_KICK_TYPE value);

	DHOST_TYPE_BOOL CheckTeamsActive(DHOST_TYPE_INT32 teamIndex);
	DHOST_TYPE_INT32 GetIndexTeamWait();

	DHOST_TYPE_FLOAT GetMaxFocusPointInTeam(DHOST_TYPE_INT32 team);
	DHOST_TYPE_CHARACTER_SN GetMaxFocusPointCharacterInTeam(DHOST_TYPE_INT32 team);
	DHOST_TYPE_CHARACTER_SN GetMaxFocusPointCharacterInEnemyTeam(DHOST_TYPE_INT32 Scoreteam);

	void UpdateRecord(DHOST_TYPE_CHARACTER_SN characterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime = false);
	void SaveRegularTimeCharacterRecord();

	void PushCharacterRecord(SRedisMatchResult* pInfo, DHOST_TYPE_FLOAT roomElapsedTime);
	void GetCharacterRecordVec(std::vector<SCharacterRecord*>& vec);
	DHOST_TYPE_UINT16 GetCharacterRecordCountWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type);
	DHOST_TYPE_BOOL GetMostCharacterRerecordWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type, DHOST_TYPE_BOOL allowEqual);

	DHOST_TYPE_CHARACTER_SN GetMVP();
	void SetMVP(DHOST_TYPE_CHARACTER_SN value);
	DHOST_TYPE_CHARACTER_SN CalcMVPScore(DHOST_TYPE_INT32 WinnerTeam);
	
	DHOST_TYPE_FLOAT CalcReplayPoint(DHOST_TYPE_GAME_TIME_F ScoredTime);
	F4PACKET::ACTION_TYPE ConvertReplayActionTypeFromOriginActionType(F4PACKET::ACTION_TYPE value);

	DHOST_TYPE_FLOAT GetReplaySkillPoint(F4PACKET::SKILL_INDEX value);

	F4PACKET::SPlayerInformationT* FindCharacterInformationWithUserID(DHOST_TYPE_USER_ID UserID);
	F4PACKET::SPlayerInformationT* FindCharacterInformationWithUserIDAndIsNotAi(DHOST_TYPE_USER_ID UserID);

	void AddCharacterShootInfo(DHOST_TYPE_CHARACTER_SN characterSN, float x, float z, uint32_t zone, bool goal);

	DHOST_TYPE_CHARACTER_SN	GetLastCharacterSN();
	void AddLastCharacterSN();

	DHOST_TYPE_BOOL GetNoTableCharacterSend();
	void SetNoTableCharacterSend(DHOST_TYPE_BOOL value);

	void PushMatchInfo(int id, int target);
	void ModifyMatchInfo(int id, int target);
	int	GetMatchInfoID(int target);
	int	GetMatchInfoTarget(int id);
	void SetSwitchID(DHOST_TYPE_INT32 team, int id);
	int	GetSwitchID(DHOST_TYPE_INT32 team);
	std::vector<MatchInfo>& GetMatchInfo();

	void OnCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, void* pData, DHOST_TYPE_GAME_TIME_F time);
	void OnCharacterPlayerCeremony(DHOST_TYPE_CHARACTER_SN characterSN, void* pData, DHOST_TYPE_GAME_TIME_F time);

	void OnCharacterMessage(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, DHOST_TYPE_GAME_TIME_F gameTime, DHOST_TYPE_UINT32 keys, F4PACKET::MOVE_MODE moveMode);
	DHOST_TYPE_BOOL GetSecondAnimationReceivePacket(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BOOL GetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);
	void SetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time);

	DHOST_TYPE_FLOAT GetCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN);
	EBURST_ON_FIRE_MODE_STEP GetCharacterBurstOnFireModeStep(DHOST_TYPE_CHARACTER_SN characterSN);
	void CheckCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType, std::vector<DHOST_TYPE_INT32>& teamScores, DHOST_TYPE_BOOL timeWeights = false);
	void UpdateCharacterBurstGauge(CCharacter* pActionOwner, CCharacter* pEffectedCharacter, DHOST_TYPE_GAME_TIME_F OnwerActionTime, uint64_t OwnerActionType, EGAME_SCORE_STATUS EffectedCharacterScoreStatus, DHOST_TYPE_FLOAT BurstConsecutiveValue, DHOST_TYPE_BOOL timeWeights = false);
	void UpdateCharacterBurstGaugeWithPassiveActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest);
	void UpdateCharacterBurstGaugeWithMedalActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest);
	void CharacterBurstGaugeWith(CCharacter* pCharacter, DHOST_TYPE_FLOAT burstValue, BURST_REQUEST burstRequest);
	void CharacterBurstGaugeWithAll(CCharacter* pCharacter, DHOST_TYPE_FLOAT burstValueTeamA, DHOST_TYPE_FLOAT burstValueTeamB, BURST_REQUEST burstRequest);
	DHOST_TYPE_FLOAT CalcBurstGaugeWhat(uint64_t ActionType);
	DHOST_TYPE_FLOAT CalcBurstGaugeWho(EBURST_ACTION_SUBJECT who);
	DHOST_TYPE_FLOAT CalcBurstGaugeWhen(EGAME_SCORE_STATUS when);
	void UpdateCharacterOnFireModeStatBuff(CCharacter* pCharacter, DHOST_TYPE_FLOAT OnFireValue, DHOST_TYPE_BOOL Update = false);
	DHOST_TYPE_UINT64 GetBurstActionTypeWithRecordType(ECHARACTER_RECORD_TYPE recordType);
	void CheckCharacterOnFireModeRelease(CCharacter* pCharacter, DHOST_TYPE_FLOAT gameTime);

	void PushSendOnFireModeCharacter(DHOST_TYPE_CHARACTER_SN characterSN);
	void PushSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);
	void ClearSendOnFireModeCharacter();
	void ClearSendOnFireModeCharacterReservation();
	void RemoveSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN);
	std::vector<DHOST_TYPE_CHARACTER_SN> GetSendOnFireModeCharacter();
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> GetSendOnFireModeCharacterReservation();
	DHOST_TYPE_FLOAT FindSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN);

	DHOST_TYPE_BOOL CheckCharacterActionPenetrateType(F4PACKET::ACTION_TYPE type);
	void GetAllCharacterBurstGauge(std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>& map);

	DHOST_TYPE_FLOAT PushAndCheckBurstConsecutive(CCharacter* pCharacter, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType);

	void IncreaseLatencyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);

	void SetAvgPingLatency(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);

	//! 20230508 비매너 감지 - by thinkingpig
	void GetBadMannerBalanceData(SBadMannerBalanceData& data);
	void RegistCallbackFuncBadManner(BadMannerCallback pFunc);
	void IncreaseBadMannerCount(DHOST_TYPE_CHARACTER_SN characterSN, EBAD_MANNER_TYPE type);
	void UpdateAwayFromKeyboardTimeAccumulate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_CHARACTER_SN characterSN);
	void UpdateAwayFromKeyboardGameTime(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_CHARACTER_SN characterSN);

	void InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN);
	void ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN);
	void UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo);
	void UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime);
	void BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value);
	//! End

	void RegistCallbackFuncOnFireMode(OnFireModeCallback pFunc);

	void IncreaseEmojiCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void IncreaseCeremonyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void IncreaseSkillSucCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value);

	void IncreaseUseSkillCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseUseSignatureCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseUseQuickChatCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseUseSwitchCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void IncreaseActionGreatDefenseCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseActionLayUpGoalCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseActionDunkGoalCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void IncreaseActionAnkleBreakeCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void IncreaseIllegalScreenCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void IncreaseNotSamePositionCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetSpeedHackClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void IncreaseHackImpossibleAction(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetMemoryTamperClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void IncreasePacketTamper(DHOST_TYPE_CHARACTER_SN characterSN);


	void AddTeamIllegalScreen(DHOST_TYPE_INT32 teamIndex);
	void InitTeamIllegalScreen();
	DHOST_TYPE_BOOL	GetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BOOL	CheckTeamIllegalScreen();
	DHOST_TYPE_BOOL SetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value, DHOST_TYPE_INT32 team);

	void UseJoystick(DHOST_TYPE_CHARACTER_SN characterSN);

	// 기권하기
	void AddTeamSurrenderCheck(DHOST_TYPE_INT32 teamIndex);
	void AddTeamSurrenderGameEndTime(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_FLOAT value);
	DHOST_TYPE_FLOAT GetTeamSurrenderCheckTime(DHOST_TYPE_INT32 teamIndex);
	DHOST_TYPE_FLOAT GetTeamSurrenderGameEndTime(DHOST_TYPE_INT32 teamIndex);
	void SetTeamSurrenderCheckTime(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_FLOAT value);
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> GetTeamSurrenderCheck();
	DHOST_TYPE_BOOL GetTeamSurrenderActivation(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BOOL GetTeamSurrenderCheckVoteFail(DHOST_TYPE_INT32 teamIndex);
	void SetTeamSurrenderCheckVoteFail(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL value);

	void AddTeamSurrenderVote(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);
	DHOST_TYPE_INT32 GetTeamSurrenderVoteCount(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL value);
	void AddTeamSurrenderVoteFail(DHOST_TYPE_CHARACTER_SN characterSN);

	void  SaveSurrenderVoteInfo(DHOST_TYPE_INT32 teamIndex);

	DHOST_TYPE_FLOAT GetCharacterDisconnectTime(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetCharacterDisconnectTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);

	DHOST_TYPE_FLOAT GetCharacterDiconnectComebackTime(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetCharacterDiconnectComebackTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);

	DHOST_TYPE_UINT32 GetPlayerNumber(DHOST_TYPE_CHARACTER_SN characterSN);

	DHOST_TYPE_BALL_NUMBER GetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value);

	DHOST_TYPE_BALL_NUMBER GetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN);
	void SetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value);

	//! CharacterPassive
	DHOST_TYPE_BOOL InitialPassiveDataSet(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo);
	DHOST_TYPE_BOOL SetPacketCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassive_data* pInfo);
	SCharacterPassive* GetCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN);

	//! Potential
	DHOST_TYPE_BOOL GetPotentialInfo(DHOST_TYPE_CHARACTER_SN id, ACTION_TYPE actionType, SHOT_TYPE shotType, SKILL_INDEX skillIndex, SPotentialInfo& sInfo);
	DHOST_TYPE_BOOL GetCharacterPotentialInfo(DHOST_TYPE_CHARACTER_SN id, F4PACKET::POTENTIAL_INDEX value, SPotentialInfo& sInfo);
	DHOST_TYPE_FLOAT GetCharacterPotentialValueSum(DHOST_TYPE_CHARACTER_SN id);

	void GetTeamsCharacterVector(std::vector<std::vector<CCharacter*>>& vecInfo);
	void GetMyTeamsCharacterVector(std::vector<CCharacter*>& vecInfo, DHOST_TYPE_INT32 teamIndex);
	void GetOppenentCharacterVector(std::vector<CCharacter*>& vecInfo, DHOST_TYPE_CHARACTER_SN characterSN);

	// 박스아웃 관련 액션 체크 3가지 
	void CheckBoxOut(CCharacter* actionCharacter, F4PACKET::MOVE_MODE moveMode);
	DHOST_TYPE_BOOL CheckBoxOutedBy(CCharacter* actionCharacter, DHOST_TYPE_CHARACTER_SN &boxoutCharacter); // 박스아웃을 당했는지 
	void CheckBoxOutedClear(); // 모두 클리어 


private:
	CHost* m_pHost;
	CDataManagerBalance* m_pBalanceData;
	CAnimationController* m_pOwnerAnimationController;
	CBallController* m_pOwnerBallController;

	std::vector<std::vector<CCharacter*>> m_vecTeamsCharacter;
	int m_teamsActive[2];
	std::vector<int> m_vecTeamsWait;

	// 일리걸스크린
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL> m_TeamIllegalScreen;
	
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*> m_mapCharacter;
	DHOST_TYPE_CHARACTER_SN m_GameMVP;
	DHOST_TYPE_CHARACTER_SN	m_LastCharacterSN;

	DHOST_TYPE_BOOL m_bNoTableCharacterSend;		// 주목할만한 선수 연출 (false : 보낼수있음, true : 이미 보냄) 경기당 한번만 보낼 수 있음!!

	std::vector<MatchInfo> m_vecMatchInfos;
	int m_SwitchIDHome;
	int	m_SwitchIDAway;

	std::vector<DHOST_TYPE_CHARACTER_SN> m_vecSendOnFireModeCharacter;
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> m_mapSendOnFireModeCharacterReservation;

	BadMannerCallback m_CallbackBadManner;
	OnFireModeCallback m_CallbackOnFireMode;
	CMannerManager* m_pMannerManager;

	// 기권하기
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> m_mapTeamSurrenderCheck;			// 해당 팀에 기권이 활성화됐었는지
	std::multimap<DHOST_TYPE_INT32, SSurrenderVoteInfo> m_mapTeamSurrenderVoteInfo;		// 기권 투표	(찬성한 캐릭터만 저장)
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL> m_mapTeamSurrenderVoteFail;			// 기권하기 투표 실패 (시간이 만료되었거나, 반대가 2표 이상일 때)
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> m_mapTeamSurrenderGameEndTime;		// 기권하기 후 종료시간 설정

public:
	void EndValidatePassiveAll();
};

