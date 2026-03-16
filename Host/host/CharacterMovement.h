#pragma once

#include <deque>
#include <map>
#include "HostCommonFunction.h"
#include "Ability.h"
#include "Util.h"
#include "FileLog.h"
#include "../../GameCore/CAnimationController.h"


//F4PACKET::EnumNamePACKET_ID((PACKET_ID)
struct ServerCharacterPosition
{
	F4PACKET::PACKET_ID  packetID;
	TB::SVector3		 positionlogic;
	DHOST_TYPE_FLOAT     speed;
	DHOST_TYPE_FLOAT     dirYaw;
	DHOST_TYPE_FLOAT     clienttime;
	uint32_t             correct;

	ServerCharacterPosition(F4PACKET::PACKET_ID id = F4PACKET::PACKET_ID::packetID_none,
		TB::SVector3 pos = TB::SVector3(0.0f, 0.0f, 0.0f),
		DHOST_TYPE_FLOAT spd = 0.0f, DHOST_TYPE_FLOAT dy = 0.0f, DHOST_TYPE_FLOAT cltime = 0.0f, uint32_t tcorrect = 0)
		: packetID(id), positionlogic(pos), speed(spd), dirYaw(dy), clienttime(cltime), correct(tcorrect) {}
};

class CHost;
class CCharacter;

class CCharacterMovement
{

public:
	CCharacterMovement();
	~CCharacterMovement();

protected:
	DHOST_TYPE_FLOAT					m_Speed;
	DHOST_TYPE_FLOAT					m_DirYaw;
	JOVECTOR3	    					m_ServerPosition;
	JOVECTOR3	    					m_ServerPositionEvent;
	JOVECTOR3							m_MoveDirection;

	DHOST_TYPE_INT32					m_nSpeedHackCheckCount;
	DHOST_TYPE_BOOL						m_bPositionWrong;

	DHOST_TYPE_BOOL						m_bIsTripleThreat;

protected:
	CHost*								m_pHost;
	CCharacter*							m_pCharacter;
	std::deque<ServerCharacterPosition> m_SereverPositionDeque;

public:
	void SetTripleThreat(DHOST_TYPE_BOOL bIsTripleThreat) { m_bIsTripleThreat = bIsTripleThreat; }
	bool IsTripleThreat() { return m_bIsTripleThreat;  }

	JOVECTOR3			GetPosition()		{ return m_ServerPosition; }
	DHOST_TYPE_FLOAT	GetDirYaw()			{ return m_DirYaw; }
	JOVECTOR3			GeMoveDirection()	{ return m_MoveDirection; }
	DHOST_TYPE_FLOAT	GeSpeed()			{ return m_Speed; }

	DHOST_TYPE_FLOAT    GetGapServerPosition(TB::SVector3 position);

	DHOST_TYPE_BOOL		IsPositionWrong(TB::SVector3 rePosition);
	void				ClearSereverPositionDeque();
	void				AddServerPostionDeque(ServerCharacterPosition position);
	std::deque<ServerCharacterPosition> GetServerPostionDeque() { return m_SereverPositionDeque; }

	void				SendSeverPositionToClient(DHOST_TYPE_FLOAT lastYaw);

	DHOST_TYPE_FLOAT	GetValidMoveRange();

protected:
	void				Update(DHOST_TYPE_FLOAT timeDelta);

private:
	void				CharacterMove(DHOST_TYPE_FLOAT fTimeDelata);

	void				Process_play_c2s_playerStand_To_play_c2s_playerMove(ServerCharacterPosition last, ServerCharacterPosition secondLast);
	DHOST_TYPE_BOOL		CheckPositionBetweenServerAndClient(ServerCharacterPosition last, ServerCharacterPosition secondLast);
	DHOST_TYPE_BOOL		IsActionPositionDeque(F4PACKET::PACKET_ID  packetID);

};

