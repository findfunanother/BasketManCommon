#pragma once

#include <deque>
#include <map>
#include <string>

#include "HostCommonFunction.h"
#include "Ability.h"
#include "Util.h"
#include "FileLog.h"
#include "../../GameCore/CAnimationController.h"

class CHost;
class CCharacter;

struct SCharacterBuff
{
	DHOST_TYPE_INT32 giveid;
	DHOST_TYPE_INT32 id;
	F4PACKET::CHARACTER_BUFF_TYPE type;
	F4PACKET::ACTION_TYPE actionType;
	DHOST_TYPE_BOOL  trigger;
	DHOST_TYPE_FLOAT durationTime;
	DHOST_TYPE_FLOAT buffValue;
	DHOST_TYPE_INT32 special;
	string strkey;

	SCharacterBuff(DHOST_TYPE_INT32 giveid, DHOST_TYPE_INT32 id, F4PACKET::CHARACTER_BUFF_TYPE t, F4PACKET::ACTION_TYPE at, DHOST_TYPE_BOOL tr, DHOST_TYPE_FLOAT dt, DHOST_TYPE_FLOAT fv, DHOST_TYPE_INT32 sp, string strKey = "None")
		: giveid(giveid), id(id), type(t), actionType(at), trigger(tr), durationTime(dt), buffValue(fv), special(sp), strkey(strKey) {}
};

class CCharacterBuff
{

public:
	static std::string GiftBuff_JumpShotValue;						// 미드레인지 점프샷과 3점샷의 성공률이 증가한다.
	static std::string GiftBuff_ShootDefensePerimeterValue;			// 미드레인지 점프샷과 3점샷에 대한 슈팅 방해가 증가한다.
	static std::string GiftBuff_AnkleBreakValue_Increase;			// 앵클브레이커 확률이 증가한다.
	static std::string GiftBuff_AnkleBreakValue_Resistance;			// 앵클브레이커 확률이 감소한다. 
	static std::string GiftBuff_BlockValue_RimAttack_Increase;		// 레이업과 덩크의 블록 확률이 증가한다.
	static std::string GiftBuff_BlockValue_RimAttack_Resistance;	// 레이업과 덩크의 블록 확률이 감소한다.
	static std::string GiftBuff_StealValue;							// 스틸 성공률이 증가한다.
	static std::string GiftBuff_NicePassValue;						// 나이스 패스 발동 확률이 증가한다.
 	

public :
	CCharacterBuff();
	~CCharacterBuff();

protected:
	CHost* m_pHost;
	CCharacter* m_pCharacter;
	std::deque<SCharacterBuff> m_CharacterBuffDeque;

public:
	void GenerateBuff(SCharacterBuff buff);
	std::deque<SCharacterBuff> GetCharacterBuffDeque() { return m_CharacterBuffDeque; }
	DHOST_TYPE_FLOAT GetBuffBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType = F4PACKET::SHOT_TYPE::shotType_none);
	DHOST_TYPE_FLOAT GetGiftBuffBonus(F4PACKET::ACTION_TYPE actionType, std::string strKey);

	// 버프 관련 
protected:
	void ProcessTerminateBuff(DHOST_TYPE_FLOAT timeDelta);
	void BroadcastBuffPacket(SCharacterBuff buff);


};

