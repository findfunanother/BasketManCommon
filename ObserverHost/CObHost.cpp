#include <string.h>

#include "CObHost.h"


#define OB_CREATE_PACKET_FOR_STRUCT_DIRECT(packetid,message,...) flatbuffers::FlatBufferBuilder __builder(1);\
														   CFlatBufPacket<F4PACKET::packetid##_data> message(F4PACKET::PACKET_ID::packetid);\
														   F4PACKET::packetid##_data __data(__VA_ARGS__);\
														   __builder.Finish(__builder.CreateStruct(__data));\
														   message.Decode((char*)__builder.GetBufferPointer(), __builder.GetSize());


CObHost::CObHost(char* hostID, int option)
{
    m_CallbackCpp = nullptr;
    //m_CallbackRedis = nullptr;
    m_CallbackLog = nullptr;
	m_HostConnected = nullptr;
    
    
}

CObHost::~CObHost()
{
	if (m_PacketDataBuffer)
	{
		free(m_PacketDataBuffer);
	}

	m_PacketDataBuffer = nullptr;
}

void CObHost::Initialize()
{
	m_PacketDataBuffer = (char*)malloc(gPacketBufferSize + kPACKET_HEADER_BUFFER_SIZE);
}

void CObHost::Release()
{
	if (false == m_HostUserMap.empty())
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;

			SAFE_DELETE(pUser);
		}
		m_HostUserMap.clear();
	}

	for (size_t i = 0; i < m_packetData.size(); i++)
	{
		if (m_packetData[i] != nullptr)
		{
			if (m_packetData[i]->pData != nullptr)
			{
				SAFE_DELETE(m_packetData[i]->pData);
			}
			SAFE_DELETE(m_packetData[i]);
		}
	}

	m_packetData.clear();
}

void CObHost::StorePacket(const char* pData, DHOST_TYPE_INT32 size)
{
	PacketData* data = new PacketData();
	char* packetdata = new char[size];
	::memcpy(packetdata, pData, size);
	data->pData = packetdata;
	data->size = size;
	m_packetData.push_back(data);
	
	//string log_message = "[PACKET_STORED] message stored : "+to_string(size);
	//ToLog(log_message.c_str());
	return;
}

