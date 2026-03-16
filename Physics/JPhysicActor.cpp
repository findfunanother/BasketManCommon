//#include "stdafx.h"
#include "JxScene.h"
#include "JPSystem.h"
#include "JPhysicActor.h"
#include "JEnviroment.h"
#include "JxInPlane.h"
#include "JxFiPlane.h"
#include "JxPhysics.h"
#include "JxCylinder.h"
#include "JxField.h"
#include "LogActor.h"

static unsigned int uiBallID = 0;

JPhysicActor::JPhysicActor(JPSystem* pJSystem)
{
	m_pJSystem = pJSystem;
}


JPhysicActor::~JPhysicActor()
{
	Destroy();
}


void JPhysicActor::CreateJActor( int nType )
{
	switch (nType)
	{
	case 0:
		CreateJBallActor(); // 볼 
		break;
	case 1:
		CreateJGroundActor(); // 그라운드 
		break;
	case 2:
		break;
	case 3:

		break;
	default:
		break;
	}
}


void JPhysicActor::CreateJFenceActor(float normal[], float distance, char name[] )
{
	
	//fDistance = distance;
	//vNormal = JOVECTOR3(normal[0], normal[1], normal[2]);
	//fBounceness = GROUNDBOUNCE;
	//bIsCollisonEvent = true; // 공과 충돌이 되게 하자 2019/05/02
	//m_pActor = new CJxInPlane(m_pJSystem->GetScene());
	//m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	//m_pActor->actorName = name;

	//ige::FileLogger myLog("1.0.4.2", "ActorName.txt");

	//myLog << ige::FileLogger::e_logType::LOG_WARNING << &name[0];

}


void JPhysicActor::CreateJFiPlaneActor(float pos[], float normal[], float depthVector[], float width, float depth, char name[])
{
	CJxfpDesc* pJxfpDesc = new CJxfpDesc;

	pJxfpDesc->vPos = JOVECTOR3(pos[0], pos[1], pos[2]);
	pJxfpDesc->vNormal = JOVECTOR3(normal[0], normal[1], normal[2]);
	pJxfpDesc->vDepthVector = JOVECTOR3(depthVector[0], depthVector[1], depthVector[2]);
	pJxfpDesc->fWidth = width;
	pJxfpDesc->fDepth = depth;
	pJxfpDesc->fBounceness = NETBOUNCE;

	//JOVECTOR3 pointMid0 = pJxfpDesc->vPos + pJxfpDesc->vDepthVector.Normalized() * depth;
	//JOVECTOR3 dirRight = CrossProduct(pJxfpDesc->vNormal, depthVector);
	//JOVECTOR3 pointMid1 = pJxfpDesc->vPos + dirRight.Normalized() * width;
	//JOVECTOR3 dirBack = CrossProduct(pJxfpDesc->vNormal, dirRight);
	//JOVECTOR3 pointMid2 = pJxfpDesc->vPos + dirBack.Normalized() * depth;
	//JOVECTOR3 dirLeft = CrossProduct(pJxfpDesc->vNormal, dirBack);
	//JOVECTOR3 pointMid3 = pJxfpDesc->vPos + dirLeft.Normalized() * width;

	//pJxfpDesc->vPoint4[0] = pJxfpDesc->vPos + (pointMid0 - pJxfpDesc->vPos) + (pointMid1 - pJxfpDesc->vPos);
	//pJxfpDesc->vPoint4[1] = pJxfpDesc->vPos + (pointMid1 - pJxfpDesc->vPos) + (pointMid2 - pJxfpDesc->vPos);
	//pJxfpDesc->vPoint4[2] = pJxfpDesc->vPos + (pointMid2 - pJxfpDesc->vPos) + (pointMid3 - pJxfpDesc->vPos);
	//pJxfpDesc->vPoint4[3] = pJxfpDesc->vPos + (pointMid3 - pJxfpDesc->vPos) + (pointMid0 - pJxfpDesc->vPos);

	//m_pActor = new CJxFiPlane(m_pJSystem->GetScene(), pJxfpDesc);
	//m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	//m_pActor->actorName = name;
}


void JPhysicActor::CreateJCylinderActor(float pos[], float normal[], float radius, float height, char name[])
{
	CJxcyDesc* pJxfpDesc = new CJxcyDesc;
	pJxfpDesc->vPos = JOVECTOR3(pos[0], pos[1], pos[2]);
	pJxfpDesc->vNormal = JOVECTOR3(normal[0], normal[1], normal[2]);
	pJxfpDesc->fRadius = radius;
	pJxfpDesc->fHeight = height;
	pJxfpDesc->fBounceness = POSTBOUNCE;
	m_pActor = new CJxCylinder(m_pJSystem->GetScene(), pJxfpDesc);
	m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	m_pActor->actorName = name;
}


void JPhysicActor::CreateF3FieldActor()
{

}

void JPhysicActor::CreateF3FieldActor(float pos[], float normal[], float halfSizeX, float halfSizeZ, float goalPosHeightY, float goalPostWidthZ, char name[])
{
	////CFieldDesc* pJxfieldDesc = new CFieldDesc;
	//_vPos = JOVECTOR3(pos[0], pos[1], pos[2]);
	//vNormal = JOVECTOR3(normal[0], normal[1], normal[2]);
	//fHalfSizeX = halfSizeX;
	//fHalfSizeZ = halfSizeZ;
	//fGoalPostHeightY = goalPosHeightY;
	//fGoalPostWidthZ  = goalPostWidthZ;
	//bIsCollisonEvent = true;

	//m_pActor = new CJxField(m_pJSystem->GetScene(), pJxfieldDesc);
	//m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	//m_pActor->actorName = name;
}

