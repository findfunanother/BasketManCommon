#pragma once
#include "F4Packet_generated.h"
#include "PacketImplement.h"

class CFlatBufPacketImplement : public CPacketImplement<F4PACKET::PACKET_ID>
{
protected:
public:
	CFlatBufPacketImplement(const F4PACKET::PACKET_ID packetID);
	virtual ~CFlatBufPacketImplement();

	virtual DHOST_TYPE_BOOL Decode(const char* pBuf, DHOST_TYPE_INT32 size);
	virtual DHOST_TYPE_BOOL Encode(char* pBuf, size_t size);
	virtual DHOST_TYPE_INT32 GetEncodeStreamSize();
};

