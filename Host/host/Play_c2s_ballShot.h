#pragma once
#include "Host.h"

class CPlay_c2s_ballShot
{

public:
    static bool     CheckNullPtr(CHost* pHost, CFlatBufPacket<play_c2s_ballShot_data>* pPacket, DHOST_TYPE_USER_ID userid);
    static float    AddMinigameCustomInfo(CHost* pHost);
};

