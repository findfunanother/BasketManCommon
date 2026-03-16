//#include "stdafx.h"
#include "JxSphere.h"
#include "JxActors.h"
#include "JxScene.h"
#include "JxInPlane.h"
#include "JxController.h"
#include "JxTorus.h"
#include "JxUserContactReport.h"
#include "JxPhysics.h"
#include "JxFiPlane.h"
#include "JxCylinder.h"
#include "JxField.h"
#include "LogActor.h"
#include "JEnviroment.h"
#include "JPhysics.h"
#include "JxBox.h"
#include "F4GoalInChecker.h"
//#include "public_resource/resource_manager.h"

//CJxSphere::CJxSphere( CJxScene* pJxScene, CJxsDesc* pJxsDesc )
//{
//	//_pJxsDesc				=	pJxsDesc;
//	m_pJxScene				=	pJxScene;
//
//	m_mtrInertiaInverse._11 = 5.0f * 0.5f *fMass*Sqr(fRadius);// 관성텐서 계산 // I = (2Mr*r) / 5 
//	m_mtrInertiaInverse._22 = 5.0f * 0.5f *fMass*Sqr(fRadius);
//	m_mtrInertiaInverse._33 = 5.0f * 0.5f *fMass*Sqr(fRadius);
//
//	m_mtrInertia._11		= 1 / m_mtrInertiaInverse._11;
//	m_mtrInertia._22		= 1 / m_mtrInertiaInverse._22;
//	m_mtrInertia._33		= 1 / m_mtrInertiaInverse._33;
//
//	//m_pJxScene->AddJxActor( this );
//
//	currentFrameIndex = 0;
//	elapsedCurrentTime = 0.0f;
//	curveValue = 0.0f;
//	firstBound = false;
//}


CJxSphere::CJxSphere(void)
{
}

/// 시뮬레이션 용 생성자 
//CJxSphere::CJxSphere( CJxsDesc* pJxsDesc )
//{
//	m_pJxScene              =   NULL; // 씬에 넣지 않는다. 
//	_pJxsDesc				=	pJxsDesc;
//
//	currentFrameIndex = 0;
//	elapsedCurrentTime = 0.0f;
//	curveValue = 0.0f;
//	firstBound = false;
//
//	m_mtrInertiaInverse._11 = 5.0f / 2*fMass*Sqr(fRadius);// 관성텐서 계산 // I = (2Mr*r) / 5 
//	m_mtrInertiaInverse._22 = 5.0f / 2*fMass*Sqr(fRadius);
//	m_mtrInertiaInverse._33 = 5.0f / 2*fMass*Sqr(fRadius);
//
//	m_mtrInertia._11		= 1 / m_mtrInertiaInverse._11;
//	m_mtrInertia._22		= 1 / m_mtrInertiaInverse._22;
//	m_mtrInertia._33		= 1 / m_mtrInertiaInverse._33;
//
//	additionalBounceFactorratio = 1.0f;
//
//}
//

CJxSphere::~CJxSphere(void)
{
	//if( m_pJxScene ) m_pJxScene->RemoveJxActor( this );
	//delete _pJxsDesc;
}


void CJxSphere::Intergrate( float fTimeDelta )
{


	if( !m_bSleep )
	{
		elapsedCurrentTime = fTimeDelta;

	 	JOVECTOR3 vPrevVelocity = _vecVelocity;

		/////////////////////// 커브 관련 /////////////////////////////////////////////
		JOVECTOR3 curveForceVector = curveMoment * curveValue * -1.0f;
		JOVECTOR3 addForce = JOVECTOR3(0.0f, JXGRAVITY, 0.0f) + curveForceVector;
		//////////////////////////////////////////////////////////////////////////////

		_vecVelocity = (_vecVelocity + addForce * fTimeDelta) * ( 1 - fTimeDelta * AIRDRAG);

		//JOVECTOR3 vCurrentPos = (vPrevVelocity*fTimeDelta) +  ( (_vecVelocity - vPrevVelocity ) * fTimeDelta * 0.5f );
		JOVECTOR3 vCurrentPos =  (_vecVelocity + vPrevVelocity) * fTimeDelta * 0.5f;

		_vecPos = _vecPos + vCurrentPos;

		if( m_bSpinAble )
		{
			//Vector3 rotationVector = Vector3.Cross(vecMoment.normalized, vecVelocity.normalized);
			//JOVECTOR3 rotationVector = CrossProduct(JOVECTOR3().up, _vMoment);
			//JOVECTOR3 rotationVector = CrossProduct(JOVECTOR3().up, curveMoment);
			JOVECTOR3 rotationVector = CrossProduct(curveMoment.Normalized(), _vecVelocity);

			JOVECTOR3 vMul = rotationVector * fTimeDelta;
			//JOQUATERNION Quer = _qOrientation * vMul;
			//_qOrientation = _qOrientation + Quer;
		}

		//_vMoment   = JOVECTOR3( 0.0f, 0.0f, 0.0f );
	}
}


