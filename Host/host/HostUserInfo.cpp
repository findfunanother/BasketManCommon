#include "HostUserInfo.h"
#include "Host.h"

CHostUserInfo::CHostUserInfo(DHOST_TYPE_INT32 UserID, void* peer, CHost* host, DHOST_TYPE_FLOAT CreateTime, EUSER_TYPE value) : m_UserID(UserID), m_Peer(peer), m_Host(host), m_CreateTime(CreateTime), m_UserType(value)
{
	//string log_messgae = "CreateUser UserID : " + to_string(UserID);
	//m_Host->ToLog(log_messgae.c_str());

	m_LoadingStartTime = kFLOAT_INIT;

	Init();
}

void CHostUserInfo::Init()
{
	m_ConnectState = ECONNECT_STATE::NONE;
	m_PingIndex = kINT32_INIT;
	m_RecvPingIndex = kINT32_INIT;
	m_nCheckSpeedCnt = kINT32_INIT;
	m_ClientElapsedTime = kFLOAT_INIT;
	m_ClientElapsedTimeAccumulate = kFLOAT_INIT;
	m_bSpeedHackCheckLog = false;

	m_PingDiffDeque.clear();
	m_PingMap.clear();
	m_ClientElapsedTimeMap.clear();
	m_vecClientElapsedTimeWithSystemTime.clear();

	m_RandomSeed = kUINT32_INIT;
	m_RandomSeedIndex = kUINT32_INIT;
	m_SendedRandomSeed = kUINT32_INIT;
	m_SendedActionKey = kUINT32_INIT;

	SetSpectatorLoadComplete(false);
	SetPingAverage(kFLOAT_INIT);
	InitUserWithAiCharacter();

	m_fPreClientElapsedTime = kFLOAT_INIT;
	m_fPreClientSystemTime = 0.0f;
	m_nDetectCount = kINT32_INIT;

	

	m_nSpeedHackDectectedCount = kINT32_INIT;

	m_curFPS = kFLOAT_INIT;

	m_PingAverage = 0.05f;
}

CHostUserInfo::~CHostUserInfo()
{
}

void CHostUserInfo::SetConnectState(ECONNECT_STATE NewState)
{
	string log_message = "SetConnectState UserID : " + to_string(this->GetUserID()) + ", CURRENT_STATE  : " + ConvertConnectState(m_ConnectState).c_str() + ", CHANGE_STATE  : " + ConvertConnectState(NewState).c_str();
	m_Host->ToLog(log_message.c_str());

	m_ConnectState = NewState;

	if (m_ConnectState == ECONNECT_STATE::DISCONNECT)
	{
		this->SetLogin(false);
	}
}

std::string CHostUserInfo::ConvertConnectState(ECONNECT_STATE NewState)
{
	std::string state = "NONE";

	switch (NewState)
	{
		case ECONNECT_STATE::CONNECT:
			state = "CONNECT";
			break;
		case ECONNECT_STATE::DISCONNECT:
			state = "DISCONNECT";
			break;
		case ECONNECT_STATE::JOIN:
			state = "JOIN";
			break;
		case ECONNECT_STATE::QUIT:
			state = "QUIT";
			break;
		case ECONNECT_STATE::WAIT:
			state = "WAIT";
			break;
		default:
			break;
	}

	return state;
}


DHOST_TYPE_FLOAT CHostUserInfo::ProcessAlivePacket(DHOST_TYPE_GAME_TIME_F serverTime)
{
	if (!m_PingDiffDeque.empty())
	{
		DHOST_TYPE_GAME_TIME_F lastValue = m_PingDiffDeque.front();
		DHOST_TYPE_FLOAT gap = serverTime - lastValue;

		PushPingMap(m_RecvPingIndex, gap);

		/*
		DHOST_TYPE_FLOAT sum = 0.0;
		for (const auto& pair : m_PingMap)
		{
			sum += pair.second;
		}
		*/
		DHOST_TYPE_FLOAT sum = 0.0;

		const float maxCnt = m_Host->GetAlivePingMaxCnt();

		auto it = m_PingMap.rbegin(); // rbegin()은 맵의 끝에서 시작하는 reverse iterator
		int count = 0;

		while (it != m_PingMap.rend() && count < maxCnt) {
			sum += it->second;
			++it;
			++count;
		}

		if (m_PingMap.size() > 0)
		{
			SetPingAverage(sum / count);
		}

		m_Host->SetAvgPingLatency(m_UserID, GetPingAverage());

		/*
		string log_message = "AvgPing : " + std::to_string(GetPingAverage()) 
			+ ", UserID : " + std::to_string(GetUserID()) 
			+ ", Diff : " + std::to_string(gap)
			+ ", Index : " + std::to_string(m_RecvPingIndex);
				m_Host->ToLog(log_message.c_str());
		*/
		

		m_RecvPingIndex++;

		m_PingDiffDeque.pop_front();

		return gap;
	}

	return 0.0f;
}


