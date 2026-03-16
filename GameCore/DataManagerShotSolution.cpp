
#include <cmath>
using std::sqrt;
using std::acos;


#define MAX_BOUNDARY_ANGLE_CNT 9
#define SOLUTION_BOUNDARY_ANGLE 5
#define MAX_BOUNDARY_DISTANCE_CNT 5
#define MAX_DISTANCE 12
#define MAX_SECTOR_COUNT 17			// ������ ���� : 31�� ����( 4 x 4 + 1)
#define MAX_BOUND_TYPE 8			// ƨ�ܳ����� ����


#define VECTOR3_DISTANCE(a, b) ( sqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()) + (a.z() - b.z()) * (a.z() - b.z())) )
#define VECTOR3_MAGNITUDE(a) ( sqrt(a.x() * a.x() + a.y() * a.y() + a.z() * a.z()) )
#define VECTOR3_SCALE(a, b) { a.set_x(a.x() / b); a.set_y(a.y() / b); a.set_z(a.z() / b); }
#define VECTOR3_DOT(a, b) ( a.x() * b.x() + a.y() * b.y() + a.z() * b.z() )
#define VECTOR3_CROSS(b, c) b.y() * c.z() - b.z() * c.y(), b.z() * c.x() - b.x() * c.z(), b.x() * c.y() - b.y() * c.x()
#define RAD2DEG 57.29578F


#include "DataController.h"
#include "DataManagerShotSolution.h"
#include "AnimationInformation_generated.h"
using namespace TB;
using namespace std;
float m_SectorBoundaryAngle[MAX_BOUNDARY_ANGLE_CNT] = { 0, 90, 115, 150, 180, 210, 245, 270, 360 };
float m_SectorBoundaryDistance[MAX_BOUNDARY_DISTANCE_CNT] = { 0.0f, 2, 6, 8, MAX_DISTANCE };

CDataManagerShotSolution::CDataManagerShotSolution()
{
	m_dataController = new CDataController<TB::F4SolutionSaveDataList>();
	m_dataController->CreateData("ShotSolutions");
	CreateDictionary();
}

CDataManagerShotSolution::~CDataManagerShotSolution()
{
	std::map<unsigned int, vector<const TB::F4SolutionTargetData*>*>::iterator it;
	for (it = m_TargetDataMap.begin(); it != m_TargetDataMap.end(); ++it) {
		it->second->clear();
		delete(it->second);
	}
	delete(m_dataController);
}

void CDataManagerShotSolution::CreateDictionary()
{
	if (!m_dataController)
		return;

	auto list = m_dataController->GetData();
	if (!list)
		return;

	auto datas = list->datas();
	if (!datas)
		return;

	for (flatbuffers::uoffset_t i = 0; i < datas->size(); i++)
	{
		auto data = datas->Get(i);
		if (!data || !data->targetdata())
			continue;

		
		unsigned int key = GetPointDataKey(data->throwtype(), data->sectornum(), data->boundtype());

		auto find = m_TargetDataMap.find(key);

		if (find == m_TargetDataMap.end())
		{
			vector<const TB::F4SolutionTargetData*>* newTargetData = new vector<const TB::F4SolutionTargetData*>();

			m_TargetDataMap.insert(pair<unsigned int, vector<const TB::F4SolutionTargetData*>*>(key, newTargetData));
		}
		else
		{

		}


		m_TargetDataMap[key]->push_back((const TB::F4SolutionTargetData*)data->targetdata());
	}
}

bool CDataManagerShotSolution::HasData()
{
	return m_TargetDataMap.size() > 0;
}

int CDataManagerShotSolution::GetSectorNumber(TB::SVector3 vecInputPosition, TB::SVector3 center)
{
	

	TB::SVector3 vecStandDir(0.f, 0.f, 1.f);
	
	TB::SVector3 vecDir(((vecInputPosition.x() > 0.0f) ? -vecInputPosition.x() : vecInputPosition.x()) - center.x(),
							0.f,
							vecInputPosition.z() - center.z());
	
	float fLength = VECTOR3_MAGNITUDE(vecDir);
	TB::SVector3 vecDirScaled(vecDir.x()/fLength, vecDir.y() / fLength, vecDir.z() / fLength);

	float fReal = VECTOR3_DOT(vecDirScaled, vecStandDir);

	TB::SVector3 vecCross(VECTOR3_CROSS(vecStandDir, vecDirScaled));
	

	float angle = RAD2DEG * acos(fReal);

	if (vecCross.y() > 0.0f) { angle = 360.0f - angle; }

	int nSectorNum = 0;

	for (int i = 1; i < SOLUTION_BOUNDARY_ANGLE; i++)
	{
		if (angle >= m_SectorBoundaryAngle[i - 1] && angle <= m_SectorBoundaryAngle[i])
		{
			for (int k = 0; k < MAX_BOUNDARY_DISTANCE_CNT - 1; k++)
			{
				if (fLength > m_SectorBoundaryDistance[k] && fLength < m_SectorBoundaryDistance[k + 1])
				{
					nSectorNum = k * (SOLUTION_BOUNDARY_ANGLE - 1) + i;
				}

				if (fLength > MAX_DISTANCE) // ���� �ʰ��� ���� ������ �� ������ ���� 
				{
					nSectorNum = (MAX_BOUNDARY_DISTANCE_CNT - 2) * (SOLUTION_BOUNDARY_ANGLE - 1) + i;
				}
			}
		}
	}

	return nSectorNum; // 0�� �����ϸ� ���� 
}

int CDataManagerShotSolution::GetCount(unsigned int key)
{
	auto find = m_TargetDataMap.find(key);

	if (find != m_TargetDataMap.end())
	{
		return static_cast<int>(m_TargetDataMap[key]->size());
	}

	return 0;
}

unsigned int CDataManagerShotSolution::GetPointDataKey(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType)
{
	return (unsigned int)throwType * (MAX_SECTOR_COUNT * MAX_BOUND_TYPE) + (unsigned int)sectorNum * MAX_BOUND_TYPE + (unsigned int)boundType;
}

const TB::F4SolutionTargetData* CDataManagerShotSolution::FindTargetDataInDictionaryIndex(F4PACKET::SHOT_THROW throwType, int sectorNum, F4PACKET::SHOT_BOUND boundType, int index)
{
	unsigned int key = GetPointDataKey(throwType, sectorNum, boundType);

	vector<const TB::F4SolutionTargetData*>* pReleaseDataList = FindTargetListInMap(key);

	if (pReleaseDataList != nullptr)
	{
		if (pReleaseDataList->size() > index)
		{
			return (*pReleaseDataList)[index];
		}
		return nullptr;
	}
	else
	{
		return nullptr;
	}
}

vector<const TB::F4SolutionTargetData*>* CDataManagerShotSolution::FindTargetListInMap(unsigned int key)
{
	map<unsigned int, vector<const TB::F4SolutionTargetData*>*>::iterator find = m_TargetDataMap.find(key);

	if (find != m_TargetDataMap.end())
	{
		return find->second;
	}

	return nullptr;
}
