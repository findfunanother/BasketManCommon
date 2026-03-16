#include "FlatBufProtocol.h"

#define CASE_DEFINE(packid) case F4PACKET::PACKET_ID::packid: \
									return new CFlatBufPacket<F4PACKET::packid##_data>(F4PACKET::PACKET_ID::packid);
namespace FlatBufProtocol
{
	CPacketImplement<F4PACKET::PACKET_ID>* CreateIncomingPacketFromPacketID(F4PACKET::PACKET_ID packetID);

	void Protocol_Start()
	{

	}

	void Protocol_End()
	{

	}

	ERESULT tryDeframeIncomingPacket(const char* msg,
		size_t size,
		CPacketImplement<F4PACKET::PACKET_ID>*& pPacket,
		DHOST_TYPE_INT32& packetID,
		DHOST_TYPE_INT32& packetType)
	{
		const char* pBuffer = msg;
		unsigned int packetLen = (unsigned int)size;

		packetLen = (unsigned char)pBuffer[0] + ((unsigned char)pBuffer[1] << 8);
		packetID = (unsigned char)pBuffer[2] + ((unsigned char)pBuffer[3] << 8);
		packetType = (unsigned char)pBuffer[4] + ((unsigned char)pBuffer[5] << 8);

		const char* pData = pBuffer + kPACKET_HEADER_SIZE;
		int data_size = packetLen - kPACKET_HEADER_SIZE;

		if (data_size <= 0)
		{
			pData = nullptr;
			pBuffer = nullptr;
			return ERESULT::CORRUPT_PACKET;
		}

		if (nullptr != pPacket)
		{
			if (pPacket->GetPacketID() != packetID)
			{
				return ERESULT::CORRUPT_PACKETID;
			}
		}

		if (nullptr == pPacket)
			pPacket = CreateIncomingPacketFromPacketID((F4PACKET::PACKET_ID)packetID);

		if (nullptr == pPacket)
		{
			pData = nullptr;
			pBuffer = nullptr;
			return ERESULT::UNDEFINED_FAILURE;
		}

		if (!pPacket->Decode(pData, data_size))
		{
			pData = nullptr;
			pBuffer = nullptr;

			return ERESULT::DECODING_FAILURE;
		}

		pData = nullptr;
		pBuffer = nullptr;

		return ERESULT::SUCCESS;
	}

	bool GetHeadInfo(Header& header, const char* pBuffer, size_t size)
	{
		if (nullptr == pBuffer)
			return false;

		if (size < sizeof(Header))
			return false;

		header.packetLen = (unsigned char)pBuffer[0] + ((unsigned char)pBuffer[1] << 8);
		header.packetID = (unsigned char)pBuffer[2] + ((unsigned char)pBuffer[3] << 8);
		header.packetType = (unsigned char)pBuffer[4] + ((unsigned char)pBuffer[5] << 8);

		return true;
	}

	const ERESULT GetPacket(CPacketImplement<F4PACKET::PACKET_ID>*& pPacket, const char* msg, size_t size)
	{
		int uCommandID = 0;
		int packet_type = 0;

		ERESULT result = tryDeframeIncomingPacket(msg, size, pPacket, uCommandID, packet_type);
		if (result != ERESULT::SUCCESS)
			return result;

		pPacket->SetPacketID(uCommandID);
		pPacket->SetPacketType(packet_type);

		return result;
	}


