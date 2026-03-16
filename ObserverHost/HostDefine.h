#pragma once
#pragma warning(disable: 4251)

#include <stdlib.h>
#include <iostream>
#include <string>

#include "HostTypeDefine.h"
#include "HostEnum.h"
#include "HostStruct.h"
#include "HostCommonData.h"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define TEXT_LOG_FILE
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define DEBUG_STRING
#endif

// 패킷을 바이너리 형태로 저장하는 작업
#define BINARY_SAVE

#define kPACKET_TYPE_ALL_WORK 1				// work server, broadcast to all client
#define kPACKET_TYPE_ALL_WORK_REMOTE 2		// work server, broadcast to remote client
#define kPACKET_TYPE_ONLY_CLIENT 3			// just broadcast client
#define kPACKET_TYPE_ONLY_CLIENT_REMOTE 4	// just broadcast remote client
#define kPACKET_TYPE_ONLY_SERVER 5			// just work server

#define kLOG_MESSAGE_SIZE 256
#define kPACKET_HEADER_SIZE 6
#define kPACKET_HEADER_VALUE_MAX 1 << 16
#define kPACKET_HEADER_BUFFER_SIZE 256

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(x) { delete x; x = nullptr; }
#endif

#ifndef SAFE_DELETES
#define SAFE_DELETES(x) if(x) { delete[] x; x = nullptr; }
#endif

#ifndef INIT_MEMORY
#define INIT_MEMORY(p,len) memset(p, 0x00, len)
#endif

#define LOG(x, b) (log(x)/log(b))
#define VECTOR3_DISTANCE(a, b) ( sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()) + (a.z() - b.z()) * (a.z() - b.z())) )
#define VECTOR3_MAGNITUDE(a) ( sqrt(a.x() * a.x() + a.y() * a.y() + a.z() * a.z()) )
#define VECTOR3_SCALE(a, b) { a.set_x(a.x() / b); a.set_y(a.y() / b); a.set_z(a.z() / b); }
#define VECTOR3_DOT(a, b) ( a.x() * b.x() + a.y() * b.y() + a.z() * b.z() )
#define VECTOR3_CROSS(a, b, c) { a.set_x(b.y() * c.z() - b.z() * c.y()); b.set_y(b.z() * c.x() - b.x() * c.z()); c.set_z(b.x() * c.y() - b.y() * c.x()); }
#define RAD2DEG 57.29578F

#define RANDOM_FLOAT(a) (((float)rand() / RAND_MAX) * a)
#define RANDOM_INT(a) (rand() % a)

//! 20200821 Host 상수 정의 - by thinkingpig
static const DHOST_TYPE_USER_ID kUSER_ID_INIT					= 0;
static const DHOST_TYPE_CHARACTER_ID kCHARACTER_ID_INIT			= 0;
static const DHOST_TYPE_HOST_SN kHOST_SN_INIT					= 0;
static const DHOST_TYPE_BALL_NUMBER kBALL_NUMBER_INIT			= 0;
static const DHOST_TYPE_SCORE kSCORE_INIT						= 0;
static const DHOST_TYPE_PLAYER_ID kHOST_PLAYER_ID				= 101;
static const DHOST_TYPE_GAME_TIME_F kTIME_F_INIT				= 0.0f;
static const DHOST_TYPE_GAME_TIME_F kGAME_TIME_INIT				= 240.f;
//static const DHOST_TYPE_GAME_TIME_F kGAME_TIME_INIT = 13.f;
static const DHOST_TYPE_GAME_TIME_F kSHOT_CLOCK_TIME_INIT		= 14.f;
static const DHOST_TYPE_GAME_TIME_I kGAME_TIME_I_INIT			= 180;
static const DHOST_TYPE_GAME_TIME_F KLAST_USER_LOAD_WAIT_TIME	= 60.0f;
static const DHOST_TYPE_UINT16 kUINT16_INIT						= 0;
static const DHOST_TYPE_UINT32 kUINT32_INIT						= 0;
static const DHOST_TYPE_UINT64 kUINT64_INIT						= 0;
static const DHOST_TYPE_FLOAT kFLOAT_INIT						= 0.0f;
static const DHOST_TYPE_INT32 kINT32_INIT						= 0;
static const DHOST_TYPE_CHARACTER_SN kCHARACTER_SN_INIT			= 0;
static const DHOST_TYPE_CHARACTER_SN kLAST_CHARACTER_SN			= 1001;
static const DHOST_TYPE_UINT16 kMAX_BALL_DEQUE					= 5;
static const size_t gPacketBufferSize							= kPACKET_HEADER_VALUE_MAX;  //	65536
static const DHOST_TYPE_USER_ID kAI_USER_ID_MIN					= 1000;
static const DHOST_TYPE_USER_ID kAI_USER_ID_MAX					= 10000;
static const DHOST_TYPE_USER_ID kIS_NOT_AI						= 0;
static const DHOST_TYPE_STAGE_ID kSTAGE_INIT					= 1000;
static const DHOST_TYPE_STAGE_ID kSTAGE_TUTORIAL				= 0; // 목욕탕 맵
static const DHOST_TYPE_GAME_TIME_F kREPLAY_ACTION_CHECK_TIME	= 2.5f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_TIME				= 5.0f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_TIME_TEST			= 30.0f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_AVG_TIME			= 3.0f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_AVG_TIME_TEST		= 15.0f;
static const DHOST_TYPE_UINT64 kMAX_PING_LATENCY_DEQUE			= 5;
static const DHOST_TYPE_FLOAT kTYPE_CAST_TO_FLOAT				= 1.0f;
static const DHOST_TYPE_FLOAT kBACK_BOARD_DURABILITY			= 100.0f;
//! 게임시간 정의
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_1				= 60.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_2				= 120.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_3				= 180.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_4				= 240.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_5				= 300.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_6				= 360.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_7				= 420.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_8				= 480.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_9				= 540.0f;
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_10				= 600.0f;


