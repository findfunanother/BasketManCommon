

#pragma once
#include "Character.h"

class CLinCC : public CCharacter
{
public:
	CLinCC(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveMax);
	~CLinCC();

private:
	float				m_fPlayTimeElapsed;

	float				m_Passive_Stamina_LinCC_Reduce;
	float				m_Passive_Stamina_LinCC_Dash;
	float				m_Passive_StaminaStandard_LinCC;

	void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) override;

	//
	void OnPlayerShot(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SHOT_TYPE value) override;
	void OnPlayerPenentrate(DHOST_TYPE_CHARACTER_SN characterSN) override;
	void OnPlayerDash(DHOST_TYPE_CHARACTER_SN characterSN) override;
	void UpdateCharacter(DHOST_TYPE_FLOAT timeDelta) override;

	// 
	void DecreaseStack(float value);


public:
	void SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) override { m_bCheckPassiveValidity = flag; }
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	DHOST_TYPE_BOOL  SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;
	DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) override;

};

