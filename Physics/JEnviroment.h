#pragma once
#include "JOMath3d.h"

////////////////////////////////////////////////////////////////////////
// 축구 인게임 환경 변수 
////////////////////////////////////////////////////////////////////////

/* 축구장 */
/*                    (Z+)
		 ─────────────────────────────────────────────              ---
(X-)	│	                   |                      │(X +)(Right)  ↑
(Left)	│-----                 |                 -----│              |
		│     |                |                |     │				 | FIELD_DEPTH
		│	  |				   |                |     │              |
		│-----                 |                 -----│              |
		│                      |                      │              ↓
		 ─────────────────────────────────────────────              ---
					  (Z-)
		|<------------- FIELD_WIDTH ------------------->
*/

/*** BASIC INFO ***/

#define PHYSICS_EPSILON         0.001f

// Court 1
#define FIELD_WIDTH              76.0f              // 105
#define FIELD_DEPTH              48.0f              // 68
#define FIELD_WIDTH_HALF         38.0f
#define FIELD_DEPTH_HALF         24.0f

//
//// Court 2
//#define FIELD_WIDTH              88.0f              
//#define FIELD_DEPTH              54.0f              
//#define FIELD_WIDTH_HALF         44.0f
//#define FIELD_DEPTH_HALF         27.0f

/*
// Court 3
#define FIELD_WIDTH              84.0f
#define FIELD_DEPTH              52.0f
#define FIELD_WIDTH_HALF         42.0f
#define FIELD_DEPTH_HALF         26.0f
*/

// Post
#define POST_RAIDUS				 0.06f
#define POST_HEIGHT              2.44f
#define POST_WIDTH               7.32f
#define POST_HEIGHT_HALF         1.22f
#define POST_WIDTH_HALF          3.66f


/* Right Side */
#define RS_POST_LEFT()			JOVECTOR3(FIELD_WIDTH_HALF, POST_HEIGHT_HALF, -POST_WIDTH_HALF)
#define RS_POST_LEFT_NORMAL()   JOVECTOR3(0.0f, 1.0f, 0.0f)

#define RS_POST_RIGHT()			JOVECTOR3(FIELD_WIDTH_HALF, POST_HEIGHT_HALF, POST_WIDTH_HALF)
#define RS_POST_RIGHT_NORMAL()  JOVECTOR3(0.0f, 1.0f, 0.0f)

#define RS_POST_TOP()           JOVECTOR3(FIELD_WIDTH_HALF, POST_HEIGHT, 0.0f)
#define RS_POST_TOP_NORMAL()    JOVECTOR3(0.0f, 0.0f, -1.0f)

/* Left Side */
#define LS_POST_LEFT()			JOVECTOR3(-FIELD_WIDTH_HALF, POST_HEIGHT_HALF, -POST_WIDTH_HALF)
#define LS_POST_LEFT_NORMAL()   JOVECTOR3(0.0f, 1.0f, 0.0f)

#define LS_POST_RIGHT()			JOVECTOR3(-FIELD_WIDTH_HALF, POST_HEIGHT_HALF, POST_WIDTH_HALF)
#define LS_POST_RIGHT_NORMAL()  JOVECTOR3(0.0f, 1.0f, 0.0f)

#define LS_POST_TOP()           JOVECTOR3(-FIELD_WIDTH_HALF, POST_HEIGHT, 0.0f)
#define LS_POST_TOP_NORMAL()    JOVECTOR3(0.0f, 0.0f, 1.0f)




////////////////////////////////////////////////////////////////////////
// 농구 환경 변수 
////////////////////////////////////////////////////////////////////////

/* 농구장 */
/*                    (X-)    corner4                corner3
		 ─────────────────────────────────────────────              ---
(Z-)	│	                   |              (1)--(2)│(Z+)(Right)  ↑
(Left)	│                      |               /      │              |
		│                      |              |       │				 | FIELD_DEPTH
		│	   				   |              |       │              |
		│                      |               \      │              |
		│                      |              (3)--(4)│              ↓
		 ─────────────────────────────────────────────              ---
					  (X+)    corner1                corner2
		|<------------- FIELD_WIDTH ------------------->
*/
#define PASS_GRAVITY 5.0f
#define SHOOT_GRVITY 9.8f
#define BALL_RELEASE_POSITION_Y 2.5f
#define GAP 2.0f

