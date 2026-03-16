#pragma once
#pragma warning(disable: 4251)

#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>

#include "HostTypeDefine.h"
#include "HostEnum.h"
#include "HostStruct.h"
#include "HostCommonData.h"
#include "Util.h"

using std::sqrt;
using std::acos;

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define TEXT_LOG_FILE
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define RANDOM_CHECK_LOG
#endif

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#define LOCAL_HOST_TEST
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

inline double SafeLog(double x, double b) {
    if (x <= 0.0 || b <= 0.0 || b == 1.0)
        throw std::domain_error("Invalid input: log(x, b) requires x > 0, b > 0, b != 1");
    return std::log(x) / std::log(b);
}

#define LOG(x, b) (log(x)/log(b))
#define VECTOR3_DISTANCE(a, b) ( sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()) + (a.z() - b.z()) * (a.z() - b.z())) )
#define VECTOR3_MAGNITUDE(a) ( sqrt(a.x() * a.x() + a.y() * a.y() + a.z() * a.z()) )
#define VECTOR3_SCALE(a, b) { a.set_x(a.x() / b); a.set_y(a.y() / b); a.set_z(a.z() / b); }
#define VECTOR3_DOT(a, b) ( a.x() * b.x() + a.y() * b.y() + a.z() * b.z() )
#define VECTOR3_CROSS(a, b, c) { a.set_x(b.y() * c.z() - b.z() * c.y()); b.set_y(b.z() * c.x() - b.x() * c.z()); c.set_z(b.x() * c.y() - b.y() * c.x()); }
#define RAD2DEG 57.29578F

#define RANDOM_FLOAT(a) (((float)rand() / RAND_MAX) * a)
#define RANDOM_INT(a) (rand() % a)

//! 20220829 KeyInput 정의 - by thinkingpig
const uint64_t kDIRECTION		= 0x000000F0;
const uint64_t kDIRECTION_UP	= 0x00000010;
const uint64_t kDIRECTION_LEFT	= 0x00000020;
const uint64_t kDIRECTION_RIGHT = 0x00000040;
const uint64_t kDIRECTION_DOWN	= 0x00000080;
const uint64_t kDIRECTION_UP_LEFT = kDIRECTION_UP | kDIRECTION_LEFT;
const uint64_t kDIRECTION_UP_RIGHT = kDIRECTION_UP | kDIRECTION_RIGHT;
const uint64_t kDIRECTION_DOWN_LEFT = kDIRECTION_DOWN | kDIRECTION_LEFT;
const uint64_t kDIRECTION_DOWN_RIGHT = kDIRECTION_DOWN | kDIRECTION_RIGHT;
//! end

//! 난수 생성 관련
static const DHOST_TYPE_UINT32 kUINT32_RANDOM_MIN = 1;
static const DHOST_TYPE_UINT32 kUINT32_RANDOM_MAX = 100;
static const DHOST_TYPE_FLOAT kFLOAT_RANDOM = 0.01f;
//!

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
static const DHOST_TYPE_GAME_TIME_F kSEND_GAME_TIME_INTERVAL	= 1.0f;
static const DHOST_TYPE_GAME_TIME_F kSEND_LAST_GAME_TIME		= -1.0f;
static const DHOST_TYPE_GAME_TIME_F kSHOT_CLOCK_TIME_INIT		= 14.f;
static const DHOST_TYPE_GAME_TIME_I kGAME_TIME_I_INIT			= 180;
static const DHOST_TYPE_GAME_TIME_F kLAST_USER_LOAD_WAIT_TIME	= 60.0f;
static const DHOST_TYPE_GAME_TIME_F kTIME_AFTER_HOST_CREATION	= 60.0f;	// 방 생성후 경과시간
static const DHOST_TYPE_GAME_TIME_F kTIME_AFTER_USER_CREATION	= 60.0f;	// 유저 생성후 경과시간
static const DHOST_TYPE_GAME_TIME_F kTIME_AFTER_USER_LOADING    = 180.0f;	// 로딩 후 경과시간
static const DHOST_TYPE_GAME_TIME_F kLAST_PACKET_RECEIVE_TIME	= 180.0f;
static const DHOST_TYPE_GAME_TIME_F kLAST_USER_LOAD_WAIT_TIME_STEP_TWO	= 180.0f;
static const DHOST_TYPE_GAME_TIME_F kMVP_SCENE_WAIT_TIME		= 9.0f;
static const DHOST_TYPE_GAME_TIME_F kSKILL_CHALLENGE_END_SCENE_WAIT_TIME = 3.0f;
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
static const DHOST_TYPE_USER_ID kAI_DEFAULT_LEVEL				= 3;
static const DHOST_TYPE_STAGE_ID kSTAGE_INIT					= 1000;
static const DHOST_TYPE_STAGE_ID kSTAGE_TUTORIAL				= 1; // 목욕탕 맵
static const DHOST_TYPE_GAME_TIME_F kREPLAY_ACTION_CHECK_TIME	= 2.5f;
static const DHOST_TYPE_FLOAT kPING_LATENCY_DEFAULT				= 1.0f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_TIME				= 5.0f;
static const DHOST_TYPE_FLOAT kPING_EXPIRED_TIME_TEST			= 30.0f;

