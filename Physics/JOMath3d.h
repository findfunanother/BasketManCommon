#pragma once
/********************************************************
*  Author: Jo, Yeong Seok   (  youngseokjoe@gmail.com )
*  Date  : 2003 / 4 / 29
*  Last Update: 2024 / 11 / 28
**********************************************************/
#ifndef _JOMATH3D_H__
#define _JOMATH3D_H__

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#ifdef EXPORT_DLL
#define JPS_API __declspec(dllexport)
#else
#define JPS_API __declspec(dllimport) 
#endif
#define JPS_API_CAL
#else
#define JPS_API __attribute__((visibility("default")))
#define JPS_API_CAL __attribute__((visibility("hidden")))
#endif

typedef float freal;
typedef freal FREAL;

#ifndef _X64
#include <math.h>
#else
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#endif

#define  PI      3.1415926536f
#define  RADIAN  (PI / 180)
#define  DEGREE  (180 / PI)

#define	 DANGLE(a) (float)((180.0f / PI)*a) // Randian(a) -> Degree
#define  RADIA(r) ( PI*r / (FLOAT)180 ) // Degree(r) -> Radian

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a, b)           (((a) > (b)) ? (a) : (b))
#define ABS(a)	            (((a) < 0) ? -(a) : (a))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


/////////////// Statr of JOVECTOR3 //////////////////////////////

class JPS_API JOVECTOR3
{

public:
	union {
			struct  { freal fX, fY, fZ; };
			freal fEle[3];
		};

	JOVECTOR3();
	JOVECTOR3(freal fArgX , freal fArgY , freal fArgZ );
	template <class T> JOVECTOR3( const T &Other );
	freal		Length(void) const;
	JOVECTOR3&	Normalize(void);
	JOVECTOR3&	Normalized(void);
	bool		IsZero(void);
	JOVECTOR3& operator *= (JOVECTOR3 const A);
	JOVECTOR3& operator *= (float const fMulti);
	JOVECTOR3& operator += (JOVECTOR3 const A);

	static JOVECTOR3 Lerp(JOVECTOR3 vector3from, JOVECTOR3 vector3to,float seq)
	{

		return JOVECTOR3((1 - seq) * vector3from.fX + seq * vector3to.fX,
						 (1 - seq) * vector3from.fY + seq * vector3to.fY,
						 (1 - seq) * vector3from.fZ + seq * vector3to.fZ);// (1 - seq)* vector3from.fX + seq * vector3to.fX;//vector3from + (vector3to - vector3from) * seq;
	}
public:
	static JOVECTOR3 up;
	static JOVECTOR3 zero;
	static JOVECTOR3 common;
	static JOVECTOR3 forward;
/*
	// Operators.
	JOVECTOR3 operator^( const JOVECTOR3& V ) 
	{// Cross Product

		this->fX = this->fY * V.fZ - this->fZ * V.fY;
		this->fY = this->fZ * V.fX - this->fX * V.fZ;
		this->fZ = this->fX * V.fY - this->fY * V.fX;

		return (*this);

	}
	float operator|( const JOVECTOR3& V ) 
	{// Dot Product
		return fX*V.fX + fY*V.fY + fZ*V.fZ;
	}
*/
    
	template <class T> JOVECTOR3& operator = ( T const A );
	bool operator == (JOVECTOR3 const A);
	bool operator != (JOVECTOR3 const A);

};


template <class T> 
JOVECTOR3::JOVECTOR3( const T &Other )
{
	*this = *((JOVECTOR3*)(&Other));
}

// ?隔? ???? ?獵? ?? ??음 by findfun 2019-03-28
template <class T>
JOVECTOR3& JOVECTOR3::operator = ( T const A )
{
	*this = *((JOVECTOR3*)(&A));
	return (*this);
}


inline const JOVECTOR3 operator + (JOVECTOR3 const &A, JOVECTOR3 const &B);
inline const JOVECTOR3 operator - (JOVECTOR3 const &A, JOVECTOR3 const &B);
inline const freal     operator * (JOVECTOR3 const &A, JOVECTOR3 const &B);


