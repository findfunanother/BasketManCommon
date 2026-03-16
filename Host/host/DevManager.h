#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#pragma warning(disable: 4996) 
#endif

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>
#include <locale.h>
#include <tchar.h>
#endif
#include <map>
#include "FlatBufProtocol.h"


class CHost;

class CDevManager
{
public:

	CDevManager();
	~CDevManager();

	void UpdateDevManager(DHOST_TYPE_FLOAT timeDelta);

	void DevPlayPacketSave(DHOST_TYPE_BOOL value);
	DHOST_TYPE_BOOL GetPlayPacketSave() { return m_bPlayPacketSave; }
	void SetPlayPacketSave(DHOST_TYPE_BOOL value) { m_bPlayPacketSave = value; }
	void SaveBinary(const char* pData, DHOST_TYPE_UINT32 size, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_FLOAT time);
	void LoadBinary(DHOST_TYPE_USER_ID UserID);

	void InitPacketCount();
	DHOST_TYPE_UINT64 GetPacketTotalSize();
	void IncreasePacketReceiveCount(DHOST_TYPE_UINT16 packet_id, DHOST_TYPE_UINT64 packet_size);
	void GetPacketCountReport(std::map<DHOST_TYPE_UINT16, DHOST_TYPE_INT32>& mapInfo);

	void DevConsole();
	void DevAiLevelSetting(DHOST_TYPE_UINT32 team, DHOST_TYPE_UINT32 aiLevel);

	DHOST_TYPE_UINT32 GetHomeAiLevel() { return m_DevHomeAiLevel; }
	void SetHomeAiLevel(DHOST_TYPE_UINT32 value) { m_DevHomeAiLevel = value; }

	DHOST_TYPE_UINT32 GetAwayAiLevel() { return m_DevAwayAiLevel; }
	void SetAwayAiLevel(DHOST_TYPE_UINT32 value) { m_DevAwayAiLevel = value; }
	
	DHOST_TYPE_BOOL GetDevNoKick() { return m_DevNoKick; }
	void SetDevNoKick(DHOST_TYPE_BOOL value) { m_DevNoKick = value; }

	DHOST_TYPE_UINT64 GetPacketMaximumSize() { return m_PacketMaximumSize; }
	DHOST_TYPE_UINT64 GetPacketMinimumSize() { return m_PacketMinimumSize; }
	DHOST_TYPE_UINT16 GetPacketMaximumID() { return m_PacketMaximumID; }
	DHOST_TYPE_UINT16 GetPacketMinimumID() { return m_PacketMinimumID; }

	DHOST_TYPE_UINT64 m_MovePB_PacketSize;
	DHOST_TYPE_UINT64 m_MoveFB_PacketSize;

private:

	DHOST_TYPE_BOOL m_bPlayPacketSave;	// 패킷을 바이너리 형태로 저장

	std::map<DHOST_TYPE_UINT16, DHOST_TYPE_INT32> m_mapPacketReceiveCount;	// 패킷 카운터
	DHOST_TYPE_UINT64 m_PacketSize;

	//! 확인용 코드 나중에 삭제하자
	DHOST_TYPE_UINT64 m_PacketMaximumSize;	// 최대크기인 패킷 사이즈
	DHOST_TYPE_UINT16 m_PacketMaximumID;	// 최대크기의 패킷
	DHOST_TYPE_UINT64 m_PacketMinimumSize;	// 최소크기인 패킷 사이즈
	DHOST_TYPE_UINT16 m_PacketMinimumID;	// 최소크기의 패킷
	
	DHOST_TYPE_UINT32 m_DevHomeAiLevel;	// 홈팀 AI 레벨 셋팅값
	DHOST_TYPE_UINT32 m_DevAwayAiLevel;	// 어웨이팀 AI 레벨 셋팅값

	DHOST_TYPE_BOOL m_DevNoKick;
};

