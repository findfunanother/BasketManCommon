//#include "stdafx.h"
#include "JPlaneActor.h"
#include "JxScene.h"
#include "JxInPlane.h"
#include "JxActors.h"

JPlaneActor::JPlaneActor(JPSystem* pSystem)
{
	m_pSystem = pSystem;
}


JPlaneActor::~JPlaneActor()
{
	Destroy();
}


void JPlaneActor::CreateActor(float fDistance, const JOVECTOR3& vecNormal, std::string name)
{
	//CJxipDesc* pJxipDesc = new CJxipDesc;
	//pJxipDesc->fDistance = fDistance;
	//pJxipDesc->vNormal = vecNormal;
	//m_pActor = new CJxInPlane(m_pSystem->GetScene(), pJxipDesc);
	//m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	//m_pActor->actorName = name;
	//m_pSystem->GetJxControllerManager()->AddJxActor(m_pActor); // 캐릭터 매니저에 포함
}

void JPlaneActor::Destroy()
{
	if (m_pSystem)
	{
		m_pSystem->GetScene()->RemoveJxActor(m_pActor);
		//m_pSystem->GetJxControllerManager()->RemoveJxActor(m_pActor);
		delete m_pActor;
	}
}


