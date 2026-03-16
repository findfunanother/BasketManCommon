#include "EnvironmentObject.h"

CEnvironmentObject::CEnvironmentObject(CHost * host)
{
	m_pHost = host;

	positionDestCat.mutate_x(10.0f);
	positionDestCat.mutate_y(0.0f);
	positionDestCat.mutate_z(0.0f);

	catMoveNumber = 0;

	CatReserveMove();
	PigeonReserveAppear();
}

void CEnvironmentObject::CatReserveMove()
{
	positionDestCat.mutate_x((((float)rand() / RAND_MAX) * 16.0f - 8.0f));
	positionDestCat.mutate_y(0.0f);
	positionDestCat.mutate_z((((float)rand() / RAND_MAX) * 12.0f + 1.0f));

	TimeCatMove = m_pHost->GetGameTime() - (((float)rand() / RAND_MAX) * 20.0f + 40.0f);
}

bool CEnvironmentObject::CatSetMove(int moveNumber, TB::SVector3 positionDest, bool run)
{
	bool retValue = false;

	if (moveNumber == catMoveNumber)
	{
		catMoveNumber++;

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_environmentCat, message, send_data);
		send_data.add_number(catMoveNumber);
		send_data.add_runmode(run);
		send_data.add_positiondest(&positionDest);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		CatReserveMove();

		retValue = true;
	}

	return retValue;
}

void CEnvironmentObject::PigeonReserveAppear()
{
	TimePigeonAppear = m_pHost->GetGameTime() - (((float)rand() / RAND_MAX) * 10.0f + 10.0f);

	activePigeon = false;
}

bool CEnvironmentObject::PigeonSetAppear(bool appear)
{
	bool retValue = false;

	if (appear != activePigeon)
	{
		int appearType;

		if (appear == true)
		{
			appearType = rand() % 2 + 2;
		}
		else
		{
			appearType = rand() % 2;
		}

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_environmentPigeon, message, send_data);
		send_data.add_appeartype(appearType);
		STORE_FBPACKET(builder, message, send_data)
		
		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		activePigeon = appear;

		retValue = true;
	}

	return retValue;
}

void CEnvironmentObject::Update(float timeDelta)
{
	float gameTime = m_pHost->GetGameTime();
	if (activePigeon == false)
	{
		if (gameTime < TimePigeonAppear)
		{
			PigeonSetAppear(true);
		}
	}

	if (gameTime < TimeCatMove)
	{
		CatSetMove(catMoveNumber, positionDestCat, false);
	}
}

