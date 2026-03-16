#pragma once
#include "State.h"

class CState_Score : public CState
{
public:
	CState_Score(CHost* host) : CState(host) {}

	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(play_c2s_playerEmotion);
	DECLARATION_PACKET_PROCESS(play_c2s_ballLerp);
	DECLARATION_PACKET_PROCESS(play_c2s_ballShot);
	DECLARATION_PACKET_PROCESS(play_c2s_ballSimulation);
	DECLARATION_PACKET_PROCESS(play_c2s_ballRebound);
	DECLARATION_PACKET_PROCESS(play_c2s_playerCeremonyEnd);
	DECLARATION_PACKET_PROCESS(play_c2s_playerCoupleCeremony);
	DECLARATION_PACKET_PROCESS(play_c2s_playerMove)
	DECLARATION_PACKET_PROCESS(play_c2s_playerStand);

private:
	DHOST_TYPE_CHARACTER_SN solo = kCHARACTER_SN_INIT;
	DHOST_TYPE_CHARACTER_SN duo = kCHARACTER_SN_INIT;
	
	//vector<DHOST_TYPE_CHARACTER_SN> coupleCeremonyCandidate;
	vector<DHOST_TYPE_CHARACTER_SN> coupleCeremony;
	//bool isSendCoupleCeremony = false;

	// 버저비터 하이라이트를 위해 클라이언트와 맞춘 변수 들 (steven, 2020-08-23)
	const float startAnimationTime = 1.0f;
	const float balloutElapsedTime = 0.7f; // 버저비터 1, 2 번 카메라는 이시간까지만 보여줘야 함 
	const float scaleTimeLastCamera = 2.0f;
	const float serverWaitTimeAfterGoalIn = 0.5f;

	float timeCeremony = 0.0f;
	float timeStop = 0.0f;
	float timeWait = 0.0f;
	float timeElapsed = 0.0f;
	//float timeTemp = 0.0f;
	
	bool sendStopPacket = false;
	bool m_replay = false;
	bool m_CheckReplay = false;
	bool m_BuzzerBeater = false;
};

