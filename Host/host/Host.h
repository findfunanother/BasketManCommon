#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable: 4996) 
#else
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wswitch"
#endif

#ifdef _WIN32
#include <Windows.h>
#include <thread>
#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#endif

#include <list>
#include <functional>
#include <string>
#include <deque>

#include "PacketBase.h"
#include "FlatBufProtocol.h"
#include "CharacterManager.h"
#include "HostUserInfo.h"
#include "Util.h"
#include "ShotSolution.h"
#include "EnvironmentObject.h"
#include "FileLog.h"
#include "HeaderDefine.h"
#include "GameManager.h"
#include "TimeManager.h"
#include "DevManager.h"


typedef struct _HostMessage
{
	int userID;
	int size;
	void* data;
	const void* peer;
}HostMessage;

typedef std::function<void(HostMessage*)> HostMessageCallback;
typedef std::function<void(const char*, int, int)> ProcessPacketCallbackToCpp;
typedef std::function<void(SRedisMatchResult*)> RedisCallback;
typedef std::function<void(const char*)> LogCallback;
typedef std::function<float(const char*)> BalanceValueCallback;
typedef std::function<void(uint32_t, uint32_t)> RedisSendCallback;


class CState;
class CHostUserInfo;
class CPhysicsHandler;
class CBallController;
class CEnvironmentObject;
class CCharacterManager;
class CAnimationController;
class CDataManagerBalance;
class CDataManagerShotSolution;
class CVerifyManager;
struct SCharacterAction;

using HostStateMapType = std::map<EHOST_STATE, CState*>;
using HostUserMapType = std::map<DHOST_TYPE_USER_ID, CHostUserInfo*>;
using HostBadConnectUserListType = std::list<DHOST_TYPE_USER_ID>;
using HostCandidateUserVecType = std::vector<DHOST_TYPE_USER_ID>;		// ai ��Ʈ���� ������ �� �ִ� ���� �ĺ��ڵ�
//DLLEXPORT template class DECLSPEC std::vector<DHOST_TYPE_USER_ID>;
using HostCallbackEnetDisConnectUserVecType = std::vector<DHOST_TYPE_USER_ID>;

using namespace std;

enum class SYNCINFO_TYPE
{
	NONE = 0,

	PICK = 1,
	REBOUND = 2,
	READY = 3,
	PICK_DONE = 4,
	REBOUND_DONE = 5,

	BOXOUT_FLAG_ON = 6,
	ENHANCED_BOXOUT_FLAG_ON = 7,

	BOXOUT_FLAG_OFF = 8,
	ENHANCED_BOXOUT_FLAG_OFF = 9,
};

enum class LOG_TYPE
{
	LOG_SPEEDHACK = 0,
	LOG_POTENTIAL = 1,
};

#define HOST_VERSIONINFO 251118 // Ŭ��� ������ ����üũ ��ȣ ( ���� svn Ŀ�Թ�ȣ ) 

class DECLSPEC CHost
{
	typedef std::deque<SBallActionInfo>		BallActionInfoDeque;

private:
	std::array<uint8_t, 32> m_serverBalanceDataHash;
	std::array<uint8_t, 32> m_serverAnimDataHash;

public:

	CHost(char* hostID, int option);
	~CHost();

	void									Initialize();
	void									Release();

	DHOST_TYPE_HOST_ID						GetHostID();

