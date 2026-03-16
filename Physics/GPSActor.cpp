//#include "stdafx.h"

#include "GPSActor.h"
#include "JPSystem.h"
//#include "CharacterControl.h"
#include "JxControllerManager.h"
#include "JxController.h"
#include "JxSphere.h"
#include "JxScene.h"
#include "ContactReport.h"

//ControllerHitReport  gControllerHitReport;


//// About GPS2Character //////////////////////////////////////////

//static unsigned int uiCharID = 0;

GPS2Character::GPS2Character(void)
{

}


GPS2Character::~GPS2Character(void)
{
	Destroy();
}


//int GPS2Character::CreateActor(const JOVECTOR3& vPos , float fHeight , float fRadius, ControllerHitReport* pHitReport , void* pCharacter)
//{
//	CJxcDesc* pJxcDesc				= new CJxcDesc;
//	pJxcDesc->fRadius				= fRadius;
//	pJxcDesc->pControllerHitReport	= (void*)pHitReport;
//
//	m_pJxController = JPSystem::GetInstance()->CreateController(pJxcDesc, m_iControllerID );
//	m_pJxController->_vecPos		= vPos;
//	m_pJxController->userData		= pCharacter;
//	m_pJxController->SetCollisionGroup( COLLISION_GROUP_A | COLLISION_GROUP_C );
//
//	uiCharID++;
//	//char Buffer[256];
//	//sprintf_s( Buffer, "CharID%d", uiCharID );
//	m_pJxController->actorName      = "Character";
//
//	return 1;
//}


void GPS2Character::Destroy()
{
	JPSystem::GetInstance()->GetJxControllerManager()->ReleaseController(m_pJxController);
}


void GPS2Character::Move( JOVECTOR3& vMoveDelta)
{
	m_pJxController->Move( vMoveDelta );
}


JOVECTOR3&	GPS2Character::GetCharactersPosition()
{
//	return JOVECTOR3( 0.0f, 0.0f, 0.0f ); 
	return m_pJxController->_vecPos;
}


CJxActor*    GPS2Character::GetActor(void)
{
	return NULL;
}


CJxController*   GPS2Character::GetController(void)
{
	return NULL;
}

void GPS2Character::AddForce(JOVECTOR3& vFroce)
{
}


void GPS2Character::SetCollisionEnable(bool bEnable)
{
	m_pJxController->SetCollidable(bEnable);
}

void GPS2Character::SetCollisionGroup(unsigned int uiGroup)
{
	if( m_pJxController->GetCollisionGroup() & uiGroup )
		m_pJxController->SetCollisionGroup( m_pJxController->GetCollisionGroup() - uiGroup );
	else
		m_pJxController->SetCollisionGroup( m_pJxController->GetCollisionGroup() | uiGroup );
}

unsigned int GPS2Character::GetCollisionGroup()
{
	return m_pJxController->GetCollisionGroup();
}


bool GPS2Character::IsSetCollision(){ return m_pJxController->GetCollidable(); }



void GPS2Character::SetGPSPosition(JOVECTOR3& Point)
{
	m_pJxController->_vecPos		= Point;

	// 강제 위치 셋팅할 때는 이전 위치도 같은 위치로 셋팅한다
	m_pJxController->_vecPrePos		= Point;
}


void GPS2Character::SetRadius(float fRadius)
{
	m_pJxController->SetRadius(fRadius);
}

float GPS2Character::GetRadius()
{
	return m_pJxController->_pJxcDesc->fRadius;

}

void GPS2Character::SetCollisionLevel(int iLevel)
{

	m_pJxController->_nPriority = iLevel;
}

int GPS2Character::GetCollisionLevel()
{

	return m_pJxController->_nPriority;
}

void GPS2Character::SetEllipse(bool enable)
{
	m_pJxController->SetEllipse( enable );
}

void GPS2Character::SetFront(float front)
{
	m_pJxController->SetFront(front);
}
///////////////////////////////////////////////// About GPS2 BALL ////////////////////////////////////////////////
static unsigned int uiBallID = 0;

GPS2Ball::GPS2Ball()
{

}

GPS2Ball::~GPS2Ball()
{

	Destroy();
}

void GPS2Ball::CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall )
{
	uiBallID++;

	if(JPSystem::GetInstance()->GetScene() == NULL) return;

	////CJxsDesc* pJxsDesc					= new CJxsDesc;
	///*pJxsDesc->fMass						= 0.50f;
	//pJxsDesc->pJxUserContactReport		= (CJxUserContactReport*) (JPSystem::GetInstance()->GetContactReport());*/
	//m_pActor							= new CJxSphere( ((JPSystem*)JPSystem::GetInstance())->GetScene(), pJxsDesc );
	//m_pActor->_vecPos					= vPos;
	//m_pActor->_vecVelocity				= vInitialVelocity;
	////char Buffer[256];
	////sprintf_s( Buffer, "Ball%d", uiBallID );
	//m_pActor->actorName                 = "Ball";

	//m_pActor->SetCollisionGroup( COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_C | COLLISION_GROUP_D );

	//((JPSystem*)JPSystem::GetInstance())->GetJxControllerManager()->AddJxActor( m_pActor ); // 캐릭터 매니저도 관리  

	//m_pActor->userData = pBall; 

}


