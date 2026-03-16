#pragma once
#include "CObHost.h"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#ifdef HOST_EXPORTS
#define HOST_API __declspec(dllexport)
#else
#define HOST_API __declspec(dllimport)
#endif
#define DLLOCAL
#else
#define HOST_API __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif



// 콜백 함수 선언
extern "C"
{
	// HostTest 에서 사용
	HOST_API CObHost* ObHostCreate(char* hostID, int option);
	HOST_API void ObHostDelete(CObHost* pHost);
	HOST_API void ObHostUpdate(CObHost* pHost, float timeDelta);
	HOST_API void ObHostAddUser(CObHost* pHost, DHOST_TYPE_USER_ID userid, void* peer);
	HOST_API void ObHostProcessPacket(CObHost* pHost, const char* pData, DHOST_TYPE_SIZE size, void* peer);
	HOST_API DHOST_TYPE_USER_ID ObHostGetUserID(CObHost* pHost, void* peer);
	HOST_API void ObHostRegistCallbackFuncSendPacket(CObHost* pHost, HostMessageCallback messageCallback);

	// Skynet 에서 사용
	HOST_API void ObHostRegistCallbackFunc(CObHost* pHost, ProcessPacketCallbackToCpp pFunc);
	HOST_API void ObHostOnJoin(CObHost* pHost, unsigned int UserID);
	HOST_API void ObHostOnConnect(CObHost* pHost, unsigned int UserID);
	HOST_API void ObHostOnDisconnect(CObHost* pHost, unsigned int UserID);
	HOST_API void ObHostOnQuit(CObHost* pHost, unsigned int UserID);
	HOST_API void ObHostOnExit(CObHost* pHost);
	HOST_API void ObHostRegistCallbackFuncLog(CObHost* pHost, LogCallback pFunc);

	// Redis
	HOST_API void ObHostOnRedisMatchInfo(CObHost* pHost, SRedisMatchInfo* pData);
	//HOST_API void HostRegistCallbackFuncRedis(CObHost* pHost, RedisCallback pFunc);

	// 개발용 콘솔 출력 (HostTest 에서만 작용하게 한다)
	
}