#pragma once
#include "State.h"

class CState_Ready : public CState
{
public:
	CState_Ready(CHost* host) : CState(host) 
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

protected:
	SVector3 position [9];
	
	float timeElapsed;
	float timeWait;
	int timeCount;

	int32_t SetPlayers(int startIndex, DHOST_TYPE_INT32 offenseTeam, F4PACKET::system_s2c_readyToPlay_dataBuilder* gameReadyToPlay);

	DECLARATION_PACKET_PROCESS(play_c2s_playerMove);
	DECLARATION_PACKET_PROCESS(play_c2s_playerStand);
};

