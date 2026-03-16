#pragma once

class CJxActor;

class CJxUserContactReport
{
public:
	CJxUserContactReport(void);
	virtual ~CJxUserContactReport(void);

	virtual void onContactNotify( CJxActor* pJxActorA, CJxActor* pJxActorB, int events ){}

};
