#include "FlatBufPacketImplement.h"


CFlatBufPacketImplement::CFlatBufPacketImplement(const F4PACKET::PACKET_ID packetID)
	:CPacketImplement(packetID)
{
	m_isFlatbuf = true;
}

CFlatBufPacketImplement::~CFlatBufPacketImplement()
{
}


DHOST_TYPE_BOOL CFlatBufPacketImplement::Decode(const char* pBuf, DHOST_TYPE_INT32 size)
{
	
	return false;// flatbuffers::GetMutableRoot(pBuf, size);
}


DHOST_TYPE_BOOL CFlatBufPacketImplement::Encode(char* pBuf, size_t size)
{
	return false;
}


DHOST_TYPE_INT32 CFlatBufPacketImplement::GetEncodeStreamSize()
{
	//if (nullptr == m_message)
		return 0;

	//return (DHOST_TYPE_INT32)m_message->ByteSizeLong();
}
