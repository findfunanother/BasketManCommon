#include "CharacterMovement.h"
#include "Host.h"
#include "DataManagerBalance.h"

// 스피드핵의 방법 
// 1. 시간을 빠르게 하는 법 , 이것은 확인이 가능함 
// 2. 클라이언트 시간을 제어하지 않고 빠르게 하는 방법이 있다( 이것은 아직 안밝혀짐 ) 

// 클라이언트 시간을 빠르게 해서 하는 스피드핵은 잡기 어려우므로, 
// 
// 서버에서 좌표 계산을 해야 한다. 
// 이것은 랙이든 스피드핵이든 서버와 좌표차이를 본다 클라이언트 좌표가 서버에서 계산하는 좌표보다 더 멀리 가지 않도록 
// 1.  보정을 한다.  2. 액션을 실패 시킨다. 3. 보정을 하고 액션을 실패시킨다. 

// 클라이언트 포지션은 서버 포지션보다 더 많이 갈 수 없다. 




// 클라이언트 수정사항 

// *** 슛을 할때 100% 서버 포지션으로 할 수 있는가, 이것을 모든 액션으로 확장할 수 있는가 ? 

// *** 스틸도 100% 서버 포지션으로 할 수 있는가 

// 1. 스탠드, 스탠드 이동에 슬러핑이 없다. 
// 2. 레이업, 덩크가 두번재 패킷에 좌표 이동이 없다, 스케일 때문에 그렇게 만든듯 
// 3. 포지션코렉션 패킷을 
// 4. 클라위치가 서버보다 더 짭은 경우, 더 긴경우로 나눠서 랙유저와 스피드핵 유저를 구분한다. 
// 5. 돌파 중 슛의 위치를 서버가 트레이싱 할 수 있는가 ? 돌파 시작점은 서보로 보내줄수 있다, 

// 그러나 돌파애니메이션 속도는 제어가 안되기 때무넹 슛타이밍이 빠른 것은 안됨( 애니메이션 길이를 서버에 보낸 후 그 시간을 체크하는 방법 )( 포지션 코렉트 패킷을 클라에서 의무적으로 보내자 ) 




//***** 2025-03- 18 *** 
// 1. 여기서 Process_play_c2s_playerStand_To_play_c2s_playerMove 이동 중에만 스피드핵 체크하는 것을 이동, 스탠드, 슛 패킷 받는 부분에서 거리를 판단해서 바로 체크하는 방식으로 바꿈 
// 2. 서버가 모든 클라이언트의 위치를 받아 온 후 이동을 계산해서 

CCharacterMovement::CCharacterMovement()
{
	m_pHost = nullptr;
	m_pCharacter = nullptr;
	m_SereverPositionDeque.clear();
	m_nSpeedHackCheckCount = 0;
	m_bPositionWrong = false;

	m_ServerPosition = JOVECTOR3(0.0f, 0.0f, 0.0f);
	m_ServerPositionEvent = JOVECTOR3(0.0f, 0.0f, 0.0f);
	m_DirYaw = 0.0f;
	m_Speed = 0.0f;
	m_MoveDirection = JOVECTOR3(0.0f, 0.0f, 0.0f);
}

CCharacterMovement::~CCharacterMovement()
{

}

void CCharacterMovement::ClearSereverPositionDeque()
{
	m_SereverPositionDeque.clear();
}

