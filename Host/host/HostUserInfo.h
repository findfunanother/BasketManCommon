#pragma once
#include <deque>
#include <map>
#include "HostDefine.h"

class CHost;


enum class SPEEDHACK_CHECK : int32_t {
	MOVE = 0b0001, // 1 (0x1)
	SHOT = 0b0010, // 2 (0x2)
	PICK = 0b0100, // 4 (0x4)
	Step04 = 0b1000  // 8 (0x8)
};

class CHostUserInfo
{
public:

	typedef std::deque<DHOST_TYPE_GAME_TIME_F> USER_PING_DEQUE;
	typedef std::deque<SPingRecvData> USER_PING_RECV_DATA_DEQUE;
	typedef std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> USER_PING_MAP;

	CHostUserInfo(DHOST_TYPE_INT32 UserID, void * peer, CHost* host, DHOST_TYPE_FLOAT CreateTime, EUSER_TYPE value = EUSER_TYPE::NORMAL);
	~CHostUserInfo();

	void Init();

	DHOST_TYPE_FLOAT GetCreateTime() { return m_CreateTime; }
	DHOST_TYPE_FLOAT GetLoadingStartime() { return m_LoadingStartTime; }
	void             SetLoadingStartime(DHOST_TYPE_FLOAT time) { m_LoadingStartTime = time; }

	const DHOST_TYPE_INT32 GetUserID() const { return m_UserID; }
	const void * GetPeer() const { return m_Peer; }

	const ECONNECT_STATE GetConnectState() { return m_ConnectState; }
	void SetConnectState(ECONNECT_STATE NewState);
	std::string ConvertConnectState(ECONNECT_STATE NewState);

	void SetLogin(DHOST_TYPE_BOOL Login) { m_Login = Login; }
	const DHOST_TYPE_BOOL GetLogin() const { return m_Login; }

	void PushUserGamePingDiffDeque(DHOST_TYPE_INT32 idx, DHOST_TYPE_GAME_TIME_F value);
	USER_PING_DEQUE GetUserGamePingDiffDeque() { return m_PingDiffDeque; }

	void ClearPingData();

	void SetPingAverage(DHOST_TYPE_GAME_TIME_F value) { m_PingAverage = value; }  
	DHOST_TYPE_GAME_TIME_F GetPingAverage() { return m_PingAverage; }

	void SetCurFPS(DHOST_TYPE_GAME_TIME_F value) { m_curFPS = value; }  
	DHOST_TYPE_GAME_TIME_F GetCurFPS() { return m_curFPS; }

	EUSER_TYPE GetUserType() { return m_UserType; }

	void InitUserWithAiCharacter() { m_VecUserWithAiCharacter.clear(); }
	void PushUserWithAiCharacter(DHOST_TYPE_CHARACTER_SN value);
	void RemoveUserWithAiCharacter(DHOST_TYPE_CHARACTER_SN value);
	std::vector<DHOST_TYPE_CHARACTER_SN>& GetUserWithAiCharacter() { return m_VecUserWithAiCharacter; }

	void PushPingMap(DHOST_TYPE_INT32 key, DHOST_TYPE_FLOAT value);
	USER_PING_MAP GetPingMap() { return m_PingMap; }

	void PushClientElapsedTimeMap(DHOST_TYPE_INT32 key, DHOST_TYPE_FLOAT value);
	void PushClientElapsedTimeWithSystemTime(DHOST_TYPE_FLOAT clientElapsedTime, DHOST_TYPE_DOUBLE clientSystemTime);
	USER_PING_MAP GetClientElapsedTimeMap() { return m_ClientElapsedTimeMap; }
	DHOST_TYPE_FLOAT FindClientElapsedTimeMap(DHOST_TYPE_INT32 key);
	std::vector<std::pair<DHOST_TYPE_FLOAT, DHOST_TYPE_DOUBLE>> GetClientElapsedTimeWithSystemTime() { return m_vecClientElapsedTimeWithSystemTime; }

	void IncreasePingIndex();
	void SetPingIndex(DHOST_TYPE_INT32 value) { m_PingIndex = value; }
	DHOST_TYPE_INT32 GetPingIndex() { return m_PingIndex; }
	DHOST_TYPE_FLOAT GetPingIndexElapsedTime(DHOST_TYPE_INT32 key);
	DHOST_TYPE_INT32 GetCheckPingIndex();

	// Steven enw Alive Packet Check;
	DHOST_TYPE_INT32 GetCheckPingIndexEx();
	void PushUserGamePingDeque(DHOST_TYPE_GAME_TIME_F value);
	DHOST_TYPE_FLOAT ProcessAlivePacket(DHOST_TYPE_GAME_TIME_F serverTime);

	void SetPreClientElapsedTime(DHOST_TYPE_FLOAT value) { m_ClientElapsedTime = value; }
	DHOST_TYPE_FLOAT GetPreClientElapsedTime() { return m_ClientElapsedTime; }

	void AddClientElapsedTimeAccumulate(DHOST_TYPE_FLOAT value) { m_ClientElapsedTimeAccumulate += value; }
	DHOST_TYPE_FLOAT GetClientElapsedTimeAccumulate() { return m_ClientElapsedTimeAccumulate; }