	//! Packet
	void									Update(float timeDelta);
	void									ProcessPacketWaterMarkOnly(const char* pData, DHOST_TYPE_INT32 size, void* peer);
	void									ProcessPacket(const char* pData, DHOST_TYPE_INT32 size, void* peer);
	void									ProcessPacketWithMagic(const char* pData, DHOST_TYPE_INT32 size, void* peer);
	DHOST_TYPE_INT32						SendPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_INT32						BroadcastPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID = kUSER_ID_INIT);
	//! End

	//! CallBackFunction
	void									RegistCallbackFunc(ProcessPacketCallbackToCpp pFunc);
	void									RegistCallbackFuncRedis(RedisCallback pFunc);
	void									RegistCallbackFuncLog(LogCallback pFunc);
	void									RegistCallbackFuncSendPacket(HostMessageCallback pFunc);
	void									RegistCallbackFuncRedisSend(RedisSendCallback pFunc);
	//! End

	//! �������� ����
	void									OnHostJoin(DHOST_TYPE_INT32 UserID, int spectator = 0);
	void									OnHostJoinForce(DHOST_TYPE_INT32 UserID);
	void									OnHostConnect(DHOST_TYPE_INT32 UserID, int spectator = 0);
	void									OnHostDisconnect(DHOST_TYPE_INT32 UserID, int spectator = 0);
	void									OnHostQuit(DHOST_TYPE_INT32 UserID);
	void									OnHostExit();
	void									OnGiveGift(SGiveGiftInfo * pGiftInfo);
	void									OnPacketGuardRule(SPacketGuardRule* pGiftInfo);
	//! End

	//! ������� ����
	DHOST_TYPE_BOOL							CheckObserverUser(DHOST_TYPE_INT32 UserID);
	void									OnPacketObserverUser(CPacketBase* pPacket, void* peer, const DHOST_TYPE_FLOAT& recv_time);
	void									RemoveObserverUser(DHOST_TYPE_INT32 UserID);
	//! End

	void									RedisSaveMatchResult(int score = 0);
	void									OnRedisMatchInfo(SRedisMatchInfo* pData);
	void									RedisKickReasonSend(DHOST_TYPE_USER_ID UserID, ECHARACTER_KICK_TYPE KickType);

	void									ToLog(const char* msg);
	void									ToLog(const char* msg, LOG_TYPE logType);
	void									ToLogInvalidAnimation(F4PACKET::SPlayerInformationT* pInfo, DHOST_TYPE_INT32 AniIndex, DHOST_TYPE_INT32 Result, F4PACKET::PACKET_ID PacketID);
	void									BadMannerNotice(uint32_t CharacterSN, uint16_t BadMannerType, uint16_t Count);

	void									CheckGameProcessWithBadConnectUser();

	//! DataVerify
	void									GenerateKey();
	void									GenerateKeyStr();
	DHOST_TYPE_INT32						GetKey();
	DHOST_TYPE_STR							GetKeyStr();
	DHOST_TYPE_BOOL							VerifyHMAC(DHOST_TYPE_INT32 value, const flatbuffers::Vector<int8_t>* hmac);
	void									GenerateHMAC(DHOST_TYPE_INT32 value, std::vector<int8_t>& hmac);
	void									GenerateHMAC(string& data, string& hmac);
	void									VerifyAction(const SPlayerAction* pInfo, string& str);
	DHOST_TYPE_INT32						VerifyVersionPacket(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid = kUSER_ID_INIT);

	bool									BuildDigestFromMemory(const uint8_t* data, size_t length, std::array<uint8_t, 32>& out);
	bool									CompareWithClientHash(const uint8_t clientHash[32], uint32_t clientSize, std::array<uint8_t, 32> severSha256, uint32_t serverSize);

	void									ForceActionStand(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid, EFORCE_ACTION_STAND_TYPE type);
	void									ForceActionStandExtend(DHOST_TYPE_INT32 packetID, void* pData, CCharacter* pCharacter, DHOST_TYPE_FLOAT recvTime, DHOST_TYPE_BOOL noraml); // ���� ���� �� 
	void									ForceActionModifyPositionOfServer(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_USER_ID userid, DHOST_TYPE_FLOAT recvTime);
	void									GetClientHMAC(const flatbuffers::Vector<int8_t>* hmac, string& str);
	template<typename T>
	DHOST_TYPE_INT32						GetTypeConvert(T value);
	//!

	//! ActionVerify
	DHOST_TYPE_BOOL							CheckShotTypeRimAttack(SHOT_TYPE value);
	DHOST_TYPE_BOOL							CheckHaveSkill(DHOST_TYPE_CHARACTER_SN id, F4PACKET::SKILL_INDEX value);
	DHOST_TYPE_FLOAT						GetSkillLevel(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value);
	DHOST_TYPE_FLOAT						GetRandomValueAlgorithm(DHOST_TYPE_USER_ID userid, DHOST_TYPE_BOOL xorShift);
	EACTION_VERIFY							VerifyPotentialFxLevel(DHOST_TYPE_USER_ID userid, DHOST_TYPE_CHARACTER_SN id, DHOST_TYPE_INT32 fxlevel, POTENTIAL_INDEX value);
	DHOST_TYPE_BOOL							CheckPositionIsNan(const TB::SVector3* position);

	//!

protected:

	DHOST_TYPE_HOST_ID						m_HostID;
	char*									m_PacketDataBuffer;
	DHOST_TYPE_INT32						m_Key;
	DHOST_TYPE_STR							m_KeyStr;

	HostMessageCallback						m_CallbackBroadcast;
	ProcessPacketCallbackToCpp				m_CallbackCpp;
	RedisCallback							m_CallbackRedis;
	LogCallback								m_CallbackLog;
	RedisSendCallback						m_CallbackRedisSend;

	CVerifyManager*							m_VerifyManager;

	SPacketGuardRule						m_PacketGuardRule;

	//! CharacterManager
