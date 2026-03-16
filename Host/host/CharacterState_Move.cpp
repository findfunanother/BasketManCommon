#include "CharacterState_Move.h"
#include "Host.h"
CCharacterState_Move::CCharacterState_Move(CCharacter* pInfo) : CCharacterState(pInfo)
{
	Init();
}

CCharacterState_Move::~CCharacterState_Move()
{
	
}

void CCharacterState_Move::OnEnter(void* pData)
{
	m_pOwner->InitCharacterStateStartAnimationPosition();

	Init();
}

void CCharacterState_Move::OnUpdate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	switch (m_pOwner->GetCharacterStateActionType())
	{
		//case F4PACKET::ACTION_TYPE::action_penetrateReady:
		//case F4PACKET::ACTION_TYPE::action_penetrate:
		case F4PACKET::ACTION_TYPE::dash:
		case F4PACKET::ACTION_TYPE::action_cutIn:
		case F4PACKET::ACTION_TYPE::action_vCut:
		case F4PACKET::ACTION_TYPE::action_hopStep:
		case F4PACKET::ACTION_TYPE::action_penetratePostUp:
		case F4PACKET::ACTION_TYPE::action_pickAndMove:
		case F4PACKET::ACTION_TYPE::action_pickAndSlip:
		case F4PACKET::ACTION_TYPE::action_postUpReady:
		case F4PACKET::ACTION_TYPE::action_postUpPenetrate:
		case F4PACKET::ACTION_TYPE::action_postUpStepBack:
		case F4PACKET::ACTION_TYPE::action_screen:
		case F4PACKET::ACTION_TYPE::action_shakeAndBake:
		case F4PACKET::ACTION_TYPE::action_slideStep:
		case F4PACKET::ACTION_TYPE::action_spinMove:
		case F4PACKET::ACTION_TYPE::action_slipAndSlide:
		case F4PACKET::ACTION_TYPE::action_stun:
		case F4PACKET::ACTION_TYPE::action_collision:
		case F4PACKET::ACTION_TYPE::action_closeOut:
		case F4PACKET::ACTION_TYPE::action_chaseContest:
		case F4PACKET::ACTION_TYPE::action_goAndCatch:
		case F4PACKET::ACTION_TYPE::catchAndShot_Move:
		case F4PACKET::ACTION_TYPE::action_crossOverPenetrate:
		case F4PACKET::ACTION_TYPE::action_handCheck:
		case F4PACKET::ACTION_TYPE::action_hookHook:
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

			// 첫번째 키프레임에서 오차나는거 일단 예외처리
			if (m_LastAniTime == kFLOAT_INIT && m_pOwner->GetCharacterStateAction()->skillindex() == F4PACKET::SKILL_INDEX::skill_penetrateSideStep)
			{
				m_LastAniTime = animation_time;
			}

			if (m_LastAniTime == kFLOAT_INIT && m_pOwner->GetCharacterStateActionType() == F4PACKET::ACTION_TYPE::action_penetratePostUp)
			{
				m_LastAniTime = animation_time;
			}

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

			m_pOwner->SetCharacterStateActionPosition(sResultPos.fX, sResultPos.fY, sResultPos.fZ, "CCharacterState_Move::OnUpdate CorrOutSideEx");
#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] ActionName : %s, AnimTime : %f, Pos x : %f, z : %f, DestYaw : %f", F4PACKET::EnumNameACTION_TYPE(m_pOwner->GetCharacterStateActionType()), animation_time, sResultPos.fX, sResultPos.fZ, yawDest);
#endif
		}
		break;
		case F4PACKET::ACTION_TYPE::action_move:
		{

			if (m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::boxOut || m_pOwner->GetMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
			{
				m_pOwner->GetHost()->GetCharacterManager()->CheckBoxOut(m_pOwner, m_pOwner->GetMoveMode());
				
				//string _log = "*** BoxOUt Sucess Move  **** ";
				//m_pOwner->GetHost()->ToLog(_log.c_str());
				
			}
			else
			{
				if (m_pOwner->GetPreMoveMode() == F4PACKET::MOVE_MODE::boxOut || m_pOwner->GetPreMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut)
				{
					m_pOwner->SetPreMoveMode(m_pOwner->GetMoveMode());
					//m_pOwner->GetHost()->ToLog(_log.c_str());
					m_pOwner->RemoveBoxOutedCharacterAll();
				}

				//LOGGER->Log("*** BoxOUt End");
			}

			DHOST_TYPE_FLOAT moveDelta = timeDelta * m_Speed;

			JOVECTOR3 sCalcPos;
			CommonFunction::SJoVectorMultiply(m_MoveDirection, moveDelta, sCalcPos);

			JOVECTOR3 sResultPos;
			sResultPos.fX = sCalcPos.fX + m_pOwner->GetCharacterStateAction()->positionlogic().x();
			sResultPos.fY = sCalcPos.fY + m_pOwner->GetCharacterStateAction()->positionlogic().y();
			sResultPos.fZ = sCalcPos.fZ + m_pOwner->GetCharacterStateAction()->positionlogic().z();

			if (isnan(sResultPos.fX)) sResultPos.fX = 0.0f;
			if (isnan(sResultPos.fY)) sResultPos.fY = 0.0f;
			if (isnan(sResultPos.fZ)) sResultPos.fZ = 3.0f;

			switch (m_pOwner->GetCharacterRestriction())
			{
				case EMOVEMENT_RESTRICTION::OUTSIDE_TO_INSIDE_RESTRICTION:
				{
					TB::SVector3 temp = CommonFunction::ConvertJoVectorToTBVector(sResultPos);
					TB::SVector3 result = CommonFunction::CorrPositionThreePointOutEx(temp);
					m_pOwner->SetCharacterStateActionPosition(result.x(), result.y(), result.z(), "CCharacterState_Move::OnUpdate CorrPositionThreePointOutEx");
				}
				break;
				case EMOVEMENT_RESTRICTION::INSIDE_TO_OUTSIDE_RESTRICTION:
				{
					TB::SVector3 temp = CommonFunction::ConvertJoVectorToTBVector(sResultPos);
					TB::SVector3 result = CommonFunction::CorrPositionThreePointInEx(temp);
					m_pOwner->SetCharacterStateActionPosition(result.x(), result.y(), result.z(), "CCharacterState_Move::OnUpdate CorrPositionThreePointInEx");
				}
				break;
				default:
				{
					CommonFunction::CorrOutSideEx(sResultPos);
					m_pOwner->SetCharacterStateActionPosition(sResultPos.fX, sResultPos.fY, sResultPos.fZ, "CCharacterState_Move::OnUpdate CorrOutSideEx");
				}
				break;
			}
//#ifdef TEXT_LOG_FILE
//					LOGGER->Log("[CCHARACTER_STATE_MOVE] OnUpdate moveDelta : %f", moveDelta);
//					LOGGER->Log("[CCHARACTER_STATE_MOVE] OnUpdate move x : %f, z : %f", sResultPos.fX, sResultPos.fZ);
//					LOGGER->Log("[CCHARACTER_STATE_MOVE] OnUpdate pos x : %f, z : %f", m_pOwner->GetCharacterStateAction()->positionlogic().x(), m_pOwner->GetCharacterStateAction()->positionlogic().z());
//#endif
		}
		break;
		default:
		{
			//! To do
		}
		break;
	}
}