void CHostUserInfo::PushUserGamePingDeque(DHOST_TYPE_GAME_TIME_F value)
{
	if (m_PingDiffDeque.size() > m_Host->GetAlivePingMaxCnt()) // 못받아서 쌓인 경우, 최대 7, 연결이 없다고 보고 킥을 위한 전처리 기능 
	{
		string log_message = "AlivePacket NoReply : " + std::to_string(GetPingAverage())
			+ " RoomID : " + m_Host->GetHostID()
			+ " RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime())
			+ ", UserID : " + std::to_string(GetUserID())
			+ ", Count : " + std::to_string(m_PingDiffDeque.size());

		m_Host->ToLog(log_message.c_str());

		m_PingMap.clear();

		SetPingAverage(m_Host->GetAlivePingMaxCnt()); // 강제 셋팅 7초 
	}
	else
	{
		
		/*
		string log_message = "m_PingDiffDeque Size : " + std::to_string(m_PingDiffDeque.size())
			+ ", UserID : " + std::to_string(GetUserID())
			+ ", Count : " + std::to_string(m_PingDiffDeque.size());
		m_Host->ToLog(log_message.c_str());
		*/
		
		
	}

	m_PingDiffDeque.push_back(value);
}


void CHostUserInfo::PushUserGamePingDiffDeque(DHOST_TYPE_INT32 idx, DHOST_TYPE_GAME_TIME_F value)
{
	if (m_PingDiffDeque.size() > m_Host->GetAlivePingMaxCnt())
	{
		USER_PING_DEQUE::iterator iter;

		DHOST_TYPE_GAME_TIME_F result = kTIME_F_INIT;

		for (iter = m_PingDiffDeque.begin(); iter != m_PingDiffDeque.end(); ++iter)
		{
			result += *iter;
		}

		SetPingAverage(result / m_PingDiffDeque.size());

		m_Host->SetAvgPingLatency(m_UserID, GetPingAverage());

		if (GetPingAverage() > kPING_EXPIRED_AVG_TIME)
		{
			for (iter = m_PingDiffDeque.begin(); iter != m_PingDiffDeque.end(); ++iter)
			{
				string log_message = "PingDiffDeque RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(GetUserID()) + ", Diff : " + std::to_string(*iter);
				m_Host->ToLog(log_message.c_str());
			}
		}

		m_PingDiffDeque.pop_front();
	}

	m_PingDiffDeque.push_back(value);

	IncreasePingIndex();
}

void CHostUserInfo::ClearPingData()
{
	if (m_Host != nullptr)
	{
		string log_message = "[AvgPing] UserID : " + to_string(this->GetUserID())
			+ " RoomID : " + m_Host->GetHostID()
			+ " AvgPing : " + to_string(m_PingAverage);

		m_Host->ToLog(log_message.c_str());
	}

	m_bSpeedHackCheckLog = false;

	m_nSpeedHackDectectedCount = 0;

	m_LoadingStartTime = kFLOAT_INIT;

	SetPingIndex(kINT32_INIT);
	SetPingAverage(kFLOAT_INIT);

	m_nDetectCount = 0;

	m_PingMap.clear();
	m_PingDiffDeque.clear();
	m_ClientElapsedTimeMap.clear();
	m_vecClientElapsedTimeWithSystemTime.clear();
}

void CHostUserInfo::PushUserWithAiCharacter(DHOST_TYPE_CHARACTER_SN value)
{
	for (int i = 0; i < m_VecUserWithAiCharacter.size(); ++i)
	{
		if (m_VecUserWithAiCharacter[i] == value)
		{
			return;
		}
	}

	m_VecUserWithAiCharacter.push_back(value);
}

