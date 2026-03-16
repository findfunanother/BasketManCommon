//#pragma once
//
//#include <string>
//#include <vector>
//#include <map>
//
//#include "CSVLoader.h"
////#include "../../Data/ShotSolutionTable.h"
//#include "Common_generated.h"
//#include "F4Packet_generated.h"
//
//namespace TB {
//	struct F4SolutionSaveDataList;
//	struct F4SolutionSaveData;
//	struct F4SolutionTargetData;
//}
//namespace ShotSolution
//{
//
//#define MAX_BOUNDARY_ANGLE_CNT 9
//#define SOLUTION_BOUNDARY_ANGLE 5
//#define MAX_BOUNDARY_DISTANCE_CNT 4
//#define MAX_DISTANCE 12
//#define MAX_SECTOR_COUNT 13			// 섹터의 숫자 : 31개 고정( 4 x 3 + 1)
//#define MAX_BOUND_TYPE 8			// 튕겨나가는 종류
//	
//	class CShotSolution 
//	{
//	public:
//		typedef struct _ShotSolutionData
//		{
//			F4PACKET::SHOT_THROW throwType;
//			int sectorNumber;
//			F4PACKET::SHOT_BOUND boundType;
//			float reachTime;
//			TB::SVector3 position;
//			TB::SVector3 velocityAtTarget;
//			std::string shotEvent;
//
//		} ShotSolutionData;
//
//	public:
//		CShotSolution();
//		~CShotSolution();
//
//		void Initialize(TB::F4SolutionSaveDataList* table);
//
//		int GetSectorNumber(TB::SVector3 position, TB::SVector3 center);
//		int GetCount(unsigned int key);
//		unsigned int GetPointDataKey(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType);
//		
//		const TB::F4SolutionTargetData* FindTargetDataInDictionaryIndex(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType, int index);
//
//	protected:
//		void InitData(TB::F4SolutionSaveDataList* datas);
//		vector<const TB::F4SolutionTargetData*>* FindTargetListInMap(unsigned int key);
//
//	protected:
//		map<unsigned int, vector<const TB::F4SolutionTargetData*>> m_TargetDataMap;
//		
//		float m_SectorBoundaryAngle[MAX_BOUNDARY_ANGLE_CNT] = { 0, 90, 115, 150, 180, 210, 245, 270, 360 };
//		float m_SectorBoundaryDistance[MAX_BOUNDARY_DISTANCE_CNT] = { 0.0f, 2, 6, MAX_DISTANCE };
//	};
//}