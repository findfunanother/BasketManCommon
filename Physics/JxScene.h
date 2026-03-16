#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable : 4251)
#else
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif
class CJxActor;

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

using std::map;
using std::vector;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#ifdef EXPORT_DLL
#define JPS_API __declspec(dllexport)
#else
#define JPS_API __declspec(dllimport) 
#endif
#define JPS_API_CAL
#else
#define JPS_API __attribute__((visibility("default")))
#define JPS_API_CAL __attribute__((visibility("hidden")))
#endif


class JPS_API CJxScene
{
public:
	CJxScene(void);
	~CJxScene(void);

private:
//	typedef  std::map< int, CJxActor* >  JxActorMap;
//	JxActorMap							m_JxActorMap;

	typedef std::vector< CJxActor* >	JxActorVector;
	JxActorVector						m_JxActorVector;


private:
	bool								m_bCollision;


public:
	void								SetCollisonEnable( bool bEnable ) { m_bCollision = bEnable; }

	void								CollisionExcute( CJxActor* pJxActorA, CJxActor* pJxActorB );
	void								Simulate( float fTimeDelta );
	void								SimulateJxActor( CJxActor* pJxActor, float fTimeDelta );
	void								AddJxActor( CJxActor* pJxActor );
	void								AddJxActorUnique(CJxActor* pJxActor);
	void								RemoveJxActor( CJxActor* pJxActor );
	void								RemoveJxActorClear(CJxActor* pJxActor);
	void								SimulateActorOnly(CJxActor* jxActor, float fDeltaTime);
	
	void								RemoveAll(void);
	void								ReadyToNextStep( float fTimeDelta );

	CJxActor*							GetActors( int nIndex );
	int									GetNumberActors(void);

};
