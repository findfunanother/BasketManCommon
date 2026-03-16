#pragma once
#include "Character.h"


class CAldo : public CCharacter
{
public:
	CAldo(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax);
	~CAldo();
};