void CObHost::ProcessPacket(const char* pData, DHOST_TYPE_INT32 size, void* peer)
{
	
	
	FlatBufProtocol::Header header_info;
	DHOST_TYPE_USER_ID userID = UserGetID(peer);

	if (!FlatBufProtocol::GetHeadInfo(header_info, pData, size))
	{
		return;
	}

	//CPacketImplement<F4PACKET::PACKET_ID>* pPacket = nullptr;
	//auto result = FlatBufProtocol::GetPacket(pPacket, pData, size);
	
	if (header_info.packetID == 30000)
	{
		return;
	}
	// CProtoBufPacketImplement* pPacket = nullptr;

	//auto result = ProtoBufProtocol::GetPacket(pPacket, pData, size);

	// if (nullptr == pPacket)
	// {
		// string log_message = "[PACKET_RECEIVED] Is not ProtoMessage : "+to_string(header_info.packetID);
		// ToLog(log_message.c_str());
		// return;
	// }
	
	if (m_CallbackCpp != nullptr)
	{
		if(!UserGetID(peer)) // Only HostMessage is available broadcasting
		{
			
			{
				//string log_message = "[PACKET_RECEIVED] Packet received ID stored : " + to_string(header_info.packetID);
				//ToLog(log_message.c_str());
			}
			
			StorePacket(pData, size);
			for (auto& it : m_HostUserMap)	// Broadcast Packet
			{
				auto pUser = it.second;
				if (nullptr == pUser)
				{
					continue;
				}
	
				if (peer == pUser->GetPeer())
				{
					continue;
				}
	
				if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
				{
					if(pUser->GetConnectState() == ECONNECT_STATE::JOIN)
					{
						if(m_packetData.size()>=3)
						{
							for (size_t i = 1; i < 3; i++)
							{
								auto data = m_packetData[i];
								m_CallbackCpp(data->pData, data->size, UserGetID(peer));
							}
						}
					}
					continue;
				}
				else
				{
					m_CallbackCpp(pData, size, it.first);
				}

				/*{
					string log_message = "[PACKET_SEND] message userID: " + to_string(it.first) + " size: " + to_string(size);
					ToLog(log_message.c_str());
				}*/
				
			}
		}
		else
		{
			//CProtoBufPacketImplement* pPacket = nullptr;
			//string log_message = "[PACKET_RECEIVED] User Data";
			//ToLog(log_message.c_str());
			//auto packetdata = ProtoBufProtocol::GetPacket(pPacket, pData, size);

			switch((F4PACKET::PACKET_ID)header_info.packetID)
			{
				case F4PACKET::PACKET_ID::system_c2s_login:
				{	
					CPacketImplement<F4PACKET::PACKET_ID>* packet = nullptr;
					auto packetresult = FlatBufProtocol::GetPacket(packet, pData, size);
					auto pPacket = reinterpret_cast<CFlatBufPacket<F4PACKET::system_c2s_login_data>*>(packet);
					auto& data = *pPacket->GetData();

					F4PACKET::RESULT result = F4PACKET::RESULT::fail;
					F4PACKET::LOGIN_TYPE type = F4PACKET::LOGIN_TYPE::none;
					F4PACKET::EXIT_TYPE exitType = F4PACKET::EXIT_TYPE::lobby;

					
					flatbuffers::FlatBufferBuilder login_builder(1);
					CFlatBufPacket<F4PACKET::system_s2c_login_data> message(F4PACKET::PACKET_ID::system_s2c_login);
					auto login_packet = F4PACKET::Createsystem_s2c_login_data(login_builder, result, type, exitType, 240.f);
					login_builder.Finish(login_packet);
					message.Decode((const char*)login_builder.GetBufferPointer(), login_builder.GetSize());
					SendPacket(message, data.userid());

					if (m_packetData.size() >= 3)
					{
						for (size_t i = 1; i < 3; i++)
						{
							auto data = m_packetData[i];
							m_CallbackCpp(data->pData, data->size, UserGetID(peer));


						}
					}
					else
					{
						m_UIDwaiting.push_back(UserGetID(peer));
						string log_message = "[OBHost] connect early " + to_string(UserGetID(peer)) + " message count: " + to_string(m_packetData.size());
						ToLog(log_message.c_str());
					}
					break;
				}
				case F4PACKET::PACKET_ID::system_c2s_readyToLoad:
				{
					break;
				}
				case F4PACKET::PACKET_ID::system_c2c_loadingPercentage:
				{
					CPacketImplement<F4PACKET::PACKET_ID>* packet = nullptr;
					auto packetresult = FlatBufProtocol::GetPacket(packet, pData, size);
					auto pPacket = reinterpret_cast<CFlatBufPacket<F4PACKET::system_c2c_loadingPercentage_data>*>(packet);
					auto& data = *pPacket->GetData();


					flatbuffers::FlatBufferBuilder loading_builder(1);
					CFlatBufPacket<F4PACKET::system_c2c_loadingPercentage_data> message(F4PACKET::PACKET_ID::system_c2c_loadingPercentage);
					auto loading_packet = F4PACKET::Createsystem_c2c_loadingPercentage_data(loading_builder, data.userid(), data.percentage());
					loading_builder.Finish(loading_packet);
					message.Decode((const char*)loading_builder.GetBufferPointer(), loading_builder.GetSize());
					SendPacket(message, data.userid());
				}
				break;
				case F4PACKET::PACKET_ID::system_c2s_loadComplete:
				{
					
					for (size_t i = 3; i < m_packetData.size(); i++)
					{
						auto data = m_packetData[i];
						m_CallbackCpp(data->pData, data->size, UserGetID(peer));
			
					}
					CObUserInfo* pUser = this->ObserverFind(UserGetID(peer));
					if(pUser)
						pUser->SetConnectState(ECONNECT_STATE::CONNECT);
					else
					{
						string log_message = "[PACKET_SEND] not exist userID: "+to_string(UserGetID(peer))+" message count: "+to_string(m_packetData.size());
						ToLog(log_message.c_str());
					}
					
					break;
				}
				case F4PACKET::PACKET_ID::system_c2s_exitGame:
				{
					OnHostDisconnect(UserGetID(peer));
				}
				break;
			}
		}
	}
	
}