// 얼라이브 패킷 관련해서 수정 , 현재는 타이트 하다, steven 
static const DHOST_TYPE_FLOAT kPING_EXPIRED_AVG_TIME			= 3.0f; //  
static const DHOST_TYPE_FLOAT kALIVE_CHECK_TIME_INIT            = 1.0f; // 
static const DHOST_TYPE_UINT64 kMAX_PING_LATENCY_DEQUE          = 7;    // 

static const DHOST_TYPE_FLOAT kPING_EXPIRED_AVG_TIME_TEST		= 15.0f;
static const DHOST_TYPE_UINT64 kMAX_PING_MAP_SIZE				= 5;
static const DHOST_TYPE_FLOAT kTYPE_CAST_TO_FLOAT				= 1.0f;
static const DHOST_TYPE_FLOAT kBACK_BOARD_DURABILITY			= 100.0f;
static const DHOST_TYPE_FLOAT kPING_LATENCY_100					= 0.1f;
static const DHOST_TYPE_FLOAT kPING_LATENCY_200					= 0.2f;
static const DHOST_TYPE_FLOAT kBUZZER_BEATER_TIME				= 1.0f;
static const DHOST_TYPE_FLOAT kBUZZER_BEATER_DISTANCE			= 3.0f;
static const DHOST_TYPE_FLOAT kCLIENT_REBOUND_BALL_IN_TIME		= 0.2f;	// 클라에 정의된 리바운드 볼인타이밍의 공의 위치
static const DHOST_TYPE_FLOAT kCLIENT_BLOCK_EVENT_TIME			= 0.4f;
static const DHOST_TYPE_FLOAT kCLIENT_SNATCH_BLOCK_EVENT_TIME	= 0.35f;
static const DHOST_TYPE_FLOAT kCLIENT_DOUBLE_HANDS_UP_BLOCK_EVENT_TIME	= 0.34f;
//! 스피드핵 체크
static const DHOST_TYPE_FLOAT kHACK_CHECK_CLIENT_ELAPSED		= 0.3f;
static const DHOST_TYPE_FLOAT kHACK_CHECK_PING_INTERVAL			= 0.96f;
static const DHOST_TYPE_INT32 kHACK_CHECK_PING_DATA_TIME		= 960;
static const DHOST_TYPE_FLOAT kHACK_CHECK_CLIENT_ELAPSED_DIFF	= 1.01f;
static const DHOST_TYPE_FLOAT kHACK_CHECK_RESTRICTION_STEP_ONE	= 0.1f;
static const DHOST_TYPE_FLOAT kHACK_CHECK_RESTRICTION_STEP_TWO	= 0.3f;
static const DHOST_TYPE_FLOAT kSPEED_HACK_DETECT_VALUE			= 0.005f;
static const DHOST_TYPE_UINT32 kKICK_SWTICH_TO_AI_LEVEL			= 3;
static const DHOST_TYPE_UINT32 kHACK_CHECK_LEVEL_ONE			= 1;
static const DHOST_TYPE_UINT32 kHACK_CHECK_LEVEL_TWO			= 2;
static const DHOST_TYPE_UINT32 kHACK_CHECK_LEVEL_THREE			= 3;

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
static const DHOST_TYPE_FLOAT kHOST_TIME_MINUTE_100				= 6000.0f;
//! Scene
static const DHOST_TYPE_INT32 kDEFAULT_SCENE_INDEX				= 1000;
static const DHOST_TYPE_INT32 HOST_OPTION_SCENE_RANDOM_COUNT	= 2;	// 1002씬은 아직 개발중이라 일단 두개가 맥스임
//! 기권
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_INIT		= -1.0f;				// 초기값
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_EXPIRE		= 0.0f;					// 투표실패나 투표시간 만료됐을 때
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_ACTIVATION_GAME_TIME = 90.0f;		// 게임 시작후 경과시간
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_ACTIVATION_ELAPSED_TIME = 120.0f;	// 투표 활성화 경과시간
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_GAME_END_DELAY_TIME = 3.0f;			// 기권하기 동의후 게임 종료 딜레이 시간
static const DHOST_TYPE_FLOAT kTEAM_SURRENDER_CHECK_INACTIVATION_GAME_TIME = 30.0f;		// 게임 남은시간
static const DHOST_TYPE_UINT32 kTEAM_SURRENDER_CHECK_INACTIVATION_SCORE_GAP = 5;		// 비활성화 점수차이
static const DHOST_TYPE_UINT32 kTEAM_SURRENDER_CHECK_ACTIVATION_SCORE_GAP = 12;			// 활성화 점수차이

