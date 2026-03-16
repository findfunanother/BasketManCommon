#include "HostCommonFunction.h"

namespace CommonFunction
{
	void SVectorSet(TB::SVector3& dst, TB::SVector3& src)
	{
		dst.mutate_x(src.x());
		dst.mutate_y(src.y());
		dst.mutate_z(src.z());
	}

	void SVectorSet(TB::SVector3& dst, TB::SVector3 src)
	{
		dst.mutate_x(src.x());
		dst.mutate_y(src.y());
		dst.mutate_z(src.z());
	}

	void SJoVectorMultiply(JOVECTOR3 pA, float mul, JOVECTOR3& calcPos)
	{
		calcPos.fX = (pA.fX * mul);
		calcPos.fY = (pA.fY * mul);
		calcPos.fZ = (pA.fZ * mul);
	}

	void SVectorYaw(JOVECTOR3& rootPos, float angle, JOVECTOR3& calcPos)
	{
		float angleRad = angle * DEG2RAD();
		float cosA = cos(angleRad);
		float sinA = -sin(angleRad);

		calcPos.fX = (rootPos.fX * cosA - rootPos.fZ * sinA);
		calcPos.fY = (rootPos.fY);
		calcPos.fZ = (rootPos.fX * sinA + rootPos.fZ * cosA);
	}

	float SVectorDistance(TB::SVector3& pVector1, TB::SVector3& pVector2)
	{
		float x = pVector1.x() - pVector2.x();
		float y = pVector1.y() - pVector2.y();
		float z = pVector1.z() - pVector2.z();

		return sqrt(static_cast<double>(x * x + y * y + z * z));
	}

	float SVectorDistanceXZ(TB::SVector3& pPosition, float pX, float pZ)
	{
		float x = pPosition.x() - pX;
		float z = pPosition.z() - pZ;

		return sqrt(static_cast<double>(x * x + z * z));
	}

	float SVectorDistanceXZ(const TB::SVector3& a, const TB::SVector3& b)
	{
		float x = a.x() - b.x();
		float z = a.z() - b.z();

		return sqrt(static_cast<double>(x * x + z * z));
	}

	float CalcDestYaw(JOVECTOR3 StartPos)
	{
		float dest_yaw = 0.0f;

		JOVECTOR3 delta;
		delta.fX = RIM_POS_X - StartPos.fX;
		delta.fZ = RIM_POS_Z_FIXED - StartPos.fZ;

		float x, z;
		x = delta.fX;
		z = delta.fZ;

		dest_yaw = atan2(x, z) * RAD2DEG;

		return dest_yaw;
	}

	float ToYaw(JOVECTOR3 StartPos)
	{
		float dest_yaw = 0.0f;

		float x, z;
		x = StartPos.fX;
		z = StartPos.fZ;

		dest_yaw = atan2(x, z) * RAD2DEG;

		return dest_yaw;
	}

	float OptimizeAngle(float angle)
	{
		float fullCircle = 360.0f;
		angle = fmodf(angle, fullCircle);

		if (angle > 180.0f)
		{
			angle -= fullCircle;
		}
		else if (angle < -180.0f)
		{
			angle += fullCircle;
		}

		return angle;
	}

	float SVectorDistance(TB::SVector3& pVector)
	{
		float x = pVector.x();
		float y = pVector.y();
		float z = pVector.z();

		return sqrt(static_cast<double>(x * x + y * y + z * z));
	}

	TB::SVector3 SVectorNormalize(TB::SVector3& pA)
	{
		TB::SVector3 retValue;

		float distance = SVectorDistance(pA);

		if (distance <= 0.0f) { distance = 1.0f; }

		retValue.mutate_x(pA.x() / distance);
		retValue.mutate_y(pA.y() / distance);
		retValue.mutate_z(pA.z() / distance);

		return retValue;
	}

	TB::SVector3 SVectorAdd(JOVECTOR3 a, JOVECTOR3 b)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(a.fX + b.fX);
		retValue.mutate_y(a.fY + b.fY);
		retValue.mutate_z(a.fZ + b.fZ);

