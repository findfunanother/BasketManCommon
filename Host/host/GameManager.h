#pragma once
#include "F4Packet_generated.h"
#include "HostDefine.h"
class CHost;

class CGameManager	//! 게임과 관련된 정보들을 관리하는 클래스 (게임모드, 맵, 백보드, 스코어, 게임 환경 오브젝트(개, 돼지) 등)
{
public:

	CGameManager(CHost* pHost, int option);
	~CGameManager();

	void UpdateGameManager(DHOST_TYPE_FLOAT timeDelta);

	DHOST_TYPE_INT32 GetOption() { return m_Option; }
	void SetOption(DHOST_TYPE_UINT64 value) { m_Option |= value; }
	DHOST_TYPE_BOOL CheckOption(DHOST_TYPE_UINT64 flag);

	DHOST_TYPE_STAGE_ID GetStageID() { return m_StageID; }
	F4PACKET::EGAME_MODE GetConvertGameMode();
	void SetStageID(DHOST_TYPE_STAGE_ID value) { m_StageID = value; }

	EEXIT_TYPE GetExitType() { return m_ExitType; }
	F4PACKET::EXIT_TYPE GetConvertExitType();
	void SetExitType(EEXIT_TYPE value) { m_ExitType = value; }

	F4PACKET::BB_STATE GetBackBoardState() { return m_BackBoardState; }
	void SetBackBoardState(F4PACKET::BB_STATE value) { m_BackBoardState = value; }

	void CheckBackBoardState(DHOST_TYPE_FLOAT value);

	EMODE_TYPE GetModeType() { return m_ModeType; }
	void SetModeType(EMODE_TYPE value) { m_ModeType = value; }

	DHOST_TYPE_BOOL GetIsNormalGame() { return m_isNormalGame; }
	void SetIsNormalGame(DHOST_TYPE_BOOL value) { m_isNormalGame = value; }

	void SetDebug(DHOST_TYPE_BOOL value) { m_bIsDebug = value; }
	DHOST_TYPE_BOOL GetDebug() { return m_bIsDebug; }

	void SetNoTieMode(DHOST_TYPE_BOOL value) { m_bNoTieMode = value; }
	DHOST_TYPE_BOOL GetNoTieMode() { return m_bNoTieMode; }

	DHOST_TYPE_BOOL CheckSingleMode();
	DHOST_TYPE_BOOL CheckSkillChallengeMode();

	DHOST_TYPE_INT32 GetScore(DHOST_TYPE_INT32 teamIndex);
	void SetScore(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_INT32 value);
	DHOST_TYPE_FLOAT GetScoredTime() { return m_ScoredTime; }
	void SetScoredTime(DHOST_TYPE_FLOAT value) { m_ScoredTime = value; }

	DHOST_TYPE_BOOL GetLoadComplete() { return m_bLoadComplete; }
	void SetLoadComplete(DHOST_TYPE_BOOL value) { m_bLoadComplete = value; }

	DHOST_TYPE_FLOAT GetReplayPoint() { return m_fReplayPoint; }
	void SetReplayPoint(DHOST_TYPE_FLOAT value) { m_fReplayPoint = value; }
	DHOST_TYPE_BOOL GetReplayAssistCheck() { return m_bReplayAssistCheck; }
	void SetReplayAssistCheck(DHOST_TYPE_BOOL value) { m_bReplayAssistCheck = value; }

	DHOST_TYPE_INT32 GetOffenseTeam() { return m_OffenseTeam; }
	void SetOffenseTeam(DHOST_TYPE_INT32  value) { m_OffenseTeam = value; }
	DHOST_TYPE_INT32 GetWinnerTeam() { return m_WinnerTeam; }
	void SetWinnerTeam();

	DHOST_TYPE_INT32 GetAbNormalEndUserTeam() { return m_AbNormalEndUserTeam; }
	void SetAbNormalEndUserTeam(DHOST_TYPE_INT32 team) { m_AbNormalEndUserTeam = team; }
	DHOST_TYPE_BOOL GetAbnormalExpireRoom() { return m_bAbnormalExpireRoom; }
	void SetAbnormalExpireRoom(DHOST_TYPE_BOOL value) { m_bAbnormalExpireRoom = value; }

	DHOST_TYPE_BOOL GetRedisLoadValue() { return m_bRedisLoadValue; }
	void SetRedisLoadValue(DHOST_TYPE_BOOL value, DHOST_TYPE_FLOAT time);

	DHOST_TYPE_FLOAT GetRedisLoadRoomElapsedTime() { return m_fRedisLoadRoomElapsedTime; }
	void SetRedisLoadRoomElapsedTime(DHOST_TYPE_FLOAT value) { m_fRedisLoadRoomElapsedTime = value; }

	DHOST_TYPE_BOOL GetRedisSaveMatchResult() { return m_bRedisSaveMatchResult; }
	void SetRedisSaveMatchResult(DHOST_TYPE_BOOL value) { m_bRedisSaveMatchResult = value; }

	std::vector<DHOST_TYPE_INT32>& GetVecScores() { return m_vecScores; }

	DHOST_TYPE_FLOAT GetSpeedHackRestrictionStepOne() { return m_SpeedHackRestrictionStepOne; }
	void SetSpeedHackRestrictionStepOne(DHOST_TYPE_FLOAT value) { m_SpeedHackRestrictionStepOne = value; }
	DHOST_TYPE_FLOAT GetSpeedHackRestrictionStepTwo() { return m_SpeedHackRestrictionStepTwo; }
	void SetSpeedHackRestrictionStepTwo(DHOST_TYPE_FLOAT value) { m_SpeedHackRestrictionStepTwo = value; }
	DHOST_TYPE_UINT32 GetSwitchToAiLevel() { return m_SwitchToAiLevel; }
	void SetSwitchToAiLevel(DHOST_TYPE_UINT32 value) { m_SwitchToAiLevel = value; }