void CCharacterMovement::AddServerPostionDeque(ServerCharacterPosition position)
{
	m_SereverPositionDeque.push_back(position);

	// 덱에 최소 2개 이상의 원소가 있는 경우 체크
	if (m_SereverPositionDeque.size() >= 2)
	{
		const ServerCharacterPosition& last = m_SereverPositionDeque.back();
		const ServerCharacterPosition& secondLast = m_SereverPositionDeque[m_SereverPositionDeque.size() - 2];
	}

	if (m_SereverPositionDeque.size() > 0)
	{
		const ServerCharacterPosition& last = m_SereverPositionDeque.back();

		m_ServerPositionEvent = last.positionlogic;

		// addballque 당시 서버에 포지션 값을 셋팅안하고 데이터를 넘겨주는 케이스가 있다. playercollsion 
		// play_s2c_playerCollision 
		// play_s2c_playerPostUpCollision 
		// 상호 작용하는 것들은 정상일지 테스트가 필요하다 
		if (last.positionlogic.x() == 0.0f && last.positionlogic.z() == 0.0f) //
		{
			return; // 서버를 갱신 안함, play_s2c_playerStealFail  
		}

		if (last.packetID != F4PACKET::PACKET_ID::play_c2s_playerSyncInfo
			&& last.packetID != F4PACKET::PACKET_ID::play_c2s_playerPositionCorrect 
			&& last.packetID != F4PACKET::PACKET_ID::play_c2s_playerStand )
		{
			m_pCharacter->SetCurrentAction(last.packetID); // 위 3가지 상태는 빼자 
		}

		if (last.packetID == F4PACKET::PACKET_ID::play_c2s_playerSyncInfo) // 게임 준비 시만, 속도 0.0f, 점프볼 
		{
			// 이것도 랜덤 시드일 때만 적용하도록 하자 
			m_ServerPosition = last.positionlogic;
			m_DirYaw = last.dirYaw; // 방향은 유지해야함 
			m_Speed = 0.0f; // 
		}

		if (last.packetID == F4PACKET::PACKET_ID::play_c2s_playerPositionCorrect) // 액션이 끝날 때 받아 옴 
		{
			SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
			CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);
			if (pHostUser)
			{
				if (pUser->ailevel == kIS_NOT_AI && !m_pCharacter->IsAICharacter()) // 재접속, 트리오, 튜토리얼 때문에 이렇게 추가됨 , 재접속이 된다고 F4PACKET::SPlayerInformationT* pCharacterInformation ailevel 이 수정되지 않음 
				{
					if (pHostUser->IsValidAction()) // 두개의 랜덤시가 같을때만 유효하다 , 핵킹으로 무작위로 포지션을 보낼 수 있으므로 
					{
						m_ServerPosition = last.positionlogic;
						m_DirYaw = last.dirYaw; // 방향은 유지해야함 
					}
					else
					{
						// 핵킹 유저 
					}
				}
				else 
				{
					m_ServerPosition = last.positionlogic;
					m_DirYaw = last.dirYaw; // 방향은 유지해야함 
				}
			}
			else
			{
				m_ServerPosition = last.positionlogic;
				m_DirYaw = last.dirYaw; // 방향은 유지해야함 
			}
			
			// 여기는 속도 변경 부분이 없기 때문에 이전 속도를 그대로 유지함 
		}
		else
		if (last.packetID == F4PACKET::PACKET_ID::play_c2s_playerMove || last.packetID == F4PACKET::PACKET_ID::play_c2s_playerStand)
		{
			if (m_pHost->GetCurrentState() == EHOST_STATE::READY) // 턴전환 시 캐릭터가 순간 이동하는 문제 해결 
			{
				m_ServerPosition = last.positionlogic;

				JOVECTOR3 gapDir = m_ServerPosition - last.positionlogic;
				if (gapDir.Length() > 1.0f)
				{
					float distance = gapDir.Length();

					SendSeverPositionToClient(m_DirYaw);

					string invalid_buffer_log = "** Ready Position Sliped **  RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
						+ std::to_string(m_pCharacter->GetCharacterInformation()->userid)
						+ ", RoomID : " + m_pHost->GetHostID()
						+ ", distance : " + std::to_string(distance);
					m_pHost->ToLog(invalid_buffer_log.c_str());
				}
				else
				{
					m_ServerPosition = last.positionlogic;
					m_Speed = last.speed; // 속도는 move 와 stand 에서만 받아옴
					m_DirYaw = last.dirYaw;
					CommonFunction::SVectorYaw(JOVECTOR3::forward, last.dirYaw, m_MoveDirection);
				}

			}
			else
			{
				m_ServerPosition = last.positionlogic;
				m_Speed = last.speed; // 속도는 move 와 stand 에서만 받아옴
				m_DirYaw = last.dirYaw;
				CommonFunction::SVectorYaw(JOVECTOR3::forward, last.dirYaw, m_MoveDirection);
			}
		}
		else
		if (last.packetID == F4PACKET::PACKET_ID::play_c2s_playerPivot)
		{
			m_ServerPosition = last.positionlogic;
			m_Speed = 0.0f;
		}
		else
		if (last.packetID == F4PACKET::PACKET_ID::play_c2s_playerShot || 
			last.packetID == F4PACKET::PACKET_ID::play_c2s_playerBehindStepBackJumperShot) // 나머지 샷들도 해줘야 하나? 팁인, 엘리웁 등 
		{
			m_ServerPosition = last.positionlogic;
			m_Speed = 0.0f;
		}
		else
		{
			m_ServerPosition = last.positionlogic;
		}

	}


	//Update(0.0f);
}

