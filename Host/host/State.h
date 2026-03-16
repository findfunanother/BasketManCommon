#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))

#else
#pragma clang diagnostic ignored "-Wswitch"
#endif
#include <vector>
#include "Host.h"
#include "HostDefine.h"
#include "HostStruct.h"
#include "F4Packet_generated.h"
#include "FlatBufPacket.h"


// 안전하게 버전 체크하여 정의하는 방식
/*
#if __cplusplus < 201402L
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
*/

#define CASE_CALL(packid, data) case PACKET_ID::packid: return ConvertPacket_##packid(userid, peer, reinterpret_cast<CProtoBufPacket<data>*>(pPacket), recv_time);
#define CASE_FB_CALL(packetid) case (PACKET_ID)F4PACKET::PACKET_ID::packetid: \
				return ConvertPacket_##packetid(userid, peer, reinterpret_cast<CFlatBufPacket<F4PACKET::packetid##_data>*>(pPacket), recv_time);

#define CASE_FB_CALL_EXTEND(packetid) case (PACKET_ID)F4PACKET::PACKET_ID::packetid: \
				return ConvertPacketExtend_##packetid(userid, peer, reinterpret_cast<CFlatBufPacket<F4PACKET::packetid##_data>*>(pPacket), pData, size, recv_time);


#define DEFAULT_CALL(classType) default: return classType::OnPacket(pPacket, peer, recv_time);
#define BASE_CALL() default: return CState::OnPacket(pPacket, peer, recv_time);

#define DECLARATION_PACKET_PROCESS(packetid) const DHOST_TYPE_BOOL virtual ConvertPacket_##packetid(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::packetid##_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time);
#define DECLARATION_PACKET_PROCESS_EXTEND(packetid) const DHOST_TYPE_BOOL virtual ConvertPacketExtend_##packetid(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::packetid##_data>* pPacket, const char* pData, int size, const DHOST_TYPE_GAME_TIME_F& recv_time);

using namespace std;
using namespace F4PACKET;
using namespace TB;

class CHost;
class CDataManagerBalance;

enum class ABNORMAL
{
	Data_Quit = -1,
	Data_implicit = -2,
	Data_PositionNan = -3,
	Data_GetValueNone = -4,
	Data_SpeedNan = -5,
	Data_PlayerGetValueNan = -6,
	Data_Vector3Nan = -7,
	Data_NeverZero = -8,
	Data_Crash = -9,
	Data_ReboundAble = -10,
	DATA = -1000,
};

class CCoolTimer
{

public:

	CCoolTimer(CHost* host);
	~CCoolTimer();

	enum class TYPE
	{
		NONE = -1,
		SCREEN_COLLISION = 0,
		RECEIVE_PASS = 1,
	};

	void bookMark(TYPE);

	bool withinTime(TYPE type, float time);



private:
	map<TYPE, float> datas;
	CHost* m_pHost;
};


struct ReboundBallPacket
{
	CPacketImplement<F4PACKET::PACKET_ID>* pPacket; // = nullptr;

	DHOST_TYPE_USER_ID			 userID;
	DHOST_TYPE_CHARACTER_SN      characterID;
	DHOST_TYPE_GAME_TIME_F		 receivedTime;
	DHOST_TYPE_FLOAT			 reboundAbility;
	DHOST_TYPE_FLOAT			 canWaitTime;
	DHOST_TYPE_BOOL              sucess;

	DHOST_TYPE_FLOAT		fastestRecvePacketPoint;
	DHOST_TYPE_FLOAT        bigiestReboundAbilityPoint;
	DHOST_TYPE_FLOAT		defendingPoint;
	DHOST_TYPE_FLOAT		score3LoosingPoint;
	DHOST_TYPE_FLOAT		score4To6LoosingPoint;
	DHOST_TYPE_FLOAT		score7LoosingPoint;
	DHOST_TYPE_FLOAT		getTotalPoint;

	DHOST_TYPE_INT32        priority;

