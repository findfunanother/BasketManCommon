#pragma once

#include "State.h"


class CProcessPacket
{

public:
	static CState* m_pState;
	static CHost* m_pHost;

public:
	CProcessPacket();
	~CProcessPacket();
};
