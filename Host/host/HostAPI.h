#pragma once
#include "Host.h"

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

extern "C"
{
	typedef void(*HostSendPacketCallback)(HostMessage* hostMessage);

	HOST_API void* HostCreate(char* hostID, int option, HostSendPacketCallback callback);
	HOST_API void* HostDelete(void* host);
	HOST_API void* HostAddUser(void* host, int userID, void* peer);
	HOST_API void HostProcessPacket(void* host, void* data, int size, void* peer);
	HOST_API void HostUpdate(void* host, float timeDelta);
}

