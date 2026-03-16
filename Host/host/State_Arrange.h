#pragma once
#include "State.h"
#include <float.h>

class CState_Arrange : public CState
{
public:
	CState_Arrange(CHost* host) : CState(host)
	{
		position[0].mutate_x(0.0f);
		position[0].mutate_y(0.0f);
		position[0].mutate_z(2.0f);

		position[1].mutate_x(-5.0f);
		position[1].mutate_y(0.0f);
		position[1].mutate_z(3.0f);

		position[2].mutate_x(5.0f);
		position[2].mutate_y(0.0f);
		position[2].mutate_z(3.0f);

		position[3].mutate_x(0.0f);
		position[3].mutate_y(0.0f);
		position[3].mutate_z(5.0f);

		position[4].mutate_x(-4.0f);
		position[4].mutate_y(0.0f);
		position[4].mutate_z(6.0f);
		
		position[5].mutate_x(4.0f);
		position[5].mutate_y(0.0f);
		position[5].mutate_z(6.0f);

		position[6].mutate_x(0.0f);
		position[6].mutate_y(0.0f);
		position[6].mutate_z(5.0f);

		position[7].mutate_x(-4.0f);
		position[7].mutate_y(0.0f);
		position[7].mutate_z(6.0f);

		position[8].mutate_x(4.0f);
		position[8].mutate_y(0.0f);
		position[8].mutate_z(6.0f);
	}

	virtual void OnEnter(void* pData);
	virtual void OnExit();
	virtual void OnUpdate(float timeDelta);
	virtual DHOST_TYPE_BOOL OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time);

protected:
	DECLARATION_PACKET_PROCESS(system_c2s_arrangeReady);
	DECLARATION_PACKET_PROCESS(play_c2s_ballClear);

protected:
	float timeArrange = 0.0f;
	float timeElapsed = 0.0f;
	int count = 0;
	
	SVector3 position [9];
	
	int startIndex;

	int32_t SetPlayers(int startIndex, DHOST_TYPE_INT32 offenseTeam, F4PACKET::system_s2c_arrange_dataBuilder* gameReadyToPlay);
};

