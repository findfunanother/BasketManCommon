#include "CharacterState_Shot.h"

CCharacterState_Shot::CCharacterState_Shot(CCharacter* pInfo) : CCharacterState(pInfo)
{
	Init();
}

CCharacterState_Shot::~CCharacterState_Shot()
{

}

void CCharacterState_Shot::OnEnter(void* pData)
{
	Init();
}

void CCharacterState_Shot::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	switch (m_pOwner->GetCharacterStateActionType())
	{
		case F4PACKET::ACTION_TYPE::action_fakeShot:
		{
			return;
		}
		break;
		case F4PACKET::ACTION_TYPE::action_shot:
		case F4PACKET::ACTION_TYPE::action_alleyOopShot:
		case F4PACKET::ACTION_TYPE::action_tipIn:
		{
			DHOST_TYPE_FLOAT param_yaw = m_pOwner->GetStartYaw() + m_SlideYaw;
			DHOST_TYPE_FLOAT yawDest = CommonFunction::OptimizeAngle(param_yaw);

			if (m_DoubleClutchYawDest == kFLOAT_INIT)
			{
				m_DoubleClutchYawDest = yawDest;
			}

			if (m_DoubleClutchPacketReceive)
			{
				yawDest = m_DoubleClutchYawDest;
			}

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

			m_pOwner->SetCharacterStateActionPosition(sResultPos.fX, sResultPos.fY, sResultPos.fZ, "CCharacterState_Shot::OnUpdate action_shot");

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] ActionName : %s, AnimTime : %f, Pos x : %f, z : %f, DestYaw : %f", F4PACKET::EnumNameACTION_TYPE(m_pOwner->GetCharacterStateActionType()), animation_time, sResultPos.fX, sResultPos.fZ, yawDest);
#endif

			// 앨리웁슛인 경우
			if (m_SecondAnimationID > kINT32_INIT)
			{
				if (animation_time == m_SlideTimePosition && m_bCheckSecondAnimationExcute == false)
				{
					DHOST_TYPE_FLOAT param_time = elapsedTime + time_delta;
					SetSecondAnimation(elapsedTime + time_delta);
				}
			}
		}
		break;
		default:
		{
			//! To do
		}
		break;
	}
}

void CCharacterState_Shot::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerShot:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerShot_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_shot, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_out");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_IsLeftHanded = packet_data->GetData()->islefthanded();
			m_pOwner->SetShotIsLeftHanded(packet_data->GetData()->islefthanded());
			m_ShotType = packet_data->GetData()->shottype();
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slideposition(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketName : %s, AnimID : %d, AnimationDuration : %f, ShotType : %s", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration, F4PACKET::EnumNameSHOT_TYPE(packet_data->GetData()->shottype()));
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerDoubleClutch:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerDoubleClutch_data>*)pData;

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

			if (data->syncedposition() == nullptr || data->slideposition() == nullptr)
			{
				return;
			}
			//!

			// 더클 패킷이 이미 날라온 시점에는 추가적으로 오는 패킷은 처리하지 않는다.
			if (m_DoubleClutchPacketReceive == true)
			{
				return;
			}

			if (time - m_pOwner->GetCharacterStateActionTime() < 0.3f)
			{
				m_DoubleClutchPacketReceive = true;

				m_pOwner->SetStartYaw(m_pOwner->GetCharacterStateAction()->yawlogic());
				m_pOwner->SetStartPosition(m_pOwner->GetCharacterStateAction());

				m_pOwner->SetCharacterStateActionPosition(packet_data->GetData()->syncedposition()->x(), packet_data->GetData()->syncedposition()->y(), packet_data->GetData()->syncedposition()->z(), "null");
				m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
				m_pOwner->SetCharacterStateStartAnimationPosition(*packet_data->GetData()->syncedposition());

				m_pOwner->GetCharacterStateAction()->mutate_skillindex(F4PACKET::SKILL_INDEX::skill_doubleCluch);

				m_pOwner->SetCharacterStateActionTime(time);

				m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), "ball_out");
				m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
				m_Mirror = packet_data->GetData()->ismirroranim();

				TB::SVector3 position = CommonFunction::SVectorSlidePosition(*packet_data->GetData()->syncedposition(), *packet_data->GetData()->slideposition(), m_pOwner->GetStartPosition());
				CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

				DHOST_TYPE_FLOAT target_yaw = m_pOwner->GetCharacterStateAction()->yawlogic() - m_pOwner->GetStartYaw();
				m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

				m_LastAniTime = kFLOAT_INIT;

