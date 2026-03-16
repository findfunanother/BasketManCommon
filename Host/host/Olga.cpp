#include "Olga.h"
#include "Host.h"
#include "DataManagerBalance.h"
#include "Character.h"

COlga::COlga(CHost* pHost, CDataManagerBalance* pBalanceData, F4PACKET::SPlayerInformationT* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo,
	DHOST_TYPE_UINT32 playerNumber, DHOST_TYPE_FLOAT shotBonusMax) :
	CCharacter(pHost, pBalanceData, pInfo, pAniInfo, pBallInfo, playerNumber)
{
	m_CharacterPassive->valueMax = shotBonusMax; //

	if (shotBonusMax == 0.0f)
	{
		m_CharacterPassive->valueMax = 0.20f;
	}
}

COlga::~COlga()
{
}


DHOST_TYPE_BOOL COlga::BeginValidatePassiveValue(DHOST_TYPE_FLOAT passiveCurrent, DHOST_TYPE_FLOAT passiveMax)
{

	m_CharacterPassive->valueCurr = passiveCurrent;

	// 1. 클라이언트에서 오는 맥스값이 테이블값보다 클 수 없다
	if (m_CharacterPassive->valueMax < passiveMax)
	{
		m_bCheckPassiveValidity = false;
		return m_bCheckPassiveValidity;
	}

	// 2. 클라이언트에서 오는 현재 패시브 값이 서버의 맥스 값보다 클 수 없다 
	if ( m_CharacterPassive->valueMax < passiveCurrent )
	{
		m_bCheckPassiveValidity = false;
		return m_bCheckPassiveValidity;
	}

	m_bCheckPassiveValidity = true;
	return m_bCheckPassiveValidity;
}

void COlga::EndValidatePassiveValue(F4PACKET::SHOT_TYPE shotType)
{
	float stackDelta = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Olga_Success");

	float stackMax = m_pBalanceData->GetValue("VPassive_JumpShotProbability_Olga_Stack");

	stackMax = stackMax <= 0 ? 0.00001f : stackMax;

	float stackNew = m_CharacterPassive->valueCurr + stackDelta;
	stackNew = stackNew < 0 ? 0 : stackNew;

	stackNew = stackNew > stackMax ? stackMax : stackNew;

	/*
	if (stackNew == m_CharacterPassive->valueCurr)
		return;*/

	float rate = stackNew / stackMax;

	DHOST_TYPE_BOOL effecttrigger = false;
	DHOST_TYPE_BOOL activate = false;

	if (rate >= 1.0f)
	{
		effecttrigger = true;
	}

	if (stackNew > 0.0f)
	{
		activate = true;
	}

	//SendData(Type, stackNew, stackMax, stackNew > 0, rate >= 1);

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, play_s2c_playerPassive, message, send_data);

	send_data.add_idplayer(m_Information->id);
	send_data.add_type(CHARACTER_PASSIVE_TYPE::shot);
	send_data.add_valuecurr(stackNew);
	send_data.add_valuemax(stackMax);
	send_data.add_activate(activate);
	send_data.add_effecttrigger(effecttrigger);
	send_data.add_starttime(0.0f);
	send_data.add_hostpermit(true);
	STORE_FBPACKET(builder, message, send_data)
	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

DHOST_TYPE_FLOAT COlga::GetBonusShotPassiveCurrent()
{
	return m_CharacterPassive->valueCurr;
}

DHOST_TYPE_FLOAT COlga::GetBonusShotPassiveMax()
{
	return m_CharacterPassive->valueMax;
}



