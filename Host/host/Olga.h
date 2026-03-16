#pragma once
#include "Character.h"

class COlga : public CCharacter
{

public:
	COlga(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax);
	~COlga();

private:

public:
	DHOST_TYPE_FLOAT GetBonusShotPassiveCurrent() override;
	DHOST_TYPE_FLOAT GetBonusShotPassiveMax() override;
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	void			 EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType) override;

};

