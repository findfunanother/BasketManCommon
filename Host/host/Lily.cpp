#include "Lily.h"

#include "Character.h"

CLily::CLily(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo, DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT passiveValueMax)
	: CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{

	m_CharacterPassive->valueMax = passiveValueMax; //

	if (passiveValueMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 0.30f;
	}

}

CLily::~CLily()
{

}