	const ERESULT Encode(CPacketBase* pPacket, char** ppBuffer, int& bufferSize, Allocate fncallocate, Free fncfree)
	{
		ERESULT result = ERESULT::SUCCESS;

		CFlatBufPacketImplement* gPacket = reinterpret_cast<CFlatBufPacketImplement*>(pPacket);

		if (nullptr == gPacket)
		{
			result = ERESULT::CORRUPT_PACKET;
			return result;
		}

		DHOST_TYPE_UINT16 packetLen = gPacket->GetEncodeStreamSize();

		if (0 >= packetLen)
		{
			result = ERESULT::INSUFFICIENT_BUFFER;
			return result;
		}

		Header header;

		DHOST_TYPE_UINT16 header_size = kPACKET_HEADER_SIZE;

		header.packetLen = packetLen + header_size;
		header.packetID = (DHOST_TYPE_UINT16)gPacket->GetPacketID();
		header.packetType = (DHOST_TYPE_UINT16)gPacket->GetPacketType();

		char* pBuffer = *ppBuffer;

		pBuffer[0] = header.packetLen & 0xFF;
		pBuffer[1] = (header.packetLen & 0xFF00) >> 8;
		pBuffer[2] = header.packetID & 0xFF;
		pBuffer[3] = (header.packetID & 0xFF00) >> 8;
		pBuffer[4] = header.packetType & 0xFF;
		pBuffer[5] = (header.packetType & 0xFF00) >> 8;

		if (!gPacket->Encode(&pBuffer[header_size], header.packetLen - header_size))
		{
			result = ERESULT::ENCODING_FAILURE;
			if (fncfree)
			{
				fprintf(stderr, "[MEMORY_LECK] fncfree PacketID : %u, fncfree : %p", header.packetID, fncfree);
				fncfree(pBuffer);

			}
			else
			{
				fprintf(stderr, "[MEMORY_LECK] NewChar Delete PacketID : %u, pBuffer : %p", header.packetID, pBuffer);
				delete[] pBuffer;
			}

			pBuffer = nullptr;
		}

		*ppBuffer = pBuffer;
		pBuffer = nullptr;

		bufferSize = header.packetLen;

		return result;
	}



