#pragma once
#include "Host.h"

class CHost;

class CEnvironmentObject
{
private:
	CHost* m_pHost;

	int catMoveNumber;

	bool activePigeon;
	TB::SVector3 positionDestCat;

public:
	float TimePigeonAppear;
	float TimeCatMove;

public:
	CEnvironmentObject(CHost * host);

	void CatReserveMove();
	bool CatSetMove(int moveNumber, TB::SVector3 positionDest, bool run);

	void PigeonReserveAppear();
	bool PigeonSetAppear(bool appear);

	void Update(float timeDelta);
};

