#pragma once
#include "Character.h"

class CMina : public CCharacter
{

public:
	CMina(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax);
	~CMina();

private:

public:
	void OnTurnOver(DHOST_TYPE_CHARACTER_SN turnoverCauser, DHOST_TYPE_CHARACTER_SN turnoverTarget) override;

	DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) override;
	DHOST_TYPE_FLOAT GetBonusShotPassiveCurrent() override;
	DHOST_TYPE_FLOAT GetBonusShotPassiveMax() override;
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	void			 EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType) override;

	DHOST_TYPE_BOOL SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;


};