#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable : 4190)
#else
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
#include "JOMath3d.h"
#include "JxPhysics.h"
#include "JPSApi.h"
#include "JxInPlane.h"
extern "C"
{
	JPS_API bool linePlaneIntersection(JOVECTOR3& contact, JOVECTOR3 ray, JOVECTOR3 rayOrigin, JOVECTOR3 normal, JOVECTOR3 coord);
	JPS_API JOVECTOR3  GetInitialVelocity(float fDeltaTime, JOVECTOR3 fStartPos, JOVECTOR3 fDestPos);
	JPS_API float	GetPlaneAddFriction(JOVECTOR3 curVelocity);
	JPS_API float GetDistanceToPointFromPlane(JOVECTOR3 normal/*평면의노멀*/, JOVECTOR3 onPoint/*평면의점*/, JOVECTOR3 toPoint/*떨어진 점*/);
	JPS_API void SetJBallActor(JPhysicActor* pJBallActor,
							   float fPrePos[],
							   float fStartPos[],
							   float fVelocity[],
							   float fTorqueNormal[],
							   float curveValue,
							   float BounceFactor);
	JPS_API void    ApplyRotate(JPhysicActor* pJBallActor, float fDirVector[]); 
	JPS_API float   GetFixedUpdateTime(void);

}