void CJxSphere::ReadyToNextStep( float fTimeDelta )
{
	_vecPrePos = _vecPos;

}

// collision
bool CJxSphere::CollideWithOtherActor( CJxActor * pJxOtherActor, CContactInfo* pContactInfo )
{
	EKindOfActor eActor = pJxOtherActor->GetKindOfActor();
	switch( eActor )
	{
		case KE_Controllers		: CollideWithController( (CJxController*)pJxOtherActor, pContactInfo );			break;
		case KE_Sphere			: CollideWithSphere( (CJxSphere*)pJxOtherActor, pContactInfo );					break; 
		case KE_InPlane			: CollideWithInPlane( (CJxInPlane*)pJxOtherActor, pContactInfo );				break; 
		case KE_FPlane			: CollideWithJxFiPlane((CJxFiPlane*)pJxOtherActor, pContactInfo);				break;
		case KE_Cylinder		: CollideWithJxCylinder((CJxCylinder*)pJxOtherActor, pContactInfo);				break;
		case KE_F3Field			: CollideWithField((CJxField*)pJxOtherActor, pContactInfo);						break;
		case KE_Box				: CollideWithJxBox((CJxBox*)pJxOtherActor, pContactInfo);						break;
		case KE_Torus			: CollideWithJxTorus((CJxTorus*)pJxOtherActor, pContactInfo);					break;
		case KE_F4GoalInChecker	: CollideWithJxF4GoalInChecker((F4GoalInChecker*)pJxOtherActor, pContactInfo);  break;
		default:
			break;
	}
	return true;
}


bool CJxSphere::CollideWithJxFiPlane(CJxFiPlane * pJxFiPlane, CContactInfo* pContactInfo)
{
	return pJxFiPlane->CollideWithSphere((CJxSphere*)this, pContactInfo);
}


bool CJxSphere::CollideWithController( CJxController * pJxController, CContactInfo* pContactInfo )
{
	return ((CJxController*)pJxController)->CollideWithSphere( (CJxSphere*)this, pContactInfo );
}


bool CJxSphere::CollideWithInPlane( CJxInPlane* pJxInPlane, CContactInfo* pContactInfo )
{
	float fD1 = pJxInPlane->DistanceToPoint( _vecPrePos );
	float fD2 = pJxInPlane->DistanceToPoint( _vecPos );

	if( fD1 > fRadius && fD2 < fRadius ) 
	{
		float T = ( fD1 - fRadius ) /  ( fD1 - fD2 );
		//T = T - 0.001f;

		pContactInfo->vNormal = pJxInPlane->vNormal;
		pContactInfo->vNewPosition = _vecPos*T + _vecPrePos*( 1.0f - T ) + (pContactInfo->vNormal * PHYSICS_EPSILON);
		

		///
	    /// 보정된 좌표 까지 실제로 걸리는 시간 계산, 현재 흐른 시간을 넣어 줌 
		/// 
		JOVECTOR3 realMoveDistance = pContactInfo->vNewPosition - _vecPrePos;
		elapsedCurrentTime = realMoveDistance.Length() / _vecVelocity.Length(); 

		pContactInfo->pActorA = this;
		pContactInfo->pActorB = pJxInPlane;

		if( !m_bSleep )
		{

			// 평면과 구와 충돌 
			//if( _pJxsDesc->pJxUserContactReport) 
				//_pJxsDesc->pJxUserContactReport->onContactNotify( this, pJxInPlane, 2 ); // 라인아웃 이벤트 


			if( pJxInPlane->GetCollidable() )
			{
				ResolveWithInPlane( pContactInfo );
			}

			// 면과 충돌 하면 면의 이름을 이벤트 이름으로 
			if (pJxInPlane->bIsCollisonEvent)
			{
				//collisionEventName[currentFrameIndex] = pJxInPlane->actorName;
				//ige::FileLogger myLog("1.0.4.2", "testfile2.txt");
				//myLog << ige::FileLogger::e_logType::LOG_WARNING << pJxInPlane->actorName.c_str();

			}

			// 평면과 구와 충돌 
			//if( _pJxsDesc->pJxUserContactReport)
				//_pJxsDesc->pJxUserContactReport->onContactNotify( this, pJxInPlane, 1 ); // 일반적인 충돌 


		}
	}

	return true;
}


