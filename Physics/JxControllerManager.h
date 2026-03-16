#pragma once

class CJxController;
class CJxScene;
class CJxcDesc;
class CJxActor;

#include <list>

using namespace std;

class CJxControllerManager
{
public:
	typedef		list<CJxActor*>		     JxControllerList; // 컨트롤러 및 다른 객체 통합 
	typedef     JxControllerList::iterator       JxControllerListIT; 

public:
	CJxControllerManager(void);
	~CJxControllerManager(void);

private:
	JxControllerList					m_JxControllerList;

public:
	CJxController*						CreateJxController( CJxScene* pJxScene, CJxcDesc* pjxcDesc );
	void								ReleaseController( CJxController* pJxController );
	void								ReleaseAllController( CJxController* pJxController );
	void								UpdateControllers( float fTimeDelta );

	void								AddJxActor( CJxActor* pJxActor );
	void								RemoveJxActor( CJxActor* pJxActor );

	void								CollisionExcute( CJxActor* pJxActorA, CJxActor* pJxActorB, float timeDelta );

	size_t								GetControllers(void);
	JxControllerListIT					GetFirstController(void) { return m_JxControllerList.begin(); }
	JxControllerListIT					GetEndController(void){  return m_JxControllerList.end(); }

private:


};