static const DHOST_TYPE_FLOAT kCHARACTER_NOT_SAME_POSITION = 1.5f;				// 캐릭터의 위치 오차값
static const DHOST_TYPE_FLOAT kCHARACTER_POSITION_CORRECT = 0.9f;				// 캐릭터의 위치 오차 수정, 서버에 알리는 기준 변수 

static const DHOST_TYPE_UINT32 kSKILL_INDEX_DIVIDE = 10;						// 스킬인덱스 나누는값
static const DHOST_TYPE_UINT32 kSKILL_INDEX_DIVIDE_REMAINDER = 1;				// 스킬인덱스 나눈 나머지값(상위호환 스킬인지 확인용)

static const DHOST_TYPE_FLOAT kBURST_BUFFER_DISTANCE = 0.1f;				// 버스트로 인해 능력치 증감이 생길 때 눈감아주는 거리 (0.1m)
static const DHOST_TYPE_FLOAT kBURST_BUFFER_HEIGHT = 0.3f;				// 버스트로 인해 능력치 증감이 생길 때 눈감아주는 높이 (0.3m)
static const DHOST_TYPE_FLOAT kCALC_CLIENT_SERVER_GAP = 0.01f;				// 클라가 계산한값과 서버가 계산한값의 오차범위

// 호스트 생성시 옵션
const uint64_t HOST_OPTION_NO_CHECK_GOAL						= 0x00000001;
const uint64_t HOST_OPTION_NO_CHECK_TIME						= 0x00000002;
const uint64_t HOST_OPTION_NO_CHECK_SHOTCLOCK					= 0x00000004;
const uint64_t HOST_OPTION_NO_AUTO_AI							= 0x00000008;
const uint64_t HOST_OPTION_NO_REDIS								= 0x00000010;
const uint64_t HOST_OPTION_NO_CHECK_REPLAY					    = 0x00000020;
const uint64_t HOST_OPTION_CHOOSE_NORMAL_BETWEEN_NORMAL_AND_RANK= 0x00000040;
const uint64_t HOST_OPTION_LOG_ALIVE_CHECK						= 0x00000080;
const uint64_t HOST_FUNC_SHOTFAIL								= 0x00000100;
const uint64_t HOST_OPTION_LOG_SHOT_CHECK						= 0x00000200;
const uint64_t HOST_OPTION_NO_KICK								= 0x00000400;
const uint64_t HOST_OPTION_PREVENT_AFK							= 0x00000800;