void CHostUserInfo::RemoveUserWithAiCharacter(DHOST_TYPE_CHARACTER_SN value)
{
	std::vector<DHOST_TYPE_CHARACTER_SN>::iterator iter;

	for (iter = m_VecUserWithAiCharacter.begin(); iter != m_VecUserWithAiCharacter.end(); ++iter)
	{
		if (*iter == value)
		{
			m_VecUserWithAiCharacter.erase(iter);

			break;
		}
	}
}

void CHostUserInfo::PushPingMap(DHOST_TYPE_INT32 key, DHOST_TYPE_FLOAT value)
{
	std::pair<CHostUserInfo::USER_PING_MAP::iterator, bool> iter = m_PingMap.insert(std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::value_type(key, value));

	if (iter.second == false)
	{
		string log_message = "FailPushPingMap RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(GetUserID()) + ", Key : " + std::to_string(key) + ", Value : " + std::to_string(value);
		m_Host->ToLog(log_message.c_str());
	}
}

void CHostUserInfo::PushClientElapsedTimeMap(DHOST_TYPE_INT32 key, DHOST_TYPE_FLOAT value)
{
	std::pair<CHostUserInfo::USER_PING_MAP::iterator, bool> iter = m_ClientElapsedTimeMap.insert(std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::value_type(key, value));

	if (iter.second == false)
	{
		string log_message = "FailPushClientElapsedTimeMap RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(GetUserID()) + ", Key : " + std::to_string(key) + ", Value : " + std::to_string(value);
		m_Host->ToLog(log_message.c_str());
	}
}

void CHostUserInfo::PushClientElapsedTimeWithSystemTime(DHOST_TYPE_FLOAT clientElapsedTime, DHOST_TYPE_DOUBLE clientSystemTime)
{

	DHOST_TYPE_FLOAT dff1 = std::abs(clientElapsedTime - m_fPreClientElapsedTime);
	DHOST_TYPE_FLOAT dff2 = (float)std::abs(clientSystemTime - m_fPreClientSystemTime);
	DHOST_TYPE_FLOAT diff3 = std::abs(dff1 - dff2);


	if (m_Host->GetCurrentState() != EHOST_STATE::PLAY)
	{
		m_fPreClientElapsedTime = clientElapsedTime;
		m_fPreClientSystemTime = clientSystemTime;
		return;
	}

	if (dff1 <= 0.0f || dff2 <= 0.0f) // 네트웍이 안좋은 경우 이렇게 온다 
	{
		m_nDetectCount++;

		if (m_nDetectCount > 5)
		{
			m_nDetectCount = 0;
			m_Host->KickToTheLobby(GetUserID(), F4PACKET::EKICK_TYPE::long_latency);
		}

		m_fPreClientElapsedTime = clientElapsedTime;
		m_fPreClientSystemTime = clientSystemTime;
		return;
	}

	if (kSPEED_HACK_DETECT_VALUE < diff3 && GetSpeedHackCheckLog() == false )
	{
		m_nDetectCount++;

		if (m_nDetectCount > 3)
		{
			if (diff3 > kSPEED_HACK_DETECT_VALUE * 2)
			{
				if (m_Host->FindUser(GetUserID())->GetPingAverage() < 0.07f)
				{
					//m_Host->SetSpeedHackLevel(m_UserID, kHACK_CHECK_LEVEL_TWO);
					//m_Host->KickToTheLobby(GetUserID(), F4PACKET::EKICK_TYPE::speed_hack);

					m_Host->KickToTheLobby(GetUserID(), F4PACKET::EKICK_TYPE::long_latency);

					string log_message = "[HACK_CHECK] [SPEED_HACK_CHECK_KICK] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
						+ ", UserID : " + std::to_string(GetUserID())
						+ ", m_nDetectCount : " + std::to_string(m_nDetectCount)
						+ ", dff1 : " + std::to_string(dff1)
						+ ", dff2 : " + std::to_string(dff2)
						+ ", diff3 : " + std::to_string(diff3)
						+ ", Ping : " + std::to_string(m_Host->FindUser(GetUserID())->GetPingAverage());

					m_Host->ToLog(log_message.c_str());
				}
				else
				{
					//m_Host->SetSpeedHackLevel(m_UserID, kHACK_CHECK_LEVEL_THREE);

					m_Host->KickToTheLobby(GetUserID(), F4PACKET::EKICK_TYPE::long_latency);

					string log_message = "[HACK_CHECK] [SPEED_HACK_CHECK_KICK] [long_latency] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
						+ ", UserID : " + std::to_string(GetUserID())
						+ ", m_nDetectCount : " + std::to_string(m_nDetectCount)
						+ ", dff1 : " + std::to_string(dff1)
						+ ", dff2 : " + std::to_string(dff2)
						+ ", diff3 : " + std::to_string(diff3)
						+ ", Ping : " + std::to_string(m_Host->FindUser(GetUserID())->GetPingAverage());

					m_Host->ToLog(log_message.c_str());
				}
			}
			else
			{
				if (m_Host->FindUser(GetUserID())->GetPingAverage() < 0.07f)
				{
					// 처벌은 안하고 메일만 보낸다. 
					//m_Host->SetSpeedHackLevel(m_UserID, kHACK_CHECK_LEVEL_ONE);

					string log_message = "[HACK_CHECK] [SPEED_HACK_CHECK] [THREE] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
						+ ", UserID : " + std::to_string(GetUserID())
						+ ", m_nDetectCount : " + std::to_string(m_nDetectCount)
						+ ", dff1 : " + std::to_string(dff1)
						+ ", dff2 : " + std::to_string(dff2)
						+ ", diff3 : " + std::to_string(diff3)
						+ ", Ping : " + std::to_string(m_Host->FindUser(GetUserID())->GetPingAverage());

					m_Host->ToLog(log_message.c_str());
				}
				else
				{
					
					// 처벌은 안하고 메일만 보낸다. 
					//m_Host->SetSpeedHackLevel(m_UserID, kHACK_CHECK_LEVEL_THREE);

					string log_message = "[HACK_CHECK] [SPEED_HACK_CHECK] [THREE] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
						+ ", UserID : " + std::to_string(GetUserID())
						+ ", m_nDetectCount : " + std::to_string(m_nDetectCount)
						+ ", dff1 : " + std::to_string(dff1)
						+ ", dff2 : " + std::to_string(dff2)
						+ ", diff3 : " + std::to_string(diff3)
						+ ", Ping : " + std::to_string(m_Host->FindUser(GetUserID())->GetPingAverage());

					m_Host->ToLog(log_message.c_str());
					
				}
			}
		}
	}
	else
	{
		m_nDetectCount = 0;
	}

	m_fPreClientElapsedTime = clientElapsedTime;
	m_fPreClientSystemTime = clientSystemTime;
}

