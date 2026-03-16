#include <stdio.h>
//#include "stdafx.h"
#include "JxPhysics.h"
#include "JPSystem.h"
#include "ICallBackGPS2.h"
#include "GPSActor.h"
#include "JxControllerManager.h"
#include "JxScene.h"
#include "JxController.h"
#include "JxSphere.h"
#include "ContactReport.h"
#include "JEnviroment.h"
#include "LogActor.h"
//using namespace rwf;
//EventCallBackFromBall* gEventCallBackFromGPS2Ball = NULL;

JPSystem::JPSystem(void)
{
	return;
	m_iCharacterCreateCount		= 0;
	m_fRemainTime				= 0.0f;
	m_bFrameSync				= true; // sclee /// true;
	m_pJxControllerManager		= NULL;


	// About Simulation
	/*m_pJxsDesc							= new CJxsDesc;

	m_pJxsDesc->fRadius					= BALL_RADIUS;
	m_pJxsDesc->fMass					= 0.05f;
	m_pJxsDesc->pJxUserContactReport	= NULL;

	m_pNewActor							= new CJxSphere( m_pJxsDesc );

	m_pContactReport					= new ContactReport();

	m_pSimulBallInfoBuffer = new SimulBallInfo[MAX_SIMULBALL];*/
}


JPSystem::~JPSystem(void)
{
	ExitJx();

	//delete m_pNewActor;
	//delete m_pContactReport;
	//delete[] m_pSimulBallInfoBuffer;
}


JPSystem* JPSystem::GetInstance()
{
	static JPSystem gps;
	return &gps;

}



bool JPSystem::Initialize()
{
	InitJx();
	
	return true;
}


bool JPSystem::Destroy()
{
	return true;
}


void JPSystem::JCallBackUpdate( void )
{
	// 공 액터를 가져온다 
	int nCntActors = m_pJxScene->GetNumberActors();

	for( int i = 0; i < nCntActors; i++ )
	{
		CJxActor* pActor = m_pJxScene->GetActors( i );
		if( pActor )
		{
			if( pActor->GetKindOfActor() == KE_Sphere )
			{
				if( pActor->userData ) 
				{
					((ICallBackJPS*)pActor->userData)->OnUpdate( &pActor->_vecPos, &pActor->_qOrientation );
				}
			}
		}
	}
}


void JPSystem::ControllerCallBackUpdate( void )
{
	// JxControllerManager
	int nNumber = static_cast<int>(m_pJxControllerManager->GetControllers());
	CJxControllerManager::JxControllerListIT IterBegin = m_pJxControllerManager->GetFirstController();
	for( int i = 0; i< nNumber; i++ )
	{
		CJxActor* pJxController = *IterBegin;

		if( ((CJxController*)pJxController)->GetKindOfActor() == KE_Controllers ) // 캐릭터만 업데이트 
		{
			void* pUserData =  ((CJxController*)pJxController)->GetUserData();
			if( ((CJxController*)pJxController)->GetUserData() )
			{
				((ICallBackGPS2*)pUserData)->OnUpdate( NULL, NULL );
			}
		}

		IterBegin++;
	}
//	JxControllerListIT IterEnd = m_pJxControllerManager->GetEndController();
}

