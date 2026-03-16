#include "DevManager.h"

CDevManager::CDevManager()
{
	m_bPlayPacketSave = false;
	m_DevNoKick = false;
	m_mapPacketReceiveCount.clear();
	m_PacketSize = kUINT64_INIT;
	m_MoveFB_PacketSize = 0;
	m_MovePB_PacketSize = 0;


	m_PacketMaximumSize = kUINT64_INIT;
	m_PacketMinimumSize = kUINT64_INIT;
	m_PacketMaximumID = kUINT16_INIT;
	m_PacketMinimumID = kUINT16_INIT;

	m_DevHomeAiLevel = 0;
	m_DevAwayAiLevel = 0;
}

CDevManager::~CDevManager()
{
	m_mapPacketReceiveCount.clear();
}

void CDevManager::UpdateDevManager(DHOST_TYPE_FLOAT timeDelta)
{

}

void CDevManager::DevPlayPacketSave(DHOST_TYPE_BOOL value)
{
	SetPlayPacketSave(value);
}

void CDevManager::SaveBinary(const char* pData, DHOST_TYPE_UINT32 size, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT time)
{
	// 유저아이디로 파일 이름을 결정
	std::string fileName = std::to_string(UserID) + ".bin";

	const void* buffer = &pData;

	// 파일을 쓰기/바이너리 모드(ab)로 열기
	FILE* fW = fopen(fileName.c_str(), "ab");

	// 게임 시간
	fwrite(&time, sizeof(float), 1, fW);

	// 패킷 사이즈 
	fwrite(&size, sizeof(uint32_t), 1, fW);

	// 패킷 데이터
	fwrite(pData, sizeof(char), size, fW);

	// 파일 포인터 닫기
	fclose(fW);
}

void CDevManager::LoadBinary(DHOST_TYPE_USER_ID UserID)
{
	std::string fileName = std::to_string(UserID);
	fileName.append(".bin");

	FILE* fR = fopen(fileName.c_str(), "rb"); // 파일을 쓰기/바이너리 모드(wb)로 열기

	if (fR != nullptr)
	{
		uint32_t dataSize;

		fread(&dataSize, sizeof(uint32_t), 1, fR);

		void* buffer = malloc(sizeof(char) * dataSize);

		fread(buffer, sizeof(char), dataSize, fR);

		const char* pData = (const char*)buffer;

		FlatBufProtocol::Header header_info;

		if (!FlatBufProtocol::GetHeadInfo(header_info, pData, dataSize))
		{
			return;
		}

		//CFlatBufPacketImplement* pPacket = nullptr;
		CPacketImplement<F4PACKET::PACKET_ID>* pPacket = nullptr;
		auto result = FlatBufProtocol::GetPacket(pPacket, pData, dataSize);

		if (nullptr == pPacket)
		{
			return;
		}

		fclose(fR);
	}
}


void CDevManager::InitPacketCount()
{
	m_PacketSize = kUINT64_INIT;

	if (!m_mapPacketReceiveCount.empty())
	{
		m_mapPacketReceiveCount.clear();
	}
}

DHOST_TYPE_UINT64 CDevManager::GetPacketTotalSize()
{
	return m_PacketSize;
}

void CDevManager::IncreasePacketReceiveCount(DHOST_TYPE_UINT16 packet_id, DHOST_TYPE_UINT64 packet_size)
{
	if (m_PacketSize == kUINT64_INIT)
	{
		m_PacketMaximumSize = packet_size;
		m_PacketMaximumID = packet_id;

		m_PacketMinimumSize = packet_size;
		m_PacketMinimumID = packet_id;
	}

	m_PacketSize += packet_size;

	if (m_PacketMaximumSize < packet_size)
	{
		m_PacketMaximumSize = packet_size;
		m_PacketMaximumID = packet_id;
	}

	if (m_PacketMinimumSize > packet_size)
	{
		m_PacketMinimumSize = packet_size;
		m_PacketMinimumID = packet_id;
	}

	auto it = m_mapPacketReceiveCount.find(packet_id);
	if (it == m_mapPacketReceiveCount.end())
	{
		m_mapPacketReceiveCount.insert({ packet_id, 1 });
	}
	else
	{
		(it->second)++;
	}
}

void CDevManager::GetPacketCountReport(std::map<DHOST_TYPE_UINT16, DHOST_TYPE_INT32>& mapInfo)
{
	for (auto element : m_mapPacketReceiveCount)
	{
		mapInfo.insert(std::pair<DHOST_TYPE_UINT16, DHOST_TYPE_INT32>(element.first, element.second));
	}
}

void CDevManager::DevConsole()
{
#ifdef _WIN32
	if (FreeConsole())
	{
		if (AllocConsole())
		{
			std::cout.clear();
			std::cerr.clear();
			std::cin.clear();

			SetConsoleTitle(TEXT("HostTest"));
			_tfreopen(_T("CONOUT$"), _T("w"), stdout);
			_tfreopen(_T("CONIN$"), _T("r"), stdin);
			_tfreopen(_T("CONERR$"), _T("w"), stderr);
			_tsetlocale(LC_ALL, _T(""));
		}
	}

#endif
}

void CDevManager::DevAiLevelSetting(DHOST_TYPE_UINT32 team, DHOST_TYPE_UINT32 aiLevel)
{
	// team value
	// 0 = none, 1 = home, 2 = away
	switch (team)
	{
		case 1:
			SetHomeAiLevel(aiLevel);
			break;
		case 2:
			SetAwayAiLevel(aiLevel);
			break;
		default:
			break;
	}
}