bool CJxSphere::CollideWithSphere( CJxSphere * pJxSphere, CContactInfo* pContactInfo )
{
	return true;

	float fSize = pJxSphere->fRadius + fRadius;

	JOVECTOR3 vLength = _vecPos - pJxSphere->_vecPos;
	float     fLength = vLength.Length();

	if( fLength < fSize ) // 충돌 
	{
		pContactInfo->pActorA = this;
		pContactInfo->pActorB = pJxSphere;
		pContactInfo->vNormal = vLength.Normalize();

		SetSleep( false );
		pJxSphere->SetSleep( false );

		ResolveWithSphere( pContactInfo );

	}

	return true;


}


void CJxSphere::ResolveWithSphere( CContactInfo* pContactInfo )
{
	CJxSphere* pSphereA = (CJxSphere*)pContactInfo->pActorA;
	CJxSphere* pSphereB = (CJxSphere*)pContactInfo->pActorB;

	JOVECTOR3   vR_A =  1.0f*pContactInfo->vNormal*pSphereA->fRadius;
	JOVECTOR3   vR_B =  -1.0f*pContactInfo->vNormal*pSphereB->fRadius;

	JOVECTOR3   vPointVelocityA   = CrossProduct( pSphereA->_vecAngularVelocity, vR_A ); // 각속도는 이전껏..접선 속도 v = w x r 
	JOVECTOR3   vPointVelocityB   = CrossProduct( pSphereB->_vecAngularVelocity, vR_B ); // 접선 속도 v = w x r 

	JOVECTOR3   vNewVelocityA     = pSphereA->_vecVelocity + vPointVelocityA;
	JOVECTOR3   vNewVelocityB     = pSphereB->_vecVelocity + vPointVelocityB;
	JOVECTOR3   vNewVelocityAB    = vNewVelocityA - vNewVelocityB;

	float		fNormalComponent = DotProduct( vNewVelocityAB, pContactInfo->vNormal ); 


	if( fNormalComponent < 0.0f )
	{

		const float fRestic = 0.78f; // 반발 계수  
		// 충격량 분자구하기.
		float fImpulseNumerator = -(1.0f + fRestic ) * fNormalComponent;


		JOVECTOR3 vArg1		= CrossProduct( vR_A, pContactInfo->vNormal );
		vArg1               = pSphereA->GetMtrInertiaInverse()*vArg1;
		JOVECTOR3 vArg2		= CrossProduct( vArg1, vR_A );

        // 충격량 분모구하기.
		float fImpulseDenominator = 1 / pSphereA->fMass + DotProduct( vArg2, pContactInfo->vNormal ) ;


		vArg1					= CrossProduct( vR_B, pContactInfo->vNormal);
		vArg2					= pSphereB->GetMtrInertiaInverse()*vArg1;
		fImpulseDenominator		+= 1 / pSphereB->fMass + DotProduct( vArg2, pContactInfo->vNormal );

		// 충격량 
		float     fImpulseScalar = fImpulseNumerator / fImpulseDenominator;
		JOVECTOR3 vImpulse      =  fImpulseScalar * pContactInfo->vNormal;

		
		// final velocity = v1 + ( fImpulse* normal ) / mass
		pSphereA->_vecVelocity = pSphereA->_vecVelocity + (vImpulse)*(1 / pSphereA->fMass);

		pSphereB->_vecVelocity = pSphereB->_vecVelocity + (vImpulse)*(1 / pSphereB->fMass)*(-1.0f);
	}

}


void CJxSphere::ResolveWithController( CContactInfo* pContactInfo )
{

}

//ige::FileLogger myLogBounce("1.0.4.2", "testfile_Bounce.txt");