///
/// 인덴스를 가지고 볼을 시뮬레이션 할 때 사용 
/// 
SimulBallInfo* JPSystem::SimulateBallStartFixedFrame(int ballIndex, int frameCount, float inPos[], float inVel[], float inRotation[])
{
	CJxSphere * pSphere = (CJxSphere*)m_pNewActor;
	pSphere->currentFrameIndex = 0;

	m_pNewActor->_vecPos = JOVECTOR3(inPos[0], inPos[1], inPos[2]);
	m_pNewActor->_vecVelocity = JOVECTOR3(inVel[0], inVel[1], inVel[2]);
	m_pNewActor->_qOrientation = JOQUATERNION(inRotation[3], inRotation[0], inRotation[1], inRotation[2]);

	/* 이 함수를 쓴다면 아래값을 입력으로 받아야 함 
	pSphere->curveMoment = JOVECTOR3(inTorqueNormal[0], inTorqueNormal[1], inTorqueNormal[2]);
	pSphere->curveValue = curveValue;
	pSphere->firstBound = false;
	*/

	//char Buffer[256];
	//sprintf_s(Buffer, "Ball%d", 100000);
	m_pNewActor->actorName = "Actor";
	m_pNewActor->SetCollisionGroup(COLLISION_GROUP_B | COLLISION_GROUP_D);

	m_pSimulBallInfoBuffer[ballIndex].fPos[0] = m_pNewActor->_vecPos.fX;
	m_pSimulBallInfoBuffer[ballIndex].fPos[1] = m_pNewActor->_vecPos.fY;
	m_pSimulBallInfoBuffer[ballIndex].fPos[2] = m_pNewActor->_vecPos.fZ;

	m_pSimulBallInfoBuffer[ballIndex].fVel[0] = m_pNewActor->_vecVelocity.fX;
	m_pSimulBallInfoBuffer[ballIndex].fVel[1] = m_pNewActor->_vecVelocity.fY;
	m_pSimulBallInfoBuffer[ballIndex].fVel[2] = m_pNewActor->_vecVelocity.fZ;

	m_pSimulBallInfoBuffer[ballIndex].fRot[0] = m_pNewActor->_qOrientation.v.fX;
	m_pSimulBallInfoBuffer[ballIndex].fRot[1] = m_pNewActor->_qOrientation.v.fY;
	m_pSimulBallInfoBuffer[ballIndex].fRot[2] = m_pNewActor->_qOrientation.v.fZ;
	m_pSimulBallInfoBuffer[ballIndex].fRot[3] = m_pNewActor->_qOrientation.w;

	// 이벤트 이름 복사 
	// gcc 에서는 strcpy 를 지원하지 않아서 주석처리함
	//strcpy_s(m_pSimulBallInfoBuffer[ballIndex].eventName, pSphere->collisionEventName[pSphere->currentFrameIndex].c_str());
	//strcpy_s(m_pSimulBallInfoBuffer[ballIndex].eventName, "Event");
	//pSphere->collisionEventName[pSphere->currentFrameIndex].clear(); // 클리어 
	pSphere->currentFrameIndex++;

	for (int i = ballIndex + 1; i < ballIndex + frameCount; i++)
	{
		if (m_pJxScene) m_pJxScene->SimulateJxActor(m_pNewActor, FIXEDUPDATETIME);

		m_pSimulBallInfoBuffer[i].fPos[0] = m_pNewActor->_vecPos.fX;
		m_pSimulBallInfoBuffer[i].fPos[1] = m_pNewActor->_vecPos.fY;
		m_pSimulBallInfoBuffer[i].fPos[2] = m_pNewActor->_vecPos.fZ;

		m_pSimulBallInfoBuffer[i].fVel[0] = m_pNewActor->_vecVelocity.fX;
		m_pSimulBallInfoBuffer[i].fVel[1] = m_pNewActor->_vecVelocity.fY;
		m_pSimulBallInfoBuffer[i].fVel[2] = m_pNewActor->_vecVelocity.fZ;

		m_pSimulBallInfoBuffer[i].fRot[0] = m_pNewActor->_qOrientation.v.fX;
		m_pSimulBallInfoBuffer[i].fRot[1] = m_pNewActor->_qOrientation.v.fY;
		m_pSimulBallInfoBuffer[i].fRot[2] = m_pNewActor->_qOrientation.v.fZ;
		m_pSimulBallInfoBuffer[i].fRot[3] = m_pNewActor->_qOrientation.w;

		// 이벤트 이름 복사 
		//strcpy_s(m_pSimulBallInfoBuffer[i].eventName, pSphere->collisionEventName[pSphere->currentFrameIndex].c_str());
		//strcpy_s(m_pSimulBallInfoBuffer[i].eventName, "Event");
		//pSphere->collisionEventName[pSphere->currentFrameIndex].clear(); // 클리어 
		pSphere->currentFrameIndex++;
	}

	return &m_pSimulBallInfoBuffer[ballIndex];
}