DHOST_TYPE_FLOAT CCharacterMovement::GetValidMoveRange()
{
	const DHOST_TYPE_FLOAT avgSpeed = 4.0f;
	const DHOST_TYPE_FLOAT adjustmentValue = 2.0f;

	SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
	CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

	DHOST_TYPE_FLOAT ping = pHostUser->GetPingAverage();

	if (ping < 0.0f)
	{
		ping = 0.05f;
	}
	else
	if (ping > 0.5f)
	{
		ping = 0.5f;
	}

	DHOST_TYPE_FLOAT returnValue = ping * avgSpeed * adjustmentValue;

	return returnValue; // kCHARACTER_POSITION_CORRECT;
	//return 0.1f;
}


void CCharacterMovement::SendSeverPositionToClient(DHOST_TYPE_FLOAT lastYaw)
{
	SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
	CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

	if (pHostUser)
	{
		TB::SVector3 serverPosition = TB::SVector3(m_ServerPosition.fX, 0.0f, m_ServerPosition.fZ);

		CREATE_BUILDER(builder)
		CREATE_FBPACKET(builder, play_s2c_playerPositionCorrect, message, send_data);
		send_data.add_id(pUser->id);
		send_data.add_position(&serverPosition);
		send_data.add_yaw(lastYaw);

		STORE_FBPACKET(builder, message, send_data)

		m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		/*
		F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);

		string invalid_buffer_log = "[HACK_CHECK] SendSeverToClient_playerPositionCorrect RoomElapsed: " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
			+ std::to_string(pUser->userid)
			+ ", RoomID : " + m_pHost->GetHostID()
			+ ", UserName : " + pUser->name
			+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
			+ " , X : " + std::to_string(m_ServerPosition.fX)
			+ " , Z : " + std::to_string(m_ServerPosition.fZ)
			+ " , Yaw : " + std::to_string(m_DirYaw)
			+ " , Ping : " + std::to_string(pHostUser->GetPingAverage())
			+ " , FPS : " + std::to_string(pHostUser->GetCurFPS());

		m_pHost->ToLog(invalid_buffer_log.c_str());
		*/


		//
		// 보정을 할 때 기존의 서버 위치 체크하는 것도 갱신해야햠, 나중에 기존 서버 위치 체크하는 것을 사용하지 말자 
		// 
		m_pCharacter->SetCharacterStateActionPosition(m_ServerPosition.fX, m_ServerPosition.fY, m_ServerPosition.fZ, "newSeverPosition");
	}

}


