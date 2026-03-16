#pragma once
#pragma warning(disable: 4251)
#include "F4Packet_generated.h"
#include "AnimationInformation_generated.h"
//#ifdef GAMECORE_EXPORT_DLL // this is predefined in DLL1
//// Avoid duplicate instantation within DLL1, except for inheritance.
//template class CDataManager<int, TB::F4SolutionSaveDataList>;
//#else
//// Provide instantiation for other DLLs.
//template class DECLSPECT CDataManager<int, TB::F4SolutionSaveDataList>;
//#endif
//namespace TB
//{
//	struct F4SolutionSaveDataList;
//	struct Vector3Data;
//}

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

template<typename> class CDataController;

class GAMECORE_DECLSPEC CDataManagerShotSolution
{
	
protected:
	CDataController<TB::F4SolutionSaveDataList>* m_dataController;
public:

	CDataManagerShotSolution();
	virtual  ~CDataManagerShotSolution();
	int  GetSectorNumber(TB::SVector3 position, TB::SVector3 center);
	int  GetCount(unsigned int key);
	unsigned int GetPointDataKey(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType);

	const TB::F4SolutionTargetData* FindTargetDataInDictionaryIndex(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType, int index);
	bool HasData();
protected:
	vector<const TB::F4SolutionTargetData*>* FindTargetListInMap(unsigned int key);
	map<unsigned int, vector<const TB::F4SolutionTargetData*>*> m_TargetDataMap;
	void CreateDictionary();
	
	
};