void JPhysicActor::SetBeActive(bool bActive)
{
	m_pActor->SetSleep(bActive);
}

void JPhysicActor::SetSleep(bool bActive)
{
	m_pActor->SetSleep(bActive);
}


void JPhysicActor::CreateJBallActor(EventCallType pCallBack)
{
	uiBallID++;

	//CJxActor* pJxActor = new CJxSphere()
	//CJxsDesc* pJxsDesc = new CJxsDesc;

	//pJxsDesc->fMass = 0.50f;
	//pJxsDesc->fRadius = BALL_RADIUS;
	//pJxsDesc->pJxUserContactReport = (CJxUserContactReport*)m_pJSystem->GetContactReport();
	//m_pActor = new CJxSphere(m_pJSystem->GetScene(), pJxsDesc);

	//m_pActor->_vecPos = JOVECTOR3(0.0f, 5.0f, 0.0f); // 시작 위치 
	//m_pActor->_vecVelocity = JOVECTOR3(5.0f, 5.0f, 0.0f);

	////char Buffer[256];
	////sprintf_s(Buffer, "Ball%d", uiBallID);
	//m_pActor->actorName = "JBallActor";

	//m_pActor->SetCollisionGroup(COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_C | COLLISION_GROUP_D);

	////m_pSystem->GetJxControllerManager()->AddJxActor(m_pActor); 

	//m_pActor->userData = this;

	//EventCallBack = &(*pCallBack);
}


void JPhysicActor::CreateJBallActor()
{
	uiBallID++;

	//CJxActor* pJxActor = new CJxSphere()
	//CJxsDesc* pJxsDesc = new CJxsDesc;
	//pJxsDesc->fMass = 0.50f;
	//pJxsDesc->fRadius = BALL_RADIUS;
	//pJxsDesc->pJxUserContactReport = (CJxUserContactReport*)m_pJSystem->GetContactReport();
	//m_pActor = new CJxSphere(m_pJSystem->GetScene(), pJxsDesc);

	//m_pActor->_vecPos = JOVECTOR3(0.0f, 5.0f, 0.0f); // 시작 위치 
	//m_pActor->_vecVelocity = JOVECTOR3(5.0f, 5.0f, 0.0f);

	////char Buffer[256];
	////sprintf_s(Buffer, "Ball%d", uiBallID);
	//m_pActor->actorName = "JBallActor";

	//m_pActor->SetCollisionGroup(COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_C | COLLISION_GROUP_D);

	////m_pSystem->GetJxControllerManager()->AddJxActor(m_pActor); 

	//m_pActor->userData = this;
}

void JPhysicActor::CreateJGroundActor()
{
	//CJxipDesc* pJxipDesc = new CJxipDesc;
	//

	//pJxipDesc->fDistance = 0.0f;
	//pJxipDesc->vNormal = JOVECTOR3(0.0f, 1.0f, 0.0f);
	//pJxipDesc->fBounceness = GROUNDBOUNCE;
	////pJxipDesc->bIsCollisonEvent = true;
	//m_pActor = new CJxInPlane(m_pJSystem->GetScene(), pJxipDesc);

	//m_pActor->SetCollisionGroup(COLLISION_GROUP_B); // 바닥은 볼하고만 
	//m_pActor->actorName = "Ground";
}


void JPhysicActor::SetPos(float pos[])
{
	m_pActor->_vecPos.fX = pos[0];
	m_pActor->_vecPos.fY = pos[1];
	m_pActor->_vecPos.fZ = pos[2];
}

void JPhysicActor::SetPrePos(float pos[])
{
	m_pActor->_vecPrePos.fX = pos[0];
	m_pActor->_vecPrePos.fY = pos[1];
	m_pActor->_vecPrePos.fZ = pos[2];
}


void JPhysicActor::SetVelocity(float velocity[])
{
	m_pActor->_vecVelocity.fX = velocity[0];
	m_pActor->_vecVelocity.fY = velocity[1];
	m_pActor->_vecVelocity.fZ = velocity[2];
}

void JPhysicActor::Initialzie()
{
	m_pActor->_qOrientation = JOQUATERNION();
}

void JPhysicActor::SetMoment(float pos[])
{
	m_pActor->_vMoment.fX = pos[0];
	m_pActor->_vMoment.fY = pos[1];
	m_pActor->_vMoment.fZ = pos[2];
}

void JPhysicActor::Destroy()
{
	if (m_pJSystem)
	{
		m_pJSystem->GetScene()->RemoveJxActor(m_pActor);
		//m_pSystem->GetJxControllerManager()->RemoveJxActor(m_pActor);
		delete m_pActor;
	}
}

void JPhysicActor::OnCollisionEvent(void* pParam1, void* pParam2, void* pParam3, const char* ObjectName)
{
	//CJxSphere* pSphere = (CJxSphere*)pParam1;
	//int nEventNum = *((int*)pParam3);

	//if (EventCallBack)
	//{
	//	EventCallBack(pParam1, pParam2, pParam3, ObjectName);
	//}
	if (EventCallBackHandler)
	{
		EventCallBackHandler(pParam1, pParam2, pParam3, ObjectName);
	}
}



