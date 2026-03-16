#include "PacketImplement.h"

template<typename BUFID>
CPacketImplement<BUFID>::CPacketImplement(BUFID packetID)
{
	SetPacketID((DHOST_TYPE_INT32)packetID);
	//m_message = message;
	SetPacketType(kPACKET_TYPE_ALL_WORK);
}

template<typename BUFID>
CPacketImplement<BUFID>::~CPacketImplement()
{

}
