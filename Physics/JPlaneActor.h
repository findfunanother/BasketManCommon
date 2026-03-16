#pragma once
#include "JPSApi.h"
#include "string"

class JPS_API JPlaneActor
{
public:
	JPlaneActor(JPSystem* pSystem);
	~JPlaneActor();

private:
	JPSystem* m_pSystem;
	CJxActor*	 m_pActor;

public:
	void CreateActor(float fDistance, const JOVECTOR3& vecNormal, std::string name);
	void	Destroy();
};