DHOST_TYPE_BOOL CCharacterMovement::CheckPositionBetweenServerAndClient(ServerCharacterPosition last, ServerCharacterPosition secondLast)
{
	return false;

	JOVECTOR3 lastPosition = last.positionlogic;

	JOVECTOR3 gapDir = m_ServerPosition - lastPosition;

	float distance = gapDir.Length();

	if (distance > 0.5f && (m_pHost->GetCurrentState() == EHOST_STATE::PLAY))
	{
		SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
		CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

		if (pHostUser)
		{
			TB::SVector3 serverPosition = TB::SVector3(m_ServerPosition.fX, 0.0f, m_ServerPosition.fZ);

			JOVECTOR3 length1 = m_ServerPositionEvent - m_ServerPosition;
			JOVECTOR3 length2 = m_ServerPositionEvent - lastPosition;
			string compare = "";

			if (length1.Length() > length2.Length())
			{
				compare = "Client_Delay";
			}
			else
			{
				compare = "Client_Hack";
			}

			m_Speed = last.speed;
			m_DirYaw = last.dirYaw;
			CommonFunction::SVectorYaw(JOVECTOR3::forward, last.dirYaw, m_MoveDirection);

			CREATE_BUILDER(builder)
			CREATE_FBPACKET(builder, play_s2c_playerPositionCorrect, message, send_data);
			send_data.add_id(pUser->id);
			send_data.add_position(&serverPosition);
			send_data.add_yaw(last.dirYaw);

			STORE_FBPACKET(builder, message, send_data)

		
			m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

		    F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
			string invalid_buffer_log = "[HACK_CHECK] Server - Client PositionDifferent RoomElapsed: " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
				+ std::to_string(pUser->userid)
				+ ", RoomID : " + m_pHost->GetHostID()
				+ ", UserName : " + pUser->name
				+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
				+ ", PacketID : " + F4PACKET::EnumNamePACKET_ID(last.packetID)
				+ ", Distance : " + std::to_string(distance)
				+ " , Type : " + compare
				+ " , X : " + std::to_string(m_ServerPosition.fX)
				+ " , Z : " + std::to_string(m_ServerPosition.fZ)
				+ " , Yaw : " + std::to_string(m_DirYaw)
				+ " , Ping : " + std::to_string(pHostUser->GetPingAverage())
				+ " , FPS : " + std::to_string(pHostUser->GetCurFPS());

			m_pHost->ToLog(invalid_buffer_log.c_str());


			//
			// 보정을 할 때 기존의 서버 위치 체크하는 것도 갱신해야햠, 나중에 기존 서버 위치 체크하는 것을 사용하지 말자 
			// 
			m_pCharacter->SetCharacterStateActionPosition(m_ServerPosition.fX, m_ServerPosition.fY, m_ServerPosition.fZ, "newSeverPosition");

			return true;

		}
	}
	else
	{
		return false;
	}

	return false;
}



void CCharacterMovement::Process_play_c2s_playerStand_To_play_c2s_playerMove(ServerCharacterPosition last, ServerCharacterPosition secondLast)
{
	if (secondLast.speed > 0) // 피봇은 스피드가 0 임 
	{
		float gapTime = last.clienttime - secondLast.clienttime;
		float outDistance = CommonFunction::SVectorDistanceXZ(secondLast.positionlogic, last.positionlogic);
		float outExpectDistance = secondLast.speed * gapTime;
		float outGap = outExpectDistance - outDistance;

		if (outDistance > outExpectDistance && (m_pHost->GetCurrentState() == EHOST_STATE::PLAY))
		{
			float gap = abs(outGap);
			float ratio = outDistance / outExpectDistance;

			if (ratio > 1.07f)
			{
				m_nSpeedHackCheckCount++;

				if (m_nSpeedHackCheckCount > 5 && m_nSpeedHackCheckCount < 30) // 30번만 찍자 
				{
					SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
					CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

					if (pHostUser)
					{
						F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
						string invalid_buffer_log = "[HACK_CHECK] MoveSpeedCheck_2 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
							+ std::to_string(pUser->userid)
							+ ", RoomID : " + m_pHost->GetHostID()
							+ ", UserName : " + pUser->name
							+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
							+ ", Ratio : " + std::to_string(ratio)
							+ ", OutExpectDistance : " + std::to_string(outExpectDistance)
							+ ", OutDistance : " + std::to_string(outDistance)
							+ ", Speed : " + std::to_string(secondLast.speed)
							+ ", Ping : " + std::to_string(pHostUser->GetPingAverage())
							+ ", FPS : " + std::to_string(pHostUser->GetCurFPS());

						m_pHost->ToLog(invalid_buffer_log.c_str());
					}

					m_bPositionWrong = true;
				}
			}
			else
			{
				m_bPositionWrong = false;
			}
		}
	}
}

