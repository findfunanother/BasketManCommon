#include <stdlib.h>
#include "enet/types.h"

#ifndef __ENET_ENCRYPT_H__
#define __ENET_ENCRYPT_H__

typedef struct
{
    enet_uint8 cipher;
    int remain;
} ENetEncryptContext;

extern void enet_set_cipher(ENetEncryptContext * context, enet_uint8 cipher);
extern size_t enet_encrypt(ENetEncryptContext * context, enet_uint8 * data, size_t dataLength);
extern size_t enet_decrypt(ENetEncryptContext * context, enet_uint8 * data, size_t dataLength);

#endif

