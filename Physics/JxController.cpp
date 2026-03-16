//#include "stdafx.h"
#include "JxController.h"
#include "JxIControllerHitReport.h"
#include "JxScene.h"
#include "JxInPlane.h"
#include "JxSphere.h"


CJxController::CJxController(  CJxScene* pJxScene, CJxcDesc* pCJxcDesc )
{
	m_pJxScene					= pJxScene;
	_pJxcDesc					= pCJxcDesc;
	_nPriority					= 0;
	m_pJxIControllerHitReport   = (CJxIControllerHitReport*)pCJxcDesc->pControllerHitReport;
//	m_pJxScene->AddJxActor( this ); 씬에 포함시키지 않는다.
	_front						= 0;
	_ellipse					= false;

}

CJxController::~CJxController(void)
{
//	m_pJxScene->RemoveJxActor( this );
	delete _pJxcDesc;
}

// 키입력시 들어온 값
void CJxController::Move( JOVECTOR3 dispVector )
{
	//_vecPrePos    = _vecPos;
	JOVECTOR3 TempPos = _vecPos + dispVector;
	
// BEGIN 2011-11-15 %이전 위치 저장 판단 수정%	by smstock
	// 이전위치는 변화가 있을때만 저장하자.
	// LowFrame 사양에서 3점라인 지나가는문제수정.
	if((_vecPrePos - TempPos).Length() > 0.01f)
	{
		_vecPrePos = _vecPos;

		//char temp[255] = "";
		//sprintf(temp, "Move\t_vecPrePos : %.2f %.2f %.2f\t_vecPos : %.2f %.2f %.2f\tMoveDist %.2f %.2f %.2f\n", _vecPrePos.fX, _vecPrePos.fY, _vecPrePos.fZ, 
		//	TempPos.fX, TempPos.fY, TempPos.fZ, dispVector.fX, dispVector.fY, dispVector.fZ);
		//OutputDebugString(temp);
	}

	_vecPos = TempPos;
// END

	

	_vecVelocity  = dispVector;
}


void CJxController::Intergrate( float fTimeDelta )
{
}


void CJxController::ReadyToNextStep( float fTimeDelta )
{
	_vecVelocity = JOVECTOR3( 0.0f, 0.0f, 0.0f );
}

// collision
bool CJxController::CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo )
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch( eActor )
	{
		case KE_Controllers : CollideWithController( (CJxController*)pJxOtherActor, pContactInfo );	break;
		case KE_Sphere		: CollideWithSphere( (CJxSphere*)pJxOtherActor, pContactInfo );		break; 
		case KE_InPlane     : CollideWithInPlane( (CJxInPlane*)pJxOtherActor, pContactInfo ); break;
		default:
			break;
	}
	return true;
}

