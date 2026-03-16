
#pragma once

class CJxController;
class CJxSphere;
class CJxInPlane;

class CJxIControllerHitReport
{
public:
	CJxIControllerHitReport(){}
	~CJxIControllerHitReport(){}

public:
	virtual void	ControllerHit( CJxController* pJxControllerA, CJxController* pJxControllerB ) = 0;
	virtual void	SphereHit( CJxSphere* pJxSphere, CJxController* pJxController ) = 0;
	virtual void	BoundaryHit(CJxController* pJxController, CJxInPlane* pJxInPlane ) = 0;


};