	ReboundBallPacket(const char* pData, int size)
		: userID(0)
		, characterID(0)
		, receivedTime(0.0f)
		, reboundAbility(0.0f)
		, canWaitTime(0.0f)
		, sucess(false)
		, pPacket(nullptr)
		, fastestRecvePacketPoint(0.0f)
		, bigiestReboundAbilityPoint(0.0f)
		, defendingPoint(0.0f)
		, score3LoosingPoint(0.0f)
		, score4To6LoosingPoint(0.0f)
		, score7LoosingPoint(0.0f)
		, getTotalPoint(0.0f)
		, priority(0)
	{
		auto result = FlatBufProtocol::GetPacket(pPacket, pData, size);

		if (result != FlatBufProtocol::ERESULT::SUCCESS)
		{
			pPacket = nullptr;  // 혹은 예외 throw
		}
	}

	// 소멸자
	~ReboundBallPacket()
	{
		SAFE_DELETE(pPacket);
	}


	// 데이터 접근
	const F4PACKET::play_c2s_ballRebound_data* GetData() const
	{
		CPacketBase* pBasePacket = pPacket;

		CFlatBufPacket<play_c2s_ballRebound_data>* pballRebound = (CFlatBufPacket<play_c2s_ballRebound_data>*) pBasePacket;

		return pballRebound ? pballRebound->GetData() : nullptr;
	}

	CFlatBufPacket<F4PACKET::play_c2s_ballRebound_data>* GetPacket() const
	{
		CPacketBase* pBasePacket = pPacket;

		// const 멤버 함수에서 non-const 포인터 리턴 → const_cast로 처리
		return const_cast<CFlatBufPacket<F4PACKET::play_c2s_ballRebound_data>*>
			(static_cast<const CFlatBufPacket<F4PACKET::play_c2s_ballRebound_data>*>(pBasePacket));
	}
};


class CState
{
	typedef std::deque<map<DHOST_TYPE_CHARACTER_SN, ECHARACTER_RECORD_TYPE>>		SendRecordInfoDeque;

public:
	CState(CHost* host);
	virtual ~CState();

	virtual void OnEnter(void * pData);	
	virtual void OnExit();
	virtual void OnUpdate(float timeDelta);

	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, const char* pData, int size, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);
	virtual void OnBallEvent(int ballNumber, string eventName, int ownerID, F4PACKET::BALL_STATE ballState, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_INT32 team, int point, float ballPositionX, float ballPositionZ);

protected:
	float CalculateTargetReachTime(CDataManagerBalance* pTable, SBallShotT* pBallShot, float distance);
	float GetYaw(float directionX, float directionZ);
	bool IsPointInLine(SVector3 start, SVector3 end, SVector3 target, float distance);

protected:
	CCoolTimer* m_coolTimer;
	CHost* m_pHost;
	SendRecordInfoDeque m_SendRecordInfo;

	// 새로운 리바운드 처리 관련 
protected:
	std::list<ReboundBallPacket*> m_BallReboundPackets;


	bool m_IsBallWindowOpen = false;
	DHOST_TYPE_GAME_TIME_F m_BallWindowStartTime = 0.0f;
	DHOST_TYPE_GAME_TIME_F WINDOW_DURATION = 0.0f;

	void ProcessReboundBallSimulationPackets();
	DHOST_TYPE_BOOL OnRecvReboundBallSimulation(DHOST_TYPE_USER_ID userid, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const char* pData, DHOST_TYPE_INT32 size, int abillity);