SimulBallInfo* JPSystem::SimulateBallFixedFrame(int frameCount, float inPos[], float inVel[], float inRotation[], float inTorqueNormal[], float curveValue)
{
	CJxSphere * pSphere = (CJxSphere*)m_pNewActor;
	pSphere->currentFrameIndex = 0;

	m_pNewActor->_vecPrePos = JOVECTOR3(inPos[0], inPos[1], inPos[2]);
	m_pNewActor->_vecPos = JOVECTOR3(inPos[0], inPos[1], inPos[2]);
	m_pNewActor->_vecVelocity = JOVECTOR3(inVel[0], inVel[1], inVel[2]);
	m_pNewActor->_qOrientation = JOQUATERNION(inRotation[3], inRotation[0], inRotation[1], inRotation[2]);
	m_pNewActor->SetSpinAble(true);

	pSphere->curveMoment = JOVECTOR3(inTorqueNormal[0], inTorqueNormal[1], inTorqueNormal[2]);
	pSphere->curveValue = curveValue;
	pSphere->firstBound = false;

	// 이거 해줘야 함 
	//SetJBallActor(jBall, InputPosition, velocity);

	//char Buffer[256];
	//sprintf_s(Buffer, "Ball%d", 100000);
	m_pNewActor->actorName = "Actor";
	m_pNewActor->SetCollisionGroup(COLLISION_GROUP_B | COLLISION_GROUP_D);


	m_pSimulBallInfoBuffer[0].fPos[0] = m_pNewActor->_vecPos.fX;
	m_pSimulBallInfoBuffer[0].fPos[1] = m_pNewActor->_vecPos.fY;
	m_pSimulBallInfoBuffer[0].fPos[2] = m_pNewActor->_vecPos.fZ;

	m_pSimulBallInfoBuffer[0].fVel[0] = m_pNewActor->_vecVelocity.fX;
	m_pSimulBallInfoBuffer[0].fVel[1] = m_pNewActor->_vecVelocity.fY;
	m_pSimulBallInfoBuffer[0].fVel[2] = m_pNewActor->_vecVelocity.fZ;

	m_pSimulBallInfoBuffer[0].fRot[0] = m_pNewActor->_qOrientation.v.fX;
	m_pSimulBallInfoBuffer[0].fRot[1] = m_pNewActor->_qOrientation.v.fY;
	m_pSimulBallInfoBuffer[0].fRot[2] = m_pNewActor->_qOrientation.v.fZ;
	m_pSimulBallInfoBuffer[0].fRot[3] = m_pNewActor->_qOrientation.w;

	// 이벤트 이름 복사 
	//strcpy_s(m_pSimulBallInfoBuffer[0].eventName, pSphere->collisionEventName[pSphere->currentFrameIndex].c_str());
	//strcpy_s(m_pSimulBallInfoBuffer[0].eventName, "Event");
	//pSphere->collisionEventName[pSphere->currentFrameIndex].clear(); // 클리어 
	pSphere->currentFrameIndex++;

	//ige::FileLogger myLog("1.0.4.2", "testfile3.txt");
	//myLog << ige::FileLogger::e_logType::LOG_WARNING << "Simulation";


	for (int i = 1; i < frameCount; i++)
	{
		if (m_pJxScene) m_pJxScene->SimulateJxActor(m_pNewActor, FIXEDUPDATETIME);

		m_pSimulBallInfoBuffer[i].fPos[0] = m_pNewActor->_vecPos.fX;
		m_pSimulBallInfoBuffer[i].fPos[1] = m_pNewActor->_vecPos.fY;
		m_pSimulBallInfoBuffer[i].fPos[2] = m_pNewActor->_vecPos.fZ;

		m_pSimulBallInfoBuffer[i].fVel[0] = m_pNewActor->_vecVelocity.fX;
		m_pSimulBallInfoBuffer[i].fVel[1] = m_pNewActor->_vecVelocity.fY;
		m_pSimulBallInfoBuffer[i].fVel[2] = m_pNewActor->_vecVelocity.fZ;

		m_pSimulBallInfoBuffer[i].fRot[0] = m_pNewActor->_qOrientation.v.fX;
		m_pSimulBallInfoBuffer[i].fRot[1] = m_pNewActor->_qOrientation.v.fY;
		m_pSimulBallInfoBuffer[i].fRot[2] = m_pNewActor->_qOrientation.v.fZ;
		m_pSimulBallInfoBuffer[i].fRot[3] = m_pNewActor->_qOrientation.w;

		/*
		char s1[256];                // 변환한 문자열을 저장할 배열
		sprintf_s(s1, "Index:%d x:%f y:%f z:%f w:%f", i, m_pNewActor->_qOrientation.v.fX, m_pNewActor->_qOrientation.v.fY, m_pNewActor->_qOrientation.v.fZ, m_pNewActor->_qOrientation.w);    // %e를 지정하여 실수를 지수 표기법으로 된 문자열로 저장
		myLog << ige::FileLogger::e_logType::LOG_WARNING << s1;

		char s2[256];                // 변환한 문자열을 저장할 배열
		sprintf_s(s2, "Index:%d x:%f y:%f z:%f", i, m_pNewActor->_vecPos.fX, m_pNewActor->_vecPos.fY, m_pNewActor->_vecPos.fZ);    // %e를 지정하여 실수를 지수 표기법으로 된 문자열로 저장
		myLog << ige::FileLogger::e_logType::LOG_WARNING << s2;
		*/

		// 이벤트 이름 복사 
		//strcpy_s(m_pSimulBallInfoBuffer[i].eventName, pSphere->collisionEventName[pSphere->currentFrameIndex].c_str());
		//strcpy_s(m_pSimulBallInfoBuffer[i].eventName, "Event");
		//pSphere->collisionEventName[pSphere->currentFrameIndex].clear(); // 클리어 
		pSphere->currentFrameIndex++;
	}

	return m_pSimulBallInfoBuffer;
}

