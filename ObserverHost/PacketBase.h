#pragma once

#include "HostDefine.h"

class CPacketBase
{
public:

	CPacketBase():m_isFlatbuf(false),m_PacketID(0),m_PacketType(0),m_UserID(0){}
	virtual ~CPacketBase() {}

	void SetPacketID(DHOST_TYPE_INT32 packetID) { m_PacketID = packetID; }
	const DHOST_TYPE_INT32 GetPacketID() const { return m_PacketID; }

	void SetPacketType(DHOST_TYPE_INT32 packetType) { m_PacketType = packetType; }
	const DHOST_TYPE_INT32 GetPacketType() const { return m_PacketType; }

	void SetUserID(DHOST_TYPE_USER_ID userID) { m_UserID = userID; }
	const DHOST_TYPE_USER_ID GetUserID() const { return m_UserID; }
	bool m_isFlatbuf;
private:
	DHOST_TYPE_INT32 m_PacketID;
	DHOST_TYPE_INT32 m_PacketType;
	DHOST_TYPE_INT32 m_UserID;
	
};