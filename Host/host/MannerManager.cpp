#include "MannerManager.h"
#include "CharacterManager.h"

CMannerManager::CMannerManager(CCharacterManager* pInfo) : m_pOwner(pInfo)
{
	m_sData = { kFLOAT_INIT };

	m_pOwner->GetBadMannerBalanceData(m_sData);

	m_sBallHogPlayInfo = { kCHARACTER_SN_INIT, kFLOAT_INIT, kBALL_NUMBER_INIT };

	m_mapAwayFromKeyboardInfo.clear();
}

CMannerManager::~CMannerManager()
{

}

void CMannerManager::MannerManagerUpdate(DHOST_TYPE_FLOAT gameTime)
{
	// 의도적인 볼 호그 플레이
	if (gameTime > m_sData.IntentionalBallHogPlayRemainGameTime && m_sBallHogPlayInfo.OwnBallTime - gameTime > m_sData.IntentionalBallHogPlayOwnBall)
	{
		m_pOwner->IncreaseBadMannerCount(m_sBallHogPlayInfo.CharacterSN, EBAD_MANNER_TYPE::INTENTIONAL_BALL_HOG_PLAY);
		m_sBallHogPlayInfo.OwnBallTime = gameTime;
	}

	// 자리 비움 체크
	for (std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapAwayFromKeyboardInfo.begin(); iter != m_mapAwayFromKeyboardInfo.end(); ++iter)
	{
		CCharacter* pCharacter = m_pOwner->GetCharacter(iter->first);

		if (pCharacter == nullptr) continue;

		if (iter->second > kFLOAT_INIT && iter->second - gameTime > pCharacter->GetAutoPlayWatingTime())
		{
			m_pOwner->IncreaseBadMannerCount(iter->first, EBAD_MANNER_TYPE::AWAY_FROM_KEYBOARD);
			m_pOwner->UpdateAwayFromKeyboardGameTime(gameTime, iter->first);

			iter->second = gameTime;
		}
	}
}

void CMannerManager::InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_sBallHogPlayInfo = { characterSN, gameTime, ballNo };
}

// 의도적인 볼 호그 플레이
void CMannerManager::UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo)
{
	if (gameTime > m_sData.IntentionalBallHogPlayRemainGameTime && m_sBallHogPlayInfo.BallNo != ballNo)
	{
		m_sBallHogPlayInfo.CharacterSN = characterSN;
		m_sBallHogPlayInfo.OwnBallTime = gameTime;
		m_sBallHogPlayInfo.BallNo = ballNo;
	}
}

// 자리비움
void CMannerManager::UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime)
{
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapAwayFromKeyboardInfo.find(characterSN);

	if (iter == m_mapAwayFromKeyboardInfo.end())
	{
		m_mapAwayFromKeyboardInfo.insert(std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>(characterSN, gameTime));
	}
	else
	{
		iter->second = gameTime;
	}
}

void CMannerManager::ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN)
{
	UpdateAwayFromKeyboard(characterSN, gameTime);

	m_pOwner->UpdateAwayFromKeyboardTimeAccumulate(gameTime, characterSN);
}

std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> CMannerManager::GetAwayFromKeyboardInfo()
{
	return m_mapAwayFromKeyboardInfo;
}

DHOST_TYPE_FLOAT CMannerManager::FindAwayFromKeyboardInfo(DHOST_TYPE_CHARACTER_SN characterSN)
{
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapAwayFromKeyboardInfo.find(characterSN);

	if (iter != m_mapAwayFromKeyboardInfo.end())
	{
		return iter->second;
	}

	return kFLOAT_INIT;
}

// 불가능한 슛팅시도
void CMannerManager::BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value)
{
	if (m_sData.ImpossibleShootingShotClock > kFLOAT_INIT)
	{
		if (shotClock > m_sData.ImpossibleShootingShotClock && value < kFLOAT_INIT)
		{
			m_pOwner->IncreaseBadMannerCount(characterSN, EBAD_MANNER_TYPE::IMPOSSIBLE_SHOOTING);
		}
	}
}