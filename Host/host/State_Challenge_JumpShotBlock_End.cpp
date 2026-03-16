#include "State_Challenge_JumpShotBlock_End.h"
#include "BallController.h"

void CState_Challenge_JumpShotBlock_End::OnEnter(void* pData)
{
	timeWait = kHOST_TIME_MINUTE_10;
	timeElapsed = 0.0f;

	pChallengeData = (ChallengeData*)pData;

	int point = kINT32_INIT;

	if (pChallengeData != nullptr)
	{
		point = pChallengeData->m_point;
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, challenge_s2c_result, message, send_data);
	send_data.add_point(point);
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

	if (m_pHost->GetAbnormalExpireRoom())
	{
		timeWait = kFLOAT_INIT;
	}
}

void CState_Challenge_JumpShotBlock_End::OnUpdate(float timeDelta)
{
	timeElapsed += timeDelta;

	if (timeElapsed > timeWait)
	{
		if (pChallengeData != nullptr && m_pHost->GetRedisSaveMatchResult() == false)
		{
			m_pHost->RedisSaveMatchResult(pChallengeData->m_point);
		}

		if (pChallengeData == nullptr && m_pHost->GetRedisSaveMatchResult() == false && m_pHost->GetAbnormalExpireRoom() == true)
		{
			m_pHost->RedisSaveMatchResult(kINT32_INIT);
		}
	}
}

DHOST_TYPE_BOOL CState_Challenge_JumpShotBlock_End::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	DHOST_TYPE_USER_ID userid = pPacket->GetUserID();

	switch ((PACKET_ID)pPacket->GetPacketID())
	{
		CASE_FB_CALL(challenge_c2s_retry)
		DEFAULT_CALL(CState_Challenge_JumpShotBlock)
	}
}

const DHOST_TYPE_BOOL CState_Challenge_JumpShotBlock_End::ConvertPacket_challenge_c2s_retry(DHOST_TYPE_USER_ID userid, void* peer, CFlatBufPacket<F4PACKET::challenge_c2s_retry_data>* pPacket, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
	//! 패킷 데이터 검증 (모든 패킷에 추가할 것)
	if (pPacket == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid);
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	auto* data = pPacket->GetData();
	if (data == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] data is null RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + std::to_string(pPacket->GetPacketID());
		m_pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}
	//!

	if (data->retry() == true)	// 다시하기
	{
		m_pHost->ChangeState(EHOST_STATE::CHALLENGE_INIT, pChallengeData);
	}
	else
	{
		timeWait = kFLOAT_INIT;
	}

	CREATE_BUILDER(builder)
	CREATE_FBPACKET(builder, challenge_s2c_retry, message, send_data);
	send_data.add_retry(data->retry());
	STORE_FBPACKET(builder, message, send_data)

	m_pHost->SendPacket(message, userid);

	return true;
}