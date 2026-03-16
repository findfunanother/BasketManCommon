#include<cmath>
//#include "stdafx.h"
#include "JxPhysics.h"
#include "JPhysics.h"
#include "JOMath3d.h"
#include "LogActor.h"

//extern "C"
//{
	/*
	public static Vector3 GetInitialVelocity(float duration, Vector3 StartPos, Vector3 DestPos, float drag, Vector3 forceAditional = new Vector3())
	{

		int k = Mathf.RoundToInt(duration / Time.fixedDeltaTime);
		float p = 1 - fixedUpdateTime * drag;
		float gravity = -JPhysics.gravity;
		Vector3 distance = DestPos - StartPos;
		Vector3 a = gravity * Vector3.up - forceAditional;
		Vector3 initVelocity = Vector3.zero;
		Vector3 N = a * p / drag;
		float M = (1 - Mathf.Pow(p, k)) / (1 - p);

		float powval = Mathf.Pow(p, k);

		if (drag < 0.0001f)
		{
			initVelocity = distance - (0.5f * (k - 1) * a * Time.fixedDeltaTime * Time.fixedDeltaTime) / Time.fixedDeltaTime;
			initVelocity -= a * Time.fixedDeltaTime;
		}
		else
		{
			//initVelocity = (distance - (a - a * drag * Time.fixedDeltaTime) * Time.fixedDeltaTime / drag * (k - (1 - Mathf.Pow(p, k)) / (1 - p))) / ((1 - (Mathf.Pow(p, k))) / (1 - p) * Time.fixedDeltaTime);
			//initVelocity = (initVelocity - q) / p;
			Vector3 numerator = distance / Time.fixedDeltaTime + N * M - N * k + 0.5f * N * powval - 0.5f * N;
			float denominator = M + 0.5f * powval - 0.5f;
			initVelocity = numerator / denominator;
		}

		return initVelocity;
	}
	*/
	JPS_API bool linePlaneIntersection(JOVECTOR3& contact, JOVECTOR3 ray, JOVECTOR3 rayOrigin, JOVECTOR3 normal, JOVECTOR3 coord)
	{
	// get d value
		float d = DotProduct(normal, coord);

		if (DotProduct(normal, ray) == 0) {
			return false; // No intersection, the line is parallel to the plane
		}

	// Compute the X value for the directed line ray intersecting the plane
		float x = (d - DotProduct(normal, rayOrigin)) / DotProduct(normal, ray);

	// output contact point
		contact = rayOrigin + ray.Normalize() * x; //Make sure your ray vector is normalized
		return true;
	}


	JPS_API float GetPlaneAddFriction(JOVECTOR3 curVelocity)
	{
		const float fixedFriction = 0.30f;
		float addFriction = 1.0f - GROUNDFRICTION * (fixedFriction + ABS(curVelocity.fY) * BOUNCEFACTOR);
		return addFriction;
	}
	JPS_API float GetDistanceToPointFromPlane(JOVECTOR3 normal/*평면의노멀*/, JOVECTOR3 onPoint/*평면의점*/, JOVECTOR3 toPoint/*떨어진 점*/)
	{
		JOVECTOR3 ab = toPoint - onPoint;
		float L = DotProduct(ab, normal);
		return L;
	}
	JPS_API JOVECTOR3 GetInitialVelocity(float fDeltaTime, JOVECTOR3 fStartPos, JOVECTOR3 fDestPos)
	{
		float k = rint(MAX(1, fDeltaTime / FIXEDUPDATETIME)); //fDeltaTime / FIXEDUPDATETIME;
		float p = 1 - FIXEDUPDATETIME * AIRDRAG;
		float gravity = -JXGRAVITY;

		JOVECTOR3 q = JOVECTOR3::up * (gravity)* FIXEDUPDATETIME * p;
		JOVECTOR3 &StartPos = fStartPos;// JOVECTOR3(fStartPos[0], fStartPos[1], fStartPos[2]);
		JOVECTOR3 &DestPos = fDestPos;// JOVECTOR3(fDestPos[0], fDestPos[1], fDestPos[2]);
		JOVECTOR3 distance = DestPos - StartPos;
		JOVECTOR3 a = gravity * JOVECTOR3().up;
		JOVECTOR3 initVelocity = JOVECTOR3().zero;
		JOVECTOR3 N = a * p / AIRDRAG;
		float M = (1 - pow(p, k)) / (1 - p);
		float powval = pow(p, k);

		if (AIRDRAG < 0.0001f)
		{
			initVelocity = distance - (0.5f * (k - 1) * a * FIXEDUPDATETIME * FIXEDUPDATETIME) / FIXEDUPDATETIME;
			initVelocity = initVelocity - a * FIXEDUPDATETIME;
		}
		else
		{
			
			JOVECTOR3 numerator = distance / FIXEDUPDATETIME + N * M - N * k + 0.5f * N * powval - 0.5f * N;
			float denominator = M + 0.5f * powval - 0.5f;
			initVelocity = numerator / denominator;
		}

		//JOVECTOR3().common = initVelocity;
		
		return { initVelocity.fX,initVelocity.fY,initVelocity.fZ };
	}


	JPS_API void SetJBallActor(JPhysicActor* pJBallActor,
							   float fPrePos[],
							   float fStartPos[],
							   float fVelocity[],
							   float fTorqueNormal[],
							   float curveValue,
							   float BounceFactor)
	{
		pJBallActor->Initialzie();
		pJBallActor->SetPos(fStartPos);
		pJBallActor->SetPrePos(fPrePos);
		pJBallActor->SetVelocity(fVelocity);

		/////// 감아차기 관련 /////////////////////////
		CJxActor* jxActor = pJBallActor->GetJxActor();
		CJxSphere* pSphere = (CJxSphere*)jxActor;

		pSphere->SetSpinAble(true);
		pSphere->firstBound = false;
		pSphere->curveMoment.fX = fTorqueNormal[0];
		pSphere->curveMoment.fY = fTorqueNormal[1];
		pSphere->curveMoment.fZ = fTorqueNormal[2];
		pSphere->curveValue = curveValue;
		pSphere->additionalBounceFactorratio = BounceFactor;
		//////////////////////////////////////////////
	}

	JPS_API float GetFixedUpdateTime(void)
	{
		return FIXEDUPDATETIME;
	}

	JPS_API void ApplyRotate(JPhysicActor* pJBallActor, float fMoment[])
	{
		pJBallActor->SetMoment(fMoment);
	}

//}    