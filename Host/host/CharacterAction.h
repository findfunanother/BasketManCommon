#pragma once

#pragma once

#include <deque>
#include <map>
#include <list>
#include "HostCommonFunction.h"
#include "Ability.h"
#include "Util.h"
#include "FileLog.h"
#include "../../GameCore/CAnimationController.h"

class CHost;
class CCharacter;


class CCharacterAction
{
public:
	CCharacterAction();
	~CCharacterAction();

protected:
	F4PACKET::PACKET_ID m_CurrentAction;

	std::list<DHOST_TYPE_CHARACTER_SN> m_ListBoxOuted; // 박스아웃을 여려명의 캐릭터에게 할 수 있으므로 


public:
	std::list<DHOST_TYPE_CHARACTER_SN> GetBoxOutedList() { return m_ListBoxOuted; }

	void SetCurrentAction(F4PACKET::PACKET_ID currentAction);
	F4PACKET::PACKET_ID GetCurrentAction(void) { return m_CurrentAction;  }

	void AddBoxOutedCharacter(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::MOVE_MODE moveMode);
	void RemoveBoxOutedCharacter(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::MOVE_MODE moveMode);
	void RemoveBoxOutedCharacterAll();

protected:
	CHost* m_pHost;
	CCharacter* m_pCharacter;

};