#ifdef TEXT_LOG_FILE
				//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
				//LOGGER->Log("[CCHARACTER_STATE_SHOT] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
				//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", m_pOwner->GetCharacterStateAction()->yawlogic(), packet_data->GetData()->syncedposition()->x(), packet_data->GetData()->syncedposition()->z(), m_SlideTimePosition);
				//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
			}

		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopShot:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerAlleyOopShot_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->secondlocalpositionball() == nullptr || data->slidepositiona() == nullptr || data->slidepositionb() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_alleyOopShot, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animfirstid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animfirstid(), "ball_in");
			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animfirstid());

			m_IsLeftHanded = packet_data->GetData()->ismirror();
			m_SecondAnimationID = packet_data->GetData()->animsecondid();
			m_Mirror = packet_data->GetData()->ismirroranim();

			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->slidepositiona(), m_FirstSlidePositionA);
			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->slidepositionb(), m_SecondSlidePositionB);
			m_SecondSlideYawB = packet_data->GetData()->slideyawb();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slidepositiona(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerFakeShot:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerFakeShot_data>*)pData;

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

			TB::SVector3 current_position = m_pOwner->GetCharacterStateAction()->positionlogic();

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_fakeShot, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());
			m_Mirror = packet_data->GetData()->ismirroranim();

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerTipIn:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerTipIn_data>*)pData;

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

			if (data->playeraction() == nullptr || data->positionball() == nullptr || data->localpositionball() == nullptr || data->secondlocalpositionball() == nullptr || data->slidepositiona() == nullptr || data->slidepositionb() == nullptr)
			{
				return;
			}
			//!

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_tipIn, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			std::string event_name = "ball_out";
			std::string aniName = "";

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_IsLeftHanded = packet_data->GetData()->ismirror();
			
			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->localpositionball(), m_LocalPositionBall);
			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->secondlocalpositionball(), m_SecondLocalPositionBall);

			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->slidepositiona(), m_FirstSlidePositionA);
			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->slidepositionb(), m_SecondSlidePositionB);

			m_SecondSlideYawB = packet_data->GetData()->slideyawb();
			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 sStartPos;
			CommonFunction::ConvertTBVectorToJoVector(packet_data->GetData()->playeraction()->positionlogic(), sStartPos);

			JOVECTOR3 sBallPos;
			CommonFunction::ConvertTBVectorToJoVector(*packet_data->GetData()->positionball(), sBallPos);

			m_BallPosition = sBallPos;

			switch (packet_data->GetData()->playeraction()->skillindex())
			{
				case F4PACKET::SKILL_INDEX::skill_tapDunk:
				{
					event_name = "ball_in";

					m_YawDest = CommonFunction::SVectorSubYawEx(RIM_POS_X, RIM_POS_Y, RIM_POS_Z_FIXED, *packet_data->GetData()->positionball());

					std::string tap_dunk_ani_name = m_pOwner->GetAnimationController()->GetAnimationName(packet_data->GetData()->animid());

					if (tap_dunk_ani_name == "TAPDUNK_SHORT_0_01_SCALE")
					{
						aniName = "TAPDUNK_SHORT_0_01";
					}
					else if (tap_dunk_ani_name == "TAPDUNK_SHORT_90_01_SCALE")
					{
						aniName = "TAPDUNK_SHORT_90_01";
					}
					else if (tap_dunk_ani_name == "TAPDUNK_SHORT_180_01_SCALE")
					{
						aniName = "TAPDUNK_SHORT_180_01";
					}

					m_SecondAnimationID = m_pOwner->GetAnimationController()->FindAnimationInfo(aniName);
				}
				break;
				default:
				{
					m_YawDest = CommonFunction::OptimizeAngle(CommonFunction::SVectorSubYawEx(RIM_POS_X, RIM_POS_Y, RIM_POS_Z_FIXED, packet_data->GetData()->playeraction()->positionlogic()));
				}
				break;
			}

			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(packet_data->GetData()->animid(), event_name);

			DHOST_TYPE_FLOAT direction_delta = CommonFunction::OptimizeAngle(m_YawDest - packet_data->GetData()->playeraction()->yawlogic());

			m_SlideYaw = direction_delta;

			if (packet_data->GetData()->success())
			{
				m_SlidePosition = m_pOwner->GetAnimationController()->CalculateSlideWithLocalPosition(sStartPos, sBallPos, m_YawDest, m_LocalPositionBall);
			}

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), *packet_data->GetData()->slidepositiona(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
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

void CCharacterState_Shot::OnExit()
{

}

void CCharacterState_Shot::Init()
{
	m_BallNumber = kINT32_INIT;
	m_ShotType = F4PACKET::SHOT_TYPE::shotType_none;
	m_BallPosition.fX = kFLOAT_INIT;
	m_BallPosition.fY = kFLOAT_INIT;
	m_BallPosition.fZ = kFLOAT_INIT;
	m_Speed = kFLOAT_INIT;
	m_EnableDribble = false;
	m_ReadyAnimation = false;
	m_IsLeftHanded = false;
	m_Mirror = false;

	m_Direction.fX = kFLOAT_INIT;
	m_Direction.fY = kFLOAT_INIT;
	m_Direction.fZ = kFLOAT_INIT;
	m_SlidePosition.fX = kFLOAT_INIT;
	m_SlidePosition.fY = kFLOAT_INIT;
	m_SlidePosition.fZ = kFLOAT_INIT;
	m_SlideTimePosition = kFLOAT_INIT;
	m_SlideYaw = kFLOAT_INIT;
	m_LastAniTime = kFLOAT_INIT;
	m_AnimationDuration = kFLOAT_INIT;
	m_YawDest = kFLOAT_INIT;

	m_RimPosition.fX = RIM_POS_X;
	m_RimPosition.fY = RIM_POS_Y;
	m_RimPosition.fZ = RIM_POS_Z_FIXED;

	m_DoubleClutchPacketReceive = false;
	m_DoubleClutchYawDest = kFLOAT_INIT;
	m_LastUpdateTick = false;
	m_LastSlideUpdateTick = false;

	m_LocalPositionBall.fX = kFLOAT_INIT;
	m_LocalPositionBall.fY = kFLOAT_INIT;
	m_LocalPositionBall.fZ = kFLOAT_INIT;

	m_SecondAnimationID = kINT32_INIT;
	m_bCheckSecondAnimationExcute = false;
	m_SecondLocalPositionBall.fX = kFLOAT_INIT;
	m_SecondLocalPositionBall.fY = kFLOAT_INIT;
	m_SecondLocalPositionBall.fZ = kFLOAT_INIT;

	m_FirstSlidePositionA.fX = kFLOAT_INIT;
	m_FirstSlidePositionA.fY = kFLOAT_INIT;
	m_FirstSlidePositionA.fZ = kFLOAT_INIT;

	m_SecondSlidePositionB.fX = kFLOAT_INIT;
	m_SecondSlidePositionB.fY = kFLOAT_INIT;
	m_SecondSlidePositionB.fZ = kFLOAT_INIT;

	m_SecondSlideYawB = kFLOAT_INIT;
}

void CCharacterState_Shot::SetSecondAnimation(DHOST_TYPE_FLOAT CalcElapsedTime)
{
	// 앨리웁슛 같은 경우 한 패킷으로 두번의 애니메이션을 처리해야 한다. 
	// 그래서 업데이트틱에서 첫번째 애니메이션의 볼 이벤트(ball_in, rebound_fail)를 처리하고 두번째 애니메이션 (ball_out, 레이업이나 덩크) 을 처리한다
	if (m_pOwner->GetCharacterStateAnimationIndex() != m_SecondAnimationID && m_SecondAnimationID > kINT32_INIT)
	{
		m_SlideTimePosition = kFLOAT_INIT;
		m_SlideYaw = kFLOAT_INIT;
		m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(m_SecondAnimationID);

		EALLEYOOP_SHOT_TYPE shot_type = EALLEYOOP_SHOT_TYPE::NONE;

		std::string event_name = m_pOwner->GetAnimationController()->GetAnimationName(m_SecondAnimationID);

		if (event_name == "ALLEYOOPLAYUP_0_01" || event_name == "ALLEYOOPLAYUP_90_01" || event_name == "ALLEYOOPLAYUP_180_01")
		{
			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(m_SecondAnimationID, "ball_out");
			shot_type = EALLEYOOP_SHOT_TYPE::LAYUP;
		}
		else if (event_name == "ALLEYOOPDUNK_0_01" || event_name == "ALLEYOOPDUNK_90_01" || event_name == "ALLEYOOPDUNK_180_01"
			|| event_name == "TAPDUNK_SHORT_0_01" || event_name == "TAPDUNK_SHORT_90_01" || event_name == "TAPDUNK_SHORT_180_01"
			)
		{
			m_SlideTimePosition = m_pOwner->GetAnimationController()->GetEventTime(m_SecondAnimationID, "dunk1");
			shot_type = EALLEYOOP_SHOT_TYPE::DUNK;
		}
		else
		{
			//! To do
		}

		JOVECTOR3 sStartPos;
		CommonFunction::ConvertTBVectorToJoVector(m_pOwner->GetCharacterStateAction()->positionlogic(), sStartPos);

		m_bCheckSecondAnimationExcute = true;
		m_LastAniTime = kFLOAT_INIT;

		JOVECTOR3 packet_data_position_logic = m_pOwner->GetCharacterStateStartAnimationPosition();

		m_pOwner->SetCharacterStateActionTime(CalcElapsedTime);
		m_pOwner->SetCharacterStateAnimationIndex(m_SecondAnimationID);
		//m_pOwner->SetCharacterStateStartAnimationPosition(m_pOwner->GetCharacterStateAction()->positionlogic());

		DHOST_TYPE_FLOAT character_yaw = m_pOwner->GetCharacterStateAction()->yawlogic();

		JOVECTOR3 target_position;

		switch (shot_type)
		{
			case EALLEYOOP_SHOT_TYPE::LAYUP:
			{
				JOVECTOR3 sub;
				sub.fX = packet_data_position_logic.fX - m_RimPosition.fX;
				sub.fY = kFLOAT_INIT;
				sub.fZ = packet_data_position_logic.fZ - m_RimPosition.fZ;

				CommonFunction::SJoVectorMultiply(sub, 0.5f, target_position);
				target_position.fX += m_RimPosition.fX;
				target_position.fY += m_RimPosition.fY;
				target_position.fZ += m_RimPosition.fZ;
			}
			break;
			case EALLEYOOP_SHOT_TYPE::DUNK:
			{
				target_position.fX = m_RimPosition.fX;
				target_position.fY = m_RimPosition.fY;
				target_position.fZ = m_RimPosition.fZ;
			}
			break;
			default:
			{
				//! To do
			}
			break;
		}




		TB::SVector3 vecter_add = CommonFunction::SVectorAdd(m_FirstSlidePositionA, m_SecondSlidePositionB);

		TB::SVector3 player_pos = CommonFunction::ConvertJoVectorToTBVector(m_pOwner->GetCharacterStateAction()->positionlogic());

		TB::SVector3 position = CommonFunction::SVectorSlidePosition(CommonFunction::ConvertJoVectorToTBVector(m_pOwner->GetCharacterStateStartAnimationPosition()), vecter_add, player_pos);
		CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

		DHOST_TYPE_FLOAT target_yaw = m_pOwner->GetCharacterStateAction()->yawlogic() - m_pOwner->GetStartYaw();
		m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
		//LOGGER->Log("[CCHARACTER_STATE_SHOT] AnimID : %d, AnimationDuration : %f", m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
		//LOGGER->Log("[CCHARACTER_STATE_SHOT] playerYaw : %f, playerPos x : %f, z : %f", m_pOwner->GetCharacterStateAction()->yawlogic(), player_pos.x(), player_pos.z());
		//LOGGER->Log("[CCHARACTER_STATE_SHOT] PacketYaw : %f, PacketPos x : %f, z : %f, BallOutEventTime : %f", m_pOwner->GetCharacterStateAction()->yawlogic(), m_pOwner->GetCharacterStateStartAnimationPosition().fX, m_pOwner->GetCharacterStateStartAnimationPosition().fZ, m_SlideTimePosition);
		//LOGGER->Log("[CCHARACTER_STATE_SHOT] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
	}
}

DHOST_TYPE_BOOL CCharacterState_Shot::GetSecondAnimationReceivePacket()
{
	return m_DoubleClutchPacketReceive;
}