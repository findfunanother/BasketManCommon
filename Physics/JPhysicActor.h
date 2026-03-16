#pragma once

#include <functional>
#include "ICallBackJPS.h" 

class CJxActor;
class JPSystem;

typedef std::function<void(void*, void*, void*, const char*)> EventCallType;

class JPhysicActor : public ICallBackJPS
{
public:
	JPhysicActor(JPSystem* pJSystem);
	virtual ~JPhysicActor();

private:
	CJxActor*	 m_pActor;
	JPSystem*    m_pJSystem;

public:
	CJxActor* GetJxActor() { return m_pActor; }

public:
	//void (*EventCallBack)(void* pParam1, void* pParam2, void* pParam3, const char* ObjectName);
	
	EventCallType EventCallBackHandler;
	void SetCallbackHandler(EventCallType f) { EventCallBackHandler = std::move(f); }

public:
	void CreateJActor( int nType );
	void SetBeActive(bool bActive);
	void SetSleep(bool bSleep);

	void SetPos(float pos[]);
	void SetPrePos(float pos[]);
	void SetVelocity(float velocity[]);
	void SetMoment(float pos[]);
	void Initialzie();

private:
	void CreateJBallActor();
	void CreateJGroundActor();
	void CreateF3FieldActor();

public:
	void CreateJBallActor(EventCallType pCallBack);
	void CreateJFenceActor(float normal[], float distance, char name[]);
	void CreateJFiPlaneActor(float pos[], float normal[], float depthVector[], float width, float depth, char name[]);
	void CreateJCylinderActor(float pos[], float normal[], float radius, float height, char name[]);
	void CreateF3FieldActor(float pos[], float normal[], float halfSizeX, float halfSizeZ, float goalPosHeightY, float goalPostWidthZ, char name[]);
	
public:
	bool OnUpdate(void* pvPos, void* pqRotate) { return true; }
	void OnCollisionEvent(void* pParam1 = 0, void* pParam2 = 0, void* pParam3 = 0, const char* ObjectName = 0);

private:
	void Destroy();

};

