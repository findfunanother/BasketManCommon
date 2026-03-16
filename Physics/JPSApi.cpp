//#include "stdafx.h"
#include "JPSApi.h"
//#include "JPSystem.h"
#include "JPhysicActor.h"
#include "JxActors.h"

// extern "C"
//{
JPS_API int Example(int a, int b) {
	return a + b;
}

//
// About JPSystem
//
JPS_API JPSystem* CreateJPSystem(void) {
	JPSystem* newSystem = new JPSystem();
	newSystem->Initialize();
	return newSystem;
}

JPS_API void Simulate(JPSystem* jSystem, float fDeltaTime) {
	jSystem->Simulate(fDeltaTime);
}

JPS_API void SimulateJxActor(JPSystem* jSystem, CJxActor* pJxActor, float fDeltaTime) {
	jSystem->SimulateJxActor(pJxActor, fDeltaTime);
}

JPS_API SimulBallInfo* SimulateBall(JPSystem* jSystem,
									int frameCount,
									float inPos[],
									float inVel[],
									float inRotation[],
									float inTorqueNormal[],
									float curveValue) {
	return jSystem->SimulateBallFixedFrame(frameCount, inPos, inVel, inRotation, inTorqueNormal, curveValue);
}

JPS_API SimulBallInfo* SimulateBallWithStartIndex(JPSystem* jSystem,
												  int startIndex,
												  int frameCount,
												  float inPos[],
												  float inVel[],
												  float inRotation[]) {
	return jSystem->SimulateBallStartFixedFrame(startIndex, frameCount, inPos, inVel, inRotation);
}

JPS_API void DeleteJPSystem(JPSystem* jSystem) {
	delete jSystem;
}

//
// JPhysicActor
//
JPS_API JPhysicActor* CreateJPhysicActor(JPSystem* pJSystem, int nType) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateJActor(nType);  // 볼은 O 번
	return pJPhysicActor;
}

JPS_API JPhysicActor* CreateJSphereActor(JPSystem* pJSystem, EventCallType callBack) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateJBallActor(callBack);

	return pJPhysicActor;
}

JPS_API JPhysicActor* CreateJFenceActor(JPSystem* pJSystem, float normal[], float distance, char name[]) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateJFenceActor(normal, distance, name);
	return pJPhysicActor;
}

// 유한 평면들
JPS_API JPhysicActor* CreateJFiPlaneActor(JPSystem* pJSystem,
										  float pos[],
										  float normal[],
										  float depthVector[],
										  float width,
										  float depth,
										  char name[]) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateJFiPlaneActor(pos, normal, depthVector, width, depth, name);
	return pJPhysicActor;
}

// 골 포스트
JPS_API JPhysicActor* CreateJCylinderActor(JPSystem* pJSystem,
										   float pos[],
										   float normal[],
										   float radius,
										   float height,
										   char name[]) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateJCylinderActor(pos, normal, radius, height, name);
	return pJPhysicActor;
}

// Field, 골인과 라인아웃 처리를 위해
JPS_API JPhysicActor* CreateF3FieldActor(JPSystem* pJSystem,
										 float pos[],
										 float normal[],
										 float halfSizeX,
										 float halfSizeZ,
										 float goalPosHeightY,
										 float goalPostHeightZ,
										 char name[]) {
	JPhysicActor* pJPhysicActor = new JPhysicActor(pJSystem);
	pJPhysicActor->CreateF3FieldActor(pos, normal, halfSizeX, halfSizeZ, goalPosHeightY, goalPostHeightZ, name);
	return pJPhysicActor;
}

JPS_API void DeleteJPhysicActor(JPhysicActor* pJPhysicActor) {
	delete pJPhysicActor;
}

JPS_API void SetBeActive(JPhysicActor* pJPhysicActor, bool bActive) {
	pJPhysicActor->SetBeActive(bActive);
}

JPS_API void SetSleep(JPhysicActor* pJPhysicActor, bool bSleep) {
	pJPhysicActor->SetSleep(bSleep);
}

JPS_API float* GetJBallPosition(JPhysicActor* jxBallActor) {
	return jxBallActor->GetJxActor()->_vecPos.fEle;
}

JPS_API float* GetJBallQuternion(JPhysicActor* jxBallActor) {
	 JOQUATERNION().quaternion4[0] = jxBallActor->GetJxActor()->_qOrientation.v.fX;
	 JOQUATERNION().quaternion4[1] = jxBallActor->GetJxActor()->_qOrientation.v.fY;
	 JOQUATERNION().quaternion4[2] = jxBallActor->GetJxActor()->_qOrientation.v.fZ;
	 JOQUATERNION().quaternion4[3] = jxBallActor->GetJxActor()->_qOrientation.w;
	 return JOQUATERNION().quaternion4;
}

//
// About JBallActor
//
JPS_API JBallActor* CreateJBallActor(JPSystem* pJSystem) {
	JBallActor* pJBallActor = new JBallActor(pJSystem);
	JOVECTOR3 vVelocity = JOVECTOR3(10.0f, 0.0f, 0.0f);
	JOVECTOR3 vPos = JOVECTOR3(0.0f, 5.0f, 0.0f);
	pJBallActor->CreateActor(vPos, vVelocity, pJBallActor);
	return pJBallActor;
}

JPS_API void DeleteJBallActor(JBallActor* pJBallActor) {
	delete pJBallActor;
}

//
// About JBallActor
//
JPS_API JPlaneActor* CreateJPlaneActor(JPSystem* pJSystem) {
	JPlaneActor* pJPlaneActor = new JPlaneActor(pJSystem);
	JOVECTOR3 vNormal = JOVECTOR3(0.0f, 1.0f, 0.0f);
	pJPlaneActor->CreateActor(0.0f, vNormal, "Ground");
	return pJPlaneActor;
}

JPS_API void DeleteJPlaneActor(JPlaneActor* pJPlaneActor) {
	delete pJPlaneActor;
}
//}
