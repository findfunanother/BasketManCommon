#pragma once
#include "Character.h"


class CThompson : public CCharacter
{
public:
	CThompson(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber);
	~CThompson();

private:
	DHOST_TYPE_FLOAT value1;
	DHOST_TYPE_FLOAT value2;
	DHOST_TYPE_FLOAT value3;
public:
	DHOST_TYPE_BOOL		InitialPassiveDataSet(const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo) override;
};
