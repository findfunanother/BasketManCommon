//
//#include "ShotSolution.h"
//#include "ResourceCppHeader.h"
//
//#include <stdint.h>
//#include <string>
//#include <iostream>
//
//#include "AnimationInformation_generated.h"
//
//#define VECTOR3_DISTANCE(a, b) ( sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()) + (a.z() - b.z()) * (a.z() - b.z())) )
//#define VECTOR3_MAGNITUDE(a) ( sqrt(a.x() * a.x() + a.y() * a.y() + a.z() * a.z()) )
//#define VECTOR3_SCALE(a, b) { a.mutate_x(a.x() / b); a.mutate_y(a.y() / b); a.mutate_z(a.z() / b); }
//#define VECTOR3_DOT(a, b) ( a.x() * b.x() + a.y() * b.y() + a.z() * b.z() )
//#define VECTOR3_CROSS(b, c) b.y() * c.z() - b.z() * c.y(), b.z() * c.x() - b.x() * c.z(), b.x() * c.y() - b.y() * c.x()
//#define RAD2DEG 57.29578F
//
//namespace ShotSolution
//{
//	CShotSolution::CShotSolution()
//	{
//	}
//
//	CShotSolution::~CShotSolution()
//	{
//		m_TargetDataMap.clear();
//	}
//
//	void CShotSolution::Initialize(TB::F4SolutionSaveDataList* table)
//	{
//		InitData(table);
//	}
//
//	int CShotSolution::GetSectorNumber(TB::SVector3 vecInputPosition, TB::SVector3 center)
//	{
//		if (vecInputPosition.x() > 0.0f) vecInputPosition.mutate_x(-vecInputPosition.x());
//
//		vecInputPosition.mutate_y(0.0f);
//		center.mutate_y(0.0f);
//
//		TB::SVector3 vecStandDir;
//		vecStandDir.mutate_x(0.0f);
//		vecStandDir.mutate_y(0.0f);
//		vecStandDir.mutate_z(1.0f);
//		
//		TB::SVector3 vecDir;
//		vecDir.mutate_x(vecInputPosition.x() - center.x());
//		vecDir.mutate_y(vecInputPosition.y() - center.y());
//		vecDir.mutate_z(vecInputPosition.z() - center.z());
//		
//		float fLength = VECTOR3_MAGNITUDE(vecDir);
//		VECTOR3_SCALE(vecDir, fLength);
//
//		float fReal = VECTOR3_DOT(vecDir, vecStandDir);
//		
//		TB::SVector3 vecCross;
//		VECTOR3_CROSS(vecCross, vecStandDir, vecDir);
//
//		float angle = RAD2DEG * acos(fReal);
//
//		if (vecCross.y() > 0.0f) { angle = 360.0f - angle; }
//
//		int nSectorNum = 0;
//
//		for (int i = 1; i < SOLUTION_BOUNDARY_ANGLE; i++)
//		{
//			if (angle >= m_SectorBoundaryAngle[i - 1] && angle <= m_SectorBoundaryAngle[i])
//			{
//				for (int k = 0; k < MAX_BOUNDARY_DISTANCE_CNT - 1; k++)
//				{
//					if (fLength > m_SectorBoundaryDistance[k] && fLength < m_SectorBoundaryDistance[k + 1])
//					{
//						nSectorNum = k * (SOLUTION_BOUNDARY_ANGLE - 1) + i;
//					}
//
//					if (fLength > MAX_DISTANCE) // 영역 초과시 같은 라인의 맨 마지막 섹터 
//					{
//						nSectorNum = (MAX_BOUNDARY_DISTANCE_CNT - 2) * (SOLUTION_BOUNDARY_ANGLE - 1) + i;
//					}
//				}
//			}
//		}
//
//		return nSectorNum; // 0을 리턴하면 없다 
//	}
//
//	void CShotSolution::InitData(TB::F4SolutionSaveDataList* datas)
//	{
//		int countSol = datas->datas()->Length();
//		for (int i = 0; i < countSol; i++)
//		{
//			auto data = datas->datas()->Get(i);
//			//if (pTargetData->eventName == "")
//			//{
//			//	//Debug.LogError("Event is Null");
//			//	pTargetData->eventName = "Event_FirstRimBound";
//			//}
//
//			unsigned int key = GetPointDataKey((F4PACKET::SHOT_THROW)data->throwtype(), data->sectornum(), (F4PACKET::SHOT_BOUND)data->boundtype());
//
//			auto find = m_TargetDataMap.find(key);
//
//			if (find == m_TargetDataMap.end())
//			{
//				vector<const TB::F4SolutionTargetData*> newTargetData;
//
//				m_TargetDataMap.insert(pair<unsigned int, vector<const TB::F4SolutionTargetData*>>(key, newTargetData));
//			}
//
//			//data->index() = static_cast<int>(m_TargetDataMap[key].size());
//			m_TargetDataMap[key].push_back(data->targetdata());
//		}
//	}
//
//	unsigned int CShotSolution::GetPointDataKey(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType)
//	{
//		return (unsigned int)throwType * (MAX_SECTOR_COUNT * MAX_BOUND_TYPE) + (unsigned int)sectorNum * MAX_BOUND_TYPE + (unsigned int)boundType;
//	}
//
//	int CShotSolution::GetCount(unsigned int key)
//	{
//		auto find = m_TargetDataMap.find(key);
//
//		if (find != m_TargetDataMap.end())
//		{
//			return static_cast<int>(m_TargetDataMap[key].size());
//		}
//
//		return 0;
//	}
//
//	vector<const TB::F4SolutionTargetData*>* CShotSolution::FindTargetListInMap(unsigned int key)
//	{
//		map<unsigned int, vector<const TB::F4SolutionTargetData*>>::iterator find = m_TargetDataMap.find(key);
//
//		if (find != m_TargetDataMap.end())
//		{
//			return &find->second;// find->second;
//		}
//
//		return nullptr;
//	}
//
//	const TB::F4SolutionTargetData* CShotSolution::FindTargetDataInDictionaryIndex(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType, int index)
//	{
//		unsigned int key = GetPointDataKey(throwType, sectorNum, boundType);
//
//		vector<const TB::F4SolutionTargetData*>* pReleaseDataList = FindTargetListInMap(key);
//
//		if (pReleaseDataList != nullptr)
//		{
//			for (int i = 0; i < pReleaseDataList->size(); i++)
//			{
//				if (i == index)
//				{
//					return (*pReleaseDataList)[index];
//				}
//			}
//			return nullptr;
//		}
//		else
//		{
//			return nullptr;
//		}
//	}
//
//
//}
