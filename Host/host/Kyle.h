#pragma once

#include "Character.h"


class CKyle : public CCharacter
{
public:
	CKyle(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax);
	~CKyle();

private:
	float				m_fPlayTimeElapsed;

	EGAME_SCORE_STATUS  m_currentScoreStatus;
	DHOST_TYPE_BOOL     m_superStarMode;
	DHOST_TYPE_BOOL     m_PlayedCeremony;


	void UpdateCharacter(DHOST_TYPE_FLOAT timeDelta) override;
	void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) override;
	void OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime) override;
	void OnCharacterPlayerCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time) override;

public:
	DHOST_TYPE_BOOL  SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;
	DHOST_TYPE_FLOAT GetPassiveBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none) override;
};


