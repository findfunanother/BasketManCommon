#include "CharacterAction.h"
#include "Host.h"
#include "DataManagerBalance.h"

CCharacterAction::CCharacterAction()
{
	m_pHost = nullptr;
	m_pCharacter = nullptr;
	m_CurrentAction = F4PACKET::PACKET_ID::packetID_none;
}

CCharacterAction::~CCharacterAction()
{

}


void CCharacterAction::SetCurrentAction(F4PACKET::PACKET_ID currentAction)
{
	m_CurrentAction = currentAction;
}

void CCharacterAction::AddBoxOutedCharacter(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::MOVE_MODE moveMode)
{
	if (m_ListBoxOuted.empty())
	{
		//string _log = "*** Turn On Light **** characterSN : " + std::to_string(characterSN) + "Onwer: " + std::to_string(m_pCharacter->GetCharacterInformation()->id);
		//m_pCharacter->GetHost()->ToLog(_log.c_str());

		// 일단 모드에게 동기화 
		if (moveMode == F4PACKET::MOVE_MODE::enhancedBoxOut)
		{
			m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::ENHANCED_BOXOUT_FLAG_ON, m_pCharacter->GetCharacterInformation()->id, characterSN); // 불켜기 
		}
		else
		{
			m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::BOXOUT_FLAG_ON, m_pCharacter->GetCharacterInformation()->id, characterSN); // 불켜기 
		}
	}

	auto it = std::find(m_ListBoxOuted.begin(), m_ListBoxOuted.end(), characterSN);

	if (it == m_ListBoxOuted.end()) 
	{
		// 없으면 넣어 
		m_ListBoxOuted.push_back(characterSN);
	}

}

void CCharacterAction::RemoveBoxOutedCharacter(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::MOVE_MODE moveMode)
{
	auto it = std::find(m_ListBoxOuted.begin(), m_ListBoxOuted.end(), characterSN);

	if (it != m_ListBoxOuted.end()) 
	{
		//m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::BOXOUT_FLAG_REMOVE, m_pCharacter->GetCharacterInformation()->id, characterSN); //

		// 있으면 지워 
		m_ListBoxOuted.erase(it);

		if (m_ListBoxOuted.empty()) // 아무도 없으므로 
		{

			if (moveMode == F4PACKET::MOVE_MODE::boxOut)
			{
				// 일단 모드에게 동기화 
				m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::BOXOUT_FLAG_OFF, m_pCharacter->GetCharacterInformation()->id, characterSN); //
			}
			else
			{
				// 일단 모드에게 동기화 
				m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::ENHANCED_BOXOUT_FLAG_OFF, m_pCharacter->GetCharacterInformation()->id, characterSN); //

			}

			//string _log = "*** Turn Onff Light **** characterSN : " + std::to_string(characterSN);
			//m_pCharacter->GetHost()->ToLog(_log.c_str());

		}
	}
}

void CCharacterAction::RemoveBoxOutedCharacterAll()
{
	
	if (!m_ListBoxOuted.empty())
	{
		m_ListBoxOuted.clear();
	}
	
	m_pCharacter->GetHost()->SendSyncInfo(SYNCINFO_TYPE::BOXOUT_FLAG_OFF, m_pCharacter->GetCharacterInformation()->id, 0); //

	//string _log = "*** RemoveBoxOutedCharacterAll **** characterSN : " + std::to_string(0);
    //m_pCharacter->GetHost()->ToLog(_log.c_str());

}






