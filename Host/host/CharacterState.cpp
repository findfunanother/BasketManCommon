#include "CharacterState.h"

CCharacterState::CCharacterState(CCharacter* pInfo) : m_pOwner(pInfo)
{

}

CCharacterState::~CCharacterState()
{

}

void CCharacterState::OnEnter(void* pData)
{

}

void CCharacterState::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{

}

void CCharacterState::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{

}

void CCharacterState::OnExit()
{

}

DHOST_TYPE_BOOL CCharacterState::GetSecondAnimationReceivePacket()
{
	return false;
}