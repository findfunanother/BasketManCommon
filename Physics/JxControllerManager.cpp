//#include "stdafx.h"
#include "JxControllerManager.h"
#include "JxController.h"
#include "JxScene.h"
#include "JxUtill.h"

CJxControllerManager::CJxControllerManager(void)
{

}

CJxControllerManager::~CJxControllerManager(void)
{
	
}


CJxController*	 CJxControllerManager::CreateJxController( CJxScene* pJxScene, CJxcDesc* pjxcDesc )
{
	CJxController* pJxController = new CJxController( pJxScene, pjxcDesc );
	m_JxControllerList.push_back( pJxController );

	return pJxController;
}


void CJxControllerManager::ReleaseController( CJxController* pJxController )
{
	JxControllerList::iterator Iter;
	for( Iter = m_JxControllerList.begin(); Iter != m_JxControllerList.end(); Iter++ )
	{
		CJxActor* pActor = *Iter;

		if( pJxController == pActor )
		{
			SAFE_DELETE( pActor );
			m_JxControllerList.erase(Iter);
			break;
		}
	}
}


void CJxControllerManager::ReleaseAllController( CJxController* pJxController )
{
	JxControllerList::iterator Iter;
	for( Iter = m_JxControllerList.begin(); Iter != m_JxControllerList.end(); Iter++ )
	{
		CJxActor* pActor = *Iter;

		if( pActor )
		{
			SAFE_DELETE( pActor );
		}
	}

	m_JxControllerList.clear();

}


void CJxControllerManager::AddJxActor( CJxActor* pJxActor )
{
	m_JxControllerList.push_back( pJxActor );
}


void CJxControllerManager::RemoveJxActor( CJxActor* pJxActor )
{
	JxControllerList::iterator Iter;

	for( Iter = m_JxControllerList.begin(); Iter != m_JxControllerList.end() ; Iter++ )
	{
		CJxActor* pJxActorA = *Iter;
		if(  pJxActorA  == pJxActor )
		{
			m_JxControllerList.erase(Iter);
			return;
		}

	}

}


void CJxControllerManager::CollisionExcute( CJxActor* pJxActorA, CJxActor* pJxActorB, float timeDelta)
{
	if( 1 )
	{
		CContactInfo ContactInfo;
		ContactInfo.timeDelta = timeDelta;
		//pJxActorA->CollideWithOtherActor( pJxActorB, &ContactInfo );
	}
}


void CJxControllerManager::UpdateControllers( float fTimeDelta )
{
	JxControllerList::iterator IterA;

	// collision 검사 
	for( IterA = m_JxControllerList.begin(); IterA != m_JxControllerList.end(); ++IterA )
	{
		CJxActor* pJxActorA = *IterA;

		if( pJxActorA )
		{
			if( pJxActorA->GetCollidable() ) // 충돌 가능하다면 
			{
				JxControllerList::iterator IterB;
				IterB = IterA;
				++IterB;

				for( ; IterB != m_JxControllerList.end(); ++IterB )
				{
					CJxActor* pJxActorB = *IterB;

					if( pJxActorB )
					{
						if(  pJxActorB->GetCollidable() && (pJxActorA->GetCollisionGroup() &  pJxActorB->GetCollisionGroup()) ) /// 같은 충돌 체크 
						{
							// 공과 벽의 충돌은 막는다.
							if( pJxActorA->GetKindOfActor() != KE_InPlane || pJxActorB->GetKindOfActor() != KE_Sphere )
							{
								CollisionExcute( *IterA, *IterB, fTimeDelta ); 
							}
						}
					}
				}
			}
		}

	}
}


size_t CJxControllerManager::GetControllers(void)
{
	return m_JxControllerList.size();
}

/*
JxControllerListIT CJxControllerManager::GetFirstController(void)
{
	return m_JxControllerList.begin();
}
*/
/*
JxControllerListIT CJxControllerManager::GetEndController(void)
{
	return m_JxControllerList.end();
}


*/