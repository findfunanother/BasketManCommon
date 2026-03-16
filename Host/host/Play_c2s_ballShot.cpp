#include "Play_c2s_ballShot.h"



bool CPlay_c2s_ballShot::CheckNullPtr(CHost* pHost, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, DHOST_TYPE_USER_ID userid)
{

	auto* data = pPacket->GetData();

	flatbuffers::Verifier packet_verify(reinterpret_cast<uint8_t*>(pPacket->m_pdata), pPacket->m_size);
	bool data_check = data->Verify(packet_verify);

	if (data_check == false)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER DOES NOT PASS RoomElapsedTime : " + std::to_string(pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

	if (data->ballshot() == nullptr || data->ballshot()->positionball() == nullptr || data->ballshot()->positionowner() == nullptr || data->ballshot()->medalindices() == nullptr || data->ballshot()->hindrances() == nullptr || data->snrnsisj() == nullptr)
	{
		string invalid_buffer_log = "[PACKET_ERROR] VERIFY_FLATBUFFER ballshot is null RoomElapsedTime : " + std::to_string(pHost->GetRoomElapsedTime()) + ", UserID : " + std::to_string(userid) + ", PacketID : " + F4PACKET::EnumNamePACKET_ID((PACKET_ID)pPacket->GetPacketID());
		pHost->ToLog(invalid_buffer_log.c_str());

		return false;
	}

    return true;
}

float CPlay_c2s_ballShot::AddMinigameCustomInfo(CHost* pHost)
{
	float addShotRate = 0.0f;
	CGameManager* gameManager = pHost->GetGameManager();
	F4PACKET::MinigameCustomInfoT* pMinigameCustom = gameManager->GetMinigameCustomInfo();
	if (pMinigameCustom) // 로컬 모드 테스트 , NULL 이 아니면 OnRedisMatchInfo 에서 가져옴 
	{
		addShotRate = pMinigameCustom->addshotbuffrate;
	}

	return addShotRate;
}
