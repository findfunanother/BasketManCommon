#pragma once
#include "Host.h"

class CPlay_c2s_common
{

public:
    template <typename T>
    static bool CheckNullPtr(CHost* host, CFlatBufPacket<T>* packet, DHOST_TYPE_USER_ID userid)
    {
        if (packet == nullptr)
        {
            std::string log = "[PACKET_ERROR] pPacket is null RoomElapsedTime : "
                + std::to_string(host->GetRoomElapsedTime())
                + ", UserID : " + std::to_string(userid);
            host->ToLog(log.c_str());
            return false;
        }

        auto* data = packet->GetData();
        if (data == nullptr)
        {
            std::string log = "[PACKET_ERROR] data is null RoomElapsedTime : "
                + std::to_string(host->GetRoomElapsedTime())
                + ", UserID : " + std::to_string(userid)
                + ", PacketID : " + F4PACKET::EnumNamePACKET_ID(
                    static_cast<PACKET_ID>(packet->GetPacketID()));
            host->ToLog(log.c_str());
            return false;
        }

        return true;
    }

};

