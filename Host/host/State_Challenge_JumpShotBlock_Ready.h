#pragma once

#include "State.h"
#include "State_Challenge_JumpShotBlock.h"

class CState_Challenge_JumpShotBlock_Ready : public CState_Challenge_JumpShotBlock
{
public:
	CState_Challenge_JumpShotBlock_Ready(CHost* host) : CState_Challenge_JumpShotBlock(host)
	{
		side = 1;

		positionBalls[0].mutate_x(-7.0f);
		positionBalls[0].mutate_y(0.0f);
		positionBalls[0].mutate_z(10.0f);

		positionBalls[1].mutate_x(-4.0f);
		positionBalls[1].mutate_y(0.0f);
		positionBalls[1].mutate_z(4.0f);

		positionBalls[2].mutate_x(0.0f);
		positionBalls[2].mutate_y(0.0f);
		positionBalls[2].mutate_z(3.0f);

		positionBalls[3].mutate_x(4.0f);
		positionBalls[3].mutate_y(0.0f);
		positionBalls[3].mutate_z(4.0f);

		positionBalls[4].mutate_x(7.0f);
		positionBalls[4].mutate_y(0.0f);
		positionBalls[4].mutate_z(10.0f);
	}

public:
	virtual void OnEnter(void* pData);
	virtual void OnUpdate(float timeDelta);

protected:
	SVector3 positionBalls[5];

	float m_timeRemain;
	int m_timeInt;
	int side;
};