inline const JOVECTOR3 operator * (JOVECTOR3 const &A, freal const &B);
inline const JOVECTOR3 operator / (JOVECTOR3 const &A, freal const &B);
inline const JOVECTOR3 operator * (freal const &A, JOVECTOR3 const &B);
inline const JOVECTOR3 operator / (freal const &A, JOVECTOR3 const &B);




inline const JOVECTOR3 operator + (JOVECTOR3 const &A, JOVECTOR3 const &B)
{
	return JOVECTOR3(A.fX + B.fX, A.fY + B.fY, A.fZ + B.fZ);
}


inline const JOVECTOR3 operator - (JOVECTOR3 const &A, JOVECTOR3 const &B)
{
	return JOVECTOR3(A.fX - B.fX, A.fY - B.fY, A.fZ - B.fZ);

}

inline const freal operator * (JOVECTOR3 const &A, JOVECTOR3 const &B)
{
	return A.fX*B.fX + A.fY*B.fY + A.fZ*B.fZ;
}


inline const JOVECTOR3 operator * (JOVECTOR3 const &A, freal const &B)
{
	return JOVECTOR3(A.fX * B, A.fY * B, A.fZ * B);
}


inline const JOVECTOR3 operator * (freal const &A, JOVECTOR3 const &B)
{
	return JOVECTOR3(A * B.fX, A * B.fY, A * B.fZ);
}


inline const JOVECTOR3 operator / (JOVECTOR3 const &A, freal const &B)
{
	freal fOverB = 1.0f / B;
	return JOVECTOR3(A.fX * fOverB, A.fY * fOverB, A.fZ * fOverB);
}


inline const JOVECTOR3 operator / (freal const &A, JOVECTOR3 const &B)
{
	//freal fOverA = 1.0f / A;
	return JOVECTOR3(A * B.fX, A * B.fY, A * B.fZ);
}
//////////////////   End of JOVECTOR3   ///////////////////////////////

///////////////////  Statr of JOAMTRIX3 ///////////////////////////////
class JOMATRIX3
{

public:
    enum mode { SkewSymmetric, Zero };
	
	union {
			struct {
					freal  _11, _12, _13;
					freal  _21, _22, _23;
					freal  _31, _32, _33;
					};
	        freal fEle[3][3];
		};

	JOMATRIX3();
	JOMATRIX3(JOVECTOR3 &Vector, mode MustBeSkewSymmetric );
};

inline JOMATRIX3 operator+( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB);
inline JOMATRIX3 operator-( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB);

inline JOMATRIX3 operator*( freal const fValue, JOMATRIX3 const &JOMtr );
inline JOMATRIX3 operator*( JOMATRIX3 const &JOMtr, freal const fValue );
inline JOMATRIX3 operator*( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB);
inline JOVECTOR3 operator*( JOMATRIX3 const &JOMtr, JOVECTOR3 const &JOVec);
inline JOVECTOR3 operator*( JOVECTOR3 const &JOVec, JOMATRIX3 const &JOMtr);

inline JOMATRIX3 operator*( freal const fValue, JOMATRIX3 const &JOMtr )
{
	JOMATRIX3 ReMtr;

	for(int i = 0; i < 3; i++)
	{
		for(int k = 0; k < 3; k++)
		{
			  ReMtr.fEle[i][k] = fValue*JOMtr.fEle[i][k];
		}
	}

	return ReMtr;
}   


inline JOMATRIX3 operator*( JOMATRIX3 const &JOMtr, freal const fValue )
{
	JOMATRIX3 ReMtr;

	for(int i = 0; i < 3; i++) 
	{
		for(int k = 0; k < 3; k++) 
		{
			  ReMtr.fEle[i][k] = JOMtr.fEle[i][k]*fValue;
		}
	}

	return ReMtr;
}