public:
	void									SetPacketGuardRule(const SPacketGuardRule packetGuardRule) { m_PacketGuardRule = packetGuardRule; }
	const SPacketGuardRule & 				GetPacketGuardRule() const { return m_PacketGuardRule; }

	CCharacterManager*						GetCharacterManager();
	CVerifyManager*							GetVerifyManager();
	void									AddLastCharacterSN();
	DHOST_TYPE_CHARACTER_SN					GetLastCharacterSN();
	void									SortCharacterInformation();
	void									SortCharacterInformationWithRole();
	void									MixCharacterTeam();
	F4PACKET::SPlayerInformationT*			FindCharacterInformation(DHOST_TYPE_CHARACTER_SN characterSN);
	void									GetCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);
	void									GetHomeCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);
	void									GetAwayCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec);
	const TB::SVector3						GetCharacterPosition(DHOST_TYPE_CHARACTER_SN characterSN);
	F4PACKET::SPlayerAction*				GetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN);
	F4PACKET::SPlayerAction*				GetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN);
	F4PACKET::SHOT_TYPE						GetCharacterActionShotType(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BALL_NUMBER					GetCharacterActionPotentialBloom(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_INT32						GetCharacterTeam(DHOST_TYPE_CHARACTER_SN characterSN);
	SCharacterAction*						GetCharacterActionInfo(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType, const F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType = SHOT_TYPE::shotType_none, DHOST_TYPE_BALL_NUMBER potentialBloom = kBALL_NUMBER_INIT);
	void									SetCharacterActionOverlap(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value);
	void									SetCharacterActionOverlapDestYaw(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw);
	void									AddCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN);
	// ���� ���� 
	void									GenerateCharacterBuff(DHOST_TYPE_CHARACTER_SN characterSN, SCharacterBuff buff);

	void 									AddServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN, ServerCharacterPosition position);
	std::deque<ServerCharacterPosition>		GetServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN);
	std::deque<SCharacterAction>			GetCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_CHARACTER_SN					GetCharacterSN(DHOST_TYPE_INT32 indexTeam, DHOST_TYPE_INT32 indexCharacter);
	size_t									GetCharacterTeamSize(DHOST_TYPE_INT32 teamIndex);
	DHOST_TYPE_INT32						GetIndexOpponentTeam(DHOST_TYPE_INT32 teamIndex);
	void									SetTeamsActive(DHOST_TYPE_INT32 team1, DHOST_TYPE_INT32 team2);
	DHOST_TYPE_INT32						GetIndexTeamWait();
	void									SetCharacterReadyPosition(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, TB::SVector3 position);
	void									SetCharacterMoveLock(DHOST_TYPE_CHARACTER_SN characterSN, EMOVEMENT_RESTRICTION value);
	void									SetCharacterMoveUnLock();
	DHOST_TYPE_UINT32						GetTeamRoleCount(DHOST_TYPE_INT32 team, F4PACKET::ROLE role);
	void									CalcFocusPoint();
	DHOST_TYPE_CHARACTER_SN					GetMaxFocusPointCharacterInTeam(DHOST_TYPE_INT32 team);
	DHOST_TYPE_CHARACTER_SN					GetMaxFocusPointCharacterInEnemyTeam(DHOST_TYPE_INT32 Scoreteam);
	void									PushCharacterRecord(SRedisMatchResult* pInfo);
	F4PACKET::SPlayerInformationT*			FindCharacterInformationWithUserID(DHOST_TYPE_USER_ID UserID);
	F4PACKET::SPlayerInformationT*			FindCharacterInformationWithUserIDAndIsNotAi(DHOST_TYPE_USER_ID UserID);
	void									AddCharacterShootInfo(DHOST_TYPE_CHARACTER_SN characterSN, float x, float z, uint32_t zone, bool goal);
	void									GetCharacterRecordVec(std::vector<SCharacterRecord*>& vec);
	F4PACKET::RECORD_TYPE					ConvertToProtobufRecordType(ECHARACTER_RECORD_TYPE type);
	DHOST_TYPE_STR							ConvertToStringTeam(DHOST_TYPE_INT32 team);
	DHOST_TYPE_USER_ID						FindUserIDWithCharacterSN(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_INT32						FindUserTeam(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_CHARACTER_SN					UpdateRecord(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL assist = false);
	void									UpdateRecordBasicTrainingSystem(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber, F4PACKET::SKILL_INDEX skill, F4PACKET::SHOT_TYPE shotType = F4PACKET::SHOT_TYPE::shotType_none);
	DHOST_TYPE_UINT16						GetCharacterRecordCountWithType(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::RECORD_TYPE Type);
	DHOST_TYPE_CHARACTER_SN					CheckAssist(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber);
	DHOST_TYPE_BOOL							CheckAssistAtTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER ballNumber, DHOST_TYPE_GAME_TIME_F gameTime);
	DHOST_TYPE_BOOL							CheckIfPrevBallIsPass(DHOST_TYPE_BALL_NUMBER ballNumber);
	DHOST_TYPE_BOOL							GetMostCharacterRerecordWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type, DHOST_TYPE_BOOL allowEqual);
	void									SetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time);

	//! Flatbuffer
	F4PACKET::ROLE							ConvertToCharacterRole(ECHARACTER_ROLE value);
	//!

	DHOST_TYPE_UINT32						GetSpeedHackLevel(DHOST_TYPE_USER_ID UserID);
	void									SetSpeedHackLevel(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_UINT32 value);

	DHOST_TYPE_FLOAT						GetSpeedHackTimeAccumulate(DHOST_TYPE_USER_ID UserID);
	void									SetSpeedHackTimeAccumulate(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value);

	DHOST_TYPE_BOOL							CheckSpeedHackBothCase(DHOST_TYPE_USER_ID UserID, 
																   DHOST_TYPE_USER_ID CharID, 
																   DHOST_TYPE_FLOAT ServerElapsedTime, 
		                                                           DHOST_TYPE_FLOAT ClientElapsedTime, 
																   TB::SVector3 position,
																   DHOST_TYPE_FLOAT speed, 
																   TB::SVector3& rePosition, 
																   DHOST_TYPE_BOOL& normal);

	ECHARACTER_KICK_TYPE					GetCharacterKickType(DHOST_TYPE_USER_ID UserID);
	void									SetCharacterKickType(DHOST_TYPE_USER_ID UserID, ECHARACTER_KICK_TYPE value);

	void									IncreaseLatencyCount(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value);

	void									SetAvgPingLatency(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT value);

	//! MVP
	void									CalcMVPScore();
	DHOST_TYPE_CHARACTER_SN					GetMVP();
	void									SetMVP(DHOST_TYPE_CHARACTER_SN value);

	//! �ָ��� ����
	DHOST_TYPE_BOOL							GetNoTableCharacterSend();
	void									SetNoTableCharacterSend(DHOST_TYPE_BOOL value);
	DHOST_TYPE_CHARACTER_SN					CheckNoTableCharacterSoloCondition(DHOST_TYPE_CHARACTER_SN ScorerCharacterSN);
	DHOST_TYPE_CHARACTER_SN					CheckNoTableCharacterDuoCondition(DHOST_TYPE_CHARACTER_SN enemyTeamMaxFocusCharacterSN);

	void									PushMatchInfo(int id, int target);
	void									ModifyMatchInfo(int id, int target);
	int										GetMatchInfoID(int target);
	int										GetMatchInfoTarget(int id);
	int										GetSwitchID(DHOST_TYPE_INT32 team);
	void									SetSwitchID(DHOST_TYPE_INT32 team, int id);

	void									OnCharacterMessage(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, DHOST_TYPE_GAME_TIME_F gameTime, DHOST_TYPE_UINT32 keys, F4PACKET::MOVE_MODE moveMode = F4PACKET::MOVE_MODE::normal);
	DHOST_TYPE_BOOL							GetSecondAnimationReceivePacket(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BOOL							GetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);

	void									CheckCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType, std::vector<DHOST_TYPE_INT32>& teamScores);
	DHOST_TYPE_UINT64						GetBurstActionTypeWithRecordType(ECHARACTER_RECORD_TYPE recordType);
	DHOST_TYPE_BOOL							CheckCharacterActionPenetrateType(F4PACKET::ACTION_TYPE type);
	DHOST_TYPE_FLOAT						GetCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN);
	EBURST_ON_FIRE_MODE_STEP				GetCharacterBurstOnFireModeStep(DHOST_TYPE_CHARACTER_SN characterSN);
	void									UpdateCharacterBurstGaugeWithPassiveActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest);
	void									UpdateCharacterBurstGaugeWithMedalActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest);

	void									UpdateCharacterOnFireModeStatBuff(DHOST_TYPE_CHARACTER_SN characterSN);

	DHOST_TYPE_BOOL							GetTeamCharacterMap(DHOST_TYPE_INT32 teamIndex);

	std::vector<MatchInfo>&					GetMatchInfo();

	DHOST_TYPE_BOOL							GetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_BOOL							SetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);

	//! 20230508 ��ų� ���� - by thinkingpig
	void									InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN = kCHARACTER_SN_INIT);
	void									ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN);
	void									UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo);
	void									BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value);
	void									UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime);
	//! End

	void									IncreaseEmojiCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void									IncreaseCeremonyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void									IncreaseSkillSucCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value);

	void									IncreaseUseSkillCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseUseSignatureCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseUseQuickChatCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseUseSwitchCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void									IncreaseActionGreatDefenseCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseActionLayUpGoalCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseActionDunkGoalCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									IncreaseActionAnkleBreakeCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void									IncreaseIllegalScreenCount(DHOST_TYPE_CHARACTER_SN characterSN);

	void									IncreaseNotSamePositionCount(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetSpeedHackClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void									IncreaseHackImpossibleAction(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetMemoryTamperClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value);
	void									IncreasePacketTamper(DHOST_TYPE_CHARACTER_SN characterSN);
	
	void									CheckQuestAction(const SPlayerAction* pInfo);

	void									UseJoystick(DHOST_TYPE_CHARACTER_SN characterSN);

	void									CheckTeamSurrenderActivation(DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT gameTime);
	void									SendSurrenderActivationInfo(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL activation);
	void									AddTeamSurrenderVote(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);
	DHOST_TYPE_BOOL							GetTeamSurrenderActivation(DHOST_TYPE_CHARACTER_SN characterSN);
	DHOST_TYPE_INT32						GetTeamSurrenderVoteCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value);
	void									AddTeamSurrenderGameEndTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value);
	DHOST_TYPE_UINT16						GetObserverUserCount();
	void									SendObserverUserCountInfo();
	
	//! ĳ���� ��ġ ����ȭ
	void									CheckCharacterPositionSync(const F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_INT32 packetID);
	//! 

	DHOST_TYPE_BALL_NUMBER					GetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value = kBALL_NUMBER_INIT);

	DHOST_TYPE_BALL_NUMBER					GetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN);
	void									SetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value = kBALL_NUMBER_INIT);

	DHOST_TYPE_BOOL							CheckDoYouHaveMedal(SPlayerInformationT* pInfo, std::vector<F4PACKET::MEDAL_INDEX>& vInfo);
	DHOST_TYPE_INT32						CheckDoYouHaveMedal(SPlayerInformationT* pInfo, F4PACKET::MEDAL_INDEX value);
	DHOST_TYPE_BOOL							CheckDoYouHaveSignature(SPlayerInformationT* pInfo, DHOST_TYPE_INT32 value);
	DHOST_TYPE_BOOL							CheckNicePassBonus(F4PACKET::SBallShotT* pBallShot, SPlayerInformationT* pInfo);

	//! CharacterPassive
	DHOST_TYPE_BOOL 						SetPacketCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassive_data* pInfo);
	DHOST_TYPE_BOOL 						InitialPassiveDataSet(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo);

	//! Postential
	DHOST_TYPE_BOOL							GetPotentialInfo(DHOST_TYPE_CHARACTER_SN id, ACTION_TYPE actionType, SHOT_TYPE shotType, SKILL_INDEX skillIndex, SPotentialInfo& sInfo);
	DHOST_TYPE_BOOL							GetCharacterPotentialInfo(DHOST_TYPE_CHARACTER_SN id, F4PACKET::POTENTIAL_INDEX value, SPotentialInfo& sInfo);
	DHOST_TYPE_FLOAT						GetCharacterPotentialValueSum(DHOST_TYPE_CHARACTER_SN id);

