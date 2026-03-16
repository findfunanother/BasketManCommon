#pragma once

#include "HostDefine.h"

class CCharacterManager;

class CMannerManager
{
public:
	CMannerManager(CCharacterManager* pInfo);
	~CMannerManager();

	void MannerManagerUpdate(DHOST_TYPE_FLOAT gameTime);
	void InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN);
	
	// 의도적인 볼 호그 플레이
	void UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo);

	// 자리비움
	void UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime);
	void ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN);
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> GetAwayFromKeyboardInfo();
	DHOST_TYPE_FLOAT FindAwayFromKeyboardInfo(DHOST_TYPE_CHARACTER_SN characterSN);

	// 불가능한 슛팅시도
	void BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value);

protected:
	CCharacterManager* m_pOwner;
	SBadMannerBalanceData m_sData;
	SBadMannerIntentionalBallHogPlay m_sBallHogPlayInfo;
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> m_mapAwayFromKeyboardInfo;
};