void CJxSphere::ResolveWithInPlane( CContactInfo* pContactInfo )
{
	_vecPos = pContactInfo->vNewPosition;

	CJxSphere* pSphere = (CJxSphere*)pContactInfo->pActorA;
	CJxInPlane* pInPlane = (CJxInPlane*)pContactInfo->pActorB;

	JOVECTOR3 vTemp;
	JOVECTOR3 vContact;
	vContact = pSphere->fRadius*pContactInfo->vNormal*(-1.0f);

	JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), pContactInfo->vNormal)) * pContactInfo->vNormal + _vecVelocity;

	JOVECTOR3 addNewBounce = vecNewVelocity;

	if (true)
	{
		/*float addFriction = 1.0f - Logtwo( 200.0f, vecNewVelocity.Length() * GROUNDFRICTION);
		addFriction = CLAMP(addFriction, 0.001f, 0.99f);
		addNewBounce.fY = vecNewVelocity.fY * pInPlane->_pJxipDesc->fBounceness;
		addNewBounce.fX = addNewBounce.fX * addFriction;
		addNewBounce.fZ = addNewBounce.fZ * addFriction;*/
		//float addFriction = (1 / (vecNewVelocity.magnitude* plane.friction));

		//const float fixedFriction = 0.30f;
		const float fixedFriction = 0.30f;
		float addFriction = (1.0f - GROUNDFRICTION * fixedFriction);  //(1.0f - GROUNDFRICTION * fixedFriction);//
																  //Mathf.Log( vecNewVelocity.YZero().magnitude *
																  //JPhysics.groundFriction, 200.0f);
		
		//float addFriction = 1.0f - Mathf.Log(2, vecNewVelocity.magnitude * JPhysics.groundFriction);
		//Debug.Log(addFriction);
		//addFriction =  0.99f;

		addNewBounce.fY = vecNewVelocity.fY * GROUNDBOUNCE;
		
		addNewBounce.fX = addNewBounce.fX * addFriction;
		addNewBounce.fZ = addNewBounce.fZ * addFriction;

	}
	else
	{
		addNewBounce = addNewBounce * pInPlane->fBounceness;
	}

	_vecVelocity = addNewBounce;
	//_vMoment = _vecVelocity; // 모멘텀을 속도로 일단, curveMoment 와 통일을 해야 함 
	curveMoment = pContactInfo->vNormal;

	//char s1[256];                
	//sprintf_s(s1, "x:%f y:%f z:%f w:%f", _qOrientation.v.fX, _qOrientation.v.fY, _qOrientation.v.fZ, _qOrientation.w);    // %e를 지정하여 실수를 지수 표기법으로 된 문자열로 저장
	//myLogBounce << ige::FileLogger::e_logType::LOG_WARNING << s1;

	if (!firstBound)
	{
		curveValue = 0.0f;
		//orient = Quaternion.Euler(0.0f, 0.0f, 0.0f);
		_qOrientation = JOQUATERNION();

		firstBound = true;
	}

}

void CJxSphere::ResolveWithTorus(CContactInfo* pContactInfo)
{
	_vecPos = pContactInfo->vNewPosition;

	CJxSphere* sphere = (CJxSphere*)pContactInfo->pActorA;
	CJxTorus* torus = (CJxTorus*)pContactInfo->pActorA;

	JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), pContactInfo->vNormal)) * pContactInfo->vNormal + _vecVelocity;

	JOVECTOR3 addNewBounce = vecNewVelocity;

	const float fixedFriction = 0.30f;
	float addFriction = (1.0f - GROUNDFRICTION * fixedFriction);
	addNewBounce.fY = vecNewVelocity.fY * TORUSBOUNCENESS;
	addNewBounce.fX = addNewBounce.fX * addFriction;
	addNewBounce.fZ = addNewBounce.fZ * addFriction;

	_vecVelocity = addNewBounce;
	_vMoment = torus->vNormal;

	if (!firstBound)
	{
		curveValue = 0.0f;
		//orient = Quaternion.Euler(0.0f, 0.0f, 0.0f);
		firstBound = true;

		OnEventFirstRimBound();
	}
	else
	{
		OnEventRimBound();
	}
}

bool CJxSphere::CollideWithJxCylinder(CJxCylinder * pJxCylinder, CContactInfo* pContactInfo)
{
	pContactInfo->pActorA = pJxCylinder;
	pContactInfo->pActorB = this;

	JOVECTOR3 collisionPoint;
	if (IntersectSphereToCylinder(pJxCylinder->_vecPos, pJxCylinder->_pJxcyDesc->vNormal, pJxCylinder->_pJxcyDesc->fHeight * 2.0f, pJxCylinder->_pJxcyDesc->fRadius + fRadius, _vecPrePos, _vecPos, &collisionPoint))
	{
		JOVECTOR3 poscylinerbottom, poscylindertop;
		poscylinerbottom = pJxCylinder->_vecPos - pJxCylinder->_pJxcyDesc->vNormal * pJxCylinder->_pJxcyDesc->fHeight;
		poscylindertop = pJxCylinder->_vecPos + pJxCylinder->_pJxcyDesc->vNormal * pJxCylinder->_pJxcyDesc->fHeight;
		JOVECTOR3 newNearCylPnt; /* 점 A */
		IntersectPointToLine(collisionPoint, poscylinerbottom, poscylindertop, &newNearCylPnt);

		JOVECTOR3 newPosition = collisionPoint;
		pContactInfo->vNewPosition = newPosition;
		JOVECTOR3 newNormal = collisionPoint - newNearCylPnt;
		JOVECTOR3 collisionNormal = newNormal.Normalized();
		pContactInfo->vNormal = collisionNormal;
		/*
		if (!firstBound)
		{
			curveValue = 0.0f;
			_qOrientation = JOQUATERNION();
			firstBound = true;
			collisionEventName[currentFrameIndex] = "Event_GoalPost";
		}
		*/
		
		ResolveWithCylinder(pContactInfo, newNearCylPnt);
	}

	return true;
}