void GPS2Ball::Destroy()
{
	if(JPSystem::GetInstance()->GetScene() )
	{
		JPSystem::GetInstance()->GetScene()->RemoveJxActor( m_pActor );
		JPSystem::GetInstance()->GetJxControllerManager()->RemoveJxActor( m_pActor );
		delete m_pActor;
	}
}


JOVECTOR3& GPS2Ball::GetActorPosition()
{
	if( m_pActor )
	{
		m_vGPSPosition = m_pActor->_vecPos;
	}
	return m_vGPSPosition;
}


void GPS2Ball::SetActorVelocity( JOVECTOR3& vVelocity )
{
	if( m_pActor )
	{
		m_pActor->_vecVelocity = vVelocity;
	}
}


void GPS2Ball::SetActorAngleVelocity( JOVECTOR3& vAngleVelocity )
{
	if( m_pActor )
	{
		m_pActor->_vecAngularVelocity = vAngleVelocity;
	}
}


void GPS2Ball::SetActorPosition(const JOVECTOR3& vPos)
{
	if( m_pActor )
	{
		m_pActor->_vecPos = vPos;
	}

	return;
}


void GPS2Ball::ResetActor(const JOVECTOR3& vPos)
{
	if( m_pActor )
	{
		m_pActor->_vecVelocity = JOVECTOR3( 0.0f, 0.0f, 0.0f );
		m_pActor->_vecPos      = vPos;
	}

}

JOQUATERNION& GPS2Ball::GetActorRotate()
{

	if( m_pActor )
	{
//		NxQuat nxQuat = m_pActor->getGlobalOrientationQuat();

		m_qGPSRotate.v.fX = 0.0f;
		m_qGPSRotate.v.fY = 0.0f;
		m_qGPSRotate.v.fZ = 0.0f;
		m_qGPSRotate.w = 1.0f;
	}

	return m_qGPSRotate;
}


JOVECTOR3& GPS2Ball::GetActorVelocity()
{

	if( m_pActor )
	{
		m_vGPSVelocity = m_pActor->_vecVelocity;
	}

	return m_vGPSVelocity;
}

JOVECTOR3&	 GPS2Ball::GetActorAngleVelocity()
{
	if( m_pActor )
	{
		m_vGPSAngleVelocity = m_pActor->_vecAngularVelocity;
	}

	return m_vGPSAngleVelocity;
}


void GPS2Ball::RaiseActorFlag( bool actorFlag )
{ 
	if( m_pActor ) m_pActor->SetCollidable( false );
}


void GPS2Ball::ClearActorFlag( bool actorFlag )
{
	if( m_pActor ) m_pActor->SetCollidable( true );
}

void GPS2Ball::SetSleep( bool actorFlag )
{
	m_pActor->SetSleep( actorFlag );
}
// 2009.1.9	smstock
//void GPS2Ball::SetCollisionGroup(unsigned int uiGroup )
//{
//	// 라인아웃이 가능하게 충돌 셋팅
//	if( uiGroup == 0 )
//		m_pActor->SetCollisionGroup( COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_D );
//	// 라인 아웃 불가(공이 벽에 튕기게)
//	else
//		m_pActor->SetCollisionGroup( COLLISION_GROUP_A | COLLISION_GROUP_B | COLLISION_GROUP_C | COLLISION_GROUP_D );
//}

void GPS2Ball::ThrowBall( JOVECTOR3& vStartPos, JOVECTOR3& vVelocity, JOVECTOR3& vecAngleVelocity )
{
	if( m_pActor )
	{
		m_pActor->_vecPos				= vStartPos;
		m_pActor->_vecPrePos			= vStartPos; /// 추가 sclee 2009 08 20  - 초기화 안하니 공위치가 달랐음
		m_pActor->_vecVelocity			= vVelocity;
		m_pActor->_vecAngularVelocity	= vecAngleVelocity;
		m_pActor->SetSpinAble( true );
		m_pActor->_qOrientation			= JOQUATERNION();
		m_pActor->SetSleep( false );
//		m_pActor->SetCollidable( true );

/*
		// 백스핀을 살짝 걸어 보자 
		JOVECTOR3 vecUp  = JOVECTOR3( 0.0f, 1.0f, 0.0f );
		JOVECTOR3 vecCross = CrossProduct( vVelocity, vecUp );
		vecCross = vecCross / 100.0f;
		vTorque.x = vecCross.fX;
		vTorque.y = vecCross.fY;
		vTorque.z = vecCross.fZ;
		m_pActor->setAngularVelocity(vTorque);
		*/
//		m_pActor->addTorque( vTorque );

	}
}

JOVECTOR3 GPS2Ball::GetSimulateActorPosition( float fTime )
{
	JOVECTOR3 vBallPos;
	JOVECTOR3 pos = JOVECTOR3(0.0f, 0.0f, 0.0f);
	JPSystem::GetInstance()->SimulateJxActor( m_pActor, fTime, vBallPos, pos );

	return vBallPos;
}

JOVECTOR3 GPS2Ball::GetSimulateActorPositionForAI( float fTime)
{	
	JOVECTOR3 vBallPos;
	JPSystem::GetInstance()->SimulateJxActorForAI( m_pActor, fTime,vBallPos);
	return vBallPos;
}



bool GPS2Ball::GetSimulateActorVeoloticy( float fTime , JOVECTOR3& vBallPos ,JOVECTOR3& vBallVelocity )
{
	JPSystem::GetInstance()->SimulateJxActor( m_pActor, fTime, vBallPos , vBallVelocity);

	return true;
}