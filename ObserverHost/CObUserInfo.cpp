#include "pch.h"
#include "CObUserInfo.h"

CObUserInfo::CObUserInfo(unsigned int UserID, void* peer)
{
	m_UserID = UserID;
	m_Peer = peer;
}

CObUserInfo::~CObUserInfo()
{
}