		return retValue;
	}

	TB::SVector3 SVectorAdd(const TB::SVector3& pA, const TB::SVector3& pB)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() + pB.x());
		retValue.mutate_y(pA.y() + pB.y());
		retValue.mutate_z(pA.z() + pB.z());

		return retValue;
	}

	TB::SVector3 SVectorMultiply(TB::SVector3& pA, float mul)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() * mul);
		retValue.mutate_y(pA.y() * mul);
		retValue.mutate_z(pA.z() * mul);

		return retValue;
	}

	void SVectorMultiplyEx(JOVECTOR3 a, float mul, JOVECTOR3& calcPos)
	{
		calcPos.fX = (a.fX * mul);
		calcPos.fY = (a.fY * mul);
		calcPos.fZ = (a.fZ * mul);
	}
	
	TB::SVector3 SVectorSub(const TB::SVector3& pA, const TB::SVector3& pB)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() - pB.x());
		retValue.mutate_y(pA.y() - pB.y());
		retValue.mutate_z(pA.z() - pB.z());

		return retValue;
	}

	TB::SVector3 SVectorSubTBWithJo(const TB::SVector3& pA, JOVECTOR3& pB)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() - pB.fX);
		retValue.mutate_y(pA.y() - pB.fY);
		retValue.mutate_z(pA.z() - pB.fZ);

		return retValue;
	}

	float SVectorSubYaw(const TB::SVector3& pA, const TB::SVector3& pB)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() - pB.x());
		retValue.mutate_y(pA.y() - pB.y());
		retValue.mutate_z(pA.z() - pB.z());

		JOVECTOR3 sCalc;
		sCalc.fX = retValue.x();
		sCalc.fY = retValue.y();
		sCalc.fZ = retValue.z();

		float result = kFLOAT_INIT;
		result = ToYaw(sCalc);

		return result;
	}

	float SVectorSubYawEx(const TB::SVector3& pA, float x, float y, float z)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x() - x);
		retValue.mutate_y(pA.y() - y);
		retValue.mutate_z(pA.z() - z);

		JOVECTOR3 sCalc;
		sCalc.fX = retValue.x();
		sCalc.fY = retValue.y();
		sCalc.fZ = retValue.z();

		float result = kFLOAT_INIT;
		result = ToYaw(sCalc);

		return result;
	}

	float SVectorSubYawEx(float x, float y, float z, const TB::SVector3& pA)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(x - pA.x());
		retValue.mutate_y(y - pA.y());
		retValue.mutate_z(z - pA.z());

		JOVECTOR3 sCalc;
		sCalc.fX = retValue.x();
		sCalc.fY = retValue.y();
		sCalc.fZ = retValue.z();

		float result = kFLOAT_INIT;
		result = ToYaw(sCalc);

		return result;
	}

	void SVectorSubEx(JOVECTOR3 a, const TB::SVector3& pB, JOVECTOR3& calcPos)
	{
		calcPos.fX = a.fX - pB.x();
		calcPos.fY = a.fY - pB.y();
		calcPos.fZ = a.fZ - pB.z();
	}

	TB::SVector3 SVectorSubZ(TB::SVector3& pA, float z)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(pA.x());
		retValue.mutate_y(pA.y());
		retValue.mutate_z(pA.z() - z);

		return retValue;
	}

	TB::SVector3 SVectorLerp(const TB::SVector3& pA, const TB::SVector3& pB, float rate)
	{
		TB::SVector3 temp = SVectorSub(pB, pA);

		TB::SVector3 direction = SVectorMultiply(temp, rate);

		return SVectorAdd(pA, direction);
	}

	void CorrOutSide(TB::SVector3& pPosition)
	{
		pPosition.mutate_x(fminf(fmaxf(pPosition.x(), -COURT_X() + CHARACTER_X()), COURT_X() - CHARACTER_X()));
		pPosition.mutate_z(fminf(fmaxf(pPosition.z(), 0.0f + CHARACTER_Z_MAX()), COURT_Z() - CHARACTER_Z_MIN()));
	}

	void CorrPositionThreePointIn(TB::SVector3& pPosition)
	{
		if (pPosition.z() < POINT3_CHECK_Z())
		{
			TB::SVector3 direction = SVectorSubZ(pPosition, RIM_POSITION_Z);
			float distanceDelta = POINT3_LENGTH - (COLLISION_RADIUS * 0.5f) * 1.5f - SVectorDistance(direction);
			if (distanceDelta < 0.0f)
			{
				TB::SVector3 normalize = SVectorNormalize(direction);
				TB::SVector3 delta = SVectorMultiply(normalize, distanceDelta);
				pPosition.mutate_x(pPosition.x() + delta.x());
				pPosition.mutate_y(pPosition.y() + delta.y());
				pPosition.mutate_z(pPosition.z() + delta.z());
			}
		}
		else
		{
			if (pPosition.x() > 0.0f)
			{
				pPosition.mutate_x(fminf(POINT3_LENGTH_SIDE - (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
			else
			{
				pPosition.mutate_x(fmaxf(-POINT3_LENGTH_SIDE + (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
		}
	}

	void CorrPositionThreePointOut(TB::SVector3& pPosition)
	{
		if (pPosition.z() < POINT3_CHECK_Z())
		{
			TB::SVector3 direction = SVectorSubZ(pPosition, RIM_POSITION_Z);
			float distanceDelta = POINT3_LENGTH + (COLLISION_RADIUS * 0.5f) * 1.5f - SVectorDistance(direction);
			if (distanceDelta > 0.0f)
			{
				TB::SVector3 normalize = SVectorNormalize(direction);
				TB::SVector3 delta = SVectorMultiply(normalize, distanceDelta);
				pPosition.mutate_x(pPosition.x() + delta.x());
				pPosition.mutate_y(pPosition.y() + delta.y());
				pPosition.mutate_z(pPosition.z() + delta.z());
			}
		}
		else
		{
			if (pPosition.x() > 0.0f)
			{
				pPosition.mutate_x(fmaxf(POINT3_LENGTH_SIDE + (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
			else
			{
				pPosition.mutate_x(fminf(-POINT3_LENGTH_SIDE - (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
		}
	}

	void CorrOutSideEx(JOVECTOR3& pPosition)
	{
		if (pPosition.fX > COURT_X() - CHARACTER_X())
		{
			pPosition.fX = COURT_X() - CHARACTER_X();
		}

		if (pPosition.fX < -COURT_X() + CHARACTER_X())
		{
			pPosition.fX = -COURT_X() + CHARACTER_X();
		}

		if (pPosition.fZ > COURT_Z() - CHARACTER_Z_MIN())
		{
			pPosition.fZ = COURT_Z() - CHARACTER_Z_MIN();
		}

		if (pPosition.fZ < CHARACTER_Z_MAX())
		{
			pPosition.fZ = CHARACTER_Z_MAX();
		}
	}

	TB::SVector3 CorrPositionThreePointInEx(TB::SVector3& pPosition)
	{
		TB::SVector3 result = pPosition;

		if (pPosition.z() < POINT3_CHECK_Z())
		{
			TB::SVector3 direction = SVectorSubZ(pPosition, RIM_POSITION_Z);
			float distanceDelta = POINT3_LENGTH - (COLLISION_RADIUS * 0.5f) * 1.5f - SVectorDistance(direction);
			if (distanceDelta < 0.0f)
			{
				TB::SVector3 normalize = SVectorNormalize(direction);
				TB::SVector3 delta = SVectorMultiply(normalize, distanceDelta);

				result.mutate_x(pPosition.x() + delta.x());
				result.mutate_y(pPosition.y() + delta.y());
				result.mutate_z(pPosition.z() + delta.z());
			}
		}
		else
		{
			if (pPosition.x() > 0.0f)
			{
				result.mutate_x(fminf(POINT3_LENGTH_SIDE - (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
			else
			{
				result.mutate_x(fmaxf(-POINT3_LENGTH_SIDE + (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
		}

		return result;
	}

	TB::SVector3 CorrPositionThreePointOutEx(TB::SVector3& pPosition)
	{
		TB::SVector3 result = pPosition;

		if (pPosition.z() < POINT3_CHECK_Z())
		{
			TB::SVector3 direction = SVectorSubZ(pPosition, RIM_POSITION_Z);
			float distanceDelta = POINT3_LENGTH + (COLLISION_RADIUS * 0.5f) * 1.5f - SVectorDistance(direction);
			if (distanceDelta > 0.0f)
			{
				TB::SVector3 normalize = SVectorNormalize(direction);
				TB::SVector3 delta = SVectorMultiply(normalize, distanceDelta);

				result.mutate_x(pPosition.x() + delta.x());
				result.mutate_y(pPosition.y() + delta.y());
				result.mutate_z(pPosition.z() + delta.z());
			}
		}
		else
		{
			if (pPosition.x() > 0.0f)
			{
				result.mutate_x(fmaxf(POINT3_LENGTH_SIDE + (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
			else
			{
				result.mutate_x(fminf(-POINT3_LENGTH_SIDE - (COLLISION_RADIUS * 0.5f) * 1.5f, pPosition.x()));
			}
		}
		return result;
	}

	TB::SVector3 MoveToWards(JOVECTOR3 position, const TB::SVector3& targetPosition, float maxDistanceDelta)
	{
		TB::SVector3 sub;
		sub.mutate_x(targetPosition.x() - position.fX);
		sub.mutate_y(targetPosition.y() - position.fY);
		sub.mutate_z(targetPosition.z() - position.fZ);

		TB::SVector3 normal = SVectorNormalize(sub);
		
		TB::SVector3 multi = SVectorMultiply(normal, maxDistanceDelta);

		TB::SVector3 result;
		result.mutate_x(position.fX + multi.x());
		result.mutate_y(position.fY + multi.y());
		result.mutate_z(position.fZ + multi.z());

		return result;
	}

	void ConvertTBVectorToJoVector(const TB::SVector3& from, JOVECTOR3& to)
	{
		to.fX = from.x();
		to.fY = from.y();
		to.fZ = from.z();
	}

	void ConvertTBVectorToJoVector(const TB::SVector3* from, JOVECTOR3& to)
	{
		to.fX = from->x();
		to.fY = from->y();
		to.fZ = from->z();
	}

	TB::SVector3 ConvertJoVectorToTBVector(JOVECTOR3 from)
	{
		TB::SVector3 to;
		to.mutate_x(from.fX);
		to.mutate_y(from.fY);
		to.mutate_z(from.fZ);

		return to;
	}

	TB::SVector3 SVectorSlidePosition(const TB::SVector3& a, JOVECTOR3 c)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(a.x() - c.fX);
		retValue.mutate_y(a.y() - c.fY);
		retValue.mutate_z(a.z() - c.fZ);

		return retValue;
	}

	TB::SVector3 SVectorSlidePosition(const TB::SVector3& a, const TB::SVector3& b, JOVECTOR3 c)
	{
		TB::SVector3 retValue;

		retValue.mutate_x(a.x() + b.x() - c.fX);
		retValue.mutate_y(a.y() + b.y() - c.fY);
		retValue.mutate_z(a.z() + b.z() - c.fZ);

		return retValue;
	}

	float SVectorDot(TB::SVector3& a, TB::SVector3& b)
	{
		float result = kFLOAT_INIT;

		result = a.x() * b.x() + a.y() * b.y() + a.z() * b.z();

		return result;
	}

	float SVectorSignedAngle(TB::SVector3& a, TB::SVector3& b)
	{
		float result = kFLOAT_INIT;

		TB::SVector3 tempA;
		tempA.mutate_x(a.x());
		tempA.mutate_y(a.y());
		tempA.mutate_z(a.z());

		TB::SVector3 tempB;
		tempB.mutate_x(b.x());
		tempB.mutate_y(b.y());
		tempB.mutate_z(b.z());

		float v1 = sqrt(pow(tempA.x(), 2) + pow(tempA.y(), 2) + pow(tempA.z(), 2));
		float v2 = sqrt(pow(tempB.x(), 2) + pow(tempB.y(), 2) + pow(tempB.z(), 2));

		float innner = tempA.x() * tempB.x() + tempA.y() * tempB.y() + tempA.z() * tempB.z();

		float theta = acos(innner / (v1 * v2));

		if (isnan(theta))
		{
			result = 360.0f;
		}
		else
		{
			result = theta * RAD2DEG;
		}


		return result;
	}

	float SVectorSignedAngle(TB::SVector3& a, TB::SVector3& b, TB::SVector3& c)
	{
		float result = kFLOAT_INIT;

		TB::SVector3 cross;
		cross.mutate_x(a.y() * b.z() - a.z() * b.y());
		cross.mutate_y(a.z() * b.x() - a.x() * b.z());
		cross.mutate_z(a.x() * b.y() - a.y() * b.x());

		TB::SVector3 normalize = SVectorNormalize(c);

		float theta = atan2(SVectorDot(normalize, cross), SVectorDot(a, b));

		if (isnan(theta))
		{
			result = 360.0f;
		}
		else
		{
			result = theta * RAD2DEG;
		}

		return result;
	}
}