/*
void JPSystem::SimulateJxBall(CJxActor* pJxActor, float fTimeDelta, OUT JOVECTOR3& vecPos, OUT JOVECTOR3& vecVel)
{
	//
	m_pNewActor->_vecPos = pJxActor->_vecPos;
	m_pNewActor->_vecVelocity = pJxActor->_vecVelocity;
	m_pNewActor->_vecAngularVelocity = pJxActor->_vecAngularVelocity;

	char Buffer[256];
	sprintf_s(Buffer, "Ball%d", 100000);
	m_pNewActor->actorName = Buffer;
	m_pNewActor->SetCollisionGroup(COLLISION_GROUP_B | COLLISION_GROUP_D);
	//	pJxActor->SetCollidable( false );
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (fTimeDelta < FIXEDUPDATETIME) fTimeDelta = FIXEDUPDATETIME + 0.001f;

	float fRemainTime = 0.0f;

	if (1) // 프레임 동기화 
	{
		FREAL fTimeLeft = fTimeDelta + fRemainTime;
		FREAL fElapsedFrameTime = 0.0f;
		while (fTimeLeft > 0.0f)
		{
			fElapsedFrameTime = MIN(fTimeLeft, FIXEDUPDATETIME);
			if (fElapsedFrameTime < FIXEDUPDATETIME)  fElapsedFrameTime = FIXEDUPDATETIME;

			if (fTimeLeft < fElapsedFrameTime)
			{
				fRemainTime = fTimeLeft;
				break;
			}

			fTimeLeft -= fElapsedFrameTime;

			// 볼, 외곽, 골대 
			if (m_pJxScene) m_pJxScene->SimulateJxActor(m_pNewActor, fElapsedFrameTime);
		}
	}

	vecPos = m_pNewActor->_vecPos;
	vecVel = m_pNewActor->_vecVelocity;
}
*/

void JPSystem::SimulateJxActor( CJxActor* pJxActor, float fTimeDelta, JOVECTOR3& vecPos, JOVECTOR3& vecVel )
{
	//
	m_pNewActor->_vecPos					= pJxActor->_vecPos;
	m_pNewActor->_vecVelocity				= pJxActor->_vecVelocity;
	m_pNewActor->_vecAngularVelocity        = pJxActor->_vecAngularVelocity;

//	char Buffer[256];
//	sprintf_s( Buffer, "Ball%d", 100000 );
	m_pNewActor->actorName                 = "Ball";
	m_pNewActor->SetCollisionGroup( COLLISION_GROUP_B | COLLISION_GROUP_D );
//	pJxActor->SetCollidable( false );
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( fTimeDelta < FIXEDUPDATETIME) fTimeDelta = FIXEDUPDATETIME + 0.001f;

	float fRemainTime = 0.0f;

	FREAL fTimeLeft = fTimeDelta + fRemainTime;   
	FREAL fElapsedFrameTime = 0.0f;
	while ( fTimeLeft > 0.0f)						
	{
		fElapsedFrameTime =  MIN(fTimeLeft, FIXEDUPDATETIME);
		if( fElapsedFrameTime < FIXEDUPDATETIME)  fElapsedFrameTime = FIXEDUPDATETIME;

		if( fTimeLeft < fElapsedFrameTime )
		{
			fRemainTime = fTimeLeft;
			break;
		}

		fTimeLeft -= fElapsedFrameTime;

		// 볼, 외곽, 골대 
		if( m_pJxScene ) m_pJxScene->SimulateJxActor( m_pNewActor, fElapsedFrameTime );
	}

	vecPos = m_pNewActor->_vecPos;
	vecVel = m_pNewActor->_vecVelocity;
}


