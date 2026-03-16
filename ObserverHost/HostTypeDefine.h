#pragma once

#include <stdint.h>
#include <string>

#include "HeaderDefine.h"


using namespace std;

//! 20200821 Host 에서 사용할 타입 정의 - by thinkingpig

typedef float DHOST_TYPE_GAME_TIME_F;			// 게임 시간 float 형
typedef float DHOST_TYPE_FLOAT;					// float 기본 자료형

typedef bool DHOST_TYPE_BOOL;					// bool 기본 자료형

typedef int32_t DHOST_TYPE_INT32;				// int 기본 자료형
typedef int32_t DHOST_TYPE_BALL_NUMBER;			// 볼 번호
typedef int32_t DHOST_TYPE_PLAYER_ID;			// 접속 플레이어 ID
typedef int32_t DHOST_TYPE_SCORE;				// 점수
typedef int32_t DHOST_TYPE_GAME_TIME_I;			// 경기 시간 int 형

typedef uint16_t DHOST_TYPE_UINT16;				// uint16 기본 자료형

typedef uint32_t DHOST_TYPE_UINT32;				// uint32 기본 자료형
typedef uint32_t DHOST_TYPE_CHARACTER_ID;		// 캐릭터ID

typedef uint32_t DHOST_TYPE_SOCKET_IDX;			// 소컷 번호

typedef uint32_t DHOST_TYPE_USER_ID;			// 접속 유저 ID
typedef uint32_t DHOST_TYPE_CHARACTER_SN;		// 방에 있는 캐릭터 고유번호(인덱스)

typedef uint64_t DHOST_TYPE_UINT64;				// uin64 기본 자료형
typedef uint64_t DHOST_TYPE_HOST_SN;			// HOST SN

typedef size_t	DHOST_TYPE_SIZE;

typedef std::string DHOST_TYPE_IP;				// 호스트 IP
typedef std::string DHOST_TYPE_HOST_ID;			// HOST ID

typedef std::string DHOST_TYPE_STR;				// 스트링 자료형

typedef uint32_t DHOST_TYPE_STAGE_ID;			// 스테이지 아이디