bool CJxController::CollideWithController( CJxController * pJxController, CContactInfo* pContactInfo )
{
	#define SLIDING_VALUE 1.01f;
	// 시뮬레이션 

	// TODO : 충돌시 오류 때문에 임시로 넣음
	if( !m_pJxIControllerHitReport ) return false;

	JOVECTOR3 vecSub = _vecPos - pJxController->_vecPos; vecSub.fY = 0.0f;
	float fLength    = vecSub.Length();
	float fSize;

	// 타원으로 충돌 체크
	//if( _ellipse )
	//{

	//}
	//else
	{
		// 일반 원으로 충돌 체크
		fSize = _pJxcDesc->fRadius + pJxController->_pJxcDesc->fRadius;
	}
	

	if( fLength < fSize ) 
	{
		// 총돌 이벤트 전달 
		m_pJxIControllerHitReport->ControllerHit( this, pJxController );

		float fCollideIntoLength = fSize - fLength;

		//float fVelSize;// = _vecVelocity.Length()+pJxController->_vecVelocity.Length();//400.0f;		// 미끄러짐 속도
		//if(fVelSize == 0.0f)
		{
			//fVelSize = 400.0f;
		}
		float fVelSize = 400.0f;
		fVelSize *= pContactInfo->timeDelta;

		JOVECTOR3 vPrevSub = _vecPrePos - pJxController->_vecPrePos;
		vPrevSub.fY = 0.0f;
		float PrevSize = vPrevSub.Length();

		if(PrevSize > fLength)
			fVelSize += (PrevSize-fLength);

		if( fVelSize > fCollideIntoLength )
			fVelSize = fCollideIntoLength;

		if( _nPriority > pJxController->_nPriority )
		{
			JOVECTOR3 vNormal = pJxController->_vecPos - _vecPos;
			vNormal.fY = 0.0f; /// sclee 추가 - 높이는 충돌처리 안함.
			vNormal.Normalize();
			pJxController->_vecPos = pJxController->_vecPos + vNormal*fVelSize*SLIDING_VALUE; 
		}
		else if( _nPriority < pJxController->_nPriority ) 
		{
			JOVECTOR3 vNormal = _vecPos - pJxController->_vecPos; 
			vNormal.fY = 0.0f; /// sclee 추가 - 높이는 충돌처리 안함.
			vNormal.Normalize();
			_vecPos = _vecPos + vNormal*fVelSize*SLIDING_VALUE; 
		}
		else
		{
			//float fVelSize = _vecVelocity.Length();
			//if( fVelSize > 0.0f ) fVelSize = 1.0f;

			JOVECTOR3 vNormal = _vecPos - pJxController->_vecPos;
			vNormal.fY = 0.0f; /// sclee 추가 - 높이는 충돌처리 안함.
			vNormal.Normalize();
			_vecPos = _vecPos + vNormal*0.5f*fVelSize*SLIDING_VALUE;


			//fVelSize = pJxController->_vecVelocity.Length();
			//if( fVelSize > 0.0f ) fVelSize = 1.0f;

			vNormal = pJxController->_vecPos - _vecPos;
			vNormal.fY = 0.0f; /// sclee 추가 - 높이는 충돌처리 안함.
			vNormal.Normalize();
			pJxController->_vecPos = pJxController->_vecPos + vNormal*0.5f*fVelSize*SLIDING_VALUE;
			
		}

	}
		
	//ResolveWithController(pContactInfo) ;
	return false;
}


bool CJxController::CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo )
{

	CJxSphere *     pSphereA  = pJxSphere;
	CJxController*  pCylinder = this;

	//float fRadiusA  = pSphereA->fRadius;
	//float fRadiusB  = pCylinder->_pJxcDesc->fRadius;

	float fHeight       = 175.0f; // 실린더의 높이 
	//JOVECTOR3 vCharAxis = JOVECTOR3( 0.0f, 1.0f, 0.0f );
	JOVECTOR3 vNormal;

	JOVECTOR3 vec2DShperePos	= pSphereA->_vecPos;  vec2DShperePos.fY = 0.0f;
	JOVECTOR3 vec2DCylinderPos	= pCylinder->_vecPos;  vec2DCylinderPos.fY = 0.0f;
	JOVECTOR3 vLength			= vec2DShperePos - vec2DCylinderPos; 
	float	  fLength           = vLength.Length();

	if( fLength < pCylinder->_pJxcDesc->fRadius )
	{
		if( pSphereA->_vecPos.fY < fHeight )
		{
			//JOVECTOR3 vecTopPos = vCharAxis*fHeight + pCylinder->_vecPos;
			//JOVECTOR3 vecMidPos = vCharAxis*fHeight*0.5f + pCylinder->_vecPos;

			//JOVECTOR3 vDistance = vecTopPos - pSphereA->_vecPos;
			//float     fDistance = vDistance.Length();
			//if( fDistance < pCylinder->_pJxcDesc->fRadius ) // 윗면이라고 추측 
			//{
			//	vNormal = JOVECTOR3( 0.1f, 1.0f, 0.0f );
			//	pSphereA->_vecPos      = pSphereA->_vecPrePos;
			//	pSphereA->_vecVelocity = (2.0f*DotProduct( pSphereA->_vecVelocity*(-1.0f), vNormal ))*vNormal + pSphereA->_vecVelocity; 
			//	pSphereA->_vecVelocity *= 0.5f;
			//	pSphereA->SetSleep( false );
			//}
			//else
			//{
			//	JOVECTOR3 vec2DShpereVel = pSphereA->_vecVelocity;  vec2DShpereVel.fY = 0.0f;
			//	JOVECTOR3 vNormal = pSphereA->_vecPos  - pCylinder->_vecPos; 
			//	vNormal.Normalize();
			//	vec2DShpereVel = vNormal*5.0f + vec2DShpereVel;
			//	pSphereA->_vecVelocity.fX = vec2DShpereVel.fX;
			//	pSphereA->_vecVelocity.fZ = vec2DShpereVel.fZ;
			//	pSphereA->SetSleep( false );
			//}

			m_pJxIControllerHitReport->SphereHit( pSphereA, this );
			
			return true;

		}
	}

	return false;
}


