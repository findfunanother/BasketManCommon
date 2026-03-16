#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable : 4172)
#else
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
//#ifdef EXPORT_DLL
//#define JPS_API __declspec(dllexport)
//#else
//#define JPS_API __declspec(dllimport) 
//#endif
//#else
//#define JPS_API __attribute__((visibility("default")))
//#define JPS_API_CAL __attribute__((visibility("hidden")))
//#endif


#include "JPhysicActor.h"
#include "JxSphere.h"
#include "JBallActor.h"
#include "JPSystem.h"
#include "JPlaneActor.h"
#include "JVECTOR.h"

class JBallActor;
class JPlaneActor;
class JPhysicActor;
//class SimulBallInfo;
struct SimulBallInfo;

extern "C"
{
	JPS_API int Example(int a, int b);
	JPS_API JPSystem* CreateJPSystem(void);
	JPS_API void Simulate(JPSystem* jSystem, float fDeltaTime);
	JPS_API SimulBallInfo* SimulateBall(JPSystem* jSystem, int frameCount, float inPos[], float inVel[], float inRotation[], float inTorqueNormal[], float curveValue);
	JPS_API void SimulateJxActor(JPSystem* jSystem, CJxActor* pJxActor, float fDeltaTime);
	JPS_API void DeleteJPSystem(JPSystem* jSystem);

	JPS_API JPhysicActor* CreateJPhysicActor(JPSystem* pJSystem, int nType);
	JPS_API JPhysicActor* CreateJSphereActor(JPSystem* pJSystem, EventCallType callBack);
	JPS_API JPhysicActor* CreateJFenceActor(JPSystem* pJSystem, float normal[], float distance, char name[]);
	JPS_API JPhysicActor* CreateJFiPlaneActor(JPSystem* pJSystem, float pos[], float normal[], float depthVector[], float width, float depth, char name[]);
	JPS_API JPhysicActor* CreateJCylinderActor(JPSystem* pJSystem, float pos[], float normal[], float radius, float height, char name[]);
	JPS_API JPhysicActor* CreateF3FieldActor(JPSystem* pJSystem, float pos[], float normal[], float halfSizeX, float halfSizeZ, float goalPosHeightY, float goalPostHeightZ, char name[]);
	JPS_API void DeleteJPhysicActor(JPhysicActor* pJPhysicActor);



	JPS_API void SetBeActive(JPhysicActor* pJPhysicActor, bool bActive);
	JPS_API void SetSleep(JPhysicActor* pJPhysicActor, bool bSleep);
	JPS_API float* GetJBallPosition(JPhysicActor* jxBallActor);



	//JPS_API void FlyActor(JPhysicActor* pJPhysicActor, float position3[], float veloicty3[]);
	// 쓰지 말자, 통일 해야됨 
	JPS_API JBallActor* CreateJBallActor(JPSystem* pJSystem);
	JPS_API void DeleteJBallActor(JBallActor* pJBallActor);

	JPS_API JPlaneActor* CreateJPlaneActor(JPSystem* pJSystem);
	JPS_API void DeleteJPlaneActor(JPlaneActor* pJPlaneActor);

}


