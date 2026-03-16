#pragma once
#include <deque>

#include "F4Packet_generated.h"
#include "FlatBufPacket.h"
#include "HostTypeDefine.h"

class CObUserInfo
{
public:

	

	CObUserInfo(unsigned int UserID, void* peer);
	~CObUserInfo();

	void Init();

	const unsigned int GetUserID() const { return m_UserID; }
	const void* GetPeer() const { return m_Peer; }

	const ECONNECT_STATE GetConnectState() { return m_ConnectState; }
	void SetConnectState(ECONNECT_STATE NewState) { m_ConnectState = NewState; }

	void SetLogin(DHOST_TYPE_BOOL Login) { m_Login = Login; }
	//const DHOST_TYPE_BOOL GetLogin() const { return m_Login; }
	bool IsObserverHost(){return m_isObserverHost; }
	void SetObserverHost(){ m_isObserverHost = true; }
private:

	unsigned int m_UserID;
	void* m_Peer;
	bool m_isObserverHost = false;
	ECONNECT_STATE m_ConnectState;
	DHOST_TYPE_BOOL m_Login = false;
};