// 호스트 생성시 옵션
const uint64_t HOST_OPTION_NO_CHECK_GOAL						= 0x00000001;
const uint64_t HOST_OPTION_NO_CHECK_TIME						= 0x00000002;
const uint64_t HOST_OPTION_NO_CHECK_SHOTCLOCK					= 0x00000004;
const uint64_t HOST_OPTION_NO_AUTO_AI							= 0x00000008;
const uint64_t HOST_OPTION_NO_REDIS								= 0x00000010;
const uint64_t HOST_OPTION_NO_CHECK_REPLAY						= 0x00000020;

const uint64_t HOST_OPTION_GAMEMODE								= 0x0F000000;
const uint64_t HOST_OPTION_GAMEMODE_NORMAL						= 0x00000000;
const uint64_t HOST_OPTION_GAMEMODE_TRAINING					= 0x01000000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE1					= 0x02000000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE2					= 0x03000000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE3					= 0x04000000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE4					= 0x05000000;
const uint64_t HOST_OPTION_GAMEMODE_TRIO						= 0x06000000;
const uint64_t HOST_OPTION_GAMEMODE_CONTINUOUS					= 0x07000000;

const uint64_t HOST_OPTION_STAGE								= 0x00F00000;
const uint64_t HOST_OPTION_STAGE_RESET							= 0xFF0FFFFF;
const uint64_t HOST_OPTION_STAGE_RANDOM							= 0x00000000;
const uint64_t HOST_OPTION_STAGE_0								= 0x00100000;
const uint64_t HOST_OPTION_STAGE_1								= 0x00200000;
const uint64_t HOST_OPTION_STAGE_2								= 0x00300000;
const uint64_t HOST_OPTION_STAGE_3								= 0x00400000;

const uint64_t HOST_FUNC_SHOTFAIL								= 0x00000100;

const uint64_t HOST_TIME										= 0xF0000000;
const uint64_t HOST_TIME_RESET									= 0x0FFFFFFF;
const uint64_t HOST_TIME_MINUTE_DEFAULT							= 0x00000000;
const uint64_t HOST_TIME_MINUTE_1								= 0x10000000;
const uint64_t HOST_TIME_MINUTE_2								= 0x20000000;
const uint64_t HOST_TIME_MINUTE_3								= 0x30000000;
const uint64_t HOST_TIME_MINUTE_4								= 0x40000000;
const uint64_t HOST_TIME_MINUTE_5								= 0x50000000;
const uint64_t HOST_TIME_MINUTE_6								= 0x60000000;
const uint64_t HOST_TIME_MINUTE_7								= 0x70000000;
const uint64_t HOST_TIME_MINUTE_8								= 0x80000000;
const uint64_t HOST_TIME_MINUTE_9								= 0x90000000;
const uint64_t HOST_TIME_MINUTE_10								= 0xa0000000;
//! end

const uint64_t BURST_ACTION_TYPE								= 0x00000000;
const uint64_t BURST_ACTION_TYPE_SHOT							= 0x00001000;
const uint64_t BURST_ACTION_TYPE_SHOT_TWO						= 0x00001001;	// 2점 득점
const uint64_t BURST_ACTION_TYPE_SHOT_TWO_TIE					= 0x00001002;	// 2점 득점을하여 동점
const uint64_t BURST_ACTION_TYPE_SHOT_REVERSAL					= 0x00001004;	// 2점 득점을하여 역전
const uint64_t BURST_ACTION_TYPE_SHOT_FAIL						= 0x00001008;	// 2점 득점 실패
const uint64_t BURST_ACTION_TYPE_SHOT_DUNK						= 0x00001010;	// 덩크로 득점
const uint64_t BURST_ACTION_TYPE_SHOT_THREE						= 0x00001020;	// 3점 득점
const uint64_t BURST_ACTION_TYPE_SHOT_THREE_TIE					= 0x00001040;	// 3점 득점을하여 동점
const uint64_t BURST_ACTION_TYPE_SHOT_THREE_REVERSAL			= 0x00001080;	// 3점 득점을하여 역전
const uint64_t BURST_ACTION_TYPE_SHOT_THREE_FAIL				= 0x00001100;	// 3점 득점 실패
const uint64_t BURST_ACTION_TYPE_SHOT_FGM						= 0x00002000;	// 야투 성공
const uint64_t BURST_ACTION_TYPE_SHOT_FGM_FAIL					= 0x00004000;	// 야투 실패
const uint64_t BURST_ACTION_TYPE_REBOUND						= 0x00008000;	// 리바운드
const uint64_t BURST_ACTION_TYPE_STEAL							= 0x00010000;	// 스틸
const uint64_t BURST_ACTION_TYPE_BLOCK							= 0x00020000;	// 블락 성공
const uint64_t BURST_ACTION_TYPE_BLOCKED						= 0x00040000;	// 블락 당함
const uint64_t BURST_ACTION_TYPE_LOOSE_BALL						= 0x00080000;	// 루즈볼 획득
const uint64_t BURST_ACTION_TYPE_TURN_OVER						= 0x00100000;	// 턴오버 발생시킴
const uint64_t BURST_ACTION_TYPE_ANKLE_BREAK					= 0x00200000;	// 앵클 브레이크 성공시킴
const uint64_t BURST_ACTION_TYPE_ANKLE_BREAKED					= 0x00400000;	// 앵클 브레이크 당함
const uint64_t BURST_ACTION_TYPE_ASSIST							= 0x00800000;	// 어시스트 기록