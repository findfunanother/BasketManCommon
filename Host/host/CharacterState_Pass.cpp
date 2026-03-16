#include "CharacterState_Pass.h"

CCharacterState_Pass::CCharacterState_Pass(CCharacter* pInfo) : CCharacterState(pInfo)
{
	Init();
}

CCharacterState_Pass::~CCharacterState_Pass()
{

}

void CCharacterState_Pass::OnEnter(void* pData)
{
	Init();
}

void CCharacterState_Pass::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	switch (m_pOwner->GetCharacterStateActionType())
	{
		case F4PACKET::ACTION_TYPE::action_alleyOopPass:
		case F4PACKET::ACTION_TYPE::action_pass:
		{
			DHOST_TYPE_FLOAT param_yaw = m_pOwner->GetStartYaw() + m_SlideYaw;
			DHOST_TYPE_FLOAT yawDest = CommonFunction::OptimizeAngle(param_yaw);

			DHOST_TYPE_FLOAT animation_time = elapsedTime - m_pOwner->GetCharacterStateActionTime();

			DHOST_TYPE_FLOAT time_delta = animation_time - m_LastAniTime;

			JOVECTOR3 sPositionSlideDelta;
			if (animation_time < m_SlideTimePosition)
			{
				CommonFunction::SVectorMultiplyEx(m_SlidePosition, (time_delta / m_SlideTimePosition), sPositionSlideDelta);
			}
			else
			{
				if (m_LastAniTime < m_SlideTimePosition && animation_time > m_SlideTimePosition)
				{
					animation_time = m_SlideTimePosition;
					time_delta = animation_time - m_LastAniTime;

					CommonFunction::SVectorMultiplyEx(m_SlidePosition, (time_delta / m_SlideTimePosition), sPositionSlideDelta);
				}
			}

			if (animation_time > m_AnimationDuration)
			{
				if (m_LastUpdateTick == true)
				{
					return;
				}

				animation_time = m_AnimationDuration;

				m_LastUpdateTick = true;
			}

			JOVECTOR3 sLastPos;
			CommonFunction::ConvertTBVectorToJoVector(m_pOwner->GetCharacterStateAction()->positionlogic(), sLastPos);

			JOVECTOR3 sRootPos;
			if (m_LastAniTime == kFLOAT_INIT)
			{
				m_pOwner->GetAnimationController()->GetRootLocalPosition(m_pOwner->GetCharacterStateAnimationIndex(), animation_time, timeDelta, m_Mirror, sRootPos);
			}
			else
			{
				m_pOwner->GetAnimationController()->GetRootLocalPositionEx(m_pOwner->GetCharacterStateAnimationIndex(), animation_time, m_LastAniTime, m_Mirror, sRootPos);
			}

			JOVECTOR3 sDiffCalcPos;
			CommonFunction::SVectorYaw(sRootPos, yawDest, sDiffCalcPos);

			JOVECTOR3 sResultPos;
			sResultPos.fX = sLastPos.fX + sDiffCalcPos.fX + sPositionSlideDelta.fX;
			sResultPos.fY = sLastPos.fY + sDiffCalcPos.fY + sPositionSlideDelta.fY;
			sResultPos.fZ = sLastPos.fZ + sDiffCalcPos.fZ + sPositionSlideDelta.fZ;

			m_LastAniTime = animation_time;

			CommonFunction::CorrOutSideEx(sResultPos);

			m_pOwner->SetCharacterStateActionPosition(sResultPos.fX, sResultPos.fY, sResultPos.fZ, "CCharacterState_Pass::OnUpdate action_alleyOopPass");

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_PASS] ActionName : %s, AnimTime : %f, Pos x : %f, z : %f, DestYaw : %f", F4PACKET::EnumNameACTION_TYPE(m_pOwner->GetCharacterStateActionType()), animation_time, sResultPos.fX, sResultPos.fZ, yawDest);
#endif
		}
		break;
		default:
		{
			//! To do
		}
		break;
	}
}

void CCharacterState_Pass::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopPass:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerAlleyOopPass_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->targetball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_alleyOopPass, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_out");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slideposition(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_PASS] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPass:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPass_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_pass, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_out");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 moved_root;
			m_pOwner->GetAnimationController()->GetRootLocalPositionZero(m_pOwner->GetCharacterStateAnimationIndex(), kFLOAT_INIT, kFLOAT_INIT, m_Mirror, moved_root);

			auto animid = packet_data->GetData()->animid();

			JOVECTOR3 anim_start_delta = m_pOwner->GetAnimationController()->CalculateRotationRootLogic(moved_root, packet_data->GetData()->playeraction()->yawlogic());

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

			m_SlidePosition = anim_start_delta;

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_PASS] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
			//LOGGER->Log("[CCHARACTER_STATE_PASS] moved_root x : %f, z : %f", moved_root.fX, moved_root.fZ);
#endif
		}
		break;
		default:
		{
			CCharacterState::OnMessage(packetID, pData, time);
		}
		break;
	}
}

void CCharacterState_Pass::OnExit()
{

}

void CCharacterState_Pass::Init()
{
	m_SlideTimePosition = kFLOAT_INIT;
	m_SlideYaw = kFLOAT_INIT;
	m_LastAniTime = kFLOAT_INIT;
	m_AnimationDuration = kFLOAT_INIT;

	m_Mirror = false;
	m_LastUpdateTick = false;

	m_SlidePosition.fX = kFLOAT_INIT;
	m_SlidePosition.fY = kFLOAT_INIT;
	m_SlidePosition.fZ = kFLOAT_INIT;

	m_RimPosition.fX = RIM_POS_X;
	m_RimPosition.fY = RIM_POS_Y;
	m_RimPosition.fZ = RIM_POS_Z_FIXED;

	m_LocalPositionBall.fX = kFLOAT_INIT;
	m_LocalPositionBall.fY = kFLOAT_INIT;
	m_LocalPositionBall.fZ = kFLOAT_INIT;
}

DHOST_TYPE_BOOL CCharacterState_Pass::GetSecondAnimationReceivePacket()
{
	return false;
}