void JPSystem::SimulateJxActorForAI( CJxActor* pJxActor, float fTimeDelta, JOVECTOR3& vecPos )//for AI PJM
{
	//
	m_pNewActor->_vecPos					= pJxActor->_vecPos;
	m_pNewActor->_vecVelocity				= pJxActor->_vecVelocity;
	m_pNewActor->_vecAngularVelocity      = pJxActor->_vecAngularVelocity;

	//char Buffer[256];
	//sprintf_s( Buffer, "Ball%d", 100000 );
	m_pNewActor->actorName                 = "Actor";
	m_pNewActor->SetCollisionGroup( COLLISION_GROUP_B | COLLISION_GROUP_D );
//	pJxActor->SetCollidable( false );
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( fTimeDelta < FIXEDUPDATETIME) fTimeDelta = FIXEDUPDATETIME + 0.001f;

	float fRemainTime = 0.0f;

	if( 1 ) // 프레임 동기화 
	{
		FREAL fTimeLeft = fTimeDelta + fRemainTime;   
		FREAL fElapsedFrameTime = 0.0f;
		while ( fTimeLeft > 0.0f)						
		{
			fElapsedFrameTime =  MIN(fTimeLeft, FIXEDUPDATETIME);
			if( fElapsedFrameTime < FIXEDUPDATETIME)  fElapsedFrameTime = FIXEDUPDATETIME;

			if( fTimeLeft < fElapsedFrameTime )
			{
				fRemainTime = fTimeLeft;
				break;
			}

			fTimeLeft -= fElapsedFrameTime;

			// 볼, 외곽, 골대 
			if( m_pJxScene ) m_pJxScene->SimulateJxActor( m_pNewActor, fElapsedFrameTime );
		}
	}
	vecPos = m_pNewActor->_vecVelocity;
//	delete m_pNewActor;
}

void JPSystem::SimulateJxActor(CJxActor* pJxActor, float fTimeDelta)
{
	if (m_bFrameSync) // 프레임 동기화 
	{
		FREAL fTimeLeft = fTimeDelta + m_fRemainTime;   // 이전 남은 시간까지 포함해서 계산
		FREAL fElapsedFrameTime = 0.0f;
		while (fTimeLeft > 0.0f)						// 일정한 스텝값으로 동기화 
		{
			fElapsedFrameTime = MIN(fTimeLeft, FIXEDUPDATETIME);
			if (fElapsedFrameTime < FIXEDUPDATETIME) { fElapsedFrameTime = FIXEDUPDATETIME; }

			if (fTimeLeft < fElapsedFrameTime)
			{
				m_fRemainTime = fTimeLeft;
				break;
			}

			fTimeLeft -= fElapsedFrameTime;

			if (m_pJxScene) m_pJxScene->SimulateJxActor(pJxActor, fElapsedFrameTime);
		}
	}
	else 
	{
		if (m_pJxScene) m_pJxScene->SimulateJxActor(pJxActor, fTimeDelta);
	}
}

void JPSystem::Simulate(float fTimeDelta)
{
//	return ;
	if( fTimeDelta > 0.15f ) fTimeDelta = 0.02f;        // 큰값이 들어가면 시스템이 깨질수 있다.

	if( m_bFrameSync ) // 프레임 동기화 
	{
		FREAL fTimeLeft = fTimeDelta + m_fRemainTime;   // 이전 남은 시간까지 포함해서 계산
		FREAL fElapsedFrameTime = 0.0f;
		while ( fTimeLeft > 0.0f)						// 일정한 스텝값으로 동기화 
		{
			fElapsedFrameTime =  MIN(fTimeLeft, FIXEDUPDATETIME);
			if (fElapsedFrameTime < FIXEDUPDATETIME) { fElapsedFrameTime = FIXEDUPDATETIME; }

			if( fTimeLeft < fElapsedFrameTime )
			{
				m_fRemainTime = fTimeLeft;
				break;
			}

			fTimeLeft -= fElapsedFrameTime;

			if( m_pJxScene ) m_pJxScene->Simulate(fElapsedFrameTime);

			//JCallBackUpdate(); 
		}
	}
	else // 
	{
		// 시뮬레이션 그리고 충돌 
		if( m_pJxScene ) m_pJxScene->Simulate(fTimeDelta);

		//JCallBackUpdate(); 

	}

	//m_pJxControllerManager->UpdateControllers( fTimeDelta ); // 캐릭터와 볼, 외곽 충돌검사  
	//ControllerCallBackUpdate();

}


CJxController* JPSystem::CreateController( CJxcDesc* pjxcDesc, int& iID )
{
	iID = m_iCharacterCreateCount++;
	return m_pJxControllerManager->CreateJxController( m_pJxScene, pjxcDesc );
}


void JPSystem::InitJx(void)
{
	m_pJxScene				= new CJxScene;
	m_pJxControllerManager	= new CJxControllerManager;
}


void JPSystem::ExitJx(void)
{
	delete m_pJxScene;
	delete m_pJxControllerManager;
}