private:
	DHOST_TYPE_BOOL Process_play_c2s_ballRebound(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<play_c2s_ballRebound_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(system_c2c_loadingPercentage)
	DECLARATION_PACKET_PROCESS(system_c2s_login)
	DECLARATION_PACKET_PROCESS(system_c2s_readyToLoad)
	DECLARATION_PACKET_PROCESS(system_c2s_loadComplete)
		
	DECLARATION_PACKET_PROCESS(system_c2s_chat)
	DECLARATION_PACKET_PROCESS(system_c2c_ping)
	DECLARATION_PACKET_PROCESS(system_c2s_devGameTime)
	DECLARATION_PACKET_PROCESS(system_c2s_awayFromKeyboard)
	DECLARATION_PACKET_PROCESS(system_c2s_useJoystick)
	DECLARATION_PACKET_PROCESS(system_c2s_exitGame)
	DECLARATION_PACKET_PROCESS(system_c2s_surrenderVote)
	DECLARATION_PACKET_PROCESS(system_c2s_versionInfo)
	DECLARATION_PACKET_PROCESS(play_c2s_ballClear)
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp)
	DECLARATION_PACKET_PROCESS(play_c2s_ballShot)
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation) 

	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound)
	DECLARATION_PACKET_PROCESS_EXTEND(play_c2s_ballRebound)
	
	DECLARATION_PACKET_PROCESS(play_c2s_environmentCat) 
	DECLARATION_PACKET_PROCESS(play_c2s_onFireModeInfo)
	DECLARATION_PACKET_PROCESS(play_c2s_playerAlleyOopCut)
	DECLARATION_PACKET_PROCESS(play_c2s_playerAlleyOopPass)
	DECLARATION_PACKET_PROCESS(play_c2s_playerAlleyOopShot)
	DECLARATION_PACKET_PROCESS(play_c2s_playerBlock)
	DECLARATION_PACKET_PROCESS(play_c2s_playerBuff)
	DECLARATION_PACKET_PROCESS(play_c2s_playerHit)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCatchAndShotMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerInYourFace)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCeremony)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCloseOut)
	DECLARATION_PACKET_PROCESS(play_c2s_playerChaseContest)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCollision)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCrossOver)
	DECLARATION_PACKET_PROCESS(play_c2s_playerCutIn)
	DECLARATION_PACKET_PROCESS(play_c2s_playerDash)
	DECLARATION_PACKET_PROCESS(play_c2s_playerDenyDefense)
	DECLARATION_PACKET_PROCESS(play_c2s_playerDoubleClutch)
	DECLARATION_PACKET_PROCESS(play_c2s_playerFakeShot)
	DECLARATION_PACKET_PROCESS(play_c2s_playerGoAndCatch)
	DECLARATION_PACKET_PROCESS(play_c2s_playerHandCheck)
	DECLARATION_PACKET_PROCESS(play_c2s_playerHookHook)
	DECLARATION_PACKET_PROCESS(play_c2s_playerHopStep)
	DECLARATION_PACKET_PROCESS(play_c2s_playerIntercept)
	DECLARATION_PACKET_PROCESS(play_c2s_playerJumpBallTapOut)
	DECLARATION_PACKET_PROCESS(play_c2s_playerModificationAbility)
	DECLARATION_PACKET_PROCESS(play_c2s_playerMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerReceivePass)
	DECLARATION_PACKET_PROCESS(play_c2s_playerRebound)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPass)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPassFake)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPassive)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPenetrate)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPenetratePostUp)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPenetrateReady)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPick)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPickAndMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPickAndSlip)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPositionCorrect)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPostUpCollision)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPostUpPenetrate)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPostUpReady)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPostUpStepBack)
	DECLARATION_PACKET_PROCESS(play_c2s_playerScreen)
	DECLARATION_PACKET_PROCESS(play_c2s_playerShakeAndBake)
	DECLARATION_PACKET_PROCESS(play_c2s_playerShot)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSlipAndSlide)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSlideStep)
	DECLARATION_PACKET_PROCESS(play_c2s_playerShadowFaceupDash)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSpeaking)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSpinMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerStand)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSteal)
	DECLARATION_PACKET_PROCESS(play_c2s_playerStealFail)
	DECLARATION_PACKET_PROCESS(play_c2s_playerStun)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSwitchActive)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSwitchRequest)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSwitchResponse)
	DECLARATION_PACKET_PROCESS(play_c2s_playerBurstRequest)
	DECLARATION_PACKET_PROCESS(play_c2s_playerTapOut)
	DECLARATION_PACKET_PROCESS(play_c2s_playerTapPass)
	DECLARATION_PACKET_PROCESS(play_c2s_playerTipIn)
	DECLARATION_PACKET_PROCESS(play_c2s_playerVCut)
	DECLARATION_PACKET_PROCESS(play_c2s_playerGiveAndGoMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerOneDribblePullUpJumperMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerEscapeDribble)
	DECLARATION_PACKET_PROCESS(play_c2s_playerShotInterfere)
	DECLARATION_PACKET_PROCESS(play_c2s_fxDisplayPotential)
	DECLARATION_PACKET_PROCESS(play_c2s_playerShammgod)
	DECLARATION_PACKET_PROCESS(play_c2s_playerStunIntercept)
	DECLARATION_PACKET_PROCESS(play_c2s_playerIllegalScreen)
	DECLARATION_PACKET_PROCESS(play_c2s_playerSyncInfo)
	DECLARATION_PACKET_PROCESS(play_c2s_playerPassiveDataSet)
	DECLARATION_PACKET_PROCESS(play_c2s_playerBehindStepBackJumperShot)

	DECLARATION_PACKET_PROCESS(play_c2s_uiDisplayMedal)

	DECLARATION_PACKET_PROCESS(play_c2s_playerEmoji)

	DECLARATION_PACKET_PROCESS(play_c2s_greatDefense)

	DECLARATION_PACKET_PROCESS(system_s2s_clientAliveCheck)

	DECLARATION_PACKET_PROCESS(system_s2s_systemTimeCheck)

	DECLARATION_PACKET_PROCESS(system_c2s_test)

	DECLARATION_PACKET_PROCESS(system_c2s_blackBox)

	DECLARATION_PACKET_PROCESS(system_c2s_wkawofur)

	DECLARATION_PACKET_PROCESS(system_c2s_skinChange)