void CJxSphere::ResolveWithFiPlane(CContactInfo* pContactInfo)
{

}

void CJxSphere::ResolveWithCylinder(CContactInfo* pContactInfo, JOVECTOR3 newNearCylPnt)
{
	CJxSphere* sphereB = (CJxSphere*)pContactInfo->pActorB;
	CJxCylinder* cylinder = (CJxCylinder*)pContactInfo->pActorA;

	JOVECTOR3 velocityNormal = sphereB->_vecVelocity.Normalized();

	JOVECTOR3 reflectVector = (2.0f * DotProduct(sphereB->_vecVelocity * (-1.0f), pContactInfo->vNormal)) * pContactInfo->vNormal + sphereB->_vecVelocity;
	reflectVector.Normalize();

	sphereB->_vecVelocity = sphereB->_vecVelocity.Length() * reflectVector * POSTBOUNCE;

	const float safetyValue = 0.001f;
	float up = (cylinder->_pJxcyDesc->fRadius + sphereB->fRadius + safetyValue) + DotProduct(newNearCylPnt, pContactInfo->vNormal) - DotProduct(pContactInfo->vNewPosition, pContactInfo->vNormal);
	float down = DotProduct(velocityNormal, pContactInfo->vNormal);
	float k = up / down;

	sphereB->_vecPos = pContactInfo->vNewPosition + k * velocityNormal;

	//pJxUserContactReport->onContactNotify(this, cylinder, GOALPOST_HIT); // 10번 
}


// F3프로젝트를 위한 새로운 타입 
bool  CJxSphere::CollideWithField(CJxField * pField, CContactInfo* pContactInfo)
{
	CollideWithFieldExtraBoundary(pField, pContactInfo);

	// Right Side 
	//const float addSize = 0.04f; // 림사이즈 만큼 아래에
	//float goalInHeight = pField->rightGoalInPos.fY - addSize;

	//if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
	//{
	//	JOVECTOR3 newGoalInPos = JOVECTOR3(pField->rightGoalInPos.fX, goalInHeight, pField->rightGoalInPos.fZ);
	//	float k = DotProduct(JOVECTOR3::up * -1.f, newGoalInPos - _vecPrePos) / DotProduct(JOVECTOR3::up * -1.f, _vecPos - _vecPrePos);
	//	JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

	//	float distance = (newGoalInPos - goalInPoint).Length();
	//	if (distance < pField->_rimRadius)
	//	{
	//		OnEventRightGoalIn();
	//		//DebugBallPos("RightSide_GoalIn", vecPos);

	//		//JPhysics.CreatePrimitive(PrimitiveType.Cube, new Vector3(0.0f, 3.050f, 10.7254f), 1.0f, 1.0f, 1.0f, 1.0f, 0.05f, "RimpPos");
	//		//JPhysics.CreatePrimitive(PrimitiveType.Sphere, goalInPoint, 1.0f, 1.0f, 1.0f, 1.0f, 0.05f, "GoalInPoint");
	//		//JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPrePos, 1.0f, 1.0f, 0.0f, 1.0f, 0.05f, "GoalInSpherePos_pre");
	//		//JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPos, 1.0f, 1.0f, 0.0f, 1.0f, 0.05f, "GoalInSpherePos_cur");
	//	}
	//}
	//else // Left Side 
	//	if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
	//	{
	//		JOVECTOR3 newGoalInPos = new JOVECTOR3(pField->leftGoalInPos.fX, goalInHeight, pField->leftGoalInPos.fZ);
	//		float k = DotProduct(JOVECTOR3::up * -1.f, newGoalInPos - _vecPrePos) / DotProduct(JOVECTOR3::up * -1.f, _vecPos - _vecPrePos);
	//		JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

	//		float distance = (newGoalInPos - goalInPoint).Length();
	//		if (distance < pField->_rimRadius)
	//		{
	//			OnEventRightGoalIn();
	//		}
	//	}

	return true;

}