	CPacketImplement<F4PACKET::PACKET_ID>* CreateIncomingPacketFromPacketID(F4PACKET::PACKET_ID packetID)
	{
		switch (packetID)
		{   // 옵저버 호스트도 있으니 주의 할 것 
			CASE_DEFINE(system_c2c_loadingPercentage)
			CASE_DEFINE(system_c2s_arrangeReady)
			CASE_DEFINE(system_c2s_chat)
			CASE_DEFINE(system_c2s_loadComplete)
			CASE_DEFINE(system_c2s_login)
			CASE_DEFINE(system_c2c_ping)
			CASE_DEFINE(system_c2s_readyToChallenge)
			CASE_DEFINE(system_c2s_readyToLoad)
			CASE_DEFINE(system_c2s_sceneEnd)
			CASE_DEFINE(system_c2s_readyToTraining)
			CASE_DEFINE(system_c2s_readyToTutorial)
			CASE_DEFINE(system_c2s_exitGame)
			CASE_DEFINE(system_c2s_tutorialResult)
			CASE_DEFINE(system_c2s_devGameTime)
			CASE_DEFINE(system_c2s_versionInfo)
			CASE_DEFINE(play_c2s_playerPassiveDataSet)
			CASE_DEFINE(play_s2c_playerPassiveDataSet)
			CASE_DEFINE(play_c2s_ballClear)
			CASE_DEFINE(play_c2s_ballLerp)
			CASE_DEFINE(play_c2s_ballSimulation)
			CASE_DEFINE(play_c2s_ballShot)
			CASE_DEFINE(play_c2s_ballRebound)
			CASE_DEFINE(play_c2s_environmentCat)
			CASE_DEFINE(play_c2s_onFireModeInfo)
			CASE_DEFINE(play_c2s_playerAlleyOopCut)
			CASE_DEFINE(play_c2s_playerAlleyOopPass)
			CASE_DEFINE(play_c2s_playerAlleyOopShot)
			CASE_DEFINE(play_c2s_playerBlock)
			CASE_DEFINE(play_c2s_playerBuff)
			CASE_DEFINE(play_c2s_playerCatchAndShotMove)
			CASE_DEFINE(play_c2s_playerInYourFace)
			CASE_DEFINE(play_c2s_playerCeremony)
			CASE_DEFINE(play_c2s_playerCeremonyEnd)
			CASE_DEFINE(play_c2s_playerCloseOut)
			CASE_DEFINE(play_c2s_playerChaseContest)
			CASE_DEFINE(play_c2s_playerCollision)
			CASE_DEFINE(play_c2s_playerCoupleCeremony)
			CASE_DEFINE(play_c2s_playerCrossOver)
			CASE_DEFINE(play_c2s_playerCutIn)
			CASE_DEFINE(play_c2s_playerDenyDefense)
			CASE_DEFINE(play_c2s_playerDoubleClutch)
			CASE_DEFINE(play_c2s_playerFakeShot)
			CASE_DEFINE(play_c2s_playerGoAndCatch)
			CASE_DEFINE(play_c2s_playerHandCheck)
			CASE_DEFINE(play_c2s_playerHit)
			CASE_DEFINE(play_c2s_playerHookHook)
			CASE_DEFINE(play_c2s_playerHopStep)
			CASE_DEFINE(play_c2s_playerIntercept)
			CASE_DEFINE(play_c2s_playerJumpBallTapOut)
			CASE_DEFINE(play_c2s_playerModificationAbility)
			CASE_DEFINE(play_c2s_playerMove)
			CASE_DEFINE(play_c2s_playerPass)
			CASE_DEFINE(play_c2s_playerPassFake)
			CASE_DEFINE(play_c2s_playerPassive)
			CASE_DEFINE(play_c2s_playerPenetrate)
			CASE_DEFINE(play_c2s_playerPenetratePostUp)
			CASE_DEFINE(play_c2s_playerPenetrateReady)
			CASE_DEFINE(play_c2s_playerPick)
			CASE_DEFINE(play_c2s_playerPickAndMove)
			CASE_DEFINE(play_c2s_playerPickAndSlip)
		    CASE_DEFINE(play_c2s_playerPositionCorrect)
			CASE_DEFINE(play_c2s_playerSyncInfo)
			CASE_DEFINE(play_c2s_playerPostUpCollision)
			CASE_DEFINE(play_c2s_playerPostUpPenetrate)
			CASE_DEFINE(play_c2s_playerPostUpReady)
			CASE_DEFINE(play_c2s_playerPostUpStepBack)
			CASE_DEFINE(play_c2s_playerRebound)
			CASE_DEFINE(play_c2s_playerReceivePass)
			CASE_DEFINE(play_c2s_playerScreen)
			CASE_DEFINE(play_c2s_playerShakeAndBake)
			CASE_DEFINE(play_c2s_playerShot)
			CASE_DEFINE(play_c2s_playerSlipAndSlide)
			CASE_DEFINE(play_c2s_playerSlideStep)
			CASE_DEFINE(play_c2s_playerShadowFaceupDash)
			CASE_DEFINE(play_c2s_playerSpeaking)
			CASE_DEFINE(play_c2s_playerSpinMove)
			CASE_DEFINE(play_c2s_playerStand)
			CASE_DEFINE(play_c2s_playerSteal)
			CASE_DEFINE(play_c2s_playerStealFail)
			CASE_DEFINE(play_c2s_playerStop)
			CASE_DEFINE(play_c2s_playerStun)
			CASE_DEFINE(play_c2s_playerTapOut)
			CASE_DEFINE(play_c2s_playerTapPass)
			CASE_DEFINE(play_c2s_playerTipIn)
			CASE_DEFINE(play_c2s_playerVCut)
			CASE_DEFINE(play_c2s_playerGiveAndGoMove)
			CASE_DEFINE(play_c2s_playerOneDribblePullUpJumperMove)
			CASE_DEFINE(play_c2s_playerEscapeDribble)
			CASE_DEFINE(play_c2s_playerShotInterfere)
			CASE_DEFINE(play_c2s_fxDisplayPotential)
			CASE_DEFINE(play_c2s_playerShammgod)
			CASE_DEFINE(play_c2s_playerStunIntercept)
			CASE_DEFINE(play_c2s_playerIllegalScreen)
			CASE_DEFINE(play_c2s_playerEmotion)
			CASE_DEFINE(play_c2s_playerSwitchActive)
			CASE_DEFINE(play_c2s_playerSwitchRequest)
			CASE_DEFINE(play_c2s_playerSwitchResponse)
			CASE_DEFINE(play_c2s_playerEmoji)
			CASE_DEFINE(play_c2s_playerBurstRequest)
			CASE_DEFINE(play_c2s_uiDisplayMedal)
			CASE_DEFINE(play_c2s_greatDefense)
			CASE_DEFINE(play_c2s_playerDash)
			CASE_DEFINE(play_c2s_playerBehindStepBackJumperShot)
			CASE_DEFINE(challenge_c2s_retry)
			CASE_DEFINE(local_system_c2s_login)
			CASE_DEFINE(local_system_c2s_mixPlayer)
			CASE_DEFINE(system_s2s_clientAliveCheck)
			CASE_DEFINE(system_s2s_systemTimeCheck)
			CASE_DEFINE(system_c2s_awayFromKeyboard)
			CASE_DEFINE(system_c2s_useJoystick)
			CASE_DEFINE(system_c2s_surrenderVote)
			CASE_DEFINE(system_c2s_test)
			CASE_DEFINE(system_c2s_blackBox)
			CASE_DEFINE(system_c2s_wkawofur)
			CASE_DEFINE(system_c2s_skinChange)

			default:
				break;
		}

		return nullptr;
	}
#undef ConvertProtocolPacket
}