DHOST_TYPE_INT32 CObHost::SendPacket(CPacketBase& packet, const DHOST_TYPE_USER_ID UserID)
{
	char* pSendBuffer = m_PacketDataBuffer;

	int buff_size = 0;

	packet.SetPacketType(kPACKET_TYPE_ALL_WORK);


	{
		FlatBufProtocol::Encode(&packet, &pSendBuffer, buff_size, malloc, free);
	}

	// InGameServer
	if (m_CallbackCpp != nullptr)
	{
		if (UserID != kUSER_ID_INIT)	// Personal Packet
		{

			m_CallbackCpp(pSendBuffer, buff_size, UserID);
		}

		return 0;
	}

	if (m_CallbackBroadcast != nullptr)
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			if (UserID == it.first)
			{
				HostMessage hostMessage;

				hostMessage.data = pSendBuffer;
				hostMessage.size = buff_size;
				hostMessage.userID = it.first;
				hostMessage.peer = it.second->GetPeer();

				m_CallbackBroadcast(&hostMessage);

				return 0;
			}
		}
	}

	string log_message = "[SEND_PACKET_ERROR] PakcetID : " + to_string(packet.GetPacketID()) + ", UserID : " + to_string(UserID);
	ToLog(log_message.c_str());

	return 0;
}


void CObHost::BroadcastPacket(char* packet)
{
	char* pSendBuffer = m_PacketDataBuffer;

	int buff_size = 0;
	
	// InGameServer
	if (m_CallbackCpp != nullptr)
	{
		for (auto& it : m_HostUserMap)	// Broadcast Packet
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			/*if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
			{
				continue;
			}*/

			m_CallbackCpp(pSendBuffer, buff_size, it.first);
		}
		//m_CallbackCpp(pSendBuffer, buff_size, 0);
	}

	// HostTestServer
	if (m_CallbackBroadcast != nullptr)
	{
		for (auto& it : m_HostUserMap)
		{
			auto pUser = it.second;
			if (nullptr == pUser)
			{
				continue;
			}

			/*if (UserID == it.first)
			{
				continue;
			}

			if (pUser->GetConnectState() != ECONNECT_STATE::CONNECT)
			{
				continue;
			}*/

			HostMessage hostMessage;

			hostMessage.data = pSendBuffer;
			hostMessage.size = buff_size;
			hostMessage.userID = it.first;
			hostMessage.peer = it.second->GetPeer();

			m_CallbackBroadcast(&hostMessage);
		}
	}

}
void CObHost::Update(float timeDelta)
{
	//string log_message = "[Waiting] ---------------- 1: ";
	//	ToLog(log_message.c_str());
	if(m_UIDwaiting.size()>0&&m_packetData.size()>=3)
	{
		string log_message = "[Waiting] ---------------- : ";
		ToLog(log_message.c_str());
		vector<unsigned int>::iterator iter;			
		
		for(iter = m_UIDwaiting.begin(); iter != m_UIDwaiting.end() ; iter++)
		{
			for (size_t i = 1; i < 3; i++)
			{
				auto data = m_packetData[i];
				m_CallbackCpp(data->pData, data->size, *iter);
			}
			m_UIDwaiting.erase(iter);
			break;
		}
	}
}