protected:
	DHOST_TYPE_FLOAT curTimeDelta;

	SVector3* CopyVector(SVector3 src);
	SVector3* NewVector(float x, float y, float z);

	void PushSendRecordInfo(DHOST_TYPE_CHARACTER_SN characterSN, ECHARACTER_RECORD_TYPE type);
	void SendRecordData();
	void SendRecordData(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE recordType);

	bool Cheat(int id, const flatbuffers::String * context);
	virtual void SendMedalUIDisplay(int32_t id, MEDAL_INDEX medalIndex);

	F4PACKET::SHOT_BOUND GetSuccessShotBoundByShotType(float bonus, F4PACKET::SHOT_TYPE shotType);
	F4PACKET::SHOT_BOUND GetFailShotBoundByShotType(float bonus, F4PACKET::SHOT_TYPE shotType);

	// 슛 성공률 관련 메달
	DHOST_TYPE_FLOAT ApplyMedalEffectToShotSuccessRate(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

	// 슛 비거리 관련 메달
	DHOST_TYPE_FLOAT ApplyMedalEffectToShotRange(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

	// 슛 위치 관련 메달
	DHOST_TYPE_FLOAT ApplyMedalEffectToShotPosition(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

	// 슛 오픈 찬스 관련 메달
	DHOST_TYPE_FLOAT ApplyMedalEffectToShotWideOpen(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility);

	// 1. 슛 타입에 따른 슛 성공률
	DHOST_TYPE_FLOAT CalcShotTypeToSuccessRate(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility, DHOST_TYPE_FLOAT distanceRimToOwner, F4PACKET::BALL_STATE& ballState);

	// 2. 주 손 사용으로 인한 슛 성공률 보너스
	DHOST_TYPE_FLOAT CalcHandednessToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility);

	// 3. 스킬사용으로 인한 성공률 보너스
	DHOST_TYPE_FLOAT CalcSkillToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo, CAbility* pAbility, DHOST_TYPE_FLOAT successRate, DHOST_TYPE_BOOL bSkillForce = false);

	// 4. 스킬사용으로 인한 슛 비거리 보너스
	DHOST_TYPE_FLOAT CalcSkillToShotRangeBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

	// 5. 잠재능력(슛 보너스)
	DHOST_TYPE_FLOAT CalcPotentialToSuccessRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

	// 6. 잠재능력(숨 막힐듯한 외곽 수비)
	DHOST_TYPE_FLOAT CalcPotentialToHindrancesBonus(DHOST_TYPE_CHARACTER_SN id, SBallShotT* pShotInfo, DHOST_TYPE_INT32* fxLevel, ACTION_TYPE actionType);

	//! 패스로 BallLerp 발생시킬 때 잠재능력 활성화 여부
	DHOST_TYPE_INT32 GetPotentialForPass(DHOST_TYPE_CHARACTER_SN id, PASS_TYPE type, SKILL_INDEX skill, DHOST_TYPE_USER_ID userid);

	// 7. 팀능력치 공격, 정신력 
	DHOST_TYPE_FLOAT CalcTeamAttributesShotRateBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);

private:
	DHOST_TYPE_FLOAT TeamAttackAttributesBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);
	DHOST_TYPE_FLOAT TeamMentalAttributesBonus(F4PACKET::SPlayerInformationT* pPlayerInfo, SBallShotT* pShotInfo);
	DHOST_TYPE_FLOAT GetMentalAttributesShotBonus(SBallShotT* pShotInfo, DHOST_TYPE_FLOAT attackMentalAttribute, DHOST_TYPE_FLOAT defenceMentalAttribute);

};