inline JOMATRIX3 operator*( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB)
{
	JOMATRIX3 ReMtr;

	for(int i = 0; i < 3; i++) // Change first column
	{
		for(int k = 0; k < 3; k++)
		{
			ReMtr.fEle[i][k] = JOMtrA.fEle[i][0]* JOMtrB.fEle[0][k] +
				               JOMtrA.fEle[i][1]* JOMtrB.fEle[1][k] +
							   JOMtrA.fEle[i][2]* JOMtrB.fEle[2][k];
		}
	}

	return ReMtr;
}


inline JOVECTOR3 operator*( JOMATRIX3 const &JOMtr, JOVECTOR3 const &JOVec)
{
	return JOVECTOR3( JOVec.fX*JOMtr._11 + JOVec.fY*JOMtr._12 + JOVec.fZ*JOMtr._13,
		              JOVec.fX*JOMtr._21 + JOVec.fY*JOMtr._22 + JOVec.fZ*JOMtr._23,
					  JOVec.fX*JOMtr._31 + JOVec.fY*JOMtr._32 + JOVec.fZ*JOMtr._33);
}


inline JOVECTOR3 operator*( JOVECTOR3 const &JOVec, JOMATRIX3 const &JOMtr)
{
	return JOMtr*JOVec;

}


inline JOMATRIX3 operator+( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB)
{
	JOMATRIX3 ReMtr;

	for(int i = 0; i < 3; i++)
	{
		for(int k = 0; k < 3; k++)
		{
			ReMtr.fEle[i][k] = JOMtrA.fEle[i][k] + JOMtrB.fEle[i][k];
		}
	}

	return ReMtr;

}


inline JOMATRIX3 operator-( JOMATRIX3 const &JOMtrA, JOMATRIX3 const &JOMtrB)
{
	JOMATRIX3 ReMtr;

	for(int i = 0; i < 3; i++)
	{
		for(int k = 0; k < 3; k++)
		{
			ReMtr.fEle[i][k] = JOMtrA.fEle[i][k] - JOMtrB.fEle[i][k];
		}
	}

	return ReMtr;
}


/////////////////    End of   JOMATRIX3    ////////////////

/////////////////    Start of JOMATIRX4    ///////////////
class JOMATRIX4
{

public:
    union {
        struct {
				freal        _11, _12, _13, _14;
				freal        _21, _22, _23, _24;
				freal        _31, _32, _33, _34;
				freal        _41, _42, _43, _44;
		        };
        freal fEle[4][4];
	};

	JOMATRIX4();

};
///////////// End of JOMATIRX4 //////////////////////////////////////

///////////// Start of JOPLANE3 ////////////////////////////////


class JOPLANE3
{
public:
#define EPSILON 0.001f

enum POINTLOC {
		ptFront,
		ptBack,
		ptCoplanar

};
	JOVECTOR3 jvNormal;
	freal     fD;

	JOPLANE3();
	JOPLANE3(JOVECTOR3 const &A,  JOVECTOR3 const &B, JOVECTOR3 const &C);
	JOPLANE3(JOVECTOR3 const &N, JOVECTOR3 const &A);
	POINTLOC TestPoint(JOVECTOR3 const &A);
};


//////////// End of JOPLANE3 ///////////////////////////////////
///////// Start of global math function /////////////////////////

inline JOVECTOR3 CrossProduct(JOVECTOR3 const A, JOVECTOR3 const B);
inline freal DotProduct(JOVECTOR3 const A, JOVECTOR3 const B);
void   OrthonormalizeOrientation( JOMATRIX3 &Orientation );
inline JOMATRIX3 Transpose( JOMATRIX3 const &JOMtr );

float IntersectPointToLine(JOVECTOR3 pt, JOVECTOR3 p1, JOVECTOR3 p2, JOVECTOR3* closest);
bool IntersectSphereToCylinder(JOVECTOR3 comCylinder,  /*실린더의 위치*/
	JOVECTOR3 upCylinder, /*실린더의 노멀*/
	float heightCylinder, /*실린더의 길이*/
	float radiusToRadius/*실린더 반지름+공의반지름*/,
	JOVECTOR3 linestart,
	JOVECTOR3 lineend,
	JOVECTOR3* collisionPoint /*충돌되는 시작점*/
);

