#pragma once
/*
* GLogic 과 GPS2의 콜백 인터페이스를 정의한다 
*
*/


class ICallBackJPS
{
public:
	ICallBackJPS() {};
	~ICallBackJPS() {};



public:
	// 외부에서 일정한 간격으로 계속 호출시 
	virtual bool OnUpdate( void* pvPos , void* pqRotate ) = 0;

	// 외부에서 이벤트 발생시 호출시  
	virtual void OnCollisionEvent( void* pParam1 = 0 , void* pParam2 = 0, void* pParam3 = 0,  const char* ObjectName = 0 ){}
};



// 볼 이벤트를 처리하기 위한 콜백 함수 
//typedef int  CALLBACK  EventCallBackFromBall( void *pParam );
//extern EventCallBackFromBall* gEventCallBackFromGPS2Ball;