/*
void CHostUserInfo::PushClientElapsedTimeWithSystemTime(DHOST_TYPE_FLOAT clientElapsedTime, DHOST_TYPE_DOUBLE clientSystemTime)
{
	m_vecClientElapsedTimeWithSystemTime.push_back(std::make_pair(clientElapsedTime, clientSystemTime));

	DHOST_TYPE_FLOAT diff_clientElapsedTime3 = kFLOAT_INIT;
	DHOST_TYPE_FLOAT diff_clientElapsedTime4 = kFLOAT_INIT;
	DHOST_TYPE_FLOAT diff_clientSystemTime3 = kFLOAT_INIT;
	DHOST_TYPE_FLOAT diff_clientSystemTime4 = kFLOAT_INIT;
	DHOST_TYPE_FLOAT diff3 = kFLOAT_INIT;
	DHOST_TYPE_FLOAT diff4 = kFLOAT_INIT;

	if (m_vecClientElapsedTimeWithSystemTime.size() > 5)
	{
		try
		{
			diff_clientElapsedTime3 = m_vecClientElapsedTimeWithSystemTime.at(4).first - m_vecClientElapsedTimeWithSystemTime.at(3).first;

			diff_clientSystemTime3 = (DHOST_TYPE_FLOAT)((m_vecClientElapsedTimeWithSystemTime.at(4).second - m_vecClientElapsedTimeWithSystemTime.at(3).second) / 1000.0f);

			diff3 = std::abs(diff_clientSystemTime3 - diff_clientElapsedTime3);

			if (diff3 > kSPEED_HACK_DETECT_VALUE && GetSpeedHackCheckLog() == false || diff_clientElapsedTime3 <= 0.0f || diff_clientSystemTime3 == 0.0f)
			{
				diff_clientElapsedTime4 = m_vecClientElapsedTimeWithSystemTime.at(5).first - m_vecClientElapsedTimeWithSystemTime.at(4).first;

				diff_clientSystemTime4 = (DHOST_TYPE_FLOAT)((m_vecClientElapsedTimeWithSystemTime.at(5).second - m_vecClientElapsedTimeWithSystemTime.at(4).second) / 1000.0f);
				diff4 = std::abs(diff_clientSystemTime4 - diff_clientElapsedTime4);

				if (diff4 > kSPEED_HACK_DETECT_VALUE && GetSpeedHackCheckLog() == false || diff_clientElapsedTime4 <= 0.0f || diff_clientSystemTime4 == 0.0f )
				{
					m_nCheckSpeedCnt++;
				}

			}
		}
		catch(std::exception e)
		{
			string log_message = "[SPEED_HACK_CHECK] Exception RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
				+ ", UserID : " + std::to_string(GetUserID());
			m_Host->ToLog(log_message.c_str());
		}
		
		m_vecClientElapsedTimeWithSystemTime.erase(m_vecClientElapsedTimeWithSystemTime.begin());

		if ( m_nCheckSpeedCnt >= 3)
		{
			string log_message = "[SPEED_HACK_CHECK] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", GameTime : " + std::to_string(m_Host->GetGameTime())
				+ ", UserID : " + std::to_string(GetUserID())
				+ ", m_nCheckSpeedCnt : " + std::to_string(m_nCheckSpeedCnt)
				+ ", PreDataGapClientElapsedTime3 : " + std::to_string(diff_clientElapsedTime3)
				+ ", PreDataGapClientSystemTime3 : " + std::to_string(diff_clientSystemTime3)
				+ ", diff3 : " + std::to_string(diff3)
				+ ", PreDataGapClientElapsedTime4 : " + std::to_string(diff_clientElapsedTime4)
				+ ", PreDataGapClientSystemTime4 : " + std::to_string(diff_clientSystemTime4)
				+ ", diff4 : " + std::to_string(diff4);
			m_Host->ToLog(log_message.c_str());

			m_Host->KickToTheLobby(GetUserID(), F4PACKET::EKICK_TYPE::speed_hack);
		}
	}
}

*/