	DHOST_TYPE_BOOL GetKickReasonSaveMode();

	DHOST_TYPE_FLOAT GetJumpBallStartTime() { return m_JumpBallStartTime; }
	void SetJumpBallStartTime(DHOST_TYPE_FLOAT value) { m_JumpBallStartTime = value; }

	DHOST_TYPE_BOOL GetLogShotRate() { return m_bLogShotRate; }
	void SetLogShotRate(DHOST_TYPE_BOOL value) { m_bLogShotRate = value; }

	DHOST_TYPE_INT32 GetSceneIndex() { return m_SceneIndex; }
	void SetSceneIndex(DHOST_TYPE_INT32 value) { m_SceneIndex = value; }

	DHOST_TYPE_BOOL GetBuzzerBeater() { return m_bBuzzerBeater; }
	void SetBuzzerBeater(DHOST_TYPE_BOOL value) { m_bBuzzerBeater = value; }

	DHOST_TYPE_FLOAT GetShotRoomElapsedTime() { return m_fShotRoomElapsedTime; }
	void SetShotRoomElapsedTime(DHOST_TYPE_FLOAT value) { m_fShotRoomElapsedTime = value; }

	DHOST_TYPE_INT32 GetOverTimeCount() { return m_OverTimeCount; }
	void IncreaseOverTimeCount() { ++m_OverTimeCount; }
	
	CHost* GetHost() { return m_pHost; }

	DHOST_TYPE_FLOAT GetRescue1ScoreExtend();
	DHOST_TYPE_FLOAT GetRescue2ScoreExtend();

	void SetHandicapRulePoint(DHOST_TYPE_INT32 nSeticapRulePoint) { m_nHandicapRulePoint = nSeticapRulePoint;  }
	DHOST_TYPE_INT32 GetHandicapRulePoint() { return m_nHandicapRulePoint; }

	//F4PACKET::MinigameCustomInfo GetMinigameCustomInfo() { return m_MinigameCustom; }
	//void SetMinigameCustomInfo(F4PACKET::MinigameCustomInfo info) { m_MinigameCustom = info; }

	F4PACKET::MinigameCustomInfoT* GetMinigameCustomInfo() { return m_pMinigameCustom; }
	void SetMinigameCustom(F4PACKET::MinigameCustomInfoT* minigameCustom) { m_pMinigameCustom = minigameCustom; }

private:
	CHost*			 m_pHost;

	F4PACKET::MinigameCustomInfoT*  m_pMinigameCustom;

	DHOST_TYPE_INT32 m_nHandicapRulePoint;

	DHOST_TYPE_INT32 m_Option;	// 호스트 생성시 옵션 정보 (모드, 시간등을 구별할 수 있다)

	DHOST_TYPE_STAGE_ID m_StageID;	// 맵 정보

	EEXIT_TYPE m_ExitType;	// 클라가 나갈 때 어디로 나갈지에 대한 정보

	F4PACKET::BB_STATE m_BackBoardState;	// 백보드 정보
	DHOST_TYPE_FLOAT m_fBackBoardHP;	// 백보드 체력

	EMODE_TYPE m_ModeType;	// 게임 모드
	DHOST_TYPE_BOOL m_isNormalGame;

	DHOST_TYPE_FLOAT m_ScoredTime;	// 득점한 시간
	vector<DHOST_TYPE_INT32> m_vecScores;
	
	DHOST_TYPE_BOOL m_bLoadComplete;	// 게임로딩이 다됐는지 확인

	DHOST_TYPE_FLOAT m_fReplayPoint;	// 리플레이 포인트
	DHOST_TYPE_BOOL m_bReplayAssistCheck;

	DHOST_TYPE_INT32 m_OffenseTeam;	// 공격팀
	DHOST_TYPE_INT32 m_WinnerTeam;	// 승리팀

	DHOST_TYPE_INT32 m_AbNormalEndUserTeam;	// 비정상 종료한 유저가 있는 팀
	DHOST_TYPE_BOOL m_bAbnormalExpireRoom;	// 비정상 종료됐는지 여부
	
	DHOST_TYPE_BOOL m_bRedisLoadValue;	// 레디스 로딩이 됐는지 여부
	DHOST_TYPE_FLOAT m_fRedisLoadRoomElapsedTime;	// 레디스 로딩 시점
	DHOST_TYPE_BOOL m_bRedisSaveMatchResult;	// 레디스에 경기결과 저장 요청했는지 여부

	DHOST_TYPE_FLOAT m_SpeedHackRestrictionStepOne;	// 스피드핵 1단계
	DHOST_TYPE_FLOAT m_SpeedHackRestrictionStepTwo;	// 스피드핵 2단계
	DHOST_TYPE_UINT32 m_SwitchToAiLevel;	// 킥당한 유저의 셋팅해야할 ai 레벨

	DHOST_TYPE_FLOAT m_JumpBallStartTime;	// 최초 점프볼한 시간

	DHOST_TYPE_BOOL m_bLogShotRate;

	DHOST_TYPE_INT32 m_SceneIndex;

	DHOST_TYPE_BOOL m_bBuzzerBeater;	// 버저비터 체크

	DHOST_TYPE_FLOAT m_fShotRoomElapsedTime;	// 슛 타임

	DHOST_TYPE_INT32 m_OverTimeCount;	// 연장전 횟수

	DHOST_TYPE_BOOL  m_bIsDebug;   // 디버그 치트키를 쓸 수 있는지 

	DHOST_TYPE_BOOL  m_bNoTieMode; // 
};