void CCharacterState_Move::OnMessage(DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	switch ((F4PACKET::PACKET_ID)packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerMove:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerMove_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_move, time);

			m_Speed = packet_data->GetData()->speed();

			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 sForwardVector;
			sForwardVector.fX = 0.0f;
			sForwardVector.fY = 0.0f;
			sForwardVector.fZ = 1.0f;

			CommonFunction::SVectorYaw(sForwardVector, m_pOwner->GetCharacterStateAction()->directioninput(), m_MoveDirection);

//#ifdef TEXT_LOG_FILE
//			LOGGER->Log("[CCHARACTER_STATE_MOVE] time : %f, PacketName : %s, AnimID : %d, AnimationDuration : %f", time, F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
//			LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
//			LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
//			LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
//#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateReady:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrateReady_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_penetrateReady, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex());
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrate:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetrate_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_penetrate, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();
			
			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerCutIn:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerCutIn_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_cutIn, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerDash:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerDash_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::dash, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerVCut:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerVCut_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_vCut, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerHopStep:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerHopStep_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_hopStep, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPenetratePostUp:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPenetratePostUp_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_penetratePostUp, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex());
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndSlip:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPickAndSlip_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_pickAndSlip, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndMove:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPickAndMove_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_pickAndMove, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 direction;
			direction.fX = RIM_POS_X - packet_data->GetData()->playeraction()->positionlogic().x();
			direction.fY = RIM_POS_Y - packet_data->GetData()->playeraction()->positionlogic().y();
			direction.fZ = RIM_POS_Z_FIXED - packet_data->GetData()->playeraction()->positionlogic().z();

			DHOST_TYPE_FLOAT direction_yaw = CommonFunction::ToYaw(direction);

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpPenetrate_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_postUpPenetrate, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			JOVECTOR3 direction;

			DHOST_TYPE_FLOAT direction_yaw = kFLOAT_INIT;

			switch (packet_data->GetData()->playeraction()->skillindex())
			{
				case F4PACKET::SKILL_INDEX::skill_postUpPenetrate:
				{
					direction.fX = packet_data->GetData()->playeraction()->positionlogic().x() - RIM_POS_X;
					direction.fY = packet_data->GetData()->playeraction()->positionlogic().y() - RIM_POS_Y;
					direction.fZ = packet_data->GetData()->playeraction()->positionlogic().z() - RIM_POS_Z_FIXED;

					direction_yaw = CommonFunction::ToYaw(direction);

					m_IsEnableDribble = true;
				}
				break;
				case F4PACKET::SKILL_INDEX::skill_postUpSpin:
				{
					direction.fX = RIM_POS_X - packet_data->GetData()->playeraction()->positionlogic().x();
					direction.fY = RIM_POS_Y - packet_data->GetData()->playeraction()->positionlogic().y();
					direction.fZ = RIM_POS_Z_FIXED - packet_data->GetData()->playeraction()->positionlogic().z();

					direction_yaw = CommonFunction::ToYaw(direction);

					m_IsEnableDribble = true;
				}
				break;
				default:
				{
					direction.fX = packet_data->GetData()->playeraction()->positionlogic().x() - RIM_POS_X;
					direction.fY = packet_data->GetData()->playeraction()->positionlogic().y() - RIM_POS_Y;
					direction.fZ = packet_data->GetData()->playeraction()->positionlogic().z() - RIM_POS_Z_FIXED;

					direction_yaw = CommonFunction::ToYaw(direction);
				}
				break;
			}

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpReady:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpReady_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_postUpReady, time);

			std::string aniName = "";

			if (packet_data->GetData()->enter() == true)
			{
				aniName = "POSTUP_READY_01";
			}
			else
			{
				aniName = "POSTUPDRIBBLE_PENETRATION_READY_01";
			}

			int animid = m_pOwner->GetAnimationController()->FindAnimationInfo(aniName);

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAnimationIndex(animid);

			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);

			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpStepBack:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerPostUpStepBack_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_postUpStepBack, time);
			
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimDurationTime : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerScreen:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerScreen_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_screen, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerShakeAndBake_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_shakeAndBake, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSlideStep:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerSlideStep_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_slideStep, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSpinMove:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerSpinMove_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_spinMove, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerSlipAndSlide:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerSlipAndSlide_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_slipAndSlide, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerStun:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerStun_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_stun, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animid());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animid());
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerCollision:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerCollision_data>*)pData;

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

			int animid = packet_data->GetData()->animid();
			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_collision, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerCloseOut:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerCloseOut_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_closeOut, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerChaseContest:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerChaseContest_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_chaseContest, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerGoAndCatch:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerGoAndCatch_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_goAndCatch, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerCatchAndShotMove:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerCatchAndShotMove_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::catchAndShot_Move, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);
			m_Mirror = packet_data->GetData()->ismirroranim();

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerCrossOver:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerCrossOver_data>*)pData;

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

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_crossOverPenetrate, time);
			m_pOwner->SetCharacterStateAnimationIndex(packet_data->GetData()->animidmain());
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());
			m_Mirror = packet_data->GetData()->ismirroranim();

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(packet_data->GetData()->animidmain());

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerHandCheck:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerHandCheck_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_handCheck, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());
			m_Mirror = packet_data->GetData()->ismirroranim();

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		case F4PACKET::PACKET_ID::play_c2s_playerHookHook:
		{
			auto packet_data = (CFlatBufPacket<F4PACKET::play_c2s_playerHookHook_data>*)pData;

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

			int animid = packet_data->GetData()->animid();

			if (animid < kINT32_INIT)
			{
				return;
			}

			m_pOwner->SetCharacterStateAction(*packet_data->GetData()->playeraction(), F4PACKET::ACTION_TYPE::action_hookHook, time);
			m_pOwner->SetCharacterStateAnimationIndex(animid);
			m_pOwner->SetCharacterStateStartAnimationPosition(packet_data->GetData()->playeraction()->positionlogic());
			m_Mirror = packet_data->GetData()->ismirroranim();

			m_AnimationDuration = m_pOwner->GetAnimationController()->GetAnimationDuration(animid);

			TB::SVector3 position = CommonFunction::SVectorSlidePosition(packet_data->GetData()->playeraction()->positionlogic(), m_pOwner->GetStartPosition());
			CommonFunction::ConvertTBVectorToJoVector(position, m_SlidePosition);

			DHOST_TYPE_FLOAT target_yaw = packet_data->GetData()->playeraction()->yawlogic() - m_pOwner->GetStartYaw();
			m_SlideYaw = CommonFunction::OptimizeAngle(target_yaw);

#ifdef TEXT_LOG_FILE
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketName : %s, AnimID : %d, AnimationDuration : %f", F4PACKET::EnumNamePACKET_ID((F4PACKET::PACKET_ID)packetID), m_pOwner->GetCharacterStateAnimationIndex(), m_AnimationDuration);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] StartYaw : %f, StartPos x : %f, z : %f", m_pOwner->GetStartYaw(), m_pOwner->GetStartPosition().fX, m_pOwner->GetStartPosition().fZ);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] PacketYaw : %f, PacketPos x : %f, z : %f, m_SlideTimePosition : %f", packet_data->GetData()->playeraction()->yawlogic(), packet_data->GetData()->playeraction()->positionlogic().x(), packet_data->GetData()->playeraction()->positionlogic().z(), m_SlideTimePosition);
			//LOGGER->Log("[CCHARACTER_STATE_MOVE] m_SlideYaw : %f, m_SlidePosition x : %f, z : %f, Mirror : %d", m_SlideYaw, m_SlidePosition.fX, m_SlidePosition.fZ, m_Mirror);
