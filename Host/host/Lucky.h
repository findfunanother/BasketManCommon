#pragma once
#include "Character.h"


class CLucky : public CCharacter
{
public:
	CLucky(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax, DHOST_TYPE_FLOAT fDeltaJumpshot2Value, DHOST_TYPE_FLOAT fDeltaJumpshot3Value);
	~CLucky();

private:
	DHOST_TYPE_FLOAT m_fDeltaJumpshot2Value;
	DHOST_TYPE_FLOAT m_fDeltaJumpshot3Value;

public:
	DHOST_TYPE_BOOL		SetPacketCharacterPassive(const F4PACKET::play_c2s_playerPassive_data* pInfo) override;
	void				SetCheckPassiveValidity(DHOST_TYPE_BOOL flag) override { m_bCheckPassiveValidity = flag; }
	DHOST_TYPE_FLOAT	GetBonusShotPassiveCurrent() override;     
	DHOST_TYPE_FLOAT	GetBonusShotPassiveMax() override;          
	DHOST_TYPE_BOOL		BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax) override;
	void				EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType) override;
	DHOST_TYPE_BOOL		IsValidatePassiveValue(DHOST_TYPE_FLOAT valueCurr) override;      // 2. 확인이 되면 노골이 되거나 다른 것을 못하게 함 

};

