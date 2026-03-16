#pragma once
#include <chrono>
#include <cmath>
#include "../../Physics/JEnviroment.h"
#include "Common_generated.h"
#include "HostDefine.h"

namespace CommonFunction
{
	void SVectorSet(TB::SVector3& dst, TB::SVector3& src);

	void SVectorSet(TB::SVector3& dst, TB::SVector3 src);

	void SJoVectorMultiply(JOVECTOR3 pA, float mul, JOVECTOR3& calcPos);

	float SVectorDistance(TB::SVector3& pVector1, TB::SVector3& pVector2);

	float SVectorDistanceXZ(TB::SVector3& pPosition, float pX, float pZ);

	float SVectorDistanceXZ(const TB::SVector3& a, const TB::SVector3& b);

	float CalcDestYaw(JOVECTOR3 StartPos);

	float ToYaw(JOVECTOR3 StartPos);

	float OptimizeAngle(float angle);

	void SVectorYaw(JOVECTOR3& rootPos, float angle, JOVECTOR3& calcPos);

	float SVectorDistance(TB::SVector3& pVector);

	TB::SVector3 SVectorNormalize(TB::SVector3& pA);

	TB::SVector3 SVectorAdd(JOVECTOR3 a, JOVECTOR3 b);

	TB::SVector3 SVectorAdd(const TB::SVector3& pA, const TB::SVector3& pB);

	TB::SVector3 SVectorMultiply(TB::SVector3& pA, float mul);

	void SVectorMultiplyEx(JOVECTOR3 a, float mul, JOVECTOR3& calcPos);

	TB::SVector3 SVectorSub(const TB::SVector3& pA, const TB::SVector3& pB);

	TB::SVector3 SVectorSubTBWithJo(const TB::SVector3& pA, JOVECTOR3& pB);

	float SVectorSubYaw(const TB::SVector3& pA, const TB::SVector3& pB);

	float SVectorSubYawEx(const TB::SVector3& pA, float x, float y, float z);

	float SVectorSubYawEx(float x, float y, float z, const TB::SVector3& pA);

	void SVectorSubEx(JOVECTOR3 a, const TB::SVector3& pB, JOVECTOR3& calcPos);

	TB::SVector3 SVectorSubZ(TB::SVector3& pA, float z);

	TB::SVector3 SVectorLerp(const TB::SVector3& pA, const TB::SVector3& pB, float rate);

	void CorrOutSide(TB::SVector3& pPosition);

	void CorrPositionThreePointIn(TB::SVector3& pPosition);

	void CorrPositionThreePointOut(TB::SVector3& pPosition);

	void CorrOutSideEx(JOVECTOR3& pPosition);

	TB::SVector3 CorrPositionThreePointInEx(TB::SVector3& pPosition);

	TB::SVector3 CorrPositionThreePointOutEx(TB::SVector3& pPosition);

	TB::SVector3 MoveToWards(JOVECTOR3 position, const TB::SVector3& targetPosition, float maxDistanceDelta);
	
	void ConvertTBVectorToJoVector(const TB::SVector3& from, JOVECTOR3& to);

	void ConvertTBVectorToJoVector(const TB::SVector3* from, JOVECTOR3& to);

	TB::SVector3 ConvertJoVectorToTBVector(JOVECTOR3 from);

	TB::SVector3 SVectorSlidePosition(const TB::SVector3& a, JOVECTOR3 c);

	TB::SVector3 SVectorSlidePosition(const TB::SVector3& a, const TB::SVector3& b, JOVECTOR3 c);

	float SVectorDot(TB::SVector3& a, TB::SVector3& b);

	float SVectorSignedAngle(TB::SVector3& a, TB::SVector3& b);

	float SVectorSignedAngle(TB::SVector3& a, TB::SVector3& b, TB::SVector3& c);
}