const uint64_t HOST_OPTION_ONLY_SHOT_SOLUTION                   = 0x00004000; // 아래에 입장씬이  0x00001000 이 있으니 이렇게 함  

const uint64_t HOST_OPTION_GAMEMODE								= 0x0FF00000;
const uint64_t HOST_OPTION_GAMEMODE_NORMAL						= 0x00000000;
const uint64_t HOST_OPTION_GAMEMODE_TRAINING					= 0x00100000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE1					= 0x00200000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE2					= 0x00300000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE3					= 0x00400000;
const uint64_t HOST_OPTION_GAMEMODE_CHALLENGE4					= 0x00500000;
const uint64_t HOST_OPTION_GAMEMODE_TRIO						= 0x00600000;
const uint64_t HOST_OPTION_GAMEMODE_CONTINUOUS					= 0x00700000;
const uint64_t HOST_OPTION_GAMEMODE_CUSTOM						= 0x00800000;
const uint64_t HOST_OPTION_GAMEMODE_TRIO_PVP_MIXED				= 0x00900000;

const uint64_t HOST_OPTION_GAMEMODE_TRAINING_REBOUND			= 0x00a00000;
const uint64_t HOST_OPTION_GAMEMODE_TRAINING_JUMPSHOTBLOCK		= 0x00b00000;
const uint64_t HOST_OPTION_GAMEMODE_TRAINING_RIMATTACKBLOCK		= 0x00c00000;
const uint64_t HOST_OPTION_GAMEMODE_TRAINING_OFFBALLMOVE		= 0x00d00000;

const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_BASIC				= 0x01000000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_PASS				= 0x01100000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_STEAL				= 0x01200000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_DIVINGCATCH		= 0x01300000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOT			= 0x01400000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACK			= 0x01500000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_JUMPSHOTBLOCK		= 0x01600000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND			= 0x01700000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_RIMATTACKBLOCK		= 0x01800000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_BOXOUT				= 0x01900000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_PENETRATE			= 0x01a00000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_SHOOTINGDISTURB	= 0x01b00000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_REBOUND_BEGINNER   = 0x01c00000;
const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_BLOCK_BEGINNER     = 0x01d00000;

const uint64_t HOST_OPTION_GAMEMODE_TUTORIAL_MINIGAME_CUSTOM    = 0x01e00000;

const uint64_t HOST_OPTION_STAGE								= 0x000F0000;
const uint64_t HOST_OPTION_STAGE_RESET							= 0xFFF0FFFF;
const uint64_t HOST_OPTION_STAGE_RANDOM							= 0x00000000;
const uint64_t HOST_OPTION_STAGE_0								= 0x00010000;
const uint64_t HOST_OPTION_STAGE_1								= 0x00020000;
const uint64_t HOST_OPTION_STAGE_2								= 0x00030000;
const uint64_t HOST_OPTION_STAGE_3								= 0x00040000;
const uint64_t HOST_OPTION_STAGE_4								= 0x00050000;
const uint64_t HOST_OPTION_STAGE_5								= 0x00060000;
const uint64_t HOST_OPTION_STAGE_6								= 0x00070000;
const uint64_t HOST_OPTION_STAGE_100							= 0x00080000;

const uint64_t HOST_OPTION_SCENE								= 0x0000F000;
const uint64_t HOST_OPTION_SCENE_RESET							= 0xFFFF0FFF;
const uint64_t HOST_OPTION_SCENE_RANDOM							= 0x00000000;

const uint64_t HOST_OPTION_SCENE_1000							= 0x00001000;
const uint64_t HOST_OPTION_SCENE_1001							= 0x00002000;
const uint64_t HOST_OPTION_SCENE_1002							= 0x00003000;

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