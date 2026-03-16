#pragma once

#include "FlatBufPacketImplement.h"
//#define CREATEPACKETDATASTART(dataname,packetid) CFlatBufPacket<F4PACKET::packetid##_data> message(F4PACKET::PACKET_ID::packetid);\
//auto dataname = F4PACKET::Create##packetid##_data(__builder);
#define CREATE_AND_BROADCAST_FBPACKET(__builder,packetid,...) CFlatBufPacket<F4PACKET::packetid##_data> packetid##message(F4PACKET::PACKET_ID::packetid);\
												auto packetbuilder= F4PACKET::Create##packetid##_data(__builder,__VA_ARGS__);\
												__builder.Finish(packetbuilder);\
												auto datare = __builder.GetBufferPointer();\
												int buff_size = __builder.GetSize();\
												packetid##message.Decode((const char*)datare, buff_size);\
												m_pHost->BroadcastPacket(packetid##message, kUSER_ID_INIT);

#define CREATE_BUILDER(__builder) flatbuffers::FlatBufferBuilder __builder(1);
#define CREATE_AND_BROADCAST_FBPACKET_DIRECT(__builder,packetid,...) CFlatBufPacket<F4PACKET::packetid##_data> packetid##message(F4PACKET::PACKET_ID::packetid);\
												  auto packetbuilder= F4PACKET::Create##packetid##_dataDirect(__builder,__VA_ARGS__);\
												  __builder.Finish(packetbuilder);\
												  auto datare = __builder.GetBufferPointer();\
												  int buff_size = __builder.GetSize();\
												  packetid##message.Decode((const char*)datare, buff_size);\
												  m_pHost->BroadcastPacket(packetid##message, kUSER_ID_INIT);

#define CREATE_AND_SEND_FBPACKET(packetid,userid,...) flatbuffers::FlatBufferBuilder __builder(1);\
												  CFlatBufPacket<F4PACKET::packetid##_data> packetid##message(F4PACKET::PACKET_ID::packetid);\
												  auto packetbuilder= F4PACKET::Create##packetid##_dataDirect(__builder,__VA_ARGS__);\
												  __builder.Finish(packetbuilder);\
												  auto datare = __builder.GetBufferPointer();\
												  int buff_size = __builder.GetSize();\
												  packetid##message.Decode((const char*)datare, buff_size);\
												  m_pHost->SendPacket(packetid##message, userid);

#define CREATE_AND_SEND_FBPACKET_DIRECT(packetid,userid,...) flatbuffers::FlatBufferBuilder __builder(1);\
												  CFlatBufPacket<F4PACKET::packetid##_data> packetid##message(F4PACKET::PACKET_ID::packetid);\
												  auto packetbuilder= F4PACKET::Create##packetid##_dataDirect(__builder,__VA_ARGS__);\
												  __builder.Finish(packetbuilder);\
												  auto datare = __builder.GetBufferPointer();\
												  int buff_size = __builder.GetSize();\
												  packetid##message.Decode((const char*)datare, buff_size);\
												  m_pHost->SendPacket(packetid##message, userid);

#define CREATE_PACKET_FOR_STRUCT_DIRECT(packetid,message,...) flatbuffers::FlatBufferBuilder __builder(1);\
														   CFlatBufPacket<F4PACKET::packetid##_data> message(F4PACKET::PACKET_ID::packetid);\
														   F4PACKET::packetid##_data __data(__VA_ARGS__);\
														   __builder.Finish(__builder.CreateStruct(__data));\
														   message.Decode((char*)__builder.GetBufferPointer(), __builder.GetSize());

#define CREATE_PACKET_FOR_STRUCT(__builder,packetid,message,data) CFlatBufPacket<F4PACKET::packetid##_data> message(F4PACKET::PACKET_ID::packetid);\
															 F4PACKET::packetid##_data data;
															 
#define STORE_PACKET_FOR_STRUCT(__builder,__message,__data) __builder.Finish(__builder.CreateStruct(__data));\
																 __message.Decode((char*)__builder.GetBufferPointer(), __builder.GetSize());

//Argument (빌더 , 보낼 패킷 메세지, 빌더 정보,FB데이터,패킷ID)

#define CREATE_FBPACKET(__builder,packetid,message,databuilder)   CFlatBufPacket<F4PACKET::packetid##_data> message(F4PACKET::PACKET_ID::packetid);\
													F4PACKET::packetid##_dataBuilder databuilder(__builder);

#define STORE_FBPACKET(__builder,__message,__data) builder.Finish(__data.Finish());\
												  auto datare = __builder.GetBufferPointer();\
												  int buff_size = __builder.GetSize();\
												  __message.Decode((const char*)datare, buff_size);

#define CREATE_STRING(__builder,str, databuilder) auto databuilder = __builder.CreateString(str);

#define CREATE_DATA(__builder,structname, databuilder)  F4PACKET::structname##Builder databuilder(__builder);

#define END_DATA(__builder,databuilder,flatbufdata)	 auto dataoffset = databuilder.Finish();
												 
#define END_BUILDER(__builder,flatbufdata)	 __builder.Finish(flatbufdata);
												 

template <typename T>
class CFlatBufPacket : public CFlatBufPacketImplement
{
public:
	CFlatBufPacket(const F4PACKET::PACKET_ID packetID) : CFlatBufPacketImplement(packetID) {
		m_pdata = NULL; 
		m_size= 0;
	}
	
		
	virtual ~CFlatBufPacket() { if(m_pdata)free(m_pdata); }
	virtual DHOST_TYPE_BOOL Decode(const char* pBuf, DHOST_TYPE_INT32 size)
	{
		m_pdata = (char*)malloc(size);
		m_size = size;
		::memcpy(m_pdata, pBuf, size);
		m_Packet = flatbuffers::GetRoot<T>(m_pdata);

		flatbuffers::Verifier verify(reinterpret_cast<uint8_t*>(m_pdata), m_size);
		bool packet_verify_buffer = verify.VerifyBuffer<T>();

		if (packet_verify_buffer == false)
		{
			return false;
		}

		return m_Packet != NULL;
	}

	virtual DHOST_TYPE_BOOL Encode(char* pBuf, size_t size)
	{
		::memcpy(pBuf, m_pdata, size);
		m_size = size;
		return true;
	}
	virtual DHOST_TYPE_INT32 GetEncodeStreamSize()
	{
		return m_size;
	}

	char* m_pdata;
	int m_size = 0;
	const T* GetData() { return m_Packet; }
	const T* m_Packet;
private:
	
};