bool CJxController::CollideWithInPlane( CJxInPlane* pJxInPlane,  CContactInfo* pContactInfo )
{
	//float fD1 = pJxInPlane->DistanceToPoint( _vecPrePos );
	float fD2 = pJxInPlane->DistanceToPoint( _vecPos );

	if(  fD2 < _pJxcDesc->fRadius ) 
	{

		m_pJxIControllerHitReport->BoundaryHit( this, pJxInPlane );


		float fDistance             = pJxInPlane->DistanceToPoint( _vecPos );
		fDistance                   = _pJxcDesc->fRadius - fDistance;
		JOVECTOR3 vNewPos           = _vecPos +  pJxInPlane->vNormal*fDistance;
		pContactInfo->vNewPosition  = vNewPos;

		pContactInfo->vNormal       = pJxInPlane->vNormal;

		ResolveWithInPlane( pContactInfo );
	}

/*	if( fD1 > _pJxcDesc->fRadius && fD2 <= _pJxcDesc->fRadius ) 
	{
		float T = ( fD1 - _pJxcDesc->fRadius ) /  ( fD1 - fD2 );
		T = T - 0.001f;
		pContactInfo->vNewPosition = _vecPos*T + _vecPrePos*( 1.0f - T ); 
		pContactInfo->vNormal = pJxInPlane->_pJxipDesc->vNormal;


		ResolveWithInPlane( pContactInfo );


	}
*/
	return true;
}


void CJxController::ResolveWithInPlane( CContactInfo* pContactInfo )
{

	JOVECTOR3 vNewVel = (2.0f*DotProduct( _vecVelocity*(-1.0f), pContactInfo->vNormal ))*pContactInfo->vNormal + _vecVelocity; 
	_vecVelocity      = vNewVel*0.003f;
	_vecPos = pContactInfo->vNewPosition;

}


void CJxController::ResolveWithController( CContactInfo* pContactInfo )
{
}


void CJxController::ResolveWithSphere( CContactInfo* pContactInfo )
{

}



/*      전반사 
		JOVECTOR3 vNormal = _vecPos - pJxController->_vecPos;
		vNormal.Normalize();

		JOVECTOR3 vNewVel = (2.0f*DotProduct( _Velocity*(-1.0f), vNormal ))*vNormal + _Velocity; 
		_vecPos = _vecPos + vNewVel;


		vNormal = pJxController->_vecPos - _vecPos;
		vNormal.Normalize();

		vNewVel = (2.0f*DotProduct( pJxController->_Velocity*(-1.0f), vNormal ))*vNormal + pJxController->_Velocity; 
		pJxController->_vecPos = pJxController->_vecPos + vNewVel;
*/