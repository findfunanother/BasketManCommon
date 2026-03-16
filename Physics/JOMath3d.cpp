
//#include "stdafx.h"
#include "JOMath3d.h"


JOVECTOR3 JOVECTOR3::up = JOVECTOR3(0.0f, 1.0f, 0.0f);
JOVECTOR3 JOVECTOR3::zero = JOVECTOR3(0.0f, 0.0f, 0.0f);
JOVECTOR3 JOVECTOR3::common = JOVECTOR3(0.0f, 0.0f, 0.0f);
JOVECTOR3 JOVECTOR3::forward = JOVECTOR3(0.0f, 0.0f, 1.0f);



JOVECTOR3::JOVECTOR3()
{
	fX = 0.0f;
	fY = 0.0f;
	fZ = 0.0f;
}


JOVECTOR3::JOVECTOR3(freal fArgX, freal fArgY, freal fArgZ)
{
	fX = fArgX;
	fY = fArgY;
	fZ = fArgZ;
}


JOVECTOR3& JOVECTOR3::operator *= (JOVECTOR3 const A)
{
	this->fX = this->fX * A.fX; 
	this->fY = this->fY * A.fY; 
	this->fZ = this->fZ * A.fZ; 

	return (*this);
}

JOVECTOR3& JOVECTOR3::operator *= (float const fMulti)
{
	this->fX = this->fX * fMulti; 
	this->fY = this->fY * fMulti; 
	this->fZ = this->fZ * fMulti; 

	return (*this);
}

JOVECTOR3& JOVECTOR3::operator += (JOVECTOR3 const A)
{
	this->fX = this->fX + A.fX; 
	this->fY = this->fY + A.fY; 
	this->fZ = this->fZ + A.fZ; 

	return (*this);
}


bool JOVECTOR3::operator == (JOVECTOR3 const A)
{
	return ( this->fX == A.fX && this->fY == A.fY && this->fZ == A.fZ );
}


bool JOVECTOR3::operator != (JOVECTOR3 const A)
{
	return ( this->fX != A.fX || this->fY != A.fY || this->fZ != A.fZ );
}


freal JOVECTOR3::Length(void) const
{
	return (freal)sqrt(fX*fX + fY*fY + fZ*fZ);
}

JOVECTOR3&	JOVECTOR3::Normalized(void)
{
	freal fLength = Length();
	if (fLength == 0.0f) fLength = 1.0f;
	freal InvLength = 1.f / fLength;

	float x, y, z;

	x = this->fX * InvLength;
	y = this->fY * InvLength;
	z = this->fZ * InvLength;

	common.fX = x;
	common.fY = y;
	common.fZ = z;

	return common;
}

JOVECTOR3& JOVECTOR3::Normalize(void) 
{
	freal fLength = Length();
	if( fLength == 0.0f ) fLength = 1.0f;
    freal InvLength =  1.f / fLength;

	this->fX *= InvLength;
	this->fY *= InvLength;
	this->fZ *= InvLength;

    return (*this);
}

bool JOVECTOR3::IsZero(void)
{
	return (this->fX == 0.0f && this->fY==0.0f && this->fZ == 0.0f);
}


//////// End of JOVECTOR3 /////////////////////////////////////

JOMATRIX3::JOMATRIX3()
{
	// identity
	for(int i = 0; i < 3; i++)
	{
		for(int k = 0; k < 3; k++)
		{
			fEle[i][k] = 0;
		}
	}

	fEle[0][0] = fEle[1][1] = fEle[2][2] = 1;
}


JOMATRIX3::JOMATRIX3(JOVECTOR3  &Vector, mode MustBeSkewSymmetric )
{
	this->_11 = 0.0f      ;  this->_12 = -Vector.fY; this->_13 = Vector.fZ;
	this->_21 = Vector.fY ;  this->_22 = 0.0f      ; this->_23 = -Vector.fX;
	this->_31 = -Vector.fZ;  this->_32 = Vector.fX ; this->_33 = 0.0f; 

	// right hand coordination.
/*	this->_11 = 0.0f      ;  this->_12 = -Vector.fZ; this->_13 = Vector.fY;
	this->_21 = Vector.fZ ;  this->_22 = 0.0f      ; this->_23 = -Vector.fX;
	this->_31 = -Vector.fY;  this->_32 = Vector.fX ; this->_33 = 0.0f; 
*/
}