	void SetSpectatorLoadComplete(DHOST_TYPE_BOOL value) { m_SpectatorLoadComplete = value; }
	DHOST_TYPE_BOOL GetSpectatorLoadComplete() { return m_SpectatorLoadComplete; }

	void SetSpeedHackCheckLog(DHOST_TYPE_BOOL value) { m_bSpeedHackCheckLog = value; }
	DHOST_TYPE_BOOL GetSpeedHackCheckLog() { return m_bSpeedHackCheckLog; }

	void SetRandomSeed(DHOST_TYPE_UINT32 value) { m_RandomSeed = value; }
	DHOST_TYPE_UINT32 GetRandomSeed() { return m_RandomSeed; }

	void SetRandomSeedIndex(DHOST_TYPE_UINT32 value) { m_RandomSeedIndex = value; }
	DHOST_TYPE_UINT32 GetRandomSeedIndex() { return m_RandomSeedIndex; }


	void SetSendRandomSeed(DHOST_TYPE_UINT32 value) { m_SendedRandomSeed = value; }
	DHOST_TYPE_UINT32 GetSendRandomSeed() { return m_SendedRandomSeed; }


	void SetSendActionKey(DHOST_TYPE_UINT32 value) { m_SendedActionKey = value; }
	DHOST_TYPE_UINT32 GetSendActionKey() { return m_SendedActionKey; }


	void SetCurrentRandomKey(DHOST_TYPE_UINT32 currentKey) { m_ClientActionKey = currentKey; }
	DHOST_TYPE_BOOL IsValidAction();

	DHOST_TYPE_BOOL m_isPlaying;

private:

	DHOST_TYPE_INT32 m_UserID;
	void* m_Peer;
	CHost* m_Host;

	DHOST_TYPE_FLOAT m_CreateTime;
	DHOST_TYPE_FLOAT m_LoadingStartTime;
	
	ECONNECT_STATE m_ConnectState;
	EUSER_TYPE m_UserType;

	DHOST_TYPE_BOOL m_Login = false;

	std::vector<DHOST_TYPE_CHARACTER_SN> m_VecUserWithAiCharacter;

	
	USER_PING_DEQUE m_PingDiffDeque;			// 핑 전후 시간차 큐
	DHOST_TYPE_GAME_TIME_F m_PingAverage;		// 핑 전후 시간차 큐의 평균값

	DHOST_TYPE_GAME_TIME_F m_curFPS;		    // 호스트의 현재 FPS

	USER_PING_MAP m_PingMap;
	DHOST_TYPE_INT32 m_PingIndex;				    // 핑 인덱스 (재접하면 초기화 된다)
	DHOST_TYPE_INT32 m_RecvPingIndex;				// 받은 핑 인덱스 (재접하면 초기화 된다)
	

	DHOST_TYPE_INT32 m_nCheckSpeedCnt;
	DHOST_TYPE_FLOAT m_ClientElapsedTime;
	USER_PING_MAP m_ClientElapsedTimeMap;
	DHOST_TYPE_FLOAT m_ClientElapsedTimeAccumulate;
	std::vector<std::pair<DHOST_TYPE_FLOAT, DHOST_TYPE_DOUBLE>> m_vecClientElapsedTimeWithSystemTime;

	DHOST_TYPE_BOOL m_SpectatorLoadComplete;	// 관전하는 유저가 로드가 끝났다고 알리는 시점부터 플레이 패킷을 전달하려고
	DHOST_TYPE_BOOL m_bSpeedHackCheckLog;
	DHOST_TYPE_UINT32 m_RandomSeed;
	DHOST_TYPE_INT32 m_RandomSeedIndex;
	DHOST_TYPE_UINT32 m_SendedRandomSeed; // 보낸 
	DHOST_TYPE_UINT32 m_SendedActionKey;  // 리바운드, 픽 서버 판정 관련 보냈을 때의 키를 기억한다. 

	DHOST_TYPE_UINT32 m_ClientActionKey;

private:
	DHOST_TYPE_FLOAT  m_fPreClientElapsedTime;
	DHOST_TYPE_DOUBLE m_fPreClientSystemTime;
	DHOST_TYPE_INT32 m_nDetectCount;

	// 여러가지 스피드핵 의심 로직의 공통점을 찾아 스피드핵으로 간주 
private:
	DHOST_TYPE_INT32 m_nSpeedHackDectectedCount;

public:
	void SpeedHackDetected(SPEEDHACK_CHECK bitFlag);
	DHOST_TYPE_BOOL IsSpeedhackDectected_Low();
	DHOST_TYPE_BOOL IsSpeedhackDectected_High();

//////////////////   매크로 체크 및 스피드핵 체크 ////////// 

public:

#define CHECK_COUNT 4
#define GAP_THRESHOLD 0.015f

	DHOST_TYPE_FLOAT REBOUNDABLETIME_CLIENT;
	DHOST_TYPE_FLOAT REBOUNDABLETIME;

	std::deque<float> gapHistory;
	void AddGap(float gap) {
		if (gapHistory.size() >= CHECK_COUNT) {
			gapHistory.pop_front();
		}
		gapHistory.push_back(gap);
	}

	bool IsMacroSuspected() {
		if (gapHistory.size() < CHECK_COUNT) return false;
		float first = gapHistory.front();
		for (float g : gapHistory) {
			if (std::fabs(g - first) > GAP_THRESHOLD) return false; // 작은 오차 허용
		}
		return true;
	}
};