DHOST_TYPE_FLOAT CHostUserInfo::FindClientElapsedTimeMap(DHOST_TYPE_INT32 key)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	USER_PING_MAP::iterator iter = m_ClientElapsedTimeMap.find(key);

	if (iter != m_ClientElapsedTimeMap.end())
	{
		result = iter->second;
	}
	else
	{
		string log_message = "FindClientElapsedTimeMap InvalidKey RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(GetUserID()) + ", Key : " + std::to_string(key);
		m_Host->ToLog(log_message.c_str());
	}

	return result;
}

void CHostUserInfo::IncreasePingIndex()
{
	++m_PingIndex;
}

DHOST_TYPE_FLOAT CHostUserInfo::GetPingIndexElapsedTime(DHOST_TYPE_INT32 key)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	USER_PING_MAP::iterator iter = m_PingMap.find(key);

	if (iter != m_PingMap.end())
	{
		result = iter->second;
	}

	return result;
}

// 안쓰임
DHOST_TYPE_INT32 CHostUserInfo::GetCheckPingIndex()
{
	USER_PING_MAP::iterator iter = m_PingMap.find(m_PingIndex);

	// 이전틱에 핑을 보냈지만 응답을 받은게 없으면 레이턴시를 이전틱에서 현재틱의 차이만큼 더해줌
	if (iter != m_PingMap.end())
	{
		DHOST_TYPE_FLOAT diff = m_Host->GetRoomElapsedTime() - iter->second;

		string log_message = "[ALIVE_CHECK] ForcedPushLatency RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(this->GetUserID())
			+ ", Index : " + std::to_string(m_PingIndex) + ", IndexElapsedTime  : " + std::to_string(iter->second) + ", Latency : " + std::to_string(diff);
		m_Host->ToLog(log_message.c_str());

		PushUserGamePingDiffDeque(m_PingIndex, diff);
	}
	
	return m_PingIndex;
}

DHOST_TYPE_INT32 CHostUserInfo::GetCheckPingIndexEx()
{
	return m_PingIndex++;
}

