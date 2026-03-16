#include "OBHostAPI.h"
#include "pch.h"
#include "CObHost.h"

CObHost* ObHostCreate(char* hostID, int option)
{
#ifdef _MSC_VER
#else
	// char* cwd;
	// char wd[BUFF_SIZE];

	// cwd = getcwd(NULL, BUFF_SIZE);

	// std::string path(cwd);

	// std::string pull_path1 = path + "/game/cservice/Resource/GameCore.so";
	// std::string pull_path2 = path + "/game/cservice/Resource/Physics.so";

	// dlopen(pull_path2.c_str(), RTLD_NOW | RTLD_GLOBAL);
	// dlopen(pull_path1.c_str(), RTLD_NOW | RTLD_GLOBAL);

#endif

	CObHost* pHost = new CObHost(hostID, option);

	pHost->Initialize();

	return pHost;
}

void ObHostUpdate(CObHost* pHost, float timeDelta)
{
	if (pHost != nullptr)
	{
		pHost->Update(timeDelta);
	}
}

void ObHostAddUser(CObHost* pHost, DHOST_TYPE_USER_ID userid, void* peer)
{
	if (pHost != nullptr)
	{
		pHost->ObserverAdd(userid, peer);
	}
}

void ObHostProcessPacket(CObHost* pHost, const char* pData, DHOST_TYPE_SIZE size, void* peer)
{
	pHost->ProcessPacket(pData, size, peer);
}

void ObHostDelete(CObHost* pHost)
{
	pHost->Release();

	SAFE_DELETE(pHost);
}

DHOST_TYPE_USER_ID ObHostGetUserID(CObHost* pHost, void* peer)
{
	return pHost->UserGetID(peer);
}

void ObHostRegistCallbackFunc(CObHost* pHost, ProcessPacketCallbackToCpp pFunc)
{
	pHost->RegistCallbackFunc(pFunc);
}

void ObHostOnJoin(CObHost* pHost, unsigned int UserID)
{
	pHost->OnHostJoin(UserID);
}

void ObHostOnConnect(CObHost* pHost, unsigned int UserID)
{
	pHost->OnHostConnect(UserID);
}

void ObHostOnDisconnect(CObHost* pHost, unsigned int UserID)
{
	pHost->OnHostDisconnect(UserID);
}

void ObHostOnQuit(CObHost* pHost, unsigned int UserID)
{
	pHost->OnHostQuit(UserID);
}

void ObHostOnExit(CObHost* pHost)
{
	pHost->OnHostExit();
}

void ObHostRegistCallbackFuncLog(CObHost* pHost, LogCallback pFunc)
{
	pHost->RegistCallbackFuncLog(pFunc);
}

void ObHostOnRedisMatchInfo(CObHost* pHost, SRedisMatchInfo* pData)
{
	//pHost->OnRedisMatchInfo(pData);
}

//void HostRegistCallbackFuncRedis(CObHost* pHost, RedisCallback pFunc)
//{
	//pHost->RegistCallbackFuncRedis(pFunc);
//}

void ObHostRegistCallbackFuncSendPacket(CObHost* pHost, HostMessageCallback messageCallback)
{
	pHost->RegistCallbackFuncSendPacket(messageCallback);
}