void CJxSphere::CollideWithFieldExtraBoundary(CJxField* field, CContactInfo* infoContact)
{
	float bounceValue = 0.5f;

	// Right Side 
	if (_vecPos.fZ >= field->_courtWidth && _vecPrePos.fZ < field->_courtWidth)
	{
		JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), field->_rightNormal)) * field->_rightNormal + _vecVelocity;
		_vecVelocity = vecNewVelocity * bounceValue;
		_vecPos.fZ = _vecPrePos.fZ;
		OnEventBoundary();
	}
	//else // Left Side ( half court )
	if (_vecPos.fZ <= 0.0f && _vecPrePos.fZ > 0.0f)
	{
		JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), field->_rightNormal)) * field->_rightNormal + _vecVelocity;
		_vecVelocity = vecNewVelocity * bounceValue;
		_vecPos.fZ = _vecPrePos.fZ;
		OnEventBoundary();
	}
	//else // UpSide LineOut
	if (_vecPos.fX <= -field->_courtDepth * 0.5f && _vecPrePos.fX > -field->_courtDepth * 0.5f)
	{
		JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), field->_frontNormal) * field->_frontNormal) + _vecVelocity;
		_vecVelocity = vecNewVelocity * bounceValue;
		_vecPos.fX = _vecPrePos.fX;
		OnEventBoundary();
	}
	//else // BottomSide 
	if (_vecPos.fX >= field->_courtDepth * 0.5f && _vecPrePos.fX < field->_courtDepth * 0.5f)
	{
		JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), field->_backNormal) * field->_backNormal) + _vecVelocity;
		_vecVelocity = vecNewVelocity * bounceValue;
		_vecPos.fX = _vecPrePos.fX;
		OnEventBoundary();
	}
}

bool CJxSphere::CollideWithJxTorus(CJxTorus* jxTorus, CContactInfo* infoContact)
{
	JOVECTOR3 vecTmp = _vecPos - jxTorus->_vecPos;
	vecTmp.Normalize();

	JOVECTOR3 vecCP1 = CrossProduct(jxTorus->vNormal, vecTmp);
	JOVECTOR3 vecCP2 = CrossProduct(vecCP1, jxTorus->vNormal);
	vecCP2.Normalize();
	
	JOVECTOR3 vecCirclePoint;
	if (vecCP2.Length() == 0.0f) // 정 중앙에 맞는 경우 
	{
		JOVECTOR3 vecCirclePointDir = JOVECTOR3(1.0f, 0.0f, 0.0f);
		vecCirclePointDir = jxTorus->fDistance * vecCirclePointDir;
		vecCirclePoint = vecCirclePointDir + jxTorus->_vecPos;
		//Debug.LogError("Cent Hit!!");
	}
	else
	{
		JOVECTOR3 vecCirclePointDir = jxTorus->fDistance * vecCP2;
		vecCirclePoint = vecCirclePointDir + jxTorus->_vecPos;
	}

	JOVECTOR3 vecPreSub = _vecPrePos - vecCirclePoint;
	JOVECTOR3 vecSub = _vecPos - vecCirclePoint;

 	float fD1 = vecPreSub.Length();
	float fD2 = vecSub.Length();

	float fLine = jxTorus->fRadius + fRadius;

	if (fD1 > fLine && fD2 < fLine) // Collision 
	{
		float fT = (fD1 - fLine) / (fD1 - fD2);
		fT = fT - 0.001f;
		infoContact->vNewPosition = _vecPos * fT + _vecPrePos * (1.0f - fT);
		vecSub = infoContact->vNewPosition - vecCirclePoint;
		// 이 부분 버그 수정 ( 2020/07/03, 노멀값을 계산된 포지션이 아닌 현재 좌표로 해서 생기는 문제 해결 )
		infoContact->vNormal = vecSub.Normalized();
		/*
		JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPrePos, 0.0f, 1.0f, 1.0f, 0.5f, 0.24f, "PrePos");
		JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPos, 0.0f, 1.0f, 1.0f, 0.5f, 0.24f, "CurPos");
		JPhysics.CreatePrimitive(PrimitiveType.Sphere, infoContact.vecNewPosition, 1.0f, 1.0f, 1.0f, 0.5f, 0.24f, "AfterPos");
		Debug.DrawRay(infoContact.vecNewPosition, infoContact.vecNormal * 10.0f, Color.red, float.MaxValue);
		*/
		infoContact->pActorA= this;
		infoContact->pActorB = jxTorus;


		ResolveWithJxTorus(infoContact);
		// 평면과 토러스의 충돌 알리기
	}
	
	return true;
}
bool CJxSphere::CollideWithJxBox(CJxBox* jxBox, CContactInfo* infoContact)
{
	for (int i = 0; i < 6; i++)
	{

		float fD1 = GetDistanceToPointFromPlane(jxBox->fiPlane[i].vNormal, jxBox->fiPlane[i]._vecPos, _vecPrePos);
		float fD2 = GetDistanceToPointFromPlane(jxBox->fiPlane[i].vNormal, jxBox->fiPlane[i]._vecPos, _vecPos);

		if (fD1 >= fRadius && fD2 < fRadius)
		{

			bool inBoxCheck = true;
			for (int j = 0; j < 6; j++)
			{
				float distance = GetDistanceToPointFromPlane(jxBox->fiPlane[j].vNormal, jxBox->fiPlane[j]._vecPos, _vecPos);
				if (distance >= fRadius)
				{
					inBoxCheck = false;
				}
			}

			if (inBoxCheck)
			{
				float sub = (fD1 - fD2);
				float up = (fD1 - fRadius);
				float fT = (up / sub);

				infoContact->vNewPosition = _vecPos * fT + _vecPrePos * (1.0f - fT);
				float d = GetDistanceToPointFromPlane(jxBox->fiPlane[i].vNormal, jxBox->fiPlane[i]._vecPos, infoContact->vNewPosition);
				JOVECTOR3 bc = (infoContact->vNewPosition - jxBox->fiPlane[i]._vecPos) - jxBox->fiPlane[i].vNormal * d; // 직교로 구한 평면의 새로운 점 

				JOVECTOR3 c = jxBox->fiPlane[i]._vecPos + bc;

				//PhysicSimulator.CreateDebugObject(jxBox.fiPlane[i].vecPos, "point1");
				//JPhysics.CreatePrimitive(PrimitiveType.Sphere, infoContact.vecNewPosition, 0.0f, 1.0f, 0.0f, 1.0f, 0.15f, "BackBoard");

				OnEventFirstBackBoardBound();

				infoContact->pActorA = this;
				infoContact->pActorB = (CJxActor*)&jxBox->fiPlane[i];
				infoContact->vNormal = jxBox->fiPlane[i].vNormal;

				ResolveWithJxFiPlane(infoContact);

				break;


			}

		}

	}

	return true;
}