protected:
	CCharacterManager* m_CharacterManager;
	//! End CharacterManager


	// User
protected:
	HostUserMapType							m_HostUserMap;
	HostBadConnectUserListType				m_ListBadConnectUser;			// ���� �ҷ��� ����
	HostBadConnectUserListType				m_ListBadConnectUserCheck;		// ���� �ҷ��� ������ ó���Ǿ����� Ȯ�� �뵵
	HostCandidateUserVecType				m_VecAiHostCandidateUser;		// Ai ��Ʈ���� ������ �ĺ� ������
	HostCallbackEnetDisConnectUserVecType	m_VecCallbackEnetDisConnectUser;
	std::vector<DHOST_TYPE_USER_ID>			m_LoginCompleteUser;			// �α��� ��û ���� ��
	std::vector<DHOST_TYPE_USER_ID>			m_SendLoadCompleteUser;			// �ε� ���ø��� ������ ���� ����
	std::vector<DHOST_TYPE_USER_ID>			m_ReConnectUser;				// ������ ��û
	DHOST_TYPE_INT32						m_GameCap;
	DHOST_TYPE_FLOAT						m_AliveCheckTime;				// ���� ��üũ ����
	DHOST_TYPE_INT32						m_AliveCheckIndex;
	DHOST_TYPE_FLOAT						m_AlivePingMaxCnt;				// Max
	std::vector<DHOST_TYPE_USER_ID>			m_HostJoinUser;

