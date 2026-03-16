#pragma once
#include "Character.h"
class CDefaultCharacter : public CCharacter
{
public:
	CDefaultCharacter(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax);
	~CDefaultCharacter();
};