DHOST_TYPE_BOOL  CCharacterMovement::IsActionPositionDeque(F4PACKET::PACKET_ID  packetID)
{
	switch (packetID)
	{
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopCut:
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopPass:
		case F4PACKET::PACKET_ID::play_c2s_playerAlleyOopShot:
		case F4PACKET::PACKET_ID::play_c2s_playerBlock:
		//case F4PACKET::PACKET_ID::play_c2s_playerCeremony:
		case F4PACKET::PACKET_ID::play_c2s_playerCeremonyEnd:
		//case F4PACKET::PACKET_ID::play_c2s_playerCollision:
		//case F4PACKET::PACKET_ID::play_c2s_playerCoupleCeremony:
		case F4PACKET::PACKET_ID::play_c2s_playerCrossOver:
		case F4PACKET::PACKET_ID::play_c2s_playerCutIn:
		case F4PACKET::PACKET_ID::play_c2s_playerDenyDefense:
		case F4PACKET::PACKET_ID::play_c2s_playerDoubleClutch:
		case F4PACKET::PACKET_ID::play_c2s_playerFakeShot:
		case F4PACKET::PACKET_ID::play_c2s_playerHandCheck:
		case F4PACKET::PACKET_ID::play_c2s_playerHit:
		case F4PACKET::PACKET_ID::play_c2s_playerHopStep:
		case F4PACKET::PACKET_ID::play_c2s_playerIntercept:
		case F4PACKET::PACKET_ID::play_c2s_playerJumpBallTapOut:
		case F4PACKET::PACKET_ID::play_c2s_playerMove:
		case F4PACKET::PACKET_ID::play_c2s_playerPass:
		case F4PACKET::PACKET_ID::play_c2s_playerPassive:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrate:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateCut:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetratePostUp:
		case F4PACKET::PACKET_ID::play_c2s_playerPenetrateReady:
		case F4PACKET::PACKET_ID::play_c2s_playerPick:
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndMove:
		case F4PACKET::PACKET_ID::play_c2s_playerPickAndSlip:
		case F4PACKET::PACKET_ID::play_c2s_playerPositionCorrect:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpCollision:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpPenetrate:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpReady:
		case F4PACKET::PACKET_ID::play_c2s_playerPostUpStepBack:
		case F4PACKET::PACKET_ID::play_c2s_playerRebound:
		case F4PACKET::PACKET_ID::play_c2s_playerReceivePass:
		case F4PACKET::PACKET_ID::play_c2s_playerScreen:
		case F4PACKET::PACKET_ID::play_c2s_playerShakeAndBake:
		case F4PACKET::PACKET_ID::play_c2s_playerShot:
		case F4PACKET::PACKET_ID::play_c2s_playerBehindStepBackJumperShot:
		case F4PACKET::PACKET_ID::play_c2s_playerSlideStep:
		case F4PACKET::PACKET_ID::play_c2s_playerSpinMove:
		case F4PACKET::PACKET_ID::play_c2s_playerSlipAndSlide:
		case F4PACKET::PACKET_ID::play_c2s_playerStand:
		case F4PACKET::PACKET_ID::play_c2s_playerSteal:
		case F4PACKET::PACKET_ID::play_c2s_playerStealFail:
		case F4PACKET::PACKET_ID::play_c2s_playerStop:
		case F4PACKET::PACKET_ID::play_c2s_playerStun:
		case F4PACKET::PACKET_ID::play_c2s_playerTapOut:
		case F4PACKET::PACKET_ID::play_c2s_playerTapPass:
		case F4PACKET::PACKET_ID::play_c2s_playerTipIn:
		case F4PACKET::PACKET_ID::play_c2s_playerCloseOut:
		case F4PACKET::PACKET_ID::play_c2s_playerGoAndCatch:
		case F4PACKET::PACKET_ID::play_c2s_playerHookHook:
		case F4PACKET::PACKET_ID::play_c2s_playerVCut:
		case F4PACKET::PACKET_ID::play_c2s_playerCatchAndShotMove:
		case F4PACKET::PACKET_ID::play_c2s_playerInYourFace:
		case F4PACKET::PACKET_ID::play_c2s_playerChaseContest:
		case F4PACKET::PACKET_ID::play_c2s_playerGiveAndGoMove:
		case F4PACKET::PACKET_ID::play_c2s_playerShammgod:
		case F4PACKET::PACKET_ID::play_c2s_playerPassFake:
		case F4PACKET::PACKET_ID::play_c2s_playerStunIntercept:
		case F4PACKET::PACKET_ID::play_c2s_playerOneDribblePullUpJumperMove:
		case F4PACKET::PACKET_ID::play_c2s_playerEscapeDribble:
		case F4PACKET::PACKET_ID::play_c2s_playerShotInterfere:
		case F4PACKET::PACKET_ID::play_c2s_playerDash:
		return true;

		default :
		return false;
	}
}

