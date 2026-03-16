#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable : 4275)
#else
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif
#include "JPSApi.h"
#include "ICallBackJPS.h" // GPS2 

class CJxScene;
class JPSystem;
class CJxActor;

class CJxSphere;
class JPhysicActor;

class JPS_API JBallActor : public ICallBackJPS // JPhysicActor
{

public:
	JBallActor(JPSystem* pSystem);
	virtual ~JBallActor();

private:
	JPSystem* m_pSystem;
	CJxActor*	 m_pActor;

public:
	CJxActor* GetJxActor() { return m_pActor; }
	void CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall);
	void	Destroy();

public:
	void SetPos(float pos[]);
	void SetVelocity(float velocity[]);

public:
	bool OnUpdate(void* pvPos, void* pqRotate) { return true; }
	void OnCollisionEvent(void* pParam1 = 0, void* pParam2 = 0, void* pParam3 = 0, const char* ObjectName = NULL);

};


/*
class JPS_API JBallActorEx : public CJxSphere
{

public:
	JBallActorEx(JPSystem* pSystem);
	~JBallActorEx();

private:
	JPSystem* m_pSystem;

public:
	void	CreateActor(const JOVECTOR3& vPos, const JOVECTOR3& vInitialVelocity, void* pBall);
	void	Destroy();

//public:
	//bool OnUpdate(void* pvPos, void* pqRotate) { return true; }
	//void OnCollisionEvent(void* pParam1 = 0, void* pParam2 = 0, void* pParam3 = 0, const char* ObjectName = NULL);

};
*/

