#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <deque>
#include <map>

#include "HostDefine.h"
#include "JPSApi.h"
#include "JOMath3d.h"
#include "JPhysics.h"
#include "JEnviroment.h"
#include "JPhysicActor.h"
#include "Common_generated.h"

class CHost;

class CPhysicsHandler
{
public:
	CPhysicsHandler();
	~CPhysicsHandler();

	void Init();

	bool InitPhysics();
	
	void ClearSystem();

	DHOST_TYPE_BOOL Is3PointArea(TB::SVector3 position);
	DHOST_TYPE_BOOL Is3PointArea(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);

	DHOST_TYPE_BOOL IsPaintZoneArea(TB::SVector3 position);

	TB::SVector3 GetRimPosition() { return m_RimPosition; }
	TB::SVector3 GetDunkPosition() { return m_DunkPosition; }

	DHOST_TYPE_FLOAT GetShotPositionDegree(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);
	DHOST_TYPE_FLOAT GetShotPositionDistanceFromRim(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);
	DHOST_TYPE_UINT32 GetShotPositionZone(DHOST_TYPE_FLOAT x, DHOST_TYPE_FLOAT z);

private:

	JPSystem* m_pSystem;

	TB::SVector3 m_RimPosition;
	TB::SVector3 m_DunkPosition;
	
};