void CCharacterMovement::CharacterMove(DHOST_TYPE_FLOAT fTimeDelata)
{
	DHOST_TYPE_FLOAT moveDelta = fTimeDelata * m_Speed;
	m_MoveDirection.Normalize();
	JOVECTOR3 sCalcPos;
	CommonFunction::SJoVectorMultiply(m_MoveDirection, moveDelta, sCalcPos);

	m_ServerPosition = m_ServerPosition + sCalcPos;

	switch (m_pCharacter->GetCharacterRestriction())
	{
		case EMOVEMENT_RESTRICTION::OUTSIDE_TO_INSIDE_RESTRICTION:
		{
			TB::SVector3 temp = CommonFunction::ConvertJoVectorToTBVector(m_ServerPosition);
			TB::SVector3 result = CommonFunction::CorrPositionThreePointOutEx(temp);
			m_ServerPosition = result;
		}
		break;
		case EMOVEMENT_RESTRICTION::INSIDE_TO_OUTSIDE_RESTRICTION:
		{
			TB::SVector3 temp = CommonFunction::ConvertJoVectorToTBVector(m_ServerPosition);
			TB::SVector3 result = CommonFunction::CorrPositionThreePointInEx(temp);
			m_ServerPosition = result;
		}
		break;
		default:
		{
			CommonFunction::CorrOutSideEx(m_ServerPosition);
		}
		break;
	}
}

void CCharacterMovement::Update(DHOST_TYPE_FLOAT timeDelta)
{
	if (m_SereverPositionDeque.size() >= 1)
	{
	}


	if (m_SereverPositionDeque.size() > 10) 
	{
		m_SereverPositionDeque.pop_front();
		//m_bPositionWrong = false; // 액션 10개 이후에 풀어주기 
	}

	CharacterMove(timeDelta);
}


DHOST_TYPE_BOOL CCharacterMovement::IsPositionWrong(TB::SVector3 rePosition)
{

	//return false;
	/*
	JOVECTOR3 jCurPos;
	JOVECTOR3 jNextPos;
	CommonFunction::ConvertTBVectorToJoVector(curPos, jCurPos);
	CommonFunction::ConvertTBVectorToJoVector(position, jNextPos);

	JOVECTOR3 dir = jNextPos - jCurPos;
	dir.Normalize();

	JOVECTOR3 jExpectPos = jCurPos + outExpectDistance * dir.Normalize();
	rePosition = CommonFunction::ConvertJoVectorToTBVector(jExpectPos);
	*/
	return m_bPositionWrong;
}

DHOST_TYPE_FLOAT CCharacterMovement::GetGapServerPosition(TB::SVector3 position)
{
	if (GetPosition().fX == 0.0f && GetPosition().fZ == 0.0f)
	{
		//SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();
		//CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

		SPlayerInformationT* pUser = m_pCharacter->GetCharacterInformation();

		F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);

		string invalid_buffer_log = "Sever is Zero RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
			+ std::to_string(pUser->userid)
			+ ", RoomID : " + m_pHost->GetHostID()
			+ ", UserName : " + pUser->name
			+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result);

		m_pHost->ToLog(invalid_buffer_log.c_str());


		return 0.0f;
	}

	TB::SVector3 server_pos = CommonFunction::ConvertJoVectorToTBVector(GetPosition());
	auto client_pos = position;	// 패킷으로 받은 캐릭터의 위치
	float gap = VECTOR3_DISTANCE(server_pos, client_pos);
	return gap;
}

