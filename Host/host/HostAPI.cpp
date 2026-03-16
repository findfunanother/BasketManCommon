#include "HostAPI.h"

extern "C"
{
	HOST_API void* HostCreate(char* hostID, int option, HostSendPacketCallback callback)
	{
		CHost* host = new CHost(hostID, option);

		if (callback != nullptr)
		{
			host->RegistCallbackFuncSendPacket([callback](HostMessage* hostMessage)
			{
				callback(hostMessage);
			});
		}

		host->Initialize();

		return host;
	}

	HOST_API void* HostDelete(void* host)
	{
		if (host != nullptr)
		{
			CHost* ptr = static_cast<CHost*>(host);
			ptr->Release();
			delete ptr;
		}

		return nullptr;
	}

	HOST_API void* HostAddUser(void* host, int userID, void* peer)
	{
		if (host != nullptr)
		{
			CHost* ptr = static_cast<CHost*>(host);
			ptr->UserAdd(userID, peer);
		}

		return host;
	}

	HOST_API void HostProcessPacket(void* host, void* data, int size, void* peer)
	{
		if (host != nullptr && data != nullptr && size > 0)
		{
			CHost* ptr = static_cast<CHost*>(host);
			ptr->ProcessPacket(static_cast<const char*>(data), size, peer);
		}
	}

	HOST_API void HostUpdate(void* host, float timeDelta)
	{
		if (host != nullptr)
		{
			CHost* ptr = static_cast<CHost*>(host);
			ptr->Update(timeDelta);
		}
	}
}