#endif
		}
		break;
		default:
		{
			CCharacterState::OnMessage(packetID, pData, time);
		}
		break;
	}

	m_LastAniTime = kFLOAT_INIT;
	m_LastUpdateTick = false;
}

void CCharacterState_Move::OnExit()
{
}

void CCharacterState_Move::Init()
{
	m_Speed = kFLOAT_INIT;

	m_SlideTimePosition = kFLOAT_INIT;
	m_SlideYaw = kFLOAT_INIT;
	m_LastAniTime = kFLOAT_INIT;
	m_AnimationDuration = kFLOAT_INIT;

	m_Mirror = false;
	m_LastUpdateTick = false;
	m_IsEnableDribble = false;

	m_SlidePosition.fX = kFLOAT_INIT;
	m_SlidePosition.fY = kFLOAT_INIT;
	m_SlidePosition.fZ = kFLOAT_INIT;

	m_RimPosition.fX = RIM_POS_X;
	m_RimPosition.fY = RIM_POS_Y;
	m_RimPosition.fZ = RIM_POS_Z_FIXED;

	m_LocalPositionBall.fX = kFLOAT_INIT;
	m_LocalPositionBall.fY = kFLOAT_INIT;
	m_LocalPositionBall.fZ = kFLOAT_INIT;

	m_MoveDirection.fX = kFLOAT_INIT;
	m_MoveDirection.fY = kFLOAT_INIT;
	m_MoveDirection.fZ = kFLOAT_INIT;
}

DHOST_TYPE_BOOL CCharacterState_Move::GetSecondAnimationReceivePacket()
{
	return false;
}