void CHostUserInfo::SpeedHackDetected(SPEEDHACK_CHECK bitFlag)
{
	m_nSpeedHackDectectedCount = m_nSpeedHackDectectedCount | static_cast<int32_t>(bitFlag);


	if (IsSpeedhackDectected_Low())
	{
		//DHOST_TYPE_UINT32 hack_level = kHACK_CHECK_LEVEL_ONE;
		//m_Host->SetSpeedHackLevel(this->GetUserID(), hack_level); // 킥이 없음 

		string log_message = "[HACK_CHECK] IsSpeedhackDectected_Low RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(this->GetUserID());
		m_Host->ToLog(log_message.c_str());

	}
	
	if (IsSpeedhackDectected_High())
	{
		if (m_curFPS > 50) // 유저의 프레임 높은 경우에는 메일보내기 , 프레임이 매우 낮은 경우 이 조건에 걸리는 유저가 있다. 
		{
			DHOST_TYPE_UINT32 hack_level = kHACK_CHECK_LEVEL_TWO;
			//m_Host->SetSpeedHackLevel(this->GetUserID(), hack_level); // 메일 보내기 빼자 
			//m_Host->KickToTheLobby(this->GetUserID(), F4PACKET::EKICK_TYPE::speed_hack); // 일단 보류 , 일단 로그만 남기자 

			string log_message = "[HACK_CHECK]  IsSpeedhackDectected_High RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(this->GetUserID());
			m_Host->ToLog(log_message.c_str());
		}
		else
		{
			//m_Host->KickToTheLobby(this->GetUserID(), F4PACKET::EKICK_TYPE::speed_hack); // 일단 보류 , 일단 로그만 남기자 
			string log_message = "[HACK_CHECK] [IsSpeedhackDectected_High] [FPS_40_under] RoomElapsedTime : " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : " + std::to_string(this->GetUserID());
			m_Host->ToLog(log_message.c_str());
		}

	}
}

DHOST_TYPE_BOOL CHostUserInfo::IsSpeedhackDectected_Low()
{
	DHOST_TYPE_INT32 trueBit1 = static_cast<int32_t>(SPEEDHACK_CHECK::MOVE) | static_cast<int32_t>(SPEEDHACK_CHECK::SHOT); // 이동, 슛 
	DHOST_TYPE_INT32 trueBit2 = static_cast<int32_t>(SPEEDHACK_CHECK::MOVE) | static_cast<int32_t>(SPEEDHACK_CHECK::PICK); // 이동, 픽
	DHOST_TYPE_INT32 trueBit3 = static_cast<int32_t>(SPEEDHACK_CHECK::SHOT) | static_cast<int32_t>(SPEEDHACK_CHECK::PICK); // 슛, 픽 

	if ( (m_nSpeedHackDectectedCount & (trueBit1)) == (trueBit1) || (m_nSpeedHackDectectedCount & (trueBit2)) == (trueBit2) || (m_nSpeedHackDectectedCount & (trueBit3)) == (trueBit3))
	{
		return true;

	}
	return false;
}


DHOST_TYPE_BOOL CHostUserInfo::IsSpeedhackDectected_High()
{

	DHOST_TYPE_INT32 trueBit = static_cast<int32_t>(SPEEDHACK_CHECK::MOVE) | static_cast<int32_t>(SPEEDHACK_CHECK::SHOT) | static_cast<int32_t>(SPEEDHACK_CHECK::PICK); 

	if ((m_nSpeedHackDectectedCount & (trueBit)) == (trueBit))
	{
		return true;

	}
	return false;
}


DHOST_TYPE_BOOL CHostUserInfo::IsValidAction()
{
	return true;

	if (m_ClientActionKey == GetSendActionKey()) // 보냈을 당시 두개의 랜덤시드가 같을때만 유효하다 , 핵킹으로 무작위로 포지션을 보낼 수 있으므로 
	{
		return true;
	}
	else
	{
		// 로그 남기기 

		string invalid_buffer_log = "[HACK_CHECK] [***RandomKey****] Rebound, Pick Different RoomElapsed: " + std::to_string(m_Host->GetRoomElapsedTime()) + ", UserID : "
			+ std::to_string(this->GetUserID())
			+ ", RoomID : " + m_Host->GetHostID()
			+ " , Ping : " + std::to_string(GetPingAverage())
			+ " , FPS : " + std::to_string(GetCurFPS());

		m_Host->ToLog(invalid_buffer_log.c_str());

		return false;
	}

}