#define COURT_WIDTH() (28.6512f - GAP * 2.0f)	// 24.6512
#define COURT_WIDTH_HALF() (COURT_WIDTH() * 0.5f)	// 12.3256
#define COURT_DEPTH 15.24f
#define COURT_DEPTH_HALF 7.62f
#define POINT3_LENGTH 7.239f
#define POINT3_LENGTH_SIDE 6.7056f

#define RIM_BORDER_Z 1.6002f
#define RIM_POS_X 0.0f
#define RIM_POS_Y 3.050f
#define RIM_POS_Z_FIXED 10.7254f
#define RIM_POS_Z() (COURT_WIDTH_HALF() - RIM_BORDER_Z)	// 12.3256 - 1.6002 = 10.7254

#define COLLISION_RADIUS 0.7f
#define DEG2RAD() (PI / 180.0f)
#define COURT_X() (15.24f * 0.5f)
#define COURT_Z() ((28.6512f - 4.0f) * 0.5f)
#define RIM_POSITION_Z 10.7254f
#define CHARACTER_X() (0.3f)
#define CHARACTER_Z_MAX() (0.5f)
#define CHARACTER_Z_MIN() (0.4f)

#define RIM_POS_NEGATIVE_Z -RIM_POS_Z
#define RIM_HALF_DISTANCE 0.225f
#define RIM_HALF_CIRCLE 0.02f
#define RIM_EXTEND 0.15f
#define POINT3_CHECK_Z() (RIM_POS_Z() - sqrt(POINT3_LENGTH * POINT3_LENGTH - POINT3_LENGTH_SIDE * POINT3_LENGTH_SIDE))	// 10.7254 - (52.403121 - 44.96507136) = 10.7254 - 2.7272 = 7.9982
#define BOARD_HALF_WIDTH 0.05f // 두깨 

//#define BOARD_HALF_WIDTH = 0.05f; // 두깨 
  
#define BOARD_HALF_DEPTH_BEFORE 0.650f //0.90f;
#define BOARD_HALF_HEIGHT_BEFORE 0.350f// 0.5025f; fixed by stevn 20210817
 
#define BOARD_HALF_DEPTH 0.90f
#define BOARD_HALF_HEIGHT 0.5025f

//#define BOARD_HALF_DEPTH 0.650f
//#define BOARD_HALF_HEIGHT 0.350f
#define BOARD_EXTEND 0.15f

#define FREE_THROW_LINE_X 2.0f
#define FREE_THROW_LINE_Z 8.0f

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 원래 있던 소스
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define RIM_RADIUS 45.0f // 림지름
#define RIM_THICK  2.0f  // 림두께 
#define GAP_BOARD  15.0f // 백보드와이 갭 
#define BOARD_WIDTH		180.0f   // cm , 백보드 가로 넓이 
#define BOARD_HEIGHT	105.0f   // cm , 백보드 세로 높이
#define BOARD_THICK     10.0f    // cm , 백보드 두께.
#define UNDER_RIM		15.0f    // cm , 림 밑으로 나온 사이즈 
#define BOARD_X_POS( rimpos )		( rimpos.fX + RIM_RADIUS*0.5f + GAP_BOARD + 5.f )
#define BOARD_Y_POS( rimpos )       ( rimpos.fY + BOARD_HEIGHT*0.5f - UNDER_RIM ) 

#define RIM_POS()			JOVECTOR3(1040.0f, 305.0f, 0.0f)    /// 현재 골을 넣어야 할 림의 위치 
#define DUNK_POS()			JOVECTOR3(1040.0f, 310.0f, 0.0f)    /// 덩크슛의 릴리즈 위치   sclee 추가

#define CHARACTER_NORMAL_RADIUS 40.0f
#define CHARACTER_SCREEN_RADIUS	60.0f
#define CHARACTER_FACEUP_RADIUS 60.0f


#define	CHARACTER_DEFALUT_HEIGHT	180.0f;
#define	CENTER_DEFALUT_HEIGHT		180.0f;
#define	FORWARD_DEFALUT_HEIGHT		180.0f;
#define	GUARD_DEFALUT_HEIGHT		180.0f;


/////// 충돌 영역 이름 
#define COLLISION_WALL_BACK		"Back"
#define COLLISION_WALL_LEFT		"Left"
#define COLLISION_WALL_RIGHT	"Right"
#define COLLISION_WALL_FRONT	"Front"
#define COLLISION_WALL_BOTTOM	"Bottom"
#define COLLISION_RIM			"Rim"
#define COLLISION_BACKBOARD		"Board"



const int RELAY_CERT_KEY_LENGTH = 8;