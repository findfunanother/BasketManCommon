


#ifndef CONTACTREPORT_H
#define CONTACTREPORT_H

//#include "UserData.h"
#include "JxUserContactReport.h"
#include "GPSActor.h"
#include "ICallBackGPS2.h"

class ContactReport : public CJxUserContactReport
{
public:
	virtual void onContactNotify( CJxActor* pJxActorA /* ±¸ */, CJxActor* pJxActorB, int events )
	{

		if( pJxActorA->GetKindOfActor() == KE_Sphere )
		{
			if (pJxActorA->userData)
			{
				((ICallBackJPS*)pJxActorA->userData)->OnCollisionEvent((void*)pJxActorA, (void*)pJxActorB, &events, (char*)pJxActorB->actorName.c_str());
			}
		}
	}

};

#endif  // CONTACTREPORT_H

