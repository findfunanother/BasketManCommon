#include "CharacterState_Pick.h"

CCharacterState_Pick::CCharacterState_Pick(CCharacter* pInfo) : CCharacterState(pInfo)
{
	Init();
}

CCharacterState_Pick::~CCharacterState_Pick()
{

}

void CCharacterState_Pick::OnEnter(void* pData)
{
	Init();
}

void CCharacterState_Pick::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	switch (m_pOwner->GetCharacterStateActionType())
	{
		case F4PACKET::ACTION_TYPE::action_pick:
		case F4PACKET::ACTION_TYPE::action_intercept:
		case F4PACKET::ACTION_TYPE::action_receivePass:
		case F4PACKET::ACTION_TYPE::action_steal:
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
			m_pOwner->GetAnimationController()->GetRootLocalPositionEx(m_pOwner->GetCharacterStateAnimationIndex(), animation_time, m_LastAniTime, m_Mirror, sRootPos);

			JOVECTOR3 sDiffCalcPos;
			CommonFunction::SVectorYaw(sRootPos, yawDest, sDiffCalcPos);

			JOVECTOR3 sResultPos;
			sResultPos.fX = sLastPos.fX + sDiffCalcPos.fX + sPositionSlideDelta.fX;
			sResultPos.fY = sLastPos.fY + sDiffCalcPos.fY + sPositionSlideDelta.fY;
			sResultPos.fZ = sLastPos.fZ + sDiffCalcPos.fZ + sPositionSlideDelta.fZ;

			m_LastAniTime = animation_time;

			CommonFunction::CorrOutSideEx(sResultPos);

			m_pOwner->SetCharacterStateActionPosition(sResultPos.fX, sResultPos.fY, sResultPos.fZ, "CCharacterState_Pick::OnUpdate action_pick");

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_PICK] AnimTime : %f, Pos x : %f, z : %f, DestYaw : %f", animation_time, sResultPos.fX, sResultPos.fZ, yawDest);
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

void CCharacterState_Pick::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerPick:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPick_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_pick, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());
			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_in");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();
			m_Success = packet_data->GetData()->success();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slideposition(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_PICK] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_PICK] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_PICK] player.SyncedPosition x : %f, z : %f, fsmMessagePick.slidePosition x : %f, z :  %f, animRootInfo.startPosition x : %f, z : %f", packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), packet_data->GetData()->slideposition()->x(), packet_data->GetData()->slideposition()->z(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_PICK] SyncedYaw : %f, startYaw : %f", packet_data->GetData()->playeraction()->yawlogic(), m_pOwner->GetStartYaw());
			//LOGGER->Log("[CCHARACTER_STATE_PICK] AnimID : %d, m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_pOwner->GetCharacterStateAnimationIndex(), m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerIntercept:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerIntercept_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->slideposition() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_intercept, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			std::string event_name = "ball_in";

			std::string ani_name = m_pOwner->GetAnimationController()->GetAnimationName(packet_data->GetData()->animid());

			if (ani_name == "INTERCEPT_TOUCH_NORMAL_01" || ani_name == "INTERCEPT_TOUCH_HIGH_01")
			{
				event_name = "ball_out";
			}
			
			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), event_name);
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slideposition(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerReceivePass:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerReceivePass_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_receivePass, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_in");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSteal:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerSteal_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_steal, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 direction;
			direction.fX = packet_data->GetData()->positionball()->x() - packet_data->GetData()->playeraction()->positionlogic().x();
			direction.fY = packet_data->GetData()->positionball()->y() - packet_data->GetData()->playeraction()->positionlogic().y();
			direction.fZ = packet_data->GetData()->positionball()->z() - packet_data->GetData()->playeraction()->positionlogic().z();

			DHOST_TYPE_FLOAT direction_yaw = CommonFunction::ToYaw(direction);

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		default:
		{
			CCharacterState::OnMessage(packetID, pData, time);
		}
		break;

		m_LastAniTime = kFLOAT_INIT;
		m_LastUpdateTick = false;
	}
}

void CCharacterState_Pick::OnExit()
{

}

void CCharacterState_Pick::Init()
{
	m_Success = false;
	m_CorrPosition = false;
	m_LastUpdateTick = false;
	m_Mirror = false;

	m_BallNumber = kINT32_INIT;
	m_TargetID = kINT32_INIT;

	m_BallPosition.fX = kFLOAT_INIT;
	m_BallPosition.fY = kFLOAT_INIT;
	m_BallPosition.fZ = kFLOAT_INIT;

	m_Direction.fX = kFLOAT_INIT;
	m_Direction.fY = kFLOAT_INIT;
	m_Direction.fZ = kFLOAT_INIT;

	m_SlidePosition.fX = kFLOAT_INIT;
	m_SlidePosition.fY = kFLOAT_INIT;
	m_SlidePosition.fZ = kFLOAT_INIT;

	m_LocalPositionBall.fX = kFLOAT_INIT;
	m_LocalPositionBall.fY = kFLOAT_INIT;
	m_LocalPositionBall.fZ = kFLOAT_INIT;

	m_SlideTimePosition = kFLOAT_INIT;
	m_SlideYaw = kFLOAT_INIT;
	m_LastAniTime = kFLOAT_INIT;
	m_AnimationDuration = kFLOAT_INIT;
}

DHOST_TYPE_BOOL CCharacterState_Pick::GetSecondAnimationReceivePacket()
{
	return false;
}