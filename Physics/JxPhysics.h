#pragma once

////////////////////////////////////////////////////////////////////////
// 기본 물리 변수 
////////////////////////////////////////////////////////////////////////
#define BALL_RADIUS			0.12f // 볼반지름
#define FIXEDUPDATETIME		0.020f
#define JXGRAVITY	        9.8f
#define AIRDRAG	            0.1f
#define GROUNDFRICTION      0.9f
#define GROUNDBOUNCE        0.7f
#define NETBOUNCE           0.3f
#define POSTBOUNCE          0.5f
#define INPLANEBOUNCE       0.6f
#define BOUNCEFACTOR		0.15f
#define CURVEVALUE			0.0f
#define TORUSBOUNCENESS		0.6f
#define FIPLANEBOUNCENESS	0.6f
#define CYLINDERBOUNCENESS  0.5f


#define RIGHT_SIDE_GOAL_IN			1
#define RIGHT_SIDE_GOAL_LINE_OUT	2
#define LEFT_SIDE_GOAL_IN			3
#define LEFT_SIDE_GOAL_LINE_OUT		4
#define UP_SIDE_LINE_OUT			5
#define BOTTOM_SIDE_LINE_OUT		6
#define GOALPOST_HIT				10


#define SIMULBALLMAX				 300