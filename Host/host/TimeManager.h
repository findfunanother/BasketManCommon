#pragma once

#include "Util.h"

class CHost;

class CTimeManager	//! 게임시간과 관련된 정보들을 관리하는 클래스
{
public:

	CTimeManager();
	~CTimeManager();

	void TimeManagerUpdate(DHOST_TYPE_GAME_TIME_F timeDelta);

	DHOST_TYPE_GAME_TIME_F GetGameTime() { return m_fGameTime; }
	void SetGameTime(DHOST_TYPE_GAME_TIME_F value) { m_fGameTime = value; }

	DHOST_TYPE_GAME_TIME_F GetGameTimeInit() { return m_fGameTimeInit; }
	void SetGameTimeInit(DHOST_TYPE_GAME_TIME_F value) { m_fGameTimeInit = value;  m_fGameTime = value; }

	void SetHostCreateTime(Util::timeclock_type value) { m_fHostCreateTime = value; }
	DHOST_TYPE_GAME_TIME_F GetRoomElapsedTime() { return Util::GetElapsedTime(m_fHostCreateTime); }

	DHOST_TYPE_BOOL GetOverTime() { return m_bOverTime; }
	void SetOverTime(DHOST_TYPE_BOOL value) { m_bOverTime = value; }

	DHOST_TYPE_GAME_TIME_F GetShotClock() { return m_fShotClock; }
	void SetShotClock(DHOST_TYPE_GAME_TIME_F value) { m_fShotClock = value; }
	void ResetShotClock() { m_fShotClock = m_fShotClockInit; }

	DHOST_TYPE_GAME_TIME_F GetReplayTime() { return m_fReplayTime; }
	void SetReplayTime(DHOST_TYPE_GAME_TIME_F value) { m_fReplayTime = value; }

	DHOST_TYPE_GAME_TIME_F GetLastPacketReceiveTime() { return m_LastPacketReceiveTime; }
	void SetLastPacketReceiveTime(DHOST_TYPE_GAME_TIME_F value) { m_LastPacketReceiveTime = value; }

private:

	Util::timeclock_type m_fHostCreateTime;
	DHOST_TYPE_GAME_TIME_F m_fGameTime;
	DHOST_TYPE_GAME_TIME_F m_fGameTimeInit;
	DHOST_TYPE_BOOL m_bOverTime;
	DHOST_TYPE_GAME_TIME_F m_fShotClock;
	DHOST_TYPE_GAME_TIME_F m_fShotClockInit;
	DHOST_TYPE_GAME_TIME_F m_fReplayTime;
	DHOST_TYPE_GAME_TIME_F m_LastPacketReceiveTime;
};

