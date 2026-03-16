#pragma once
#include <stdint.h>
#include "FlatBufPacket.h"

namespace FlatBufProtocol
{
	enum class ERESULT
	{
		SUCCESS,
		INSUFFICIENT_BUFFER,
		INCOMPLETE_PACKET,
		NOT_INVALID_PACKEID,
		CORRUPT_PACKET,
		ENCODING_FAILURE,
		DECODING_FAILURE,
		UNDEFINED_FAILURE,
		PACKETIZING_ERROR,
		CORRUPT_PACKETID,
	};

	inline const char* ResultName(const ERESULT result)
	{
		switch (result)
		{
		case ERESULT::SUCCESS:
			return "SUCCESS";
		case ERESULT::INSUFFICIENT_BUFFER:
			return "INSUFFICIENT_BUFFER";
		case ERESULT::INCOMPLETE_PACKET:
			return "INCOMPLETE_PACKET";
		case ERESULT::NOT_INVALID_PACKEID:
			return "NOT_INVALID_PACKEID";
		case ERESULT::CORRUPT_PACKET:
			return "CORRUPT_PACKET";
		case ERESULT::ENCODING_FAILURE:
			return "ENCODING_FAILURE";
		case ERESULT::DECODING_FAILURE:
			return "DECODING_FAILURE";
		case ERESULT::UNDEFINED_FAILURE:
			return "UNDEFINED_FAILURE";
		case ERESULT::PACKETIZING_ERROR:
			return "PACKETIZING_ERROR";
		case ERESULT::CORRUPT_PACKETID:
			return "CORRUPT_PACKETID";
		default:
			break;
		}
		return "NO_DATA";
	}

	typedef struct _Header
	{
		DHOST_TYPE_UINT16 packetLen;
		DHOST_TYPE_UINT16 packetID;
		DHOST_TYPE_UINT16 packetType;
	}Header;

	typedef void* (*Allocate)(size_t size);
	typedef void (*Free)(void* memory);
	void Protocol_Start();
	void Protocol_End();

	bool GetHeadInfo(Header& header, const char* msg, size_t size);
	
	const ERESULT GetPacket(CPacketImplement<F4PACKET::PACKET_ID>*& pPacket, const char* msg, size_t size);

	const ERESULT Encode(CPacketBase* pPacket, char** ppBuffer, int& bufferSize, Allocate fncallocate, Free fncfree);
	
};

