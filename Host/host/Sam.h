#pragma once
#include "Character.h"

class CSam : public CCharacter
{
public:
	CSam(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration);
	~CSam();

private:
	float				m_fPlayTimeElapsed;
	DHOST_TYPE_FLOAT	m_fPassiveDuration;
	void UpdateCharacter(DHOST_TYPE_FLOAT timeDelta) override;
	void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) override;
	void OnPlayerPass(DHOST_TYPE_CHARACTER_SN SendCharacterSN, DHOST_TYPE_CHARACTER_SN RecvCharacterSN, F4PACKET::CHARACTER_BUFF_TYPE buffType) override;

public:
	DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) override;
	void SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) override { m_bCheckPassiveValidity = flag; }
	DHOST_TYPE_FLOAT GetBonusShotPassiveCurrent() override;
	DHOST_TYPE_FLOAT GetBonusShotPassiveMax() override;
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	DHOST_TYPE_BOOL SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;
};