bool IntersectTriangle(JOVECTOR3 orig, JOVECTOR3 dir, JOVECTOR3 v0, JOVECTOR3 v1, JOVECTOR3 v2, float* U, float* V, float* T);

bool IntersectTriangle(JOVECTOR3 orig, JOVECTOR3 dir, JOVECTOR3 v0, JOVECTOR3 v1, JOVECTOR3 v2, float* U, float* V, float* T);
float GetDistanceToPointFromEquation(JOVECTOR3 normal/*?????풍???*/, JOVECTOR3 onPoint/*??????점*/, JOVECTOR3 toPoint/*?????? 점*/);

inline JOVECTOR3 CrossProduct(JOVECTOR3 const A, JOVECTOR3 const B)
{
	return JOVECTOR3(A.fY*B.fZ - A.fZ*B.fY, 
		             A.fZ*B.fX - A.fX*B.fZ, 
					 A.fX*B.fY - A.fY*B.fX);
}


inline freal DotProduct(JOVECTOR3 const A, JOVECTOR3 const B)
{
	return A.fX*B.fX + A.fY*B.fY + A.fZ*B.fZ;
}

inline freal Sqr( freal fV )
{
	return fV*fV;
}


inline freal Logtwo(float base, float x) 
{
	return freal(log(x) / log(base));

}



// ???? ???? 
inline bool QuardraticFormula( FREAL fA, FREAL fB, FREAL fC, FREAL &fOutRoot1, FREAL &fOutRoot2 )
{
	FREAL	fQ = fB*fB - 4.0f * fA * fC;
	if( fQ >= 0.0f )
	{
		FREAL	fSQ = static_cast<float>(sqrt(fQ));
		FREAL   fD  = 1.0f / (2.0f * fA);

		fOutRoot1   = (-fB + fSQ) * fD;
		fOutRoot2   = (-fB - fSQ) * fD;
		return true;
	}

	return false;
}


inline JOMATRIX3 Transpose( JOMATRIX3 const &JOMtr )
{
    JOMATRIX3 ReMtr;

    for(int i = 0; i < 3; i++)
    {
		ReMtr.fEle[0][i]  = JOMtr.fEle[i][0];
		ReMtr.fEle[1][i]  = JOMtr.fEle[i][1];
		ReMtr.fEle[2][i]  = JOMtr.fEle[i][2];
    }

    return ReMtr;
}  
/*
void Zero( JOVECTOR3 &A )
{
	A = JOVECTOR3( 0.0f, 0.0f, 0.0f );
}
*/

class JPS_API JOQUATERNION
{

public:
	static JOQUATERNION identity;
    JOQUATERNION() { v = JOVECTOR3(0.0f, 0.0f, 0.0f); w = 1.0f; }
	JOQUATERNION( float fw, float x, float y, float z ) 
	{
		v.fX = x;
		v.fY = y;
		v.fZ = z;
		w    = fw;
	}
	JOQUATERNION( JOVECTOR3 V, float fW ) 
	{
		v = V;
		w = fW;
	}
	~JOQUATERNION() { w = 1.0f; }

    // assignment operators
    JOQUATERNION& operator += ( const JOQUATERNION& Q );
    JOQUATERNION& operator -= ( const JOQUATERNION& Q );
//  JOQUATERNION& operator *= ( const JOQUATERNION& Q );
    JOQUATERNION& operator *= ( float fM );
    JOQUATERNION& operator /= ( float fS );

	JOVECTOR3		v; // ??칼??
	FREAL			w; // ???? 