/// <summary>
/// 점과 라인의 최단 거리와 그 점을 구하는 함수 
/// </summary>
/// <param name="pt"></param>
/// <param name="p1"></param>
/// <param name="p2"></param>
/// <param name="closest"></param>
/// <returns></returns>
float IntersectPointToLine(JOVECTOR3 pt, JOVECTOR3 p1, JOVECTOR3 p2, JOVECTOR3* closest)
{
	JOVECTOR3 dVector = p2 - p1;

	float dVectorLength = dVector.Length();

	if (dVector.Length() == 0)
	{
		// It's a point not a line segment.
		closest->fX = p1.fX;
		closest->fY = p1.fY;
		closest->fZ = p1.fZ;

		return (dVector - p1).Length();
	}

	// Calculate the t that minimizes the distance.
	float t = DotProduct(dVector, pt - p1) / (dVectorLength * dVectorLength);

	// See if this represents one of the segment's
	// end points or a point in the middle.
	if (t < 0)
	{
		closest->fX = p1.fX;
		closest->fY = p1.fY;
		closest->fZ = p1.fZ;
	}
	else if (t > 1)
	{
		closest->fX = p2.fX;
		closest->fY = p2.fY;
		closest->fZ = p2.fZ;
	}
	else
	{
		JOVECTOR3 ct = p1 + t * dVector;
		closest->fX = ct.fX;
		closest->fY = ct.fY;
		closest->fZ = ct.fZ;
	}

	return (pt - closest).Length();
}


/// <summary>
/// 라인과 라인이 교차하는 최단 거리와 그 위치를 구하는 함수 
/// </summary>
/// <param name="l1start"></param>
/// <param name="l1end"></param>
/// <param name="l2start"></param>
/// <param name="l2end"></param>
/// <param name="nearPnt1"></param>
/// <param name="nearPnt2"></param>
/// <returns></returns>
float InterSectLineToLine(JOVECTOR3 l1start, JOVECTOR3 l1end, JOVECTOR3 l2start, JOVECTOR3 l2end, JOVECTOR3* nearPnt1, JOVECTOR3* nearPnt2)
{
	JOVECTOR3 uS = l1start;
	JOVECTOR3 uE = l1end;
	JOVECTOR3 vS = l2start;
	JOVECTOR3 vE = l2end;
	JOVECTOR3 w1 = l1start;
	JOVECTOR3 w2 = l2start;
	JOVECTOR3 u = uE - uS;
	JOVECTOR3 v = vE - vS;
	JOVECTOR3 w = w1 - w2;
	float a = DotProduct(u, u);
	float b = DotProduct(u, v);
	float c = DotProduct(v, v);
	float d = DotProduct(u, w);
	float e = DotProduct(v, w);
	float D = a * c - b * b;
	float sc, sN, sD = D;
	float tc, tN, tD = D;
	if (D < 0.01)
	{
		sN = 0;
		sD = 1;
		tN = e;
		tD = c;
	}
	else
	{
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0)
		{
			sN = 0;
			tN = e;
			tD = c;
		}
		else if (sN > sD)
		{
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}
	if (tN < 0)
	{
		tN = 0;
		if (-d < 0)
		{
			sN = 0;
		}
		else if (-d > a)
		{
			sN = sD;
		}
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)
	{
		tN = tD;
		if ((-d + b) < 0)
		{
			sN = 0;
		}
		else if ((-d + b) > a)
		{
			sN = sD;
		}
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}
	if (ABS(sN) < 0.001f)
	{
		sc = 0;
		//EndofLine
	}
	else
	{
		sc = sN / sD;
	}
	if (ABS(tN) < 0.001f)
	{
		tc = 0;
	}
	else
	{
		tc = tN / tD;
	}

	JOVECTOR3 pt1 = sc * (l1end - l1start) + l1start;
	JOVECTOR3 pt2 = tc * (l2end - l2start) + l2start;

	nearPnt1->fX = pt1.fX;
	nearPnt1->fY = pt1.fY;
	nearPnt1->fZ = pt1.fZ;

	nearPnt2->fX = pt2.fX;
	nearPnt2->fY = pt2.fY;
	nearPnt2->fZ = pt2.fZ;

	JOVECTOR3 dP = w + (sc * u) - (tc * v);
	float distance1 = dP.Length();

	return distance1;
}

