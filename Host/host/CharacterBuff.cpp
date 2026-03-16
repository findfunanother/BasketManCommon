#include "CharacterBuff.h"
#include "Host.h"
#include "DataManagerBalance.h"


std::string CCharacterBuff::GiftBuff_JumpShotValue						= "GiftBuff_JumpShotValue";
std::string CCharacterBuff::GiftBuff_ShootDefensePerimeterValue			= "GiftBuff_ShootDefensePerimeterValue";  
std::string CCharacterBuff::GiftBuff_AnkleBreakValue_Increase			= "GiftBuff_AnkleBreakValue_Increase";
std::string CCharacterBuff::GiftBuff_AnkleBreakValue_Resistance			= "GiftBuff_AnkleBreakValue_Resistance";
std::string CCharacterBuff::GiftBuff_BlockValue_RimAttack_Increase		= "GiftBuff_BlockValue_RimAttack_Increase";
std::string CCharacterBuff::GiftBuff_BlockValue_RimAttack_Resistance	= "GiftBuff_BlockValue_RimAttack_Resistance";
std::string CCharacterBuff::GiftBuff_StealValue							= "GiftBuff_StealValue"; 
std::string CCharacterBuff::GiftBuff_NicePassValue						= "GiftBuff_NicePassValue"; 


CCharacterBuff::CCharacterBuff()
{
	m_pHost = nullptr;
	m_pCharacter = nullptr;
	m_CharacterBuffDeque.clear();
}

CCharacterBuff::~CCharacterBuff()
{

}

// 버프에 넣을 타입이 같으면 제거하고, 새로운 것을 넣는다 
void CCharacterBuff::GenerateBuff(SCharacterBuff buff)
{
	for (auto iter = m_CharacterBuffDeque.begin(); iter != m_CharacterBuffDeque.end();)
	{
		if (iter->type == buff.type && iter->strkey == buff.strkey )
		{
			iter = m_CharacterBuffDeque.erase(iter); // 
		}
		else
		{
			++iter; // 다음 요소로 이동
		}
	}
	
	m_CharacterBuffDeque.push_back(buff);

	BroadcastBuffPacket(buff);
}

void CCharacterBuff::BroadcastBuffPacket(SCharacterBuff buff)
{
	CREATE_BUILDER(builder)
	auto stringKey = builder.CreateString(buff.strkey);
	CREATE_FBPACKET(builder, play_s2c_playerBuff, message, send_data);
	send_data.add_givebuffid(buff.giveid);
	send_data.add_id(buff.id);
	send_data.add_type(buff.type);
	send_data.add_ationtype(buff.actionType);
	send_data.add_trigger(buff.trigger);
	send_data.add_durationtime(buff.durationTime);
	send_data.add_strkey(stringKey);
	send_data.add_buffvalue(buff.buffValue);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}


void CCharacterBuff::ProcessTerminateBuff(DHOST_TYPE_FLOAT timeDelta)
{
	if (m_pHost->GetCurrentState() == EHOST_STATE::PLAY)
	{
		for (auto iter = m_CharacterBuffDeque.begin(); iter != m_CharacterBuffDeque.end();)
		{
			// durationTime을 timeDelta만큼 줄임
			iter->durationTime -= timeDelta;

			// durationTime이 0보다 작으면 deque에서 제거
			if (iter->durationTime < 0.0f || (iter->trigger == false))
			{
				iter->trigger = false;

				BroadcastBuffPacket(*iter);

				iter = m_CharacterBuffDeque.erase(iter); // 현재 요소 제거 후, 다음 요소로 이동
				// 캐릭터 버프 종료 
			}
			else
			{
				++iter; // durationTime이 0보다 크면 다음 요소로 이동
			}
		}
	}
}

DHOST_TYPE_FLOAT CCharacterBuff::GetBuffBonus(F4PACKET::ACTION_TYPE actionType, F4PACKET::SHOT_TYPE sthotType)
{
	DHOST_TYPE_FLOAT reValue = 0.0f;

	for (auto iter = m_CharacterBuffDeque.begin(); iter != m_CharacterBuffDeque.end(); ++iter)
	{
		if (iter->type == F4PACKET::CHARACTER_BUFF_TYPE::Niky_Passive)
		{
			DHOST_TYPE_FLOAT addValue = 0.0f;

			if (iter->special == 1)
			{
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_JumpShotProbability_Niky_Special");
					reValue += addValue;
					continue;
				}
				else
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_ThreePointShotProbability_Niky_Special");
					reValue += addValue;
					continue;
				}
			}
			else
			if (iter->special == 2)
			{
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_JumpShotProbability_Niky_Special") * m_pHost->GetBalanceTable()->GetValue("VPassive_Special_2");
					reValue += addValue;
					continue;
				}
				else
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_ThreePointShotProbability_Niky_Special") * m_pHost->GetBalanceTable()->GetValue("VPassive_Special_2");
					reValue += addValue;
					continue;
				}
			}
			else
			{
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_middle)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_JumpShotProbability_Niky");
					reValue += addValue;
					continue;
				}
				else
				if (sthotType == F4PACKET::SHOT_TYPE::shotType_threePoint)
				{
					addValue = m_pHost->GetBalanceTable()->GetValue("VPassive_ThreePointShotProbability_Niky");
					reValue += addValue;
					continue;
				}

			}
		}

		if (iter->actionType == actionType)
		{
			reValue += iter->buffValue; // 타입별로 버프 누적 적용 
		}
	}

	if (reValue > 0.0f) // 버프를 받았다면 해당 캐릭터에 패킷 보내기 
	{
		// 이것을 보내서 이펙트를 뿌리자 
		CREATE_BUILDER(builder)
		auto stringKey = builder.CreateString("None");
		CREATE_FBPACKET(builder, play_s2c_playerBuff, message, send_data);

		send_data.add_id(m_pCharacter->GetCharacterInformation()->id);
		send_data.add_type(F4PACKET::CHARACTER_BUFF_TYPE::Buff_Effect_Get);
		send_data.add_ationtype(actionType);
		send_data.add_trigger(true);
		send_data.add_durationtime(0.0f);
		send_data.add_strkey(stringKey);
		send_data.add_buffvalue(0.0f);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return reValue;
}

DHOST_TYPE_FLOAT CCharacterBuff::GetGiftBuffBonus(F4PACKET::ACTION_TYPE actionType, std::string strKey)
{
	DHOST_TYPE_FLOAT reValue = 0.0f;

	for (auto iter = m_CharacterBuffDeque.begin(); iter != m_CharacterBuffDeque.end(); ++iter)
	{
		if (iter->strkey == strKey)
		{
			reValue += iter->buffValue; // 타입별로 버프 누적 적용 
		}
	}

	if (reValue > 0.0f) // 버프를 받았다면 해당 캐릭터에 패킷 보내기 
	{
		// 이것을 보내서 이펙트를 뿌리자 
		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerBuff, message, send_data);

		send_data.add_id(m_pCharacter->GetCharacterInformation()->id);
		send_data.add_type(F4PACKET::CHARACTER_BUFF_TYPE::Buff_Effect_Get);
		send_data.add_ationtype(actionType);
		send_data.add_trigger(true);
		send_data.add_durationtime(0.0f);
		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
	}

	return reValue;
}