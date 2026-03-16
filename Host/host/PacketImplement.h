#ifndef __TemplateTest__Calc__
#define __TemplateTest__Calc__

#include "PacketBase.h"

template<typename BUFID>
class CPacketImplement :
    public CPacketBase
{
protected:
	
public:
	CPacketImplement(BUFID packetID);
	virtual ~CPacketImplement();

	virtual DHOST_TYPE_BOOL Decode(const char* pBuf, DHOST_TYPE_INT32 size) = 0;
	virtual DHOST_TYPE_BOOL Encode(char* pBuf, size_t size) = 0;
	virtual DHOST_TYPE_INT32 GetEncodeStreamSize() = 0;
protected:
	//BUFFERTYPE* m_message;
};

#include "PacketImplement.hpp"
#endif