bool IntersectSphereToCylinder(JOVECTOR3 comCylinder,  /*실린더의 위치*/
	JOVECTOR3 upCylinder, /*실린더의 노멀*/
	float heightCylinder, /*실린더의 길이*/
	float radiusToRadius/*실린더 반지름+공의반지름*/,
	JOVECTOR3 linestart,
	JOVECTOR3 lineend,
	JOVECTOR3* collisionPoint) /*충돌되는 시작점*/
{
	JOVECTOR3 poscylinerbottom, poscylindertop;
	poscylinerbottom = comCylinder - upCylinder * heightCylinder * 0.5f;
	poscylindertop = comCylinder + upCylinder * heightCylinder * 0.5f;

	JOVECTOR3 cylinderNearPnt; /*이동 라인에서 가까운 실린더 위에 있는 점*/
	JOVECTOR3 lineNearPoint;   /*실린더 라인에서 가장 가까운 이동 라인 위에 있는 점*/
	JOVECTOR3 lineRealNearPoint;

	float distance = InterSectLineToLine(poscylinerbottom, poscylindertop, linestart, lineend, &cylinderNearPnt, &lineNearPoint);
	//lineend = linestart + (lineend - linestart).normalized * 50.0f; // 무한 라인을 그린 경우 

	if (radiusToRadius >= distance)
	{
		JOVECTOR3 linedir = lineend - linestart;

		JOVECTOR3 linestartTocylinderNearPntDir = cylinderNearPnt - linestart;
		float lengthOrth = DotProduct(linestartTocylinderNearPntDir, linedir.Normalized());
		lineRealNearPoint = linedir.Normalized() * lengthOrth + linestart;
		float realDistance = (cylinderNearPnt - lineRealNearPoint).Length();

		JOVECTOR3 newNormaDir = CrossProduct(lineRealNearPoint - cylinderNearPnt, upCylinder);
		newNormaDir.Normalize();

		JOVECTOR3 collisionPointPlane = lineRealNearPoint + newNormaDir.Normalized() * static_cast<float>(sqrt(static_cast<double>(radiusToRadius) * static_cast<double>(radiusToRadius) - (static_cast<double>(realDistance) * static_cast<double>(realDistance))));
		//float fDistanceCXdash = (collisionPointPlane - cylinderNearPnt).Length();

		JOVECTOR3 newDir = CrossProduct(cylinderNearPnt - lineRealNearPoint, upCylinder);
		newDir.Normalize();

		float lengthB = (lineRealNearPoint - collisionPointPlane).Length();
		float lengthAB = DotProduct((lineRealNearPoint - linestart), newDir);
		float k = lengthB / ABS(lengthAB);

		JOVECTOR3 newCollisionPt = linestart + (1.0f - k) * (lineRealNearPoint - linestart);

		collisionPoint->fX = newCollisionPt.fX;
		collisionPoint->fY = newCollisionPt.fY;
		collisionPoint->fZ = newCollisionPt.fZ;

		return true;

	}
	else
	{
		collisionPoint->fX = 0.0f;
		collisionPoint->fY = 0.0f;
		collisionPoint->fZ = 0.0f;
		return false;
	}

}
// 

// 

bool IntersectTriangle(JOVECTOR3 orig, JOVECTOR3 dir, JOVECTOR3 v0, JOVECTOR3 v1, JOVECTOR3 v2, float* t, float* u, float* v)
{

	// Find vectors for two edges sharing vert0
	JOVECTOR3 edge1 = v1 - v0;
	JOVECTOR3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	JOVECTOR3 pvec;
	pvec = CrossProduct(dir, edge2);

	// If determinant is near zero, ray lies in plane of triangle
	float det = DotProduct(edge1, pvec);

	JOVECTOR3 tvec;
	if (det > 0)
	{
		tvec = orig - v0;
	}
	else
	{
		tvec = v0 - orig;
		det = -det;
	}

	if (det < 0.0001f)
		return false;

	// Calculate U parameter and test bounds
	*u = DotProduct(tvec, pvec);
	if (*u < 0.0f || *u > det)
		return false;

	// Prepare to test V parameter
	JOVECTOR3 qvec;
	qvec = CrossProduct(tvec, edge1);

	// Calculate V parameter and test bounds
	*v = DotProduct(dir, qvec);
	if (*v < 0.0f || *u + *v > det)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	*t = DotProduct(edge2, qvec);
	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}

/// <summary>
/// ?趾【? ?????? 점???? ?타??? ???求? ?獨? by findfun 2019/05/15
/// </summary>
/// <param name="normal"></param>
/// <param name="onPoint"></param>
/// <param name="toPoint"></param>
/// <returns></returns>
float GetDistanceToPointFromEquation(JOVECTOR3 normal/*?????풍???*/, JOVECTOR3 onPoint/*??????점*/, JOVECTOR3 toPoint/*?????? 점*/)
{
	float a = normal.fX;
	float b = normal.fY;
	float c = normal.fZ;
	float d = onPoint.fX;
	float e = onPoint.fY;
	float f = onPoint.fZ;

	float D = -(a * d + b * e + c * f);
	float up = a * toPoint.fX + b * toPoint.fY + c * toPoint.fZ + D;
	float down = static_cast<float>(sqrt(static_cast<double>(a) * static_cast<double>(a) + static_cast<double>(b) * static_cast<double>(b) + static_cast<double>(c) * static_cast<double>(c)));
	float L = up / down;

	return L;
}