public:  
	DHOST_TYPE_FLOAT                        GetAlivePingMaxCnt() { return m_AlivePingMaxCnt; }
	void									UserAdd(DHOST_TYPE_USER_ID userid, void* peer);
	int										UserCount();
	CHostUserInfo*							UserFind(DHOST_TYPE_INT32 UserID);
	DHOST_TYPE_USER_ID						UserGetID(void* peer);
	HostUserMapType&						GetHostUserMap();
	CHostUserInfo*							FindUser(DHOST_TYPE_INT32 UserID);
	CHostUserInfo*							FindUserWithConnectState(ECONNECT_STATE value);
	//! 20230728 ���� ���μ��� ���� - by thinkingpig
	CHostUserInfo*							CreateUser(DHOST_TYPE_INT32 UserID, EUSER_TYPE UserType = EUSER_TYPE::NORMAL);

	void									SetGameCap(DHOST_TYPE_INT32 gameCap);
	DHOST_TYPE_INT32						GetGameCap();
	void									BadConnectProcess(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_BOOL ByEnet = false);
	void									PushBadConnectUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							CheckBadConnectUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_UINT64						GetBadConnectUserSize();
	void									GetBadConnectUser(std::list<DHOST_TYPE_USER_ID>& vData);
	DHOST_TYPE_STR							ConvertToStringUserType(EUSER_TYPE value);
	void									PushAiHostCandidateUser(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN, DHOST_TYPE_UINT32 AiLevel = kUINT32_INIT);
	void									PushCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID);
	void									RemoveCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							CheckCallbackEnetDisConnectUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							CheckUserPingAvg(CHostUserInfo* pUser);
	DHOST_TYPE_BOOL							GetUserConnectState(CHostUserInfo* pUser);
	void									CheckAiBeingControlled();
	DHOST_TYPE_USER_ID						FindUserWithLeastAiControl();
	DHOST_TYPE_USER_ID						FindAiControlUser(DHOST_TYPE_CHARACTER_SN CharacterSN);
	void									RemoveBadConnectUser(DHOST_TYPE_USER_ID UserID);
	void									RemoveBadConnectUserCheck(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							FindBadConnectUserCheckList(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							CheckBadConnectUserCheckList();
	void									SendDelegateControlToUser();
	void									SendOnFireModeCharacterInfo();
	DHOST_TYPE_BOOL							CheckDisconnectUser(DHOST_TYPE_USER_ID UserID);
	void									SendReconnectUserWithGameInfo(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN);
	void									SendReconnectUserWithPassiveInfo(DHOST_TYPE_USER_ID UserID);
	void									SendGameScore();

	void									ReconnectUserOperation();
	void									ReconnectUserOperationBeforeLoading(DHOST_TYPE_USER_ID UserID);
	void									ReconnectUserOperationProcess(DHOST_TYPE_USER_ID UserID);
	void									ReconnectUserOperationChangeOfControl(DHOST_TYPE_USER_ID UserID);

	void									KickToTheLobby(DHOST_TYPE_USER_ID UserID, F4PACKET::EKICK_TYPE type = F4PACKET::EKICK_TYPE::none);
	void									AwayFromKeyboard(DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN CharacterSN);

	std::vector<DHOST_TYPE_USER_ID>&        GetLoginCompleteUser() { return m_LoginCompleteUser; }
	void									PushLoginCompleteUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_UINT64						GetLoginCompleteUserSize();
	void									RemoveLoginCompleteUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							CheckLoginCompleteUser(DHOST_TYPE_USER_ID UserID);

	void									PushSendLoadCompleteUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_UINT64						GetSendLoadCompleteUserSize();
	void									CheckSendLoadCompleteUser();
	DHOST_TYPE_BOOL							IsLoadCompleteUser(DHOST_TYPE_USER_ID UserID);
	void									ClearSendLoadCompleteUser();
	void									RemoveLoadCompleteUser(DHOST_TYPE_USER_ID UserID);

	void									PushReConnectUser(DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_BOOL							FindReConnectUser(DHOST_TYPE_USER_ID UserID);
	void									RemoveReConnectUser(DHOST_TYPE_USER_ID UserID);

	void									SendClientAliveCheck();
	void									SendSystemTimeCheck(CHostUserInfo* pUser);

	void									SendSyncInfo(SYNCINFO_TYPE type = SYNCINFO_TYPE::NONE);
	void									SendSyncInfo(SYNCINFO_TYPE type, DHOST_TYPE_CHARACTER_SN sn, DHOST_TYPE_CHARACTER_SN target);

	void									PushHostJoinUser(DHOST_TYPE_USER_ID UserID);
	void									RemoveHostJoinUser(DHOST_TYPE_USER_ID UserID);
	int										GetHostJoinUserCount();

	DHOST_TYPE_UINT32						GetRandomSeed(DHOST_TYPE_USER_ID UserID);

	// Ball
protected:
	int										m_BallNumber = 1;
	CBallController*						m_pBallController;
	int										m_BallOwner = 0;
	BallActionInfoDeque						m_BallMessageRecord;  // ��Ͽ� �� �޼��� ť
	std::vector<DHOST_TYPE_CHARACTER_SN>	m_BallShotHindrance;	// �� ���� ĳ���� ����Ʈ
	DHOST_TYPE_BALL_NUMBER					m_BallNumberGoalIn = 0;

public:
	TB::SVector3							GetBallPosition();
	void									BallNumberAdd(int delta) { m_BallNumber += delta; }
	void									BallNumberSet(int value) { m_BallNumber = value; }
	int										BallNumberGet() { return m_BallNumber; }
	CBallController*						BallControllerGet() { return m_pBallController; }
	void									BallOnEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);
	void									SetBallOwner(int value) { m_BallOwner = value; }
	int										GetBallOwner() { return m_BallOwner; }
	void									ClearBallActionInfoDeque();
	void									PushBallActionInfoDeque(SBallActionInfo ballInfo);
	void									DevBallActionInfoLog();
	DHOST_TYPE_BOOL							FindTargetBallActionInfo(DHOST_TYPE_BALL_NUMBER ballNumber, SBallActionInfo& sInfo);
	SBallActionInfo							FindTargetBallActionInfoWithIndex(int idx);
	F4PACKET::BALL_STATE					ConvertToProtobufBallState(EBALL_STATE state);
	EBALL_STATE								ConvertToEnumBallState(F4PACKET::BALL_STATE state);
	BallActionInfoDeque						GetBallActionInfoDeque() { return m_BallMessageRecord; }
	DHOST_TYPE_CHARACTER_SN					CheckTurnOver(DHOST_TYPE_BALL_NUMBER ballNumber, F4PACKET::BALL_STATE ballState, DHOST_TYPE_INT32 team);
	DHOST_TYPE_CHARACTER_SN					GetNeariestTeamCharacterFromRim(DHOST_TYPE_INT32 indexTeam);
	DHOST_TYPE_BOOL							CheckCurrentBallStateShot();
	void									PushBallShotHindrance(std::vector<DHOST_TYPE_CHARACTER_SN> vCharacterSN);
	std::vector<DHOST_TYPE_CHARACTER_SN>&	GetBallShotHindrance();
	void									ClearShotHindrance() { m_BallShotHindrance.clear(); }

	void									SetBallNumberGoalIn(DHOST_TYPE_BALL_NUMBER value) { m_BallNumberGoalIn = value; }
	DHOST_TYPE_BALL_NUMBER					GetBallNumberGoalIn() { return m_BallNumberGoalIn; }

	// State
protected:
	EHOST_STATE								m_CurrentState;
	std::map<EHOST_STATE, CState*>			m_States;

public :
	CState*									GetState(EHOST_STATE state);

public:
	void									ChangeState(EHOST_STATE state, void* pData = nullptr);
	EHOST_STATE								GetCurrentState() { return m_CurrentState; }
	DHOST_TYPE_STR							ConvertHostStateToStr(EHOST_STATE state);
	void									CheckExpiredRoom();
	void									ForciblyExpireRoom();
	DHOST_TYPE_BOOL							CheckForciblyExpireRoom();
	DHOST_TYPE_BOOL							CheckBadConnectUserIsEqualUserCount();
	void									CheckUserPing();
	
protected:
	CEnvironmentObject* m_pEnvironmentObject = nullptr;

public:
	void									CreateEnvironment();
	bool									EnvironmentCatSurprised(int moveNumber, TB::SVector3 position);


public: 
	DHOST_TYPE_BOOL							bTreatDelayMode;
	DHOST_TYPE_BOOL							bServerSyncPos;

	//! GameManager
public:
	//! Stage
	DHOST_TYPE_STAGE_ID GetStageID();
	F4PACKET::EGAME_MODE GetConvertGameMode();
	void SetStageID(DHOST_TYPE_STAGE_ID value);

	//! ExitType
	EEXIT_TYPE GetExitType();
	F4PACKET::EXIT_TYPE GetConvertExitType();
	void SetExitType(EEXIT_TYPE value);

	//! BackBoard
	F4PACKET::BB_STATE GetBackBoardState();
	void SetBackBoardState(F4PACKET::BB_STATE value);
	void CheckBackBoardState();

	//! GameMode
	EMODE_TYPE GetModeType();
	void SetModeType(EMODE_TYPE value);

	DHOST_TYPE_BOOL GetIsNormalGame();
	void SetIsNormalGame(DHOST_TYPE_BOOL value);

	DHOST_TYPE_BOOL GetDebug();
	void SetDebug(DHOST_TYPE_BOOL value);

	DHOST_TYPE_BOOL GetNoTieMode();
	void SetNoTieMode(DHOST_TYPE_BOOL value);

	DHOST_TYPE_BOOL CheckSingleMode();
	DHOST_TYPE_BOOL CheckSkillChallengeMode();

	//! Score
	DHOST_TYPE_INT32 GetScore(DHOST_TYPE_INT32 teamIndex);
	void SetScore(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_INT32 value);
	DHOST_TYPE_FLOAT GetScoredTime();
	void SetScoredTime(DHOST_TYPE_FLOAT value);
	DHOST_TYPE_FLOAT GetScoreRescue(DHOST_TYPE_INT32 teamIndex, F4PACKET::SBallShotT* pBallShot);
	DHOST_TYPE_FLOAT GetScoreRescue_CorrectionEarlyInTheGame(DHOST_TYPE_INT32 teamIndex, F4PACKET::SHOT_TYPE shottype);
	DHOST_TYPE_BOOL GetEnableRescue_CorrectionEarlyInTheGame(F4PACKET::SBallShotT* pBallShot, CAbility* pInfo);
	DHOST_TYPE_BOOL GetScoreLoosing(DHOST_TYPE_INT32 teamIndex);
	DHOST_TYPE_UINT32 GetScoreDifference();

	//! Time
	DHOST_TYPE_FLOAT GetJumpBallStartTime();
	void SetJumpBallStartTime(DHOST_TYPE_FLOAT value);

	//! GameProcess
	DHOST_TYPE_BOOL GetLoadComplete();
	void SetLoadComplete(DHOST_TYPE_BOOL value);

	//! Replay
	DHOST_TYPE_FLOAT GetReplayPoint();
	void SetReplayPoint(DHOST_TYPE_FLOAT value);
	DHOST_TYPE_BOOL GetReplayAssistCheck();
	void SetReplayAssistCheck(DHOST_TYPE_BOOL value);
	DHOST_TYPE_BOOL CheckReplay();
	DHOST_TYPE_BOOL CheckReplayCooldownTime();
	DHOST_TYPE_BOOL CheckReplayRequireAssist();
	DHOST_TYPE_BOOL CheckReplayQualifyingPoint();
	void CheckBuzzerBeater(DHOST_TYPE_INT32 ScoreTeam, DHOST_TYPE_INT32 Point, DHOST_TYPE_FLOAT PositionX, DHOST_TYPE_FLOAT PositionZ);
	DHOST_TYPE_BOOL GetBuzzerBeater();
	DHOST_TYPE_FLOAT GetShotRoomElapsedTime();
	void SetShotRoomElapsedTime(DHOST_TYPE_FLOAT value);

	//! Team
	DHOST_TYPE_INT32 GetOffenseTeam();
	void SetOffenseTeam(DHOST_TYPE_INT32  value);
	DHOST_TYPE_INT32 GetWinnerTeam();
	void SetWinnerTeam();
	DHOST_TYPE_INT32 GetAbNormalEndUserTeam();
	void SetAbNormalEndUserTeam(DHOST_TYPE_INT32 team);

	//! RoomInfo
	DHOST_TYPE_BOOL GetAbnormalExpireRoom();
	void SetAbnormalExpireRoom(DHOST_TYPE_BOOL value);

	//! Redis
	DHOST_TYPE_BOOL GetRedisLoadValue();
	void SetRedisLoadValue(DHOST_TYPE_BOOL value, DHOST_TYPE_FLOAT time);
	DHOST_TYPE_FLOAT GetRedisLoadRoomElapsedTime();
	void SetRedisLoadRoomElapsedTime(DHOST_TYPE_FLOAT value);
	DHOST_TYPE_BOOL	GetRedisSaveMatchResult();
	void SetRedisSaveMatchResult(DHOST_TYPE_BOOL value);

	//! Option
	DHOST_TYPE_INT32 GetOption();
	void SetOption(DHOST_TYPE_UINT64 value);
	DHOST_TYPE_BOOL CheckOption(DHOST_TYPE_UINT64 flag);

	std::vector<DHOST_TYPE_INT32>& GetVecScores();

	//! SpeedHack
	DHOST_TYPE_FLOAT GetSpeedHackRestrictionStepOne();
	void SetSpeedHackRestrictionStepOne(DHOST_TYPE_FLOAT value);
	DHOST_TYPE_FLOAT GetSpeedHackRestrictionStepTwo();
	void SetSpeedHackRestrictionStepTwo(DHOST_TYPE_FLOAT value);
	DHOST_TYPE_UINT32 GetSwitchToAiLevel();
	void SetSwitchToAiLevel(DHOST_TYPE_UINT32 value);

	DHOST_TYPE_BOOL GetLogShotRate();
	void SetLogShotRate(DHOST_TYPE_BOOL value);

	DHOST_TYPE_INT32 GetOverTimeCount();
	void IncreaseOverTimeCount();

protected:
	CGameManager* m_GameManager;
	//! End GameManager

	//! TimeManager
public:
	CGameManager* GetGameManager() { return m_GameManager; }
	
	DHOST_TYPE_GAME_TIME_F GetGameTime();
	void SetGameTime(DHOST_TYPE_GAME_TIME_F value);

	// ��� ������ 1�� �������� üũ
	DHOST_TYPE_BOOL CheckGameElapsedTimeOneMinute();

	DHOST_TYPE_FLOAT GetGameElapsedTimeSinceGameStart();

	DHOST_TYPE_GAME_TIME_F GetGameTimeInit();
	void SetGameTimeInit(DHOST_TYPE_GAME_TIME_F value);

	DHOST_TYPE_GAME_TIME_F GetRoomElapsedTime();

	DHOST_TYPE_BOOL GetOverTime();
	void SetOverTime(DHOST_TYPE_BOOL value);

	DHOST_TYPE_GAME_TIME_F GetShotClock();
	void SetShotClock(DHOST_TYPE_GAME_TIME_F value);
	void ResetShotClock();
	void ResetShotClockPacketSend();

	DHOST_TYPE_GAME_TIME_F GetReplayTime();
	void SetReplayTime(DHOST_TYPE_GAME_TIME_F value);

protected:
	CTimeManager* m_TimeManager;
	//! end


	//! DevManager
public:

#ifdef BINARY_SAVE
	// ��Ŷ�� ���̳ʸ� ���·� ����
	void DevPlayPacketSave(DHOST_TYPE_BOOL value);
	DHOST_TYPE_BOOL GetPlayPacketSave();
	void SetPlayPacketSave(DHOST_TYPE_BOOL value);
	void SaveBinary(const char* pData, DHOST_TYPE_UINT32 size, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_GAME_TIME_F time);
	void LoadBinary(DHOST_TYPE_USER_ID UserID);
#endif

	// ��Ŷ ī����
	void InitPacketCount();
	void IncreasePacketReceiveCount(DHOST_TYPE_UINT16 packet_id, DHOST_TYPE_UINT64 packet_size);
	void PrintPacketCountReport();

	//! ���߿� ȯ�漳��
	void DevConsole();
	void DevAiLevelSetting(uint32_t team, uint32_t aiLevel);
	void DevStageSetting(uint32_t stageId);
	void DevSceneSetting(int32_t value);
	DHOST_TYPE_INT32 GetSceneIndex();
	void SetSceneIndex(DHOST_TYPE_INT32 value);
	void SendDebugMessage(string str);	// ȣ��Ʈ���� Ŭ��� ����� ��Ʈ�� �޼��� ���� ��
	void SendDebugMessageUserOnly(string str, int UserID);

	DHOST_TYPE_BOOL GetDevNoKick();
	void SetDevNoKick(DHOST_TYPE_BOOL value);

	DHOST_TYPE_UINT32 GetHomeAiLevel();
	void SetHomeAiLevel(DHOST_TYPE_UINT32 value);

	DHOST_TYPE_UINT32 GetAwayAiLevel();
	void SetAwayAiLevel(DHOST_TYPE_UINT32 value);

	void SendCharacterBurstGauge();
	void DevUpdateEverySecond();

	void DevBasicTrainingSystemLog(DHOST_TYPE_CHARACTER_SN value);

protected:
	CDevManager* m_DevManager;
	//! end




protected:
	//! End AnimationController
	//static string TestPacket();

	//! Physics
public:
	CPhysicsHandler* GetPhysicsHandler() { return m_PhysicsHandler; }
	DHOST_TYPE_FLOAT GetShotPositionDegree(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);	// �� �������� �������� �Է°��� ������ ���Ѵ�.
	DHOST_TYPE_FLOAT GetShotPositionDistanceFromRim(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);
	DHOST_TYPE_UINT32 GetShotPositionZone(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);	// �� �������� �������� �� �������� ����(zone)�� ���Ѵ�.

protected:
	CPhysicsHandler* m_PhysicsHandler;
	//! End Physics


	//! Balance
public:
	CDataManagerBalance* m_pBalance;
	CDataManagerBalance* GetBalanceTable() { return  m_pBalance; };
	DHOST_TYPE_FLOAT GetBalanceValue(const char* strkey);

	CDataManagerShotSolution* m_ShotSolution;
	CDataManagerShotSolution* GetShotSolution() { return m_ShotSolution; };

	CAnimationController* m_AnimationController;
	CAnimationController* GetAnimationController() { return m_AnimationController; };
	//! End Balance
	static string TestPacket();
};
