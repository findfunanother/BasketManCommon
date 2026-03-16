//#include "stdafx.h"
#include "JxActors.h"
#include "JxScene.h"


//static unsigned int UniqueID()
//{
//	static unsigned int uiID = 0;
//	uiID++;
//	return uiID;
//}


CJxScene::CJxScene(void)
{
	m_bCollision = true;
}

CJxScene::~CJxScene(void)
{
	//for (size_t i = 0; i < m_JxActorVector.size(); i++)
	//{
	//	delete(m_JxActorVector[i]);
	//}
}


void CJxScene::CollisionExcute( CJxActor* pJxActorA, CJxActor* pJxActorB )
{
	if( m_bCollision )
	{
		CContactInfo ContactInfo;
		pJxActorA->CollideWithOtherActor( pJxActorB, &ContactInfo );
	}
}


void CJxScene::SimulateJxActor( CJxActor* pJxActor, float fTimeDelta )
{
	if( pJxActor )
	{
		pJxActor->Intergrate( fTimeDelta ); // 적분
	}

	CJxActor* pJxActorA = pJxActor;

	if( pJxActorA )
	{
		if( pJxActorA->GetCollidable() ) // 충돌 가능하다면 
		{
			JxActorVector::iterator IterB = m_JxActorVector.begin();

			for( ; IterB != m_JxActorVector.end(); ++IterB )
			{
				CJxActor* pJxActorB = *IterB;

				if( pJxActorB )
				{
					if( pJxActorB->GetKindOfActor() != KE_Sphere)
					{
						if( pJxActorB->GetCollidable() && (pJxActorA->GetCollisionGroup() &  pJxActorB->GetCollisionGroup()) ) /// 같은 충돌 체크 
						{
							CollisionExcute( pJxActorA, *IterB ); 
						}
					}
					
				}
			}
		}
	}


	if( pJxActor ) 
	{
		pJxActor->ReadyToNextStep(fTimeDelta);
	}


}


void CJxScene::Simulate( float fTimeDelta )
{
	JxActorVector::iterator IterA;

	// 강체들 적분 
	for( IterA = m_JxActorVector.begin(); IterA != m_JxActorVector.end(); ++IterA )
	{
		CJxActor* pJxActor = *IterA;

		if( pJxActor )
		{
			pJxActor->Intergrate( fTimeDelta ); // 적분
		}
	}

	// collision 검사 
	for( IterA = m_JxActorVector.begin(); IterA != m_JxActorVector.end(); ++IterA )
	{
		CJxActor* pJxActorA = *IterA;

		if( pJxActorA )
		{
			if( pJxActorA->GetCollidable() ) // 충돌 가능하다면 
			{
				JxActorVector::iterator IterB;
				IterB = IterA;
				++IterB;

				for( ; IterB != m_JxActorVector.end(); ++IterB )
				{
					CJxActor* pJxActorB = *IterB;

					if( pJxActorB )
					{
						if(  pJxActorB->GetCollidable() && (pJxActorA->GetCollisionGroup() &  pJxActorB->GetCollisionGroup()) ) /// 같은 충돌 체크 
						{
							CollisionExcute( *IterA, *IterB ); 
						}
					}
				}
			}
		}

	}

	// 다음 스텝 준비 
	for( IterA = m_JxActorVector.begin(); IterA != m_JxActorVector.end(); ++IterA )
	{
		CJxActor* pJxActor = *IterA;
		if( pJxActor ) 
		{
			pJxActor->ReadyToNextStep(fTimeDelta);
		}
	}


}


void CJxScene::ReadyToNextStep( float fTimeDelta )
{
	// 다음 스텝 준비 
	JxActorVector::iterator IterA;

	for( IterA = m_JxActorVector.begin(); IterA != m_JxActorVector.end(); ++IterA )
	{
		CJxActor* pJxActor = *IterA;
		if( pJxActor ) 
		{
			pJxActor->ReadyToNextStep(fTimeDelta);
		}
	}
}



void CJxScene::AddJxActor( CJxActor* pJxActor )
{
	//int nID = UniqueID();
	m_JxActorVector.push_back(pJxActor);
}


void CJxScene::RemoveJxActor( CJxActor* pJxActor )
{
	JxActorVector::iterator Iter;

	for( Iter = m_JxActorVector.begin(); Iter != m_JxActorVector.end() ; Iter++ )
	{
		CJxActor* pJxActorA = *Iter;
		if(  pJxActorA  == pJxActor )
		{
			m_JxActorVector.erase(Iter);
			return;
		}
	
	}
}

void CJxScene::AddJxActorUnique(CJxActor* pJxActor)
{
	auto it = std::find(m_JxActorVector.begin(), m_JxActorVector.end(), pJxActor);

	// 벡터에 pJxActor가 없으면 추가
	if (it == m_JxActorVector.end())
	{
		m_JxActorVector.push_back(pJxActor);
	}
}

void CJxScene::RemoveJxActorClear(CJxActor* pJxActor)
{
	auto newEnd = std::remove_if(m_JxActorVector.begin(), m_JxActorVector.end(), [pJxActor](CJxActor* actor)
		{ return actor == pJxActor; });

	// 같은 것들은 모두 제거 
	m_JxActorVector.erase(newEnd, m_JxActorVector.end());
}

void CJxScene::SimulateActorOnly(CJxActor* jxActor, float fDeltaTime)
{
	jxActor->_vecPrePos = jxActor->_vecPos;
	jxActor->Update(fDeltaTime);

	for (int i = 0; i < m_JxActorVector.size(); i++)
	{
		CJxActor* staticActor = m_JxActorVector[i];
		CollisionExcute(jxActor, staticActor);
	}
	
}

void CJxScene::RemoveAll(void)
{
	m_JxActorVector.clear();
}


CJxActor*	CJxScene::GetActors( int nIndex )
{
	return m_JxActorVector[nIndex];
}


int	CJxScene::GetNumberActors(void)
{
	return static_cast<int>(m_JxActorVector.size());
}