void CJxSphere::ResolveWithJxTorus(CContactInfo* infoContact)
{
	_vecPos = infoContact->vNewPosition;

	CJxSphere *sphere = (CJxSphere*)infoContact->pActorA;
	CJxTorus *torus = (CJxTorus*)infoContact->pActorB;

	JOVECTOR3 vecNewVelocity = 2.0f * DotProduct(_vecVelocity * (-1.0f), infoContact->vNormal) * infoContact->vNormal + _vecVelocity;

	JOVECTOR3 addNewBounce = vecNewVelocity;

	const float fixedFriction = 0.30f;
	float addFriction = (1.0f - GROUNDFRICTION * fixedFriction);
	addNewBounce.fY = vecNewVelocity.fY * TORUSBOUNCENESS;
	addNewBounce.fX = addNewBounce.fX * addFriction;
	addNewBounce.fZ = addNewBounce.fZ * addFriction;

	_vecVelocity = addNewBounce;
	_vMoment = torus->vNormal;

	if (!firstBound)
	{
		curveValue = 0.0f;
		//orient = Quaternion.Euler(0.0f, 0.0f, 0.0f);
		firstBound = true;

		OnEventFirstRimBound();
	}
	else
	{
		OnEventRimBound();
	}
}

void CJxSphere::Update(float fDeltaTime)
{
	if (true)
	{
		elapsedCurrentTime = fDeltaTime;
		_vecPreVelocity = _vecVelocity;
		JOVECTOR3 dir = _vecVelocity;

		/////////////////////// 커브 관련 /////////////////////////////////////////////
		//JOVECTOR3 curveForceVector = _vecMoment * curveValue * -1.0f;
		JOVECTOR3 addForce = JOVECTOR3().up * (-JXGRAVITY);// +curveForceVector;

		_vecVelocity = (_vecVelocity + addForce * fDeltaTime) * (1 - fDeltaTime * AIRDRAG); // 속도 드래그 
		JOVECTOR3 vecCurPos = (_vecPreVelocity + _vecVelocity) * 0.5f * fDeltaTime;// + ((vecVelocity - vecPreVelocity) * fDeltaTime * 0.5f);
		_vecPos = _vecPos + vecCurPos; // 현재 위치   
		float rotatePower = 5.0f;
		//Vector3 rotationVector = Vector3.Cross(vecMoment.normalized, vecVelocity.normalized);
		//float momentMag = Mathf.Clamp(vecVelocity.magnitude * rotatePower, 0.0f, 30.0f);
		//Quaternion v3Rotation = Quaternion.Euler(rotationVector * momentMag);
		//orient = orient * v3Rotation;
	}
}

