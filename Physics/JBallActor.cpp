//#include "stdafx.h"
#include "JEnviroment.h"
#include "JPhysicActor.h"
#include "JBallActor.h"
#include "JxSphere.h"
#include "JxScene.h"
#include "JxActors.h"
#include "JxPhysics.h"


static unsigned int uiBallID = 0;

JBallActor::JBallActor(JPSystem* pSystem)
{
	m_pSystem = pSystem;
}


JBallActor::~JBallActor()
{
	Destroy();
}


void JBallActor::CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall)
{
	uiBallID++;

	//CJxsDesc* pJxsDesc = new CJxsDesc;
	//pJxsDesc->fMass = 0.50f;
	//pJxsDesc->fRadius = BALL_RADIUS;
	//pJxsDesc->pJxUserContactReport = (CJxUserContactReport*)m_pSystem->GetContactReport();
	//m_pActor = new CJxSphere(m_pSystem->GetScene(), pJxsDesc);
	//m_pActor->_vecPos = vPos;
	//m_pActor->_vecVelocity = vInitialVelocity;
	////char Buffer[256];
	////sprintf_s(Buffer, "Ball%d", uiBallID);
	//m_pActor->actorName = "BallActor";

	//m_pActor->SetCollisionGroup(COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_C | COLLISION_GROUP_D);

	////m_pSystem->GetJxControllerManager()->AddJxActor(m_pActor); 

	//m_pActor->userData = pBall;
}

void JBallActor::Destroy()
{
	if (m_pSystem)
	{
		m_pSystem->GetScene()->RemoveJxActor(m_pActor);
		//m_pSystem->GetJxControllerManager()->RemoveJxActor(m_pActor);
		delete m_pActor;
	}
}


void JBallActor::OnCollisionEvent(void* pParam1, void* pParam2, void* pParam3, const char* ObjectName)
{
	// 아래는 무엇에 쓰는 물건인고?
	//int a = 4;
}


void JBallActor::SetPos(float pos[])
{
	m_pActor->_vecPos.fX = pos[0];
	m_pActor->_vecPos.fY = pos[1];
	m_pActor->_vecPos.fZ = pos[2];
}

void JBallActor::SetVelocity(float velocity[])
{
	m_pActor->_vecVelocity.fX = velocity[0];
	m_pActor->_vecVelocity.fY = velocity[1];
	m_pActor->_vecVelocity.fZ = velocity[2];
}


//////////////////////////////////

/*
void JBallActorEx::CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall)
{

}

void JBallActorEx::Destroy()
{
}

*/

