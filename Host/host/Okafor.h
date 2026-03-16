#pragma once
#include "Character.h"

class COkafor : public CCharacter
{
public:
	COkafor(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveMax);
	~COkafor();

private:
	float				m_fPlayTimeElapsed;

	float				m_Passive_Stack_TryRimAttack;
	float				m_Passive_Stack_SucessRimAttack;
	float				m_Passive_Stack_SucessJumpShot;
	int					m_Passive_Stack_FieldGoalCountMax;
	int					m_Passive_Stack_FieldGoalCount;
	

	void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) override;

	//
	void OnPlayerShot(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SHOT_TYPE value) override;
	void OnGoalResult(DHOST_TYPE_CHARACTER_SN CharacterSN, F4PACKET::SHOT_TYPE value, DHOST_TYPE_BOOL result) override;
	void UpdateCharacter(DHOST_TYPE_FLOAT timeDelta) override;

	void ClearStack();

	// 
	void IncreaseStack(float value);


public:
	void SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) override { m_bCheckPassiveValidity = flag; }
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	DHOST_TYPE_BOOL  SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;
	DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) override;
};