	inline FREAL			Magnitude(void) 
	{
		return (FREAL)sqrt( w*w + v.fX*v.fX + v.fY*v.fY + v.fZ*v.fZ );
	}


public:
	static float quaternion4[4];
	//JOQUATERNION& Euler(JOVECTOR3 vector);

	static JOQUATERNION Rotate(const JOVECTOR3& axis, float angle)
	{
		angle = angle * PI / 180.0f;
		auto r = angle * 0.5f;
		auto scale = sin(r);
		auto a = axis * scale;

		return JOQUATERNION(cos(r), a.fX, a.fY, a.fZ);
	}
	
//    FREAL x, y, z, w;
};

//JOQUATERNION JOQUATERNION::Rotate(const JOVECTOR3& axis, float angle)


inline JOQUATERNION operator+( JOQUATERNION& A, JOQUATERNION& B )
{
	JOQUATERNION Q = JOQUATERNION( A.v+B.v, A.w+B.w );
	return Q;
}


inline JOQUATERNION operator-( JOQUATERNION& A, JOQUATERNION& B )
{
	JOQUATERNION Q = JOQUATERNION( A.v-B.v, A.w-B.w );
	return Q;

}


inline JOQUATERNION operator/( JOQUATERNION& A, JOQUATERNION& B )
{
	JOQUATERNION a;
	return a;
//	return JOQUATERNION( A/B, A.w/B.w );
}

inline JOVECTOR3 operator*( JOQUATERNION& A, JOVECTOR3& B )
{
	//JOQUATERNION Q = 
	//JOQUATERNION( -( A.v.fX*B.fX + A.v.fY*B.fY + A.v.fZ*B.fZ),
	//	                   A.w*B.fX + A.v.fY*B.fZ - A.v.fZ*B.fY,
	//					   A.w*B.fY + A.v.fZ*B.fX - A.v.fX*B.fZ,
	//					   A.w*B.fZ + A.v.fX*B.fY - A.v.fY*B.fX );
	const float x = B.fX;
	const float y = B.fY;
	const float z = B.fZ;
	// Quaternion
	const float qx = A.v.fX;
	const float qy = A.v.fY;
	const float qz = A.v.fZ;
	const float qw = A.w;
	// Quaternion * Vector
	const float ix = qw * x + qy * z - qz * y;
	const float iy = qw * y + qz * x - qx * z;
	const float iz = qw * z + qx * y - qy * x;
	const float iw = -qx * x - qy * y - qz * z;
	// Final Quaternion * Vector = Result
	JOVECTOR3 result;
	result.fX = ix * qw + iw * -qx + iy * -qz - iz * -qy;
	result.fY = iy * qw + iw * -qy + iz * -qx - ix * -qz;
	result.fZ = iz * qw + iw * -qz + ix * -qy - iy * -qx;
	return result;
}


inline JOQUATERNION operator*( JOQUATERNION& A, JOQUATERNION& B)
{
	//JOQUATERNION a;
	//return a;
	return JOQUATERNION(A.w*B.w - A.v.fX*B.v.fX - A.v.fY*B.v.fY - A.v.fZ*B.v.fZ,
		                A.w*B.v.fX + A.v.fX*B.w + A.v.fY*B.v.fZ - A.v.fZ*B.v.fY);
}



inline JOQUATERNION operator*( JOQUATERNION& A, float fM )
{
	JOQUATERNION Q = JOQUATERNION( A.v*fM, A.w*fM );
	return Q;
}



inline JOQUATERNION operator*( JOVECTOR3& A, JOQUATERNION& B )
{
	JOQUATERNION Q = 
	JOQUATERNION( -( B.v.fX*A.fX + B.v.fY*A.fY + B.v.fZ*A.fZ),
		                   B.w*A.fX + B.v.fZ*A.fY - B.v.fY*A.fZ,
						   B.w*A.fY + B.v.fX*A.fZ - B.v.fZ*A.fX,
						   B.w*A.fZ + B.v.fY*A.fX - B.v.fX*A.fY );

	return Q;
}

////////////// End of global math function ///////////////////////
#endif
