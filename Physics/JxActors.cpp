//#include "stdafx.h"
#include "JxActors.h"

CJxActor::CJxActor()
	: userData(NULL),
	  actorName("0"),
	  m_bSleep(false),
	  m_bMovAble(true), 
	  m_bCollidable(true),
	  m_bSpinAble(true),
	  m_uiCollisionGroup(0)
{

}

CJxActor::~CJxActor()
{
}
void CJxActor::ReadyToNextStep(float fTimeDelta)
{
}

