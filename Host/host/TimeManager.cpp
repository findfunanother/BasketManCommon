#include "TimeManager.h"

CTimeManager::CTimeManager()
{
	m_fHostCreateTime = Util::GetNow();
	m_fGameTimeInit = kGAME_TIME_INIT;
	m_fGameTime = m_fGameTimeInit;
	m_bOverTime = false;
	m_fShotClock = kFLOAT_INIT;
	m_fShotClockInit = kSHOT_CLOCK_TIME_INIT;
	m_fReplayTime = kGAME_TIME_INIT;
	m_LastPacketReceiveTime = kFLOAT_INIT;
}

CTimeManager::~CTimeManager()
{

}

void CTimeManager::TimeManagerUpdate(DHOST_TYPE_GAME_TIME_F timeDelta)
{

}