/*
if (m_SereverPositionDeque.size() >= 2)
{

	// 맨 뒤 요소와 그 앞 요소에 접근
	const ServerCharacterPosition& firstPosition = m_SereverPositionDeque.front();
	const ServerCharacterPosition& secondPosition = m_SereverPositionDeque[1];
	//const ServerCharacterPosition& thirdPosition  = m_SereverPositionDeque[2];


	// 이동, 피벗은 스피드가 0 인 상태로 무브패밋이 들어오므로 주의
	if( firstPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerMove && firstPosition.speed > 0.0f )
	{
		if (secondPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerFakeShot)
		{
			if (thirdPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerMove && thirdPosition.speed > 0.0f)
			{
				SPlayerInformationT* pUser = GetCharacterInformation();
				CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

				if (pHostUser)
				{
					F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
					string invalid_buffer_log = "[[HACK_CHECK] FAKESHOT move wrong_2 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
						+ std::to_string(pUser->userid)
						+ ", UserName : " + pUser->name
						+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
						+ ", Speed : " + std::to_string(firstPosition.speed)
						+ ", Ping : " + std::to_string(pHostUser->GetPingAverage())
						+ ", FPS : " + std::to_string(pHostUser->GetCurFPS());

					m_pHost->ToLog(invalid_buffer_log.c_str());
				}
			}
		}
	}


	if (firstPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerMove &&
		//secondPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerMove)
		secondPosition.packetID == F4PACKET::PACKET_ID::play_c2s_playerStand)
	{
		if (firstPosition.speed > 0) // 피봇은 스피드가 0 임
		{
			float gapTime = secondPosition.clienttime - firstPosition.clienttime;

			float outDistance = CommonFunction::SVectorDistanceXZ(firstPosition.positionlogic, secondPosition.positionlogic);

			float outExpectDistance = firstPosition.speed * gapTime;

			float outGap = outExpectDistance - outDistance;

			if (outDistance > outExpectDistance)
			{
				float gap = abs(outGap);
				float ratio = outDistance / outExpectDistance;

				if (m_nSpeedHackCheckCount < 30) // 30번만 찍자
				{
					if (ratio > 1.05f && ratio < 1.1f)
					{
						CCharacter pCharacter = dynamic_cast<CCharacter*>(this);

						SPlayerInformationT* pUser = pCharacter->GetCharacterInformation();


						CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

						if (pHostUser)
						{
							F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
							string invalid_buffer_log = "[HACK_CHECK] MoveSpeedCheck_2 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
								+ std::to_string(pUser->userid)
								+ ", UserName : " + pUser->name
								+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
								+ ", Ratio : " + std::to_string(ratio)
								+ ", OutExpectDistance : " + std::to_string(outExpectDistance)
								+ ", OutDistance : " + std::to_string(outDistance)
								+ ", Speed : " + std::to_string(firstPosition.speed)
								+ ", Ping : " + std::to_string(pHostUser->GetPingAverage())
								+ ", FPS : " + std::to_string(pHostUser->GetCurFPS());

							m_pHost->ToLog(invalid_buffer_log.c_str());
						}

						m_nSpeedHackCheckCount++;
					}
					else
						if (ratio > 1.1f)
						{
							SPlayerInformationT* pUser = GetCharacterInformation();
							CHostUserInfo* pHostUser = m_pHost->FindUser(pUser->userid);

							if (pHostUser)
							{
								F4PACKET::ECHARACTER_INDEX result = (F4PACKET::ECHARACTER_INDEX)(pUser->characterid / 100);
								string invalid_buffer_log = "[HACK_CHECK] MoveSpeedCheck_3 RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : "
									+ std::to_string(pUser->userid)
									+ ", UserName : " + pUser->name
									+ ", CharName : " + F4PACKET::EnumNameECHARACTER_INDEX(result)
									+ ", Ratio : " + std::to_string(ratio)
									+ ", OutExpectDistance : " + std::to_string(outExpectDistance)
									+ ", OutDistance : " + std::to_string(outDistance)
									+ ", Speed : " + std::to_string(firstPosition.speed)
									+ ", Ping : " + std::to_string(pHostUser->GetPingAverage())
									+ ", FPS : " + std::to_string(pHostUser->GetCurFPS());

								m_pHost->ToLog(invalid_buffer_log.c_str());
							}

							m_nSpeedHackCheckCount++;
						}
				}
			}

		}
	}

	// 마지막 요소 제거
	m_SereverPositionDeque.pop_front();


}

*/