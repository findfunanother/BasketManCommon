#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <thread>

#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#endif

#include <vector>
#include <functional>
#include <string>
#include <deque>
#include <map>


#include "CObUserInfo.h"
#include "PacketBase.h"
#include "FlatBufProtocol.h"
#include "HostDefine.h"


#define HOST_CALLBACK __cdecl



typedef struct _HostMessage
{
	int userID;
	int size;
	void* data;
	const void* peer;
}HostMessage;


using ObserverUserMapType = std::map<unsigned int, CObUserInfo*>;



typedef std::function<void(HostMessage*)> HostMessageCallback;
typedef std::function<void(const char*, int, int)> ProcessPacketCallbackToCpp;
//typedef std::function<void(SRedisMatchResult*)> RedisCallback;
typedef std::function<void(const char*)> LogCallback;

//using HostStateMapType = std::map<EHOST_STATE, CState*>;
//using HostUserMapType = std::map<DHOST_TYPE_USER_ID, CHostUserInfo*>;
//using std::map<DHOST_TYPE_CHARACTER_SN, CCharacterInfo*> = std::map<DHOST_TYPE_CHARACTER_SN, CCharacterInfo*>;
//using HostBadConnectUserVecType = std::vector<DHOST_TYPE_USER_ID>;

using namespace std;

class  CObHost
{
private:
	typedef struct packetdata
	{
		char* pData;
		DHOST_TYPE_INT32 size;
	}PacketData;
	ObserverUserMapType							m_HostUserMap;
	vector<PacketData*>						m_packetData;
	CObUserInfo* 							m_HostConnected;
	vector<unsigned int>					m_UIDwaiting;
public:

	CObHost(char* hostID, int option);
	~CObHost();

	void									Initialize();
	void									Release();

	void									StorePacket(const char* pData, DHOST_TYPE_INT32 size);

	void									Update(float timeDelta);
	void									ProcessPacket(const char* pData, DHOST_TYPE_INT32 size, void* peer);
	//void 									IncreasePacketReceiveCount(uint16_t packet_id, DHOST_TYPE_UINT64 packet_size);
	//void									SendPacket(CPacketBase& packet, DHOST_TYPE_USER_ID UserID);
	DHOST_TYPE_INT32						SendPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID);
	void									BroadcastPacket(char* packet);

	void									RegistCallbackFunc(ProcessPacketCallbackToCpp pFunc);
	
	void									RegistCallbackFuncLog(LogCallback pFunc);
	void									RegistCallbackFuncSendPacket(HostMessageCallback pFunc);
	//void									ToLog(const char* msg);

	void									OnHostJoin(unsigned int UserID);
	void									OnHostConnect(unsigned int UserID);
	void									OnHostDisconnect(unsigned int UserID);
	void									OnHostQuit(unsigned int UserID);
	void ToLog(const char* msg);
	void									OnHostExit();
	DHOST_TYPE_HOST_ID						GetHostID() { return m_HostID; }

	// Redis
	//void									OnRedisMatchInfo(SRedisMatchInfo* pData);

	//DHOST_TYPE_BOOL							GetRedisLoadValue() { return m_bRedisLoadValue; }
	//void									SetRedisLoadValue(DHOST_TYPE_BOOL value) { m_bRedisLoadValue = value; }
	
	//void									DevConsole();
	//void									DevPlayPacketSave(bool value);
	//void									DevAiLevelSetting(uint32_t team, uint32_t aiLevel);
	//void									DevStageSetting(uint32_t stageId);
	void									RemoveObserver(unsigned int UserID);
protected:
	
	char* m_PacketDataBuffer;
	DHOST_TYPE_HOST_ID						m_HostID;
	HostMessageCallback						m_CallbackBroadcast;
	ProcessPacketCallbackToCpp				m_CallbackCpp;
	//RedisCallback							m_CallbackRedis;
	LogCallback								m_CallbackLog;
	DHOST_TYPE_UINT16						m_AiUserCount;
	//DHOST_TYPE_BOOL							m_bRedisLoadValue;

public:
	void									ObserverAdd(unsigned int userid, void* peer);
	int										ObserverCount();
	CObUserInfo*							ObserverFind(unsigned int UserID);
	DHOST_TYPE_USER_ID						UserGetID(void* peer);
	CObUserInfo*							FindObserver(unsigned int UserID);
	DHOST_TYPE_UINT16						GetAiUserCount() { return m_AiUserCount; }
	void									AddAiUserCount() { ++m_AiUserCount; }
	
	// Log
public:
	int m_IDX = 0;
	//DHOST_TYPE_UINT64 m_PacketSize;
	std::map<uint16_t, int> m_PacketReceiveCountMap;

#ifdef BINARY_SAVE
	// 패킷 바이너리 형태로 저장
	//bool m_bPlayPacketSave = false;
	//bool GetPlayPacketSave() { return m_bPlayPacketSave; }
	//void SetPlayPacketSave(bool value) { m_bPlayPacketSave = value; }
	//void SaveBinary(const char* pData, uint32_t size, DHOST_TYPE_USER_ID UserID, float time);
	//void LoadBinary();
#endif
};