void CObHost::RegistCallbackFunc(ProcessPacketCallbackToCpp pFunc)
{
	m_CallbackCpp = std::move(pFunc);
}

void CObHost::RegistCallbackFuncLog(LogCallback pFunc)
{
	m_CallbackLog = std::move(pFunc);
}

void CObHost::RegistCallbackFuncSendPacket(HostMessageCallback pFunc)
{
	m_CallbackBroadcast = std::move(pFunc);
}

void CObHost::OnHostJoin(unsigned int UserID)
{
	//if (m_HostUserMap.empty() == false)
	//{
	//	auto iter = m_HostUserMap.find(UserID);
	//
	//	if (iter != m_HostUserMap.end())
	//	{
	//		iter->second->SetLogin(false);
	//		return;
	//	}
	//}
	//
	CObUserInfo* pUser = this->ObserverFind(UserID);

	if (pUser == nullptr)
	{
		pUser = new CObUserInfo(UserID, (int*)UserID);
		string log_messgae = "CreateUser UserID : " + to_string(UserID);
		ToLog(log_messgae.c_str());
	}

	pUser->SetConnectState(ECONNECT_STATE::JOIN);

	string log_message = "OnHostJoin m_HostUserMap insert UserID : " + to_string(UserID);
	ToLog(log_message.c_str());

	m_HostUserMap.insert(ObserverUserMapType::value_type(UserID, pUser));
	// if(m_HostConnected==nullptr)
	// {
		// log_message = "m_HostConnected Added : " + to_string(UserID);
		// ToLog(log_message.c_str());
		// pUser->SetObserverHost();
		// m_HostConnected = pUser;
	// }
}

void CObHost::OnHostConnect(unsigned int UserID)
{
	CObUserInfo* pUser = this->FindObserver(UserID);
	string log_message = "OnHostConnect User Connected : " + to_string(UserID);
	ToLog(log_message.c_str());
	if (pUser == nullptr)
	{
		string log_message = "OnHostConnect pUser is null : " + to_string(UserID);
		ToLog(log_message.c_str());
	}
	
	

}

void CObHost::OnHostDisconnect(unsigned int UserID)
{
	CObUserInfo* pUser = FindObserver(UserID);

	if (nullptr == pUser)
		return;

	if (pUser->GetConnectState() == ECONNECT_STATE::DISCONNECT)
	{
		return;
	}
	
	pUser->SetConnectState(ECONNECT_STATE::DISCONNECT);

	pUser->SetLogin(false);

	pUser = nullptr;
	RemoveObserver(UserID);
	return;
}

void CObHost::OnHostQuit(unsigned int UserID)
{
	return;
}

void CObHost::ToLog(const char* msg)
{
	if (m_CallbackLog != nullptr)
	{
		m_CallbackLog(msg);
	}
}
void CObHost::OnHostExit()
{
	return;
}

//void CObHost::OnRedisMatchInfo(SRedisMatchInfo* pData)
//{
//}

void CObHost::ObserverAdd(unsigned int userid, void* peer)
{
}

int CObHost::ObserverCount()
{
	return m_HostUserMap.size();
}

CObUserInfo* CObHost::ObserverFind(unsigned int UserID)
{
    auto iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return nullptr;

	return iter->second;
}

void CObHost::RemoveObserver(unsigned int UserID)
{
    auto iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return;
	
	m_HostUserMap.erase(iter);
}


unsigned int CObHost::UserGetID(void* peer)
{
	for (std::map<DHOST_TYPE_USER_ID, CObUserInfo*>::iterator it = m_HostUserMap.begin(); it != m_HostUserMap.end(); ++it)
	{
		if (peer == it->second->GetPeer())
		{
			return it->first;
		}
	}

	return 0;
}

CObUserInfo* CObHost::FindObserver(unsigned int UserID)
{
	auto iter = m_HostUserMap.find(UserID);

	if (iter == m_HostUserMap.end())
		return nullptr;

	return iter->second;
}


