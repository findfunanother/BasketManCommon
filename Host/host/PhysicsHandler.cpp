#include "PhysicsHandler.h"
#include "Host.h"

CPhysicsHandler::CPhysicsHandler()
{
	m_RimPosition.mutate_x(RIM_POS_X);
	m_RimPosition.mutate_y(RIM_POS_Y);
	m_RimPosition.mutate_z(RIM_POS_Z());
	
	m_DunkPosition.mutate_x(RIM_POS_X - 0.0001f);
	m_DunkPosition.mutate_y(RIM_POS_Y + BALL_RADIUS * 0.5f);
	m_DunkPosition.mutate_z(RIM_POS_Z());
	
	Init();
}

CPhysicsHandler::~CPhysicsHandler()
{
	ClearSystem();
}

void CPhysicsHandler::Init()
{
	InitPhysics();
}

DHOST_TYPE_BOOL CPhysicsHandler::Is3PointArea(SVector3 position)
{
	float feetDelta = 0.2f;

	if (abs(position.z()) > POINT3_CHECK_Z())
	{
		return (abs(position.x()) - feetDelta) >= POINT3_LENGTH_SIDE;
	}
	else
	{
		return sqrtf(powf(RIM_POS_X - position.x(), 2) + powf(RIM_POS_Z() - position.z(), 2)) - feetDelta > POINT3_LENGTH;
	}
}

DHOST_TYPE_BOOL CPhysicsHandler::Is3PointArea(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	if (abs(z) > POINT3_CHECK_Z())
	{
		return abs(x) >= POINT3_LENGTH_SIDE;
	}
	else
	{
		return sqrtf(powf(RIM_POS_X - x, 2) + powf(RIM_POS_Z() - z, 2)) > POINT3_LENGTH;
	}
}

DHOST_TYPE_BOOL CPhysicsHandler::IsPaintZoneArea(SVector3 position)
{
	if (abs(position.z()) > FREE_THROW_LINE_Z)
	{
		return abs(position.x()) <= FREE_THROW_LINE_X;
	}

	return false;
}

DHOST_TYPE_FLOAT CPhysicsHandler::GetShotPositionDegree(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	DHOST_TYPE_FLOAT fDegree = kFLOAT_INIT;
	DHOST_TYPE_FLOAT fRimPosX, fRimPosZ = kFLOAT_INIT;
	fRimPosX = RIM_POS_X;
	fRimPosZ = COURT_WIDTH_HALF();

	fDegree = atan2f(fRimPosZ - z, fRimPosX - x) * 180 / 3.1415f;

	return abs(fDegree);
}

DHOST_TYPE_FLOAT CPhysicsHandler::GetShotPositionDistanceFromRim(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	TB::SVector3 ShotPosition;
	ShotPosition.mutate_x(x);
	ShotPosition.mutate_y(0.0f);
	ShotPosition.mutate_z(z);

	TB::SVector3 ZeroPosition;
	ZeroPosition.mutate_x(RIM_POS_X);
	ZeroPosition.mutate_y(0.0f);
	ZeroPosition.mutate_z(COURT_WIDTH_HALF());

	return VECTOR3_DISTANCE(ShotPosition, ZeroPosition);
}

//! 해당 구역은 아래 기획서 참조
//! https://docs.google.com/document/d/10aw0uCs0XabDD3kImcTAdHFi8L1DocQavGKqzhrDH_I/edit#
DHOST_TYPE_UINT32 CPhysicsHandler::GetShotPositionZone(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z)
{
	DHOST_TYPE_FLOAT degree = degree = GetShotPositionDegree(x, z);

	// 3점슛인지 체크
	if (Is3PointArea(x, z))
	{
		// 림 포지션과 사잇각 체크
		if (degree <= 36.f)
		{
			return static_cast<DHOST_TYPE_UINT32>(10);
		}
		else if (degree > 36 && degree <= 72)
		{
			return static_cast<DHOST_TYPE_UINT32>(11);
		}
		else if (degree > 72 && degree <= 108)
		{
			return static_cast<DHOST_TYPE_UINT32>(12);
		}
		else if (degree > 108 && degree <= 144)
		{
			return static_cast<DHOST_TYPE_UINT32>(13);
		}
		else
		{
			return static_cast<DHOST_TYPE_UINT32>(14);
		}
	}
	else
	{
		// 2. 거리체크
		SVector3 ShotPosition;
		ShotPosition.mutate_x(x);
		ShotPosition.mutate_y(0.0f);
		ShotPosition.mutate_z(z);

		SVector3 ZeroPosition;
		ZeroPosition.mutate_x(RIM_POS_X);
		ZeroPosition.mutate_y(0.0f);
		ZeroPosition.mutate_z(COURT_WIDTH_HALF());

		DHOST_TYPE_FLOAT distanceOwnerRim = VECTOR3_DISTANCE(ShotPosition, ZeroPosition);
		if (distanceOwnerRim <= 2.5f)
		{
			return static_cast<DHOST_TYPE_UINT32>(1);
		}
		else if (distanceOwnerRim <= 5.5f)
		{
			// 림 포지션과 사잇각 체크
			if (degree <= 60.f)
			{
				return static_cast<DHOST_TYPE_UINT32>(2);
			}
			else if (degree > 60 && degree <= 120)
			{
				return static_cast<DHOST_TYPE_UINT32>(3);
			}
			else
			{
				return static_cast<DHOST_TYPE_UINT32>(4);
			}
		}
		else
		{
			// 림 포지션과 사잇각 체크
			if (degree <= 36.f)
			{
				return static_cast<DHOST_TYPE_UINT32>(5);
			}
			else if (degree > 36 && degree <= 72)
			{
				return static_cast<DHOST_TYPE_UINT32>(6);
			}
			else if (degree > 72 && degree <= 108)
			{
				return static_cast<DHOST_TYPE_UINT32>(7);
			}
			else if (degree > 108 && degree <= 144)
			{
				return static_cast<DHOST_TYPE_UINT32>(8);
			}
			else
			{
				return static_cast<DHOST_TYPE_UINT32>(9);
			}
		}
	}

	return static_cast<DHOST_TYPE_UINT32>(0);
}

bool CPhysicsHandler::InitPhysics()
{
	m_pSystem = CreateJPSystem();
	if (nullptr == m_pSystem)
	{
		return false;
	}

	return true;
}

void CPhysicsHandler::ClearSystem()
{
	DeleteJPSystem(m_pSystem);
}