void CJxSphere::ResolveWithField(CContactInfo* pContactInfo)
{

}


void CJxSphere::ResolveWithF3GoalIn(CContactInfo* pContactInfo)
{

}

bool CJxSphere::CollideWithJxF4GoalInChecker(F4GoalInChecker* f4Field, CContactInfo* pContactInfo)
{
	// Right Side 
	const float addSize = 0.04f; // 림사이즈 만큼 아래에
	float goalInHeight = f4Field->rightGoalInPos.fY - addSize;

	if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
	{
		JOVECTOR3 newGoalInPos = JOVECTOR3(f4Field->rightGoalInPos.fX, goalInHeight, f4Field->rightGoalInPos.fZ);
		float k = DotProduct(-1.f*JOVECTOR3::up, newGoalInPos - _vecPrePos) / DotProduct(-1.f * JOVECTOR3::up, _vecPos - _vecPrePos);
		JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

		float distance = (newGoalInPos - goalInPoint).Length();
		if (distance < f4Field->rimRadius)
		{
			OnEventRightGoalIn();
			//DebugBallPos("RightSide_GoalIn", vecPos);

			//JPhysics.CreatePrimitive(PrimitiveType.Cube, new Vector3(0.0f, 3.050f, 10.7254f), 1.0f, 1.0f, 1.0f, 1.0f, 0.05f, "RimpPos");
			//JPhysics.CreatePrimitive(PrimitiveType.Sphere, goalInPoint, 1.0f, 1.0f, 1.0f, 1.0f, 0.05f, "GoalInPoint");
			//JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPrePos, 1.0f, 1.0f, 0.0f, 1.0f, 0.05f, "GoalInSpherePos_pre");
			//JPhysics.CreatePrimitive(PrimitiveType.Sphere, vecPos, 1.0f, 1.0f, 0.0f, 1.0f, 0.05f, "GoalInSpherePos_cur");
		}
	}
	else // Left Side 
		if (_vecPos.fY <= goalInHeight && _vecPrePos.fY > goalInHeight)
		{
			JOVECTOR3 newGoalInPos = JOVECTOR3(f4Field->leftGoalInPos.fX, goalInHeight, f4Field->leftGoalInPos.fZ);
			float k = DotProduct(-1.f * JOVECTOR3::up, newGoalInPos - _vecPrePos) / DotProduct(-1.f * JOVECTOR3::up, _vecPos - _vecPrePos);
			JOVECTOR3 goalInPoint = _vecPrePos + k * (_vecPos - _vecPrePos);

			float distance = DotProduct(newGoalInPos, goalInPoint);
			if (distance < f4Field->rimRadius)
			{
				OnEventLeftGoalIn();
			}
		}

	return true;
}

void CJxSphere::ResolveWithJxFiPlane(CContactInfo* infoContact)
{
	_vecPos = infoContact->vNewPosition;

	CJxSphere* sphere = (CJxSphere*)infoContact->pActorA;
	CJxFiPlane* fiPlane = (CJxFiPlane*)infoContact->pActorB;

	JOVECTOR3 vecNewVelocity = (2.0f * DotProduct(_vecVelocity * (-1.0f), infoContact->vNormal)) * infoContact->vNormal + _vecVelocity;
	//JOVECTOR3 vecNewVelocity = REFlect Vector3.Reflect(vecVelocity, infoContact.vecNormal);
	/*if (ABS(_vecVelocity.fY) >= eventboundsound)
	{
		OnEventBoundSound();
	}*/

	JOVECTOR3 addNewBounce = vecNewVelocity;

	const float fixedFriction = 0.30f;
	float addFriction = (1.0f - GROUNDFRICTION* fixedFriction);

	//addNewBounce.y = vecNewVelocity.y * JPhysics.groundBounceness;
	//addNewBounce.x = addNewBounce.x * addFriction;
	//addNewBounce.z = addNewBounce.z * addFriction;

	_vecVelocity = addNewBounce;
	_vMoment = fiPlane->vNormal;

	if (!firstBound)
	{
		curveValue = 0.0f;
		//orient = Quaternion.Euler(0.0f, 0.0f, 0.0f);
		firstBound = true;

		OnEventFirstBackBoardBound();
	}
	else
	{
		OnEventBackBoardBound();
	}

}





