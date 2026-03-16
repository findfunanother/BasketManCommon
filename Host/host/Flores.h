#pragma once
#include "Character.h"

class CFlores : public CCharacter
{
public:
	CFlores(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT passiveDuration);
	~CFlores();

private:
	float				m_fPlayTimeElapsed;
	DHOST_TYPE_FLOAT	m_fPassiveDuration;
	DHOST_TYPE_BOOL     m_PlayedCeremony;

	void OnCharacterPassive(void* pData, DHOST_TYPE_GAME_TIME_F time) override;
	void OnGoalFailed(std::vector<DHOST_TYPE_CHARACTER_SN> vBallShotHindrance) override;
	void OnTurnOver(DHOST_TYPE_CHARACTER_SN turnoverCauser, DHOST_TYPE_CHARACTER_SN turnoverTarget) override;
	void OnCharacterPlayerCoupleCeremony(void* pData, DHOST_TYPE_GAME_TIME_F time) override;
	void OnCharacterRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime) override;

	void OnPlayerShotHindrance(DHOST_TYPE_CHARACTER_SN characterSN) override;
	void OnPlayerBlock(DHOST_TYPE_CHARACTER_SN blockCharacterSN, DHOST_TYPE_CHARACTER_SN blockedCharacterSN, DHOST_TYPE_BOOL bSuccess) override;
	

	void ResultBallShotHindrance();
	void ResultTeamShotRate(DHOST_TYPE_CHARACTER_SN buffCharacterSN);


public:
	void SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) override { m_bCheckPassiveValidity = flag; }
	DHOST_TYPE_BOOL  BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	DHOST_TYPE_BOOL  SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;

};

