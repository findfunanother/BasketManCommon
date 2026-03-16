#pragma once
#include <iostream> // C++ header file for printing
#include <fstream>
#include <string>
//#include <windows.h>
#include <map>
#include "../Physics/JOMath3d.h"
#include "Common_generated.h"
#define FILE_MAX_PATH 256

#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define GAMECORE_DECLSPEC __declspec(dllexport)
#else
#define GAMECORE_DECLSPEC __declspec(dllimport) 
#endif

#define DLLOCAL
#else
#define GAMECORE_DECLSPEC __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif


using namespace std;


//extern "C"
namespace TB
{
	struct TBAnimationInformation;
	struct TBAnimationCurve;
	struct TBQuaternionCurve;
	struct TBVector3Curve;
	struct TBKeyframe;
}

using namespace TB;
class GAMECORE_DECLSPEC CAnimationController
{
	std::map<int, const TBAnimationInformation*>* m_Animinformation;
	std::string GetCurrentDirectoryStr();

	bool ReadFile(std::string filePath, unsigned char** _data, int* datalen);
	unsigned char* m_data;// = NULL;
	int m_DataLength;
public:
	unsigned char* GetData() { return m_data; }
	int GetDataLength() { return m_DataLength; }

private:
	inline const TBAnimationInformation* GetAnimationInformation(int _nameanim);
	inline static double Evaluate(const TBAnimationCurve* curve, float t);
	inline static double EvaluateFrame(float t, const TBKeyframe* keyframe0, const TB::TBKeyframe* keyframe1);
	inline static JOQUATERNION CalculateLocalRotation(const TBQuaternionCurve* quaternioninfo, float time);
	inline static JOVECTOR3 CalculateLocalPosition(const TBVector3Curve* animcurve, float time);
	inline static JOVECTOR3 CalculateLocalPositionEx(const TBVector3Curve* animcurve, float time, float Pretime);
	inline static JOVECTOR3 CalculateLocalPositionZero(const TBVector3Curve* animcurve, float time, float Pretime);

public:
	void ReadBinary();
	CAnimationController();
	~CAnimationController();
	float GetAnimationDuration(int _nameanim);
	bool IsEventExist(int, string nameKey);
	float GetEventTime(int nameAnim, string nameKey);
	bool GetBallLocalPosition(int nameAnim, float time, bool mirror, bool calculateRoot, JOVECTOR3& position);
	JOVECTOR3 CalculateSlide(int nameAnim, float timeEvent, float timeoffset, bool mirror, const JOVECTOR3& positionStart, const JOVECTOR3& positionTarget, float yaw);
	JOVECTOR3 CalculateSlideWithLocalPosition(const JOVECTOR3& positionStart, const JOVECTOR3& positionTarget, float yaw, JOVECTOR3 localPosition);
	//JOVECTOR3 CalculateBallLocalPosition(const TBAnimationInformation* animationInformation, float time, bool mirror, bool calculateRoot);
	JOQUATERNION GetBallLocalRotation(int nameAnim, float time, bool mirror);
	const bool LoadData(const char* filename);
	bool GetRootLocalInformation(int nameAnim, float time, float timeoffset, bool mirror, JOVECTOR3& position, JOQUATERNION& rotation);
	bool GetRootLocalPosition(int nameAnim, float time, float timeoffset, bool mirror, JOVECTOR3& position);
	
	bool GetRootLocalPositionEx(int nameAnim, float time, float Pretime, bool mirror, JOVECTOR3& position);
	bool GetRootLocalPositionZero(int nameAnim, float time, float Pretime, bool mirror, JOVECTOR3& position);
	JOVECTOR3 CalculateRootLocalPosition(const TBAnimationInformation* animationInformation, float time, float timeoffset, bool mirror);
	JOVECTOR3 CalculateRotationRootLogic(JOVECTOR3& movedRoot, float yaw);


	const TBAnimationInformation* FindAnimationInfo(int name);
	const char* GetAnimationName(int index);
	int FindAnimationInfo(string name);

	//! 애니메이션 데이터 유효성 검사
	int CheckAnimationData(int nameAnim);
	int CheckInsertAnimationData(const TBAnimationInformation* pInfo);
	size_t GetAnimationInfoSize();
	const char* ValidationAnimationInfo(int idx);
	void GetIndexName(int idx, string& str);
};