//////  End of JOMATIRX3 ///////////////

JOMATRIX4::JOMATRIX4()
{
	// identity
	for(int i = 0; i < 4; i++)
	{
		for(int k = 0; k < 4; k++)
		{
			fEle[i][k] = 0;
		}
	}

	fEle[0][0] = fEle[1][1] = fEle[2][2] = fEle[3][3] = 1;
}

///////// End of JOAMTRIX4 ///////////////////////////

///////// Start of global math function //////////////////////////
void OrthonormalizeOrientation( JOMATRIX3 &Orientation )
{
	JOVECTOR3 X(Orientation._11, Orientation._21, Orientation._31);
	JOVECTOR3 Y(Orientation._12, Orientation._22, Orientation._32);
	JOVECTOR3 Z;

	X.Normalize();

	Z = CrossProduct(X, Y);
	Z.Normalize();

	Y = CrossProduct(Z, X);
	Y.Normalize();

	Orientation._11 = X.fX; Orientation._12 = Y.fX; Orientation._13 = Z.fX;
	Orientation._21 = X.fY; Orientation._22 = Y.fY; Orientation._23 = Z.fY;
	Orientation._31 = X.fZ; Orientation._32 = Y.fZ; Orientation._33 = Z.fZ;

}


///////////// Start of JOPLANE3 ////////////////////////////////

JOPLANE3::JOPLANE3(JOVECTOR3 const &A,  JOVECTOR3 const &B, JOVECTOR3 const &C)
{
	JOVECTOR3 pos0, pos1;

	pos0 = C - A;
	pos1 = B - A;

	jvNormal = CrossProduct(pos0, pos1);
	jvNormal.Normalize();

	fD = -(A*jvNormal);
}


JOPLANE3::JOPLANE3(JOVECTOR3 const &N, JOVECTOR3 const &A)
{

}


JOPLANE3::POINTLOC JOPLANE3::TestPoint(JOVECTOR3 const &A)
{
	freal fRe = A*jvNormal + fD;

	if(fRe > EPSILON)
	{
		return ptFront;
	}else
	if(fRe < -EPSILON)
	{
		return ptBack;
	}

	return ptCoplanar; //   between front, back
}



//////////// End of JOPLANE3 ///////////////////////////////////


float JOQUATERNION::quaternion4[4];
JOQUATERNION JOQUATERNION::identity = JOQUATERNION(0.f, 0.f, 0.f, 1.f);

//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//JOQUATERNION& JOQUATERNION::Euler(JOVECTOR3 vector) // yaw (Z), pitch (Y), roll (X)
//{
//	float cy = cos(vector.fZ * 0.5f);
//	float sy = sin(vector.fZ * 0.5f);
//	float cp = cos(vector.fY * 0.5f);
//	float sp = sin(vector.fY * 0.5f);
//	float cr = cos(vector.fX * 0.5f);
//	float sr = sin(vector.fX * 0.5f);
//
//	JOQUATERNION q;
//
//	q.w = cy * cp * cr + sy * sp * sr;
//	q.v.fX = cy * cp * sr - sy * sp * cr;
//	q.v.fY = sy * cp * sr + cy * sp * cr;
//	q.v.fZ = sy * cp * cr - cy * sp * sr;
//
//	return q;
//}

inline JOQUATERNION& JOQUATERNION::operator += ( const JOQUATERNION& Q )
{
	w += Q.w;
	v += Q.v;

	return *this;
}

inline JOQUATERNION& JOQUATERNION::operator -= ( const JOQUATERNION& Q )
{
	w    -= Q.w;
	v.fX -= Q.v.fX;
	v.fY -= Q.v.fY;
	v.fZ -= Q.v.fZ;

	return *this;
}
/*
inline JOQUATERNION& JOQUATERNION::operator *= ( const JOQUATERNION& Q )
{
	w *= Q.w;
	v *= Q.v;

	return *this;
}
*/
inline JOQUATERNION& JOQUATERNION::operator *= ( float fM )
{
	w *= fM;
	v *= fM;

	return *this;
}

inline JOQUATERNION& JOQUATERNION::operator /= ( float fS )
{
	w /= fS;
	v.fX /= fS;
	v.fY /= fS;
	v.fZ /= fS;

	return *this;
}


