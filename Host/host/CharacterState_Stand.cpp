#include "CharacterState_Stand.h"
#include "Host.h"
#include "VerifyManager.h"

CCharacterState_Stand::CCharacterState_Stand(CCharacter* pInfo) : CCharacterState(pInfo)
{

}

CCharacterState_Stand::~CCharacterState_Stand()
{

}

void CCharacterState_Stand::OnEnter(void* pData)
{

}

void CCharacterState_Stand::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	if (m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::boxOut || m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
	{
		m_pOwner->GetHost()->GetCharacterManager()->CheckBoxOut(m_pOwner, m_pOwner->GetMoveMode());
		
		/*
		string _log = "*** Inlcude BoxOut OnUpdate **** ";
		m_pOwner->GetHost()->ToLog(_log.c_str());
		m_pOwner->GetHost()->SendSyncPosition(SYNCPOSITION_TYPE::BOXOUT_FLAG_ON);
		*/
		

	}
}

void CCharacterState_Stand::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerStand:
		{

			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerStand_data>*)pData;

			//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
			if (packet_data == nullptr)
			{
				return;
			}

			auto* data = packet_data->GetData();
			if (data == nullptr)
			{
				return;
			}

			flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(packet_data->m_pdata), packet_data->m_size);
			bool data_check = data->Verify(packet_verify);

			if (data_check == false)
			{
				return;
			}

			if (data->playeraction() == nullptr)
			{
				return;
			}
			//!

			if (m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::boxOut || m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
			{
				m_pOwner->GetHost()->GetCharacterManager()->CheckBoxOut(m_pOwner, m_pOwner->GetMoveMode());
			}
			else
			{   // 박스아웃 스탠드 
				if (m_pOwner->GetPreMoveMode() == F4PACKET::MOVE_MODE::boxOut || m_pOwner->GetPreMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
				{
					m_pOwner->RemoveBoxOutedCharacterAll();
					// 1. 박스 아웃 엔드 일때 모든 해당 캐릭터터는 BoxOuted 를 false 로 해줘야 함 , 그런데 이 캐릭터가 다른 캐릭터에 박스아웃 BoxOuted 돼 있으면 어떻하지 ? 박스아웃을 여려명이 할 수 있다
				}
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_stand, time);

			break;
		}

		default:
		{
			CCharacterState::OnMessage(packetID, pData, time);
		}

	}
}

void CCharacterState_Stand::OnExit()
{
}

DHOST_TYPE_BOOL CCharacterState_Stand::GetSecondAnimationReceivePacket()
{
	return false;
}