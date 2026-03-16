#define NOMINMAX

#include "VerifyManager.h"
#include "CharacterManager.h"
#include "Host.h"
#include "DataManagerBalance.h"
#include "Lucky.h"
#include "DefaultCharacter.h"
#include "Olga.h"
#include "Aldo.h"
#include "Ling.h"
#include "Aimar.h"
#include "Sarang.h"
#include "Uncle.h"
#include "Qiuayanyu.h"
#include "Lily.h"
#include "Luyao.h"
#include "Duke.h"
#include "Norma.h"
#include "Wuko.h"
#include "Tina.h"
#include "Ivan.h"
#include "Lia.h"
#include "Niky.h"
#include "Mina.h"
#include "Kyle.h"
#include "Kent.h"
#include "Sam.h"
#include "Flores.h"
#include "LinCC.h"
#include "Thompson.h"
#include "Qiqi.h"
#include "Kido.h"
#include "Xiao.h"
#include "Momo.h"
#include "Okafor.h"


CCharacterManager::CCharacterManager()
{
	m_mapCharacter.clear();
	m_vecTeamsCharacter.clear();
	m_GameMVP = kLAST_CHARACTER_SN;
	m_LastCharacterSN = kLAST_CHARACTER_SN;
	m_bNoTableCharacterSend = false;
	m_vecMatchInfos.clear();
	m_SwitchIDHome = kINT32_INIT;
	m_SwitchIDAway = kINT32_INIT;
	m_teamsActive[0] = 0;
	m_teamsActive[1] = 1;
	m_pBalanceData = nullptr;
	m_pOwnerAnimationController = nullptr;
	m_pOwnerBallController = nullptr;
	m_pMannerManager = nullptr;
	m_CallbackBadManner = nullptr;
	m_CallbackOnFireMode = nullptr;
	m_TeamIllegalScreen.clear();

	m_mapTeamSurrenderCheck.clear();
	m_mapTeamSurrenderGameEndTime.clear();
	m_mapTeamSurrenderVoteInfo.clear();
	m_mapTeamSurrenderVoteFail.clear();
}

CCharacterManager::~CCharacterManager()
{
	if (m_mapCharacter.empty() == false)
	{
		for (auto& it : m_mapCharacter)
		{
			auto pCharacter = it.second;

			SAFE_DELETE(pCharacter);
		}
		m_mapCharacter.clear();
	}

	for (int i = 0; i < m_vecTeamsCharacter.size(); i++)
	{
		m_vecTeamsCharacter[i].clear();
	}

	m_vecTeamsCharacter.clear();

	if (m_pMannerManager != nullptr)
	{
		SAFE_DELETE(m_pMannerManager);
	}
}

void CCharacterManager::CharacterManagerInit(CHost* pHost, CDataManagerBalance* pInfo, CAnimationController* pAniInfo, CBallController* pBallInfo)
{
	m_pHost = pHost;
	m_pBalanceData = pInfo;
	m_pOwnerAnimationController = pAniInfo;
	m_pOwnerBallController = pBallInfo;
	m_pMannerManager = new CMannerManager(this);
}

DHOST_TYPE_BOOL CCharacterManager::CreateCharacter(F4PACKET::SPlayerInformationT* pInfo, DHOST_TYPE_UINT32 playerNumber, F4PACKET::ECHARACTER_INDEX index)
{
	CCharacter* pCharacter = nullptr;
	//DHOST_TYPE_FLOAT luckyMax = kFLOAT_INIT;
		switch (index)
	{
		case F4PACKET::ECHARACTER_INDEX::Aimar:
			pCharacter = new CAimar(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_JumpShotProbability_Aimar"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Sarang:
			pCharacter = new CSarang(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Sarang2"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Uncle:
			pCharacter = new CUncle(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Uncle"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Qiuyanyu:
			pCharacter = new CQiuayanyu(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_JumpShotProbability_Qiuyanyu"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Lily:
			pCharacter = new CLily(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Lily_Max"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Luyao:
			pCharacter = new CLuyao(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
			 				m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Luyao") * m_pBalanceData->GetValue("VPassive_ThreePointShotProbability_Luyao_Max"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Duke:
			pCharacter = new CDuke(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;

		case F4PACKET::ECHARACTER_INDEX::Norma:
			pCharacter = new CNorma(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_JumpShotProbability_Norma2"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Wuko:
			pCharacter = new CWuko(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Wuko"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Tina:
			pCharacter = new CTina(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_JumpShotProbability_Tina"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Ivan:
			pCharacter = new CIvan(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, m_pBalanceData->GetValue("VPassive_JumpShotProbability_Ivan"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Lucky:
			pCharacter = new CLucky(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
									m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_Max"), 
									m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_2"),
									m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lucky_3"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Olga:

			pCharacter = new COlga(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_JumpShotProbability_Olga_Stack"));   // "Passive_Aldo_PassReceive_Range2"
			break;

		case F4PACKET::ECHARACTER_INDEX::Aldo:
			pCharacter = new CAldo(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
			m_pBalanceData->GetValue("VPassive_JumpShotProbability_Aldo2"));   // "0.15"
			break;

		case F4PACKET::ECHARACTER_INDEX::Ling:
			pCharacter = new CLing(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
			m_pBalanceData->GetValue("VPassive_MidRangeShotProbability_Ling_Stack"));   // 0.35f
			break;

		case F4PACKET::ECHARACTER_INDEX::Lia:
			pCharacter = new CLia(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
		    m_pBalanceData->GetValue("VPassive_JumpShotProbability_Lia"), m_pBalanceData->GetValue("VPassive_Lia_Time"));   // 0.35f
			break;

		case F4PACKET::ECHARACTER_INDEX::Nicky:
			pCharacter = new CNiky(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_JumpShotProbability_Niky"), m_pBalanceData->GetValue("VPassive_Niky_Time"));   // 0.35f
			break;

		case F4PACKET::ECHARACTER_INDEX::Mina:
			pCharacter = new CMina(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_JumpShotProbability_Niky"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Kyle:
			pCharacter = new CKyle(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
			m_pBalanceData->GetValue("VPassive_Kyle_Stack_Max"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Kent:
			pCharacter = new CKent(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
			m_pBalanceData->GetValue("VPassive_BlockCondition_Probability_Kent_Max"));
			break;

		case F4PACKET::ECHARACTER_INDEX::Sam:
			pCharacter = new CSam(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_ShotProbability_Sam"), m_pBalanceData->GetValue("VPassive_Sam_Time"));   
			break;

		case F4PACKET::ECHARACTER_INDEX::Flores:
			pCharacter = new CFlores(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_ShotProbability_Flores"), m_pBalanceData->GetValue("VPassive_Flores_Time"));   
			break;

		case F4PACKET::ECHARACTER_INDEX::LinCC:
			pCharacter = new CLinCC(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_Stamina_LinCC_Max"));   
			break;

		case F4PACKET::ECHARACTER_INDEX::Okafor:
			pCharacter = new COkafor(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber,
				m_pBalanceData->GetValue("VPassive_Okafor_Stack_Max"));
			break;

			///
		case F4PACKET::ECHARACTER_INDEX::Thompson:
			pCharacter = new CThompson(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;

		case F4PACKET::ECHARACTER_INDEX::Qiqi:
			pCharacter = new CQiqi(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;

		case F4PACKET::ECHARACTER_INDEX::Kido:
			pCharacter = new CKido(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;

		case F4PACKET::ECHARACTER_INDEX::Xiao:
			pCharacter = new CKido(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;

		case F4PACKET::ECHARACTER_INDEX::Momo:
			pCharacter = new CMomo(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);
			break;


		default:
			pCharacter = new CDefaultCharacter(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber, 0.0f); //SHOT_BONUS_PASSIVE_MAX);
			break;
	}

	DHOST_TYPE_FLOAT min = m_pBalanceData->GetValue("VBurst_Start_Min");
	DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VBurst_Start_Max");

	uint32_t random_start = Util::GetRandom(static_cast<uint32_t>(min), static_cast<uint32_t>(max));

	if (pCharacter != nullptr)
	{
		AddTeamIllegalScreen(pInfo->team);
		AddTeamSurrenderCheck(pInfo->team);

		pCharacter->SetBurstGauge(static_cast<DHOST_TYPE_FLOAT>(random_start));
		pCharacter->SetBurstGauge(pInfo->burstvalue);

		if (AddCharacterMap(pCharacter))
		{
			return true;
		}

		SAFE_DELETE(pCharacter);
	}

	return false;
}

// 챌린지 와 튜토리얼용 
DHOST_TYPE_BOOL CCharacterManager::CreateCharacter(F4PACKET::SPlayerInformationT* pInfo, DHOST_TYPE_UINT32 playerNumber)
{
	CCharacter* pCharacter = new CCharacter(m_pHost, m_pBalanceData, pInfo, m_pOwnerAnimationController, m_pOwnerBallController, playerNumber);

	DHOST_TYPE_FLOAT min = m_pBalanceData->GetValue("VBurst_Start_Min");
	DHOST_TYPE_FLOAT max = m_pBalanceData->GetValue("VBurst_Start_Max");

	uint32_t random_start = Util::GetRandom(static_cast<uint32_t>(min), static_cast<uint32_t>(max));

	if (pCharacter != nullptr)
	{
		AddTeamIllegalScreen(pInfo->team);
		AddTeamSurrenderCheck(pInfo->team);

		pCharacter->SetBurstGauge(static_cast<DHOST_TYPE_FLOAT>(random_start));

		if (AddCharacterMap(pCharacter))
		{
			return true;
		}

		SAFE_DELETE(pCharacter);
	}

	return false;
}

DHOST_TYPE_BOOL CCharacterManager::AddCharacterMap(CCharacter* pInfo)
{
	auto result = m_mapCharacter.insert(std::pair<DHOST_TYPE_CHARACTER_SN, CCharacter*>(pInfo->GetCharacterInformation()->id, pInfo));

	if (result.second == true)
	{
		return AddTeamCharacterMap(pInfo);
	}

	return false;
}

DHOST_TYPE_BOOL CCharacterManager::AddTeamCharacterMap(CCharacter* pInfo)
{
	while (m_vecTeamsCharacter.size() <= pInfo->GetCharacterInformation()->team)
	{
		m_vecTeamsCharacter.push_back(std::vector<CCharacter*>());
	}

	m_vecTeamsCharacter[pInfo->GetCharacterInformation()->team].push_back(pInfo);

	return true;
}

DHOST_TYPE_BOOL CCharacterManager::GetTeamCharacterMap(DHOST_TYPE_INT32 teamIndex)
{
	try
	{
		auto data = m_vecTeamsCharacter.at(teamIndex);
	}
	catch (std::exception& e)
	{
#ifdef TEXT_LOG_FILE
		LOGGER->Log("[EXCEPTION] CCharacterManager::GetTeamCharacterMap teamIndex : %d, exception : %s", teamIndex, e.what());
#endif
		return false;
	}


	return true;
}

void CCharacterManager::SetCharacterUserID(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_USER_ID value)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterUserID(value);
	}
}

void CCharacterManager::SortCharacters()
{
	for (int i = 0; i < m_vecTeamsCharacter.size(); i++)
	{
		std::sort(m_vecTeamsCharacter[i].begin(), m_vecTeamsCharacter[i].end(), Compare);
	}
}

void CCharacterManager::SortCharactersWithRole()
{
	for (int i = 0; i < m_vecTeamsCharacter.size(); i++)
	{
		if (m_vecTeamsCharacter[i].size() > 1)
		{
			std::vector<CCharacter*>::iterator iterBegin = ++m_vecTeamsCharacter[i].begin();
			std::vector<CCharacter*>::iterator iterEnd = m_vecTeamsCharacter[i].end();

			std::sort(iterBegin, iterEnd, CompareWithRole);
		}
	}
}

bool CCharacterManager::Compare(CCharacter* a, CCharacter* b)
{
	//! �����ǿ� ������� ���ٿ�� �ɷ�ġ�� ���� ���������� ����
	if (a->GetCharacterInformation()->ability->rebound == b->GetCharacterInformation()->ability->rebound)
	{
		if (a->GetCharacterInformation()->ability->vertical == b->GetCharacterInformation()->ability->vertical)
		{
			return a->GetCharacterInformation()->userid < b->GetCharacterInformation()->userid;
		}
		else
		{
			return a->GetCharacterInformation()->ability->vertical > b->GetCharacterInformation()->ability->vertical;
		}
	}
	else
	{
		return a->GetCharacterInformation()->ability->rebound > b->GetCharacterInformation()->ability->rebound;
	}
	
	/*if (a->GetCharacterInformation()->role == b->GetCharacterInformation()->role)
	{
		if (a->GetCharacterInformation()->ability->rebound == b->GetCharacterInformation()->ability->rebound)
		{
			return a->GetCharacterInformation()->ability->rebound > b->GetCharacterInformation()->ability->rebound;
		}
		else
		{
			return a->GetCharacterInformation()->userid < b->GetCharacterInformation()->userid;
		}
	}
	else
	{
		return a->GetCharacterInformation()->role < b->GetCharacterInformation()->role;
	}*/
}

bool CCharacterManager::CompareWithRole(CCharacter* a, CCharacter* b)
{
	if (a->GetCharacterInformation()->role == b->GetCharacterInformation()->role)
	{
		return a->GetCharacterInformation()->userid < b->GetCharacterInformation()->userid;
	}
	else
	{
		return a->GetCharacterInformation()->role < b->GetCharacterInformation()->role;
	}
}

bool CCharacterManager::CompareWithRecord(const std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_UINT16>& a, const std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_UINT16>& b)
{
	if (a.second == b.second)
	{
		return a.first < b.first;
	}

	//! �������� ����
	return a.second > b.second;
}

DHOST_TYPE_UINT32 CCharacterManager::GetRoleCount(DHOST_TYPE_INT32 team, F4PACKET::ROLE role)
{
	DHOST_TYPE_UINT32 count = 0;

	if (0 <= team && team < m_vecTeamsCharacter.size())
	{
		for (int i = 0; i < m_vecTeamsCharacter[team].size(); ++i)
		{
			if (m_vecTeamsCharacter[team][i]->GetCharacterInformation()->role == role)
			{
				++count;
			}
		}
	}

	return count;
}

void CCharacterManager::SetCharacterMoveLock(DHOST_TYPE_CHARACTER_SN characterSN, EMOVEMENT_RESTRICTION value)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterRestriction(value);
	}
}

void CCharacterManager::SetCharacterMoveUnLock()
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			iter->second->SetCharacterRestriction(EMOVEMENT_RESTRICTION::NONE);
		}
	}
}

void CCharacterManager::EndValidatePassiveAll()
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			iter->second->EndValidatePassive();
		}
	}
}

CCharacter* CCharacterManager::CheckCollision(CCharacter* pInfo)
{
	CCharacter* pRetValue = nullptr;

	float distance = std::numeric_limits<float>::max();

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterAction()->actionType != F4PACKET::ACTION_TYPE::action_none)
		{
			if (iter->second->GetCharacterInformation()->id != pInfo->GetCharacterInformation()->id)
			{
				TB::SVector3 directionCollision = CommonFunction::SVectorSub(iter->second->GetCharacterAction()->action->positionlogic(), pInfo->GetCharacterAction()->action->positionlogic());

				float distanceCurr = CommonFunction::SVectorDistance(directionCollision);

				float radiusCurr = COLLISION_RADIUS;

				if (distanceCurr < std::min(radiusCurr, distance))
				{
					distance = distanceCurr;
					pRetValue = iter->second;
				}
			}
		}
	}

	return pRetValue;
}

CCharacter* CCharacterManager::CheckCollisionEx(CCharacter* pInfo)
{
	CCharacter* pRetValue = nullptr;

	float distance = std::numeric_limits<float>::max();

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterStateActionType() != F4PACKET::ACTION_TYPE::action_none)
		{
			if (iter->second->GetCharacterInformation()->id != pInfo->GetCharacterInformation()->id)
			{
				TB::SVector3 directionCollision = CommonFunction::SVectorSub(iter->second->GetCharacterStateAction()->positionlogic(), pInfo->GetCharacterStateAction()->positionlogic());

				float distanceCurr = CommonFunction::SVectorDistance(directionCollision);

				float radiusCurr = COLLISION_RADIUS;

				if (distanceCurr < std::min(radiusCurr, distance))
				{
					distance = distanceCurr;
					pRetValue = iter->second;
				}
			}
		}
	}

	return pRetValue;
}

TB::SVector3 CCharacterManager::CorrectionCollision(CCharacter* pInfo1, CCharacter* pInfo2)
{
	int collisionPriority = (int)pInfo1->GetCharacterAction()->action->collisionpriority() - (int)pInfo2->GetCharacterAction()->action->collisionpriority();

	if (collisionPriority < 0)
	{
		TB::SVector3 direction = CommonFunction::SVectorSub(pInfo1->GetCharacterAction()->action->positionlogic(), pInfo2->GetCharacterAction()->action->positionlogic());
		CommonFunction::SVectorDistance(direction);

		TB::SVector3 normal = CommonFunction::SVectorNormalize(direction);

		TB::SVector3 multi = CommonFunction::SVectorMultiply(normal, COLLISION_RADIUS);

		TB::SVector3 position = CommonFunction::SVectorAdd(pInfo2->GetCharacterAction()->action->positionlogic(), multi);

		pInfo1->GetCharacterAction()->action->mutable_positionlogic().mutate_x(position.x());
		pInfo1->GetCharacterAction()->action->mutable_positionlogic().mutate_y(position.y());
		pInfo1->GetCharacterAction()->action->mutable_positionlogic().mutate_z(position.z());
	}
	else if (collisionPriority == 0)
	{
		TB::SVector3 direction = CommonFunction::SVectorSub(pInfo1->GetCharacterAction()->action->positionlogic(), pInfo2->GetCharacterAction()->action->positionlogic());
		TB::SVector3 normal = CommonFunction::SVectorNormalize(direction);
		direction = CommonFunction::SVectorMultiply(normal, COLLISION_RADIUS * 0.5f);

		TB::SVector3 center = CommonFunction::SVectorLerp(pInfo1->GetCharacterAction()->action->positionlogic(), pInfo2->GetCharacterAction()->action->positionlogic(), 0.5f);

		CommonFunction::SVectorSet(pInfo1->GetCharacterAction()->action->mutable_positionlogic(), CommonFunction::SVectorAdd(center, direction));
		CommonFunction::SVectorSet(pInfo2->GetCharacterAction()->action->mutable_positionlogic(), CommonFunction::SVectorSub(center, direction));
	}

	return pInfo1->GetCharacterAction()->action->positionlogic();
}

TB::SVector3 CCharacterManager::CorrectionCollisionEx(CCharacter* pInfo1, CCharacter* pInfo2)
{
	int collisionPriority = static_cast<int>(pInfo1->GetCharacterStateAction()->collisionpriority()) - static_cast<int>(pInfo2->GetCharacterStateAction()->collisionpriority());

	if (collisionPriority < 0)
	{
		TB::SVector3 direction = CommonFunction::SVectorSub(pInfo1->GetCharacterStateAction()->positionlogic(), pInfo2->GetCharacterStateAction()->positionlogic());
		CommonFunction::SVectorDistance(direction);

		TB::SVector3 normal = CommonFunction::SVectorNormalize(direction);

		TB::SVector3 multi = CommonFunction::SVectorMultiply(normal, COLLISION_RADIUS);

		TB::SVector3 position = CommonFunction::SVectorAdd(pInfo2->GetCharacterStateAction()->positionlogic(), multi);

		pInfo1->SetCharacterStateActionPosition(position.x(), position.y(), position.z(), "CCharacterManager::CorrectionCollisionEx collisionPriority < 0");
	}
	else if (collisionPriority == 0)
	{
		TB::SVector3 direction = CommonFunction::SVectorSub(pInfo1->GetCharacterStateAction()->positionlogic(), pInfo2->GetCharacterStateAction()->positionlogic());
		TB::SVector3 normal = CommonFunction::SVectorNormalize(direction);
		direction = CommonFunction::SVectorMultiply(normal, COLLISION_RADIUS * 0.5f);

		TB::SVector3 center = CommonFunction::SVectorLerp(pInfo1->GetCharacterStateAction()->positionlogic(), pInfo2->GetCharacterStateAction()->positionlogic(), 0.5f);

		TB::SVector3 result_add = CommonFunction::SVectorAdd(center, direction);
		pInfo1->SetCharacterStateActionPosition(result_add.x(), result_add.y(), result_add.z(), "CCharacterManager::CorrectionCollisionEx collisionPriority == 0 SVectorAdd");

		TB::SVector3 result_sub = CommonFunction::SVectorSub(center, direction);
		pInfo2->SetCharacterStateActionPosition(result_sub.x(), result_sub.y(), result_sub.z(), "CCharacterManager::CorrectionCollisionEx collisionPriority == 0 SVectorSub");
	}

	return pInfo1->GetCharacterStateAction()->positionlogic();
}

void CCharacterManager::CharacterManagerUpdate(DHOST_TYPE_FLOAT timeDelta, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_FLOAT elapsedTime, JOVECTOR3 ballPos)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			iter->second->UpdateCharacterState(gameTime, timeDelta, elapsedTime, ballPos);
			CheckCharacterOnFireModeRelease(iter->second, gameTime);

			if ((iter->second->GetMoveMode() == F4PACKET::MOVE_MODE::boxOut || iter->second->GetMoveMode() == F4PACKET::MOVE_MODE::enhancedBoxOut 
				|| iter->second->GetMoveMode() == F4PACKET::MOVE_MODE::faceUp 
				|| iter->second->GetMoveMode() == F4PACKET::MOVE_MODE::shadowfaceUp
				|| iter->second->GetMoveMode() == F4PACKET::MOVE_MODE::intercept))
			{
				m_pMannerManager->UpdateAwayFromKeyboard(iter->first, gameTime);
			}
		}
	}

	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->MannerManagerUpdate(gameTime);
	}
}

void CCharacterManager::CharacterManagerLateUpdate(DHOST_TYPE_FLOAT timeDelta)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterAction()->actionType != F4PACKET::ACTION_TYPE::action_none)
		{
			CCharacter* collisionPlayer = CheckCollision(iter->second);

			if (collisionPlayer != nullptr)
			{
				CorrectionCollision(iter->second, collisionPlayer);
			}
		}
	}
}

void CCharacterManager::CharacterManagerLateUpdateEx(DHOST_TYPE_FLOAT timeDelta)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterStateActionType() != F4PACKET::ACTION_TYPE::action_none)
		{
			CCharacter* collisionPlayer = CheckCollisionEx(iter->second);

			if (collisionPlayer != nullptr)
			{
				CorrectionCollisionEx(iter->second, collisionPlayer);
			}
		}
	}
}

void CCharacterManager::ClearCharacters()
{
	m_mapCharacter.clear();
	
	for (int i = 0; i < m_vecTeamsCharacter.size(); i++)
	{
		m_vecTeamsCharacter[i].clear();
	}

	m_vecTeamsCharacter.clear();
}

void CCharacterManager::MixCharacterTeamInRole()
{
	SortCharacters();

	std::vector<CCharacter*> TempCharacterVec;
	TempCharacterVec.clear();

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		TempCharacterVec.push_back(iter->second);
	}

	std::sort(TempCharacterVec.begin(), TempCharacterVec.end(), Compare);

	ClearCharacters();

	for (int i = 0; i < TempCharacterVec.size(); ++i)
	{
		CCharacter* pInfo = TempCharacterVec[i];

		if (pInfo != nullptr)
		{
			m_mapCharacter.insert(std::pair<DHOST_TYPE_CHARACTER_SN, CCharacter*>(pInfo->GetCharacterInformation()->id, pInfo));

			pInfo->GetCharacterInformation()->team = i % 2;

			AddTeamCharacterMap(pInfo);
		}
	}
}

CCharacter* CCharacterManager::GetCharacter(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second;
	}
	return nullptr;
}

map<DHOST_TYPE_CHARACTER_SN, CCharacter*> CCharacterManager::GetCharacters()
{
	return m_mapCharacter;
}

F4PACKET::SPlayerInformationT* CCharacterManager::GetCharacterInformation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return m_mapCharacter.find(characterSN)->second->GetCharacterInformation();
	}

	return nullptr;
}

void CCharacterManager::GetCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		vec.push_back(iter->second->GetCharacterInformation());
	}
}

void CCharacterManager::GetHomeCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second->GetCharacterInformation()->team == 0)
		{
			vec.push_back(iter->second->GetCharacterInformation());
		}
	}
}

void CCharacterManager::GetAwayCharacterInformationVec(std::vector<F4PACKET::SPlayerInformationT*>& vec)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second->GetCharacterInformation()->team == 1)
		{
			vec.push_back(iter->second->GetCharacterInformation());
		}
	}
}

const TB::SVector3 CCharacterManager::GetCharacterPosition(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterAction()->action->positionlogic();
	}

	return TB::SVector3();
}

F4PACKET::SPlayerAction* CCharacterManager::GetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterAction()->action;
	}
	return nullptr;
}

F4PACKET::SPlayerAction* CCharacterManager::GetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterStateAction();
	}
	return nullptr;
}

F4PACKET::SHOT_TYPE CCharacterManager::GetCharacterActionShotType(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterActionShotType();
	}

	return F4PACKET::SHOT_TYPE::shotType_none;
}

DHOST_TYPE_BALL_NUMBER CCharacterManager::GetCharacterActionPotentialBloom(DHOST_TYPE_CHARACTER_SN characterSN)
{
	DHOST_TYPE_BALL_NUMBER result = kBALL_NUMBER_INIT;

	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		result = iter->second->GetCharacterActionPotentialBloom();
	}

	return result;
}

SCharacterAction* CCharacterManager::GetCharacterActionInfo(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterAction();
	}

	return nullptr;
}

DHOST_TYPE_INT32 CCharacterManager::GetCharacterTeam(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterInformation()->team;
	}

	return -1;
}

CCharacter* CCharacterManager::GetCharacterTeamWithFirstPlayerNumber(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
		{
			if (iter->second != nullptr)
			{
				if (pCharacter->GetCharacterInformation()->team == iter->second->GetCharacterInformation()->team && iter->second->GetPlayerNumber() == 1)
				{
					return iter->second;
				}
			}
		}
	}

	return nullptr;
}

void CCharacterManager::SetCharacterAction(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F actionTime, F4PACKET::ACTION_TYPE actionType, F4PACKET::SPlayerAction* pInfo, DHOST_TYPE_FLOAT speed, F4PACKET::SHOT_TYPE shotType, DHOST_TYPE_BALL_NUMBER potentialBloom)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterAction(actionTime, actionType, pInfo, speed, shotType, potentialBloom);
	}
}

void CCharacterManager::SetCharacterActionOverlap(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, F4PACKET::SKILL_INDEX value)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterActionOverlap(actionType, value);
	}
}

void CCharacterManager::SetCharacterActionOverlapDestYaw(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_FLOAT DestYaw)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterActionOverlapDestYaw(actionType, DestYaw);
	}
}

void CCharacterManager::AddCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->AddCharacterActionDeque();
	}
}

void CCharacterManager::AddServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN, ServerCharacterPosition position)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->AddServerPostionDeque(position);
	}
}

void CCharacterManager::GenerateCharacterBuff(DHOST_TYPE_CHARACTER_SN characterSN, SCharacterBuff characterBuff)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->GenerateBuff(characterBuff);
	}
}

std::deque<ServerCharacterPosition> CCharacterManager::GetServerPostionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);
	
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetServerPostionDeque();
	}

	return std::deque<ServerCharacterPosition>();
}

std::deque<SCharacterAction> CCharacterManager::GetCharacterActionDeque(DHOST_TYPE_CHARACTER_SN characterSN)
{
	std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.find(characterSN);

	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterActionDeque();
	}

	return std::deque<SCharacterAction>();
}

size_t CCharacterManager::GetTeamCharacterSize(DHOST_TYPE_INT32 indexTeam)
{
	if (0 <= indexTeam && indexTeam < m_vecTeamsCharacter.size())
	{
		return m_vecTeamsCharacter[indexTeam].size();
	}

	return 0;
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::GetCharacterSN(DHOST_TYPE_INT32 indexTeam, DHOST_TYPE_INT32 indexCharacter)
{
	if (0 <= indexTeam && indexTeam < m_vecTeamsCharacter.size())
	{
		if (0 <= indexCharacter && indexCharacter < m_vecTeamsCharacter[indexTeam].size())
		{
			return m_vecTeamsCharacter[indexTeam][indexCharacter]->GetCharacterInformation()->id;
		}
	}

	return kCHARACTER_SN_INIT;
}

DHOST_TYPE_INT32 CCharacterManager::GetIndexOpponentTeam(DHOST_TYPE_INT32 indexTeam)
{
	if (m_teamsActive[0] == indexTeam)
	{
		return m_teamsActive[1];
	}
	if (m_teamsActive[1] == indexTeam)
	{
		return m_teamsActive[0];
	}
	return -1;
}

void CCharacterManager::SetCharacterReadyPosition(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::ACTION_TYPE actionType, TB::SVector3 position)
{
	m_mapCharacter.find(characterSN)->second->SetCharacterReadyPosition(actionType, position);
}

void CCharacterManager::CalcFocusPoint()
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			DHOST_TYPE_FLOAT offScore, defScore, etcScore;

			offScore = kFLOAT_INIT;
			defScore = kFLOAT_INIT;
			etcScore = kFLOAT_INIT;

			// ���� �뷱�� ������
			float offA = m_pBalanceData->GetValue("VFocusOffensePointValueA");
			float offB = kFLOAT_INIT;
			float offC = m_pBalanceData->GetValue("VFocusOffensePointValueB");
			float offD = kFLOAT_INIT;
			float offE = m_pBalanceData->GetValue("VFocusOffensePointValueC");
			float offF = kFLOAT_INIT;
			float offG = m_pBalanceData->GetValue("VFocusOffensePointValueD");

			{
				// ����
				offB = static_cast<float>(iter->second->GetCharacterRecord()->TotalScore);

				// ������
				if ((iter->second->GetCharacterRecord()->Suc2Pointcount + iter->second->GetCharacterRecord()->Suc3Pointcount) > 0 && (iter->second->GetCharacterRecord()->Try2PointCount + iter->second->GetCharacterRecord()->Try3PointCount) > 0)
				{
					offD = (static_cast<float>(iter->second->GetCharacterRecord()->Suc2Pointcount + iter->second->GetCharacterRecord()->Suc3Pointcount) / static_cast<float>(iter->second->GetCharacterRecord()->Try2PointCount + iter->second->GetCharacterRecord()->Try3PointCount));
				}

				// ��ý�Ʈ
				offF = static_cast<float>(iter->second->GetCharacterRecord()->AssistCount);

				offScore = (offA * powf(offB, offC)) * offD + offE * powf(offF, offG);

				if (offScore > m_pBalanceData->GetValue("VFocusOffensePointMaxValue"))
				{
					offScore = m_pBalanceData->GetValue("VFocusOffensePointMaxValue");
				}
			}

			// ���� �뷱�� ������
			float defA = m_pBalanceData->GetValue("VFocusDefensePointValueA");
			float defB = kFLOAT_INIT;
			float defC = m_pBalanceData->GetValue("VFocusDefensePointValueB");
			float defD = m_pBalanceData->GetValue("VFocusDefensePointValueC");
			float defE = kFLOAT_INIT;
			float defF = m_pBalanceData->GetValue("VFocusDefensePointValueD");

			{
				defB = static_cast<float>(iter->second->GetCharacterRecord()->BlockCount);
				defE = static_cast<float>(iter->second->GetCharacterRecord()->StealCount);

				defScore = (defA * powf(defB, defC)) + defD * powf(defE, defF);

				if (defScore > m_pBalanceData->GetValue("VFocusDefensePointMaxValue"))
				{
					defScore = m_pBalanceData->GetValue("VFocusDefensePointMaxValue");
				}
			}

			// ��Ÿ �뷱�� ������
			float etcA = m_pBalanceData->GetValue("VFocusETCPointValueA");
			float etcB = kFLOAT_INIT;
			float etcC = m_pBalanceData->GetValue("VFocusETCPointValueB");
			float etcD = m_pBalanceData->GetValue("VFocusETCPointValueC");
			float etcE = kFLOAT_INIT;
			float etcF = m_pBalanceData->GetValue("VFocusETCPointValueD");

			{
				etcB = static_cast<float>(iter->second->GetCharacterRecord()->ReboundCount);
				etcE = static_cast<float>(iter->second->GetCharacterRecord()->LooseBallCount);

				defScore = (etcA * powf(etcB, etcC)) + etcD * powf(etcE, etcF);

				if (etcScore > m_pBalanceData->GetValue("VFocusETCPointMaxValue"))
				{
					etcScore = m_pBalanceData->GetValue("VFocusETCPointMaxValue");
				}
			}

			iter->second->SetFocusPoint(offScore + defScore + etcScore);
		}
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetMaxFocusPointInTeam(DHOST_TYPE_INT32 team)
{
	DHOST_TYPE_FLOAT MaxFocusPoint = kFLOAT_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			if (iter->second->GetCharacterInformation()->team == team && MaxFocusPoint < iter->second->GetFocusPoint())
			{
				MaxFocusPoint = iter->second->GetFocusPoint();
			}
		}
	}

	return MaxFocusPoint;
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::GetMaxFocusPointCharacterInTeam(DHOST_TYPE_INT32 team)
{
	DHOST_TYPE_FLOAT MaxFocusPoint = kFLOAT_INIT;
	DHOST_TYPE_CHARACTER_SN MaxFocusPointCharacterSN = kCHARACTER_SN_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			if (iter->second->GetCharacterInformation()->team == team && MaxFocusPoint < iter->second->GetFocusPoint())
			{
				MaxFocusPointCharacterSN = iter->second->GetCharacterInformation()->id;
				MaxFocusPoint = iter->second->GetFocusPoint();
			}
		}
	}

	if (MaxFocusPoint > m_pBalanceData->GetValue("VFocusStandardValue"))
	{
		return MaxFocusPointCharacterSN;
	}

	return kCHARACTER_SN_INIT;
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::GetMaxFocusPointCharacterInEnemyTeam(DHOST_TYPE_INT32 Scoreteam)
{
	DHOST_TYPE_FLOAT ScorerTeamMaxFocusPoint = GetMaxFocusPointInTeam(Scoreteam);

	DHOST_TYPE_FLOAT NoTableCharacterFocusPoint = kFLOAT_INIT;
	DHOST_TYPE_CHARACTER_SN NoTableCharacterSN = kCHARACTER_SN_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			if (iter->second->GetCharacterInformation()->team != Scoreteam)
			{
				// ������ ������ ���� �������� üũ
				if (NoTableCharacterFocusPoint < iter->second->GetFocusPoint())
				{
					NoTableCharacterSN = iter->second->GetCharacterInformation()->id;
					NoTableCharacterFocusPoint = iter->second->GetFocusPoint();
				}
			}
		}
	}

	DHOST_TYPE_FLOAT minFocusPoint = kFLOAT_INIT;
	DHOST_TYPE_FLOAT maxFocusPoint = kFLOAT_INIT;

	if (m_pBalanceData->GetValue("VFocusStandardMin") > kFLOAT_INIT && m_pBalanceData->GetValue("VFocusStandardMax") > kFLOAT_INIT)
	{
		minFocusPoint = (ScorerTeamMaxFocusPoint * m_pBalanceData->GetValue("VFocusStandardMin")) / 100.f;
		maxFocusPoint = (ScorerTeamMaxFocusPoint * m_pBalanceData->GetValue("VFocusStandardMax")) / 100.f;

		if (minFocusPoint < NoTableCharacterFocusPoint && maxFocusPoint > NoTableCharacterFocusPoint)
		{
			return NoTableCharacterSN;
		}
		else
		{
			return kCHARACTER_SN_INIT;
		}
	}
	else
	{
		return kCHARACTER_SN_INIT;
	}

	return kCHARACTER_SN_INIT;
}

void CCharacterManager::UpdateRecord(DHOST_TYPE_CHARACTER_SN CharacterSN, ECHARACTER_RECORD_TYPE recordType, DHOST_TYPE_BOOL bOverTime)
{
	CCharacter* pCharacter = GetCharacter(CharacterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->UpdateCharacterRecord(recordType, bOverTime);
	}


	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			iter->second->OnCharacterRecord(CharacterSN, recordType, bOverTime); // 패시브를 위해 정보만 전달 
		}
	}
}

void CCharacterManager::SaveRegularTimeCharacterRecord()
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			iter->second->GetCharacterRecord()->RegularTotalScore = iter->second->GetCharacterRecord()->TotalScore;
			iter->second->GetCharacterRecord()->RegularTry2PointCount = iter->second->GetCharacterRecord()->Try2PointCount;
			iter->second->GetCharacterRecord()->RegularSuc2Pointcount = iter->second->GetCharacterRecord()->Suc2Pointcount;
			iter->second->GetCharacterRecord()->RegularTry3PointCount = iter->second->GetCharacterRecord()->Try3PointCount;
			iter->second->GetCharacterRecord()->RegularSuc3Pointcount = iter->second->GetCharacterRecord()->Suc3Pointcount;
			iter->second->GetCharacterRecord()->RegularAssistCount = iter->second->GetCharacterRecord()->AssistCount;
			iter->second->GetCharacterRecord()->RegularReboundCount = iter->second->GetCharacterRecord()->ReboundCount;
			iter->second->GetCharacterRecord()->RegularBlockCount = iter->second->GetCharacterRecord()->BlockCount;
			iter->second->GetCharacterRecord()->RegularStealCount = iter->second->GetCharacterRecord()->StealCount;
			iter->second->GetCharacterRecord()->RegularLooseBallCount = iter->second->GetCharacterRecord()->LooseBallCount;
			iter->second->GetCharacterRecord()->RegularTurnOverCount = iter->second->GetCharacterRecord()->TurnOverCount;
			iter->second->GetCharacterRecord()->RegularJumpShot2PointCount = iter->second->GetCharacterRecord()->JumpShot2PointCount;

			iter->second->GetCharacterRecord()->RegularNicePassCount = iter->second->GetCharacterRecord()->NicePassCount;
			iter->second->GetCharacterRecord()->RegularActionGreatDefenseCount = iter->second->GetCharacterRecord()->ActionGreatDefenseCount;
			iter->second->GetCharacterRecord()->RegularActionLayUpGoalCount = iter->second->GetCharacterRecord()->ActionLayUpGoalCount;
			iter->second->GetCharacterRecord()->RegularActionDunkGoalCount = iter->second->GetCharacterRecord()->ActionDunkGoalCount;
			iter->second->GetCharacterRecord()->RegularActionAnkleBreakeCount = iter->second->GetCharacterRecord()->ActionAnkleBreakeCount;
		}
	}
}

void CCharacterManager::PushCharacterRecord(SRedisMatchResult* pInfo, DHOST_TYPE_FLOAT roomElapsedTime)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			if (iter->second->GetBadMannerTypeCount(EBAD_MANNER_TYPE::AWAY_FROM_KEYBOARD) > kUINT16_INIT)
			{
				DHOST_TYPE_FLOAT AwayFromKeyboardGameTime = iter->second->GetCharacterRecord()->AwayFromKeyboardTimeAccumulate;

				if (AwayFromKeyboardGameTime == kFLOAT_INIT)
				{
					iter->second->UpdateAwayFromKeyboardTimeAccumulate((iter->second->GetAwayFromKeyboardGameTime() + m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_NOTHING")));
				}
			}

			if (iter->second->GetCharacterDisconnectTime() > kFLOAT_INIT)
			{
				iter->second->SetCharacterDiconnectComebackTime(roomElapsedTime);
			}

			pInfo->vCharacterRecord.push_back(iter->second->GetCharacterRecord());
		}
	}
}

void CCharacterManager::GetCharacterRecordVec(std::vector<SCharacterRecord*>& vec)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		vec.push_back(iter->second->GetCharacterRecord());
	}
}

DHOST_TYPE_UINT16 CCharacterManager::GetCharacterRecordCountWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterRecordCountWithType(Type);
	}

	return kUINT16_INIT;
}

DHOST_TYPE_BOOL CCharacterManager::GetMostCharacterRerecordWithType(DHOST_TYPE_CHARACTER_SN characterSN, RECORD_TYPE Type, DHOST_TYPE_BOOL allowEqual)
{
	DHOST_TYPE_BOOL result = false;
	DHOST_TYPE_UINT16 mostRecord = kUINT16_INIT;					// �ְ� ���
	DHOST_TYPE_UINT16 paramCharacterRecord = kUINT16_INIT;			// ��û�� ĳ������ ���
	DHOST_TYPE_UINT16 characterRecord = kUINT16_INIT;
	DHOST_TYPE_INT32 equalCount = kINT32_INIT;						// ��û�� ĳ���Ϳ� ������ ����� ���� ĳ���ͼ�

	std::vector<std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_UINT16>> vecCharacterRecord;
	vecCharacterRecord.clear();

	CCharacter* pCharacter = GetCharacter(characterSN);
	if (pCharacter != nullptr)
	{
		paramCharacterRecord = pCharacter->GetCharacterRecordCountWithType(Type);
	}

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			characterRecord = iter->second->GetCharacterRecordCountWithType(Type);

			if (paramCharacterRecord == characterRecord)
			{
				++equalCount;
			}

			vecCharacterRecord.push_back(std::make_pair(iter->first, characterRecord));
		}
	}

	if (vecCharacterRecord.size() > 0)
	{
		//! �������� ���� (����� ���� ����ĳ���� ù��° �ε����� ��)
		std::sort(vecCharacterRecord.begin(), vecCharacterRecord.end(), CompareWithRecord);

		mostRecord = vecCharacterRecord.front().second;

		//! ��û�� ĳ������ ����� �ְ������ ���
		if (paramCharacterRecord == mostRecord)
		{
			result = true;

			//! ������ ����� ������� ���� �� ������ ����� ������ �ִ� ĳ���Ͱ� �ִٸ� �ְ� ����� �ƴѰ���
			if (allowEqual == false && equalCount > 1)
			{
				result = false;
			}
		}
	}

	return result;
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::GetMVP()
{
	return m_GameMVP;
}

void CCharacterManager::SetMVP(DHOST_TYPE_CHARACTER_SN value)
{
	m_GameMVP = value;
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::CalcMVPScore(DHOST_TYPE_INT32 WinnerTeam)
{
	// ���� �뷱�� ������
	float offA = m_pBalanceData->GetValue("VMVPOffensePointValueA");
	float offB = kFLOAT_INIT;
	float offC = m_pBalanceData->GetValue("VMVPOffensePointValueB");
	float offD = kFLOAT_INIT;
	float offE = m_pBalanceData->GetValue("VMVPOffensePointValueC");
	float offF = kFLOAT_INIT;
	float offG = m_pBalanceData->GetValue("VMVPOffensePointValueD");
	float offMax = m_pBalanceData->GetValue("VMVPOffensePointMaxValue");

	// ���� �뷱�� ������
	float defA = m_pBalanceData->GetValue("VMVPDefensePointValueA");
	float defB = kFLOAT_INIT;
	float defC = m_pBalanceData->GetValue("VMVPDefensePointValueB");
	float defD = m_pBalanceData->GetValue("VMVPDefensePointValueC");
	float defE = kFLOAT_INIT;
	float defF = m_pBalanceData->GetValue("VMVPDefensePointValueD");
	float defMax = m_pBalanceData->GetValue("VMVPDefensePointMaxValue");

	// ��Ÿ �뷱�� ������
	float etcA = m_pBalanceData->GetValue("VMVPETCPointValueA");
	float etcB = kFLOAT_INIT;
	float etcC = m_pBalanceData->GetValue("VMVPETCPointValueB");
	float etcD = m_pBalanceData->GetValue("VMVPETCPointValueC");
	float etcE = kFLOAT_INIT;
	float etcF = m_pBalanceData->GetValue("VMVPETCPointValueD");
	float etcMax = m_pBalanceData->GetValue("VMVPETCPointMaxValue");

	// ���� �뷱�� ������
	float winScore = m_pBalanceData->GetValue("VMVPWinPointValue");
	float loseScore = m_pBalanceData->GetValue("VMVPLosePointValue");

	// �߰�����
	float addA = m_pBalanceData->GetValue("VMVPAdditionalPointValueA");
	float addB = kFLOAT_INIT;
	float addC = m_pBalanceData->GetValue("VMVPAdditionalPointValueB");
	float addD = m_pBalanceData->GetValue("VMVPAdditionalPointValueC");
	float addE = kFLOAT_INIT;
	float addF = m_pBalanceData->GetValue("VMVPAdditionalPointValueD");
	float addG = m_pBalanceData->GetValue("VMVPAdditionalPointValueE");
	float addH = kFLOAT_INIT;
	float addJ = m_pBalanceData->GetValue("VMVPAdditionalPointValueF");
	float addK = m_pBalanceData->GetValue("VMVPAdditionalPointValueG");
	float addL = kFLOAT_INIT;
	float addM = m_pBalanceData->GetValue("VMVPAdditionalPointValueH");
	float addMax = m_pBalanceData->GetValue("VMVPAdditionalPointMaxValue");


	DHOST_TYPE_CHARACTER_SN mvpCharacterSN = kLAST_CHARACTER_SN;
	DHOST_TYPE_FLOAT mvpScore = kFLOAT_INIT;
	DHOST_TYPE_FLOAT CharacterMVPScore = kFLOAT_INIT;

	DHOST_TYPE_FLOAT offScore, defScore, etcScore, addScore;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			offScore = kFLOAT_INIT;
			defScore = kFLOAT_INIT;
			etcScore = kFLOAT_INIT;
			addScore = kFLOAT_INIT;

			CCharacter* pCharacter = iter->second;
			if (pCharacter != nullptr)
			{
				CharacterMVPScore = kFLOAT_INIT;

				auto SCharacterRecord = pCharacter->GetCharacterRecord();
				if (SCharacterRecord != nullptr)
				{
					// ���� ������
					offB = (float)SCharacterRecord->TotalScore;	// ĳ������ ����
					offD = kFLOAT_INIT;

					if ((SCharacterRecord->Suc2Pointcount + SCharacterRecord->Suc3Pointcount) > 0 && (SCharacterRecord->Try2PointCount + SCharacterRecord->Try3PointCount) > 0)
					{
						offD = ((float)(SCharacterRecord->Suc2Pointcount + SCharacterRecord->Suc3Pointcount) / (float)(SCharacterRecord->Try2PointCount + SCharacterRecord->Try3PointCount));	// ������
					}
					offF = (float)SCharacterRecord->AssistCount;	// ĳ������ ��ý�Ʈ

					// ���� ������
					defB = (float)SCharacterRecord->BlockCount;
					defE = (float)SCharacterRecord->StealCount;

					// ��Ÿ ������
					etcB = (float)SCharacterRecord->ReboundCount;
					etcE = (float)SCharacterRecord->LooseBallCount;

					// �߰� ������
					addB = (float)SCharacterRecord->PassCount;
					addE = (float)SCharacterRecord->NicePassCount;
					addH = (float)SCharacterRecord->DivingCatchCount;
					addL = (float)SCharacterRecord->OverPassCount;

					offScore = (offA * powf(offB, offC)) * offD + offE * powf(offF, offG);
					if (offScore > offMax)
					{
						offScore = offMax;
					}

					defScore = (defA * powf(defB, defC)) + defD * powf(defE, defF);
					if (defScore > defMax)
					{
						defScore = defMax;
					}

					etcScore = (etcA * powf(etcB, etcC)) + etcD * powf(etcE, etcF);
					if (etcScore > etcMax)
					{
						etcScore = etcMax;
					}

					addScore = (addA * powf(addB, addC)) + (addD * powf(addE, addF)) + (addG * powf(addH, addJ)) + (addK * powf(addL, addM));
					if (addScore > addMax)
					{
						addScore = addMax;
					}

#ifdef TEXT_LOG_FILE
					LOGGER->Log("[MVP_SCORE] characterSN : %u, characterModel : %u, Pass : %u, NicePass : %u, DivingCatch : %u, OverPass : %u, AddScore : %f", pCharacter->GetCharacterInformation()->id, pCharacter->GetCharacterInformation()->characterid, SCharacterRecord->PassCount, SCharacterRecord->NicePassCount, SCharacterRecord->DivingCatchCount, SCharacterRecord->OverPassCount, addScore);
#endif

					auto winner_team_buff = (WinnerTeam == pCharacter->GetCharacterInformation()->team) ? winScore : loseScore;

					CharacterMVPScore = (offScore + defScore + etcScore + addScore) * winner_team_buff;

					if (CharacterMVPScore < kFLOAT_INIT)
					{
						CharacterMVPScore = kFLOAT_INIT;
					}

#ifdef TEXT_LOG_FILE
					LOGGER->Log("[MVP_SCORE] characterSN : %u, MVPScore : %f", pCharacter->GetCharacterInformation()->id, CharacterMVPScore);
#endif

					SCharacterRecord->MVPScore = static_cast<DHOST_TYPE_UINT16>(CharacterMVPScore);

					if (CharacterMVPScore > mvpScore)
					{
						mvpScore = CharacterMVPScore;
						mvpCharacterSN = pCharacter->GetCharacterInformation()->id;
					}
				}
			}
		}
	}

	//! 20220922 MVP ������ ������ ĳ���� �ִٸ� ���� MVP ĳ���Ϳ� ������ +1 ���ֱ�� �ߴ� (�߱��� ��û����)
	CCharacter* mvp_character = GetCharacter(mvpCharacterSN);
	if (mvp_character != nullptr)
	{
		for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
		{
			if (iter->second != nullptr)
			{
				CCharacter* pCharacter = iter->second;

				if (pCharacter != nullptr && mvp_character->GetCharacterInformation()->id != pCharacter->GetCharacterInformation()->id)
				{
					if (pCharacter->GetCharacterRecord()->MVPScore == mvp_character->GetCharacterRecord()->MVPScore)
					{
						mvp_character->GetCharacterRecord()->MVPScore += 1;
					}
				}
			}
		}
	}

	return mvpCharacterSN;
}

DHOST_TYPE_FLOAT CCharacterManager::CalcReplayPoint(DHOST_TYPE_GAME_TIME_F ScoredTime)
{
	DHOST_TYPE_FLOAT replayPoint = kFLOAT_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			CCharacter* pCharacter = iter->second;
			if (pCharacter != nullptr)
			{
				std::deque<SCharacterAction> actionInfo = pCharacter->GetCharacterActionDeque();

				DHOST_TYPE_INT32 idx = pCharacter->GetActionIDX();

				for (int i = idx; i < actionInfo.size(); ++i)
				{
					if (actionInfo[i].actionTime - ScoredTime < kREPLAY_ACTION_CHECK_TIME)
					{
						DHOST_TYPE_FLOAT actionPoint = kFLOAT_INIT;
						DHOST_TYPE_FLOAT skillPoint = kFLOAT_INIT;

						F4PACKET::ACTION_TYPE actionType = ConvertReplayActionTypeFromOriginActionType(actionInfo[i].actionType);

						switch (actionType)
						{
						case F4PACKET::ACTION_TYPE::action_pick: actionPoint = m_pBalanceData->GetValue("VReplayAddTouch"); break;
						case F4PACKET::ACTION_TYPE::action_shotDunk: actionPoint = m_pBalanceData->GetValue("VReplayAddDunk"); break;
						case F4PACKET::ACTION_TYPE::action_shotLayUp: actionPoint = m_pBalanceData->GetValue("VReplayAddLayup"); break;
						case F4PACKET::ACTION_TYPE::action_shotJump: actionPoint = m_pBalanceData->GetValue("VReplayAddJump"); break;
						case F4PACKET::ACTION_TYPE::action_block: actionPoint = m_pBalanceData->GetValue("VReplayAddBlock"); break;
						case F4PACKET::ACTION_TYPE::action_penetrate: actionPoint = m_pBalanceData->GetValue("VReplayAddPenetration"); break;
						case F4PACKET::ACTION_TYPE::action_screen: actionPoint = m_pBalanceData->GetValue("VReplayAddScreened"); break;
						default: break;
						}

						if (actionInfo[i].action->skillindex() != F4PACKET::SKILL_INDEX::skill_none)
						{
							skillPoint = GetReplaySkillPoint(actionInfo[i].action->skillindex());
						}

						if (actionPoint > kFLOAT_INIT || skillPoint > kFLOAT_INIT)
						{
							replayPoint += (actionPoint + skillPoint);
						}
					}
					++idx;
				}
				pCharacter->SetActionIDX(idx);
			}
		}
	}

	return replayPoint;
}

F4PACKET::ACTION_TYPE CCharacterManager::ConvertReplayActionTypeFromOriginActionType(F4PACKET::ACTION_TYPE value)
{
	switch (value)
	{
	case F4PACKET::ACTION_TYPE::action_alleyOopCut:
		return F4PACKET::ACTION_TYPE::action_block;
	case F4PACKET::ACTION_TYPE::action_hopStep:
	case F4PACKET::ACTION_TYPE::action_postUpPenetrate:
		return F4PACKET::ACTION_TYPE::action_penetrate;
	default:
		break;
	}
	return value;
}

DHOST_TYPE_FLOAT CCharacterManager::GetReplaySkillPoint(F4PACKET::SKILL_INDEX value)
{
	DHOST_TYPE_FLOAT point = kFLOAT_INIT;

	switch (value)
	{
	case F4PACKET::SKILL_INDEX::skill_reverseLayUp: point = m_pBalanceData->GetValue("VReplaySkillReverseLayUp"); break;
	case F4PACKET::SKILL_INDEX::skill_shoulderFake: point = m_pBalanceData->GetValue("VReplaySkillShoulderFake"); break;
	case F4PACKET::SKILL_INDEX::skill_slideStep: point = m_pBalanceData->GetValue("VReplaySkillSlideStep"); break;
	case F4PACKET::SKILL_INDEX::skill_postUpSpin: point = m_pBalanceData->GetValue("VReplaySkillPostUpSpin"); break;
	case F4PACKET::SKILL_INDEX::skill_hookShot: point = m_pBalanceData->GetValue("VReplaySkillHookShot"); break;
	case F4PACKET::SKILL_INDEX::skill_hookShotSky: point = m_pBalanceData->GetValue("VReplaySkillHookShotSky"); break;
	case F4PACKET::SKILL_INDEX::skill_doubleCluch: point = m_pBalanceData->GetValue("VReplaySkillDoubleCluch"); break;
	case F4PACKET::SKILL_INDEX::skill_pullUpJumper: point = m_pBalanceData->GetValue("VReplaySkillPullUpJumper"); break;
	case F4PACKET::SKILL_INDEX::skill_stepBack: point = m_pBalanceData->GetValue("VReplaySkillStepBack"); break;
	case F4PACKET::SKILL_INDEX::skill_stepBackJumper: point = m_pBalanceData->GetValue("VReplaySkillStepBackJumper"); break;
	case F4PACKET::SKILL_INDEX::skill_fadeAway: point = m_pBalanceData->GetValue("VReplaySkillFadeAway2"); break;
	case F4PACKET::SKILL_INDEX::skill_fadeAway3: point = m_pBalanceData->GetValue("VReplaySkillFadeAway3"); break;
	case F4PACKET::SKILL_INDEX::skill_spotUpShot: point = m_pBalanceData->GetValue("VReplaySkillSpotUpShot"); break;
	case F4PACKET::SKILL_INDEX::skill_spotUpShot3: point = m_pBalanceData->GetValue("VReplaySkillSpotUpShot3"); break;
	case F4PACKET::SKILL_INDEX::skill_jumpShotPass: point = m_pBalanceData->GetValue("VReplaySkillJumpShotPass"); break;
	case F4PACKET::SKILL_INDEX::skill_divingCatch: point = m_pBalanceData->GetValue("VReplaySkillDivingCatch"); break;
	case F4PACKET::SKILL_INDEX::skill_hopStep: point = m_pBalanceData->GetValue("VReplaySkillHopStep"); break;
	case F4PACKET::SKILL_INDEX::skill_layUpPass: point = m_pBalanceData->GetValue("VReplaySkillLayUpPass"); break;
	case F4PACKET::SKILL_INDEX::skill_powerRebound: point = m_pBalanceData->GetValue("VReplaySkillPowerRebound"); break;
	case F4PACKET::SKILL_INDEX::skill_tapOut: point = m_pBalanceData->GetValue("VReplaySkillTapOut"); break;
	case F4PACKET::SKILL_INDEX::skill_blockCatch: point = m_pBalanceData->GetValue("VReplaySkillBlockCatch"); break;
	case F4PACKET::SKILL_INDEX::skill_tipIn: point = m_pBalanceData->GetValue("VReplaySkillTipIn"); break;
	case F4PACKET::SKILL_INDEX::skill_tapDunk: point = m_pBalanceData->GetValue("VReplaySkillTapDunk"); break;
	case F4PACKET::SKILL_INDEX::skill_powerBlock: point = m_pBalanceData->GetValue("VReplaySkillPowerBlock"); break;
	case F4PACKET::SKILL_INDEX::skill_blockCatchDunk: point = m_pBalanceData->GetValue("VReplaySkillBlockCatchDunk"); break;
	case F4PACKET::SKILL_INDEX::skill_chasedownBlock: point = m_pBalanceData->GetValue("VReplaySkillChasedownBlock"); break;
	case F4PACKET::SKILL_INDEX::skill_chasedownCatch: point = m_pBalanceData->GetValue("VReplaySkillChasedownCatch"); break;
	case F4PACKET::SKILL_INDEX::skill_blockPass: point = m_pBalanceData->GetValue("VReplaySkillBlockPass"); break;
	case F4PACKET::SKILL_INDEX::skill_alleyOopPass: point = m_pBalanceData->GetValue("VReplaySkillAlleyoop"); break;
	case F4PACKET::SKILL_INDEX::skill_selfAlleyOop: point = m_pBalanceData->GetValue("VReplaySkillSelfAlleyoop"); break;
	case F4PACKET::SKILL_INDEX::skill_shakeAndBake: point = m_pBalanceData->GetValue("VReplaySkillShake"); break;
	case F4PACKET::SKILL_INDEX::skill_turnAroundFadeAway: point = m_pBalanceData->GetValue("VReplaySkillTurnAround"); break;
	case F4PACKET::SKILL_INDEX::skill_oneLegedFadeAway: point = m_pBalanceData->GetValue("VReplaySkillOneLeg"); break;
	case F4PACKET::SKILL_INDEX::skill_turningShot: point = m_pBalanceData->GetValue("VReplaySkillTurningShot"); break;
	case F4PACKET::SKILL_INDEX::skill_putback: point = m_pBalanceData->GetValue("VReplaySkillPutBack"); break;
	case F4PACKET::SKILL_INDEX::skill_closeReverseLayUp: point = m_pBalanceData->GetValue("VReplaySkillCloseReverseLayup"); break;
	case F4PACKET::SKILL_INDEX::skill_closeReverseDunk: point = m_pBalanceData->GetValue("VReplaySkillCloseReverseDunk"); break;
	case F4PACKET::SKILL_INDEX::skill_jellyLayUp: point = m_pBalanceData->GetValue("VReplaySkillJellyLayup"); break;
	case F4PACKET::SKILL_INDEX::skill_aimShot: point = m_pBalanceData->GetValue("VReplaySkillAimShot"); break;
	case F4PACKET::SKILL_INDEX::skill_reverseDunk: point = m_pBalanceData->GetValue("VReplaySkillReverseDunk"); break;
	case F4PACKET::SKILL_INDEX::skill_runner: point = m_pBalanceData->GetValue("VReplaySkillRunner"); break;
	case F4PACKET::SKILL_INDEX::skill_skipPass: point = m_pBalanceData->GetValue("VReplaySkillSkipPass"); break;
	case F4PACKET::SKILL_INDEX::skill_bulletPass: point = m_pBalanceData->GetValue("VReplaySkillBulletPass"); break;
	case F4PACKET::SKILL_INDEX::skill_slipAndSlide: point = m_pBalanceData->GetValue("VReplaySkillSlipAndSlide"); break;
	case F4PACKET::SKILL_INDEX::skill_penetrateSideStep: point = m_pBalanceData->GetValue("VReplaySkillPenetrateSideStep"); break;
	case F4PACKET::SKILL_INDEX::skill_penetrateBetweenTheLegs: point = m_pBalanceData->GetValue("VReplaySkillPenetrateBetweenTheLegs"); break;
	case F4PACKET::SKILL_INDEX::skill_divingCatchPass: point = m_pBalanceData->GetValue("VReplaySkillDivingCatchPass"); break;
	case F4PACKET::SKILL_INDEX::skill_cutIn: point = m_pBalanceData->GetValue("VReplaySkillCutIn"); break;	
	default: break;
	}

	return point;
}

F4PACKET::SPlayerInformationT* CCharacterManager::FindCharacterInformationWithUserID(DHOST_TYPE_USER_ID UserID)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterInformation()->userid == UserID)
		{
			return iter->second->GetCharacterInformation();
		}
	}
	return nullptr;
}

F4PACKET::SPlayerInformationT* CCharacterManager::FindCharacterInformationWithUserIDAndIsNotAi(DHOST_TYPE_USER_ID UserID)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr && iter->second->GetCharacterInformation()->userid == UserID && iter->second->GetCharacterInformation()->ailevel == kIS_NOT_AI)
		{
			return iter->second->GetCharacterInformation();
		}
	}
	return nullptr;
}

void CCharacterManager::AddCharacterShootInfo(DHOST_TYPE_CHARACTER_SN characterSN, float x, float z, uint32_t zone, bool goal)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->AddCharacterShootInfo(x, z, zone, goal);
	}
}

DHOST_TYPE_CHARACTER_SN CCharacterManager::GetLastCharacterSN()
{
	return m_LastCharacterSN;
}

void CCharacterManager::AddLastCharacterSN()
{
	++m_LastCharacterSN;
}

DHOST_TYPE_BOOL CCharacterManager::GetNoTableCharacterSend()
{
	return m_bNoTableCharacterSend;
}

void CCharacterManager::SetNoTableCharacterSend(DHOST_TYPE_BOOL value)
{
	m_bNoTableCharacterSend = value;
}

void CCharacterManager::PushMatchInfo(int id, int target)
{
	if (id != 0)
	{
		MatchInfo matchInfo;
		matchInfo.id = id;
		matchInfo.target = target;
		m_vecMatchInfos.push_back(matchInfo);
	}
}

void CCharacterManager::ModifyMatchInfo(int id, int target)
{
	for (int i = 0; i < m_vecMatchInfos.size(); i++)
	{
		if (m_vecMatchInfos[i].id == id)
		{
			m_vecMatchInfos[i].target = target;
		}
	}
}

int CCharacterManager::GetMatchInfoID(int target)
{
	if (target != 0)
	{
		for (int i = 0; i < m_vecMatchInfos.size(); i++)
		{
			if (m_vecMatchInfos[i].target == target)
			{
				return m_vecMatchInfos[i].id;
			}
		}
	}

	return 0;
}

int CCharacterManager::GetMatchInfoTarget(int id)
{
	if (id != 0)
	{
		for (int i = 0; i < m_vecMatchInfos.size(); i++)
		{
			if (m_vecMatchInfos[i].id == id)
			{
				return m_vecMatchInfos[i].target;
			}
		}
	}

	return 0;
}

void CCharacterManager::SetSwitchID(DHOST_TYPE_INT32 team, int id)
{
	if (team == 0)
	{
		m_SwitchIDHome = id;
	}
	else
	{
		m_SwitchIDAway = id;
	}
}

int CCharacterManager::GetSwitchID(DHOST_TYPE_INT32 team)
{
	if (team == 0)
	{
		return m_SwitchIDHome;
	}
	else
	{
		return m_SwitchIDAway;
	}
}

std::vector<MatchInfo>& CCharacterManager::GetMatchInfo()
{
	return m_vecMatchInfos;
}

void CCharacterManager::SetSwitchToAi(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetSwitchToAi(value);
	}
}

DHOST_TYPE_BOOL CCharacterManager::GetSwitchToAi(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetSwitchToAi();
	}

	return false;
}

DHOST_TYPE_UINT32 CCharacterManager::GetSpeedHackLevel(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetSpeedHackLevel();
	}

	return kUINT32_INIT;
}

void CCharacterManager::SetSpeedHackLevel(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetSpeedHackLevel(value);
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetSpeedHackTimeAccumulate(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetSpeedHackTimeAccumulate();
	}

	return kFLOAT_INIT;
}

void CCharacterManager::SetSpeedHackTimeAccumulate(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetSpeedHackTimeAccumulate(value);
	}
}

ECHARACTER_KICK_TYPE CCharacterManager::GetCharacterKickType(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterKickType();
	}

	return ECHARACTER_KICK_TYPE::NONE;
}

void CCharacterManager::SetCharacterKickType(DHOST_TYPE_CHARACTER_SN characterSN, ECHARACTER_KICK_TYPE value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetCharacterKickType(value);
	}
}

void CCharacterManager::SetTeamsActive(DHOST_TYPE_INT32 team1, DHOST_TYPE_INT32 team2)
{
	m_teamsActive[0] = team1;
	m_teamsActive[1] = team2;

	m_vecTeamsWait.clear();

	for (int i = 0; i < m_vecTeamsCharacter.size(); i++)
	{
		if (i != team1 && i != team2)
		{
			m_vecTeamsWait.push_back(i);
		}
	}
}

void CCharacterManager::GetTeamsActive(DHOST_TYPE_INT32 & team1, DHOST_TYPE_INT32 & team2)
{
	team1 = m_teamsActive[0];
	team2 = m_teamsActive[1];
}

DHOST_TYPE_BOOL CCharacterManager::CheckTeamsActive(DHOST_TYPE_INT32 teamIndex)
{
	if (m_teamsActive[0] == teamIndex || m_teamsActive[1] == teamIndex)
	{
		return true;
	}

	return false;
}

DHOST_TYPE_INT32 CCharacterManager::GetIndexTeamWait()
{
	if (m_vecTeamsWait.size() > 0)
	{
		return m_vecTeamsWait[0];
	}

	return -1;
}

void CCharacterManager::OnCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->OnCharacterPassive(pData, time);
	}
}


void CCharacterManager::OnCharacterPlayerCeremony(DHOST_TYPE_CHARACTER_SN characterSN, void* pData, DHOST_TYPE_GAME_TIME_F time)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->OnCharacterPlayerCeremony(pData, time);
	}
}

void CCharacterManager::OnCharacterMessage(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 packetID, void* pData, DHOST_TYPE_GAME_TIME_F time, DHOST_TYPE_GAME_TIME_F gameTime, DHOST_TYPE_UINT32 keys, F4PACKET::MOVE_MODE moveMode)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->OnCharacterMessage(packetID, pData, time, moveMode);

		if (m_pMannerManager != nullptr && keys > kUINT32_INIT)
		{
			m_pMannerManager->UpdateAwayFromKeyboard(characterSN, gameTime);

			//string str = "*** UpdateAwayFromKeyboard RoomElapsedTime CharacterSN : " + std::to_string(characterSN) + " gameTime: " + std::to_string(gameTime);
		            //ToLog(str.c_str());

			//LOGGER->Log(str);

		}
	}
}

DHOST_TYPE_BOOL CCharacterManager::GetSecondAnimationReceivePacket(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetSecondAnimationReceivePacket();
	}

	return false;
}

DHOST_TYPE_BOOL CCharacterManager::GetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetShotIsLeftHanded();
	}

	return false;
}

void CCharacterManager::SetShotIsLeftHanded(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetShotIsLeftHanded(value);
	}
}

void CCharacterManager::SetCharacterStateAction(DHOST_TYPE_CHARACTER_SN characterSN, F4PACKET::SPlayerAction sInfo, F4PACKET::ACTION_TYPE actionType, DHOST_TYPE_GAME_TIME_F time)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		iter->second->SetCharacterStateAction(sInfo, actionType, time);
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterBurstGauge();
	}

	return kFLOAT_INIT;
}

EBURST_ON_FIRE_MODE_STEP CCharacterManager::GetCharacterBurstOnFireModeStep(DHOST_TYPE_CHARACTER_SN characterSN)
{
	auto iter = m_mapCharacter.find(characterSN);
	if (iter != m_mapCharacter.end())
	{
		return iter->second->GetCharacterBurstOnFireModeStep();
	}

	return EBURST_ON_FIRE_MODE_STEP::NONE;
}

void CCharacterManager::CheckCharacterBurstGauge(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType, std::vector<DHOST_TYPE_INT32>& teamScores, DHOST_TYPE_BOOL timeWeights)
{
	// �׼��� ��ü
	CCharacter* pActionOwner = GetCharacter(characterSN);

	if (pActionOwner == nullptr)
	{
		return;
	}

	// ���ھ� ����� �� ī��Ʈ ���� ������ ������ ���� ����
	DHOST_TYPE_INT32 ActionOwnerTeamScore = pActionOwner->GetCharacterInformation()->team < teamScores.size() ? teamScores[pActionOwner->GetCharacterInformation()->team] : 0;
	DHOST_TYPE_INT32 OppornentTeamScore = 0;

	if (teamScores.size() > 1)
	{
		DHOST_TYPE_INT32 OppornentTeamScore = teamScores[GetIndexOpponentTeam(pActionOwner->GetCharacterInformation()->team)];
	}
	
	EGAME_SCORE_STATUS ActionOwnerTeam = EGAME_SCORE_STATUS::NONE;
	EGAME_SCORE_STATUS ActionOwnerOpponentTeam = EGAME_SCORE_STATUS::NONE;

	if (ActionOwnerTeamScore > OppornentTeamScore)
	{
		ActionOwnerTeam = EGAME_SCORE_STATUS::WINNING_IN_SCORE;
		ActionOwnerOpponentTeam = EGAME_SCORE_STATUS::LOOSING_IN_SCORE;
	}
	else if (ActionOwnerTeamScore < OppornentTeamScore)
	{
		ActionOwnerTeam = EGAME_SCORE_STATUS::LOOSING_IN_SCORE;
		ActionOwnerOpponentTeam = EGAME_SCORE_STATUS::WINNING_IN_SCORE;
	}
	else
	{
		ActionOwnerTeam = EGAME_SCORE_STATUS::TIE_IN_SCORE;
		ActionOwnerOpponentTeam = EGAME_SCORE_STATUS::TIE_IN_SCORE;
	}

	DHOST_TYPE_FLOAT BurstConsecutiveValue = PushAndCheckBurstConsecutive(pActionOwner, time, actionType);

//#ifdef TEXT_LOG_FILE
//	LOGGER->Log("[BURST] CheckCharacterBurstGauge characterSN : %u, actionType : %u, BurstConsecutiveValue : %f", pActionOwner->GetCharacterInformation()->id, actionType, BurstConsecutiveValue);
//#endif

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			// 1. �׼��� ������ü�� ���� ������ ���
			if (pActionOwner->GetCharacterInformation()->team == iter->second->GetCharacterInformation()->team)
			{
				UpdateCharacterBurstGauge(pActionOwner, iter->second, time, actionType, ActionOwnerTeam, BurstConsecutiveValue, timeWeights);
			}
			else
			{
				UpdateCharacterBurstGauge(pActionOwner, iter->second, time, actionType, ActionOwnerOpponentTeam, BurstConsecutiveValue, timeWeights);
			}
		}
	}
}

void CCharacterManager::UpdateCharacterBurstGauge(CCharacter* pActionOwner, CCharacter* pEffectedCharacter, DHOST_TYPE_GAME_TIME_F OnwerActionTime, uint64_t OwnerActionType, EGAME_SCORE_STATUS EffectedCharacterScoreStatus, DHOST_TYPE_FLOAT BurstConsecutiveValue, DHOST_TYPE_BOOL timeWeights)
{
	// pActionOwner �׼��� ��ü
	// pEffectedCharacter �׼��� ��ü�� ���� ������ �޴� ĳ���� (���� �Ǵ� �����)
	// OnwerActionTime �׼��� ����ð�
	// OwnerActionType � �׼�����
	// EffectedCharacterScoreStatus ������ �޴� ĳ������ ���� ��Ȳ (�̱�� �ִ��� ���� �ִ���, ��������)

	DHOST_TYPE_FLOAT burst_gauge_what = kFLOAT_INIT;	// � �׼��� �ߴ���
	DHOST_TYPE_FLOAT burst_gauge_who = kFLOAT_INIT;		// ��ü�� ������ (��, ����, �����)
	DHOST_TYPE_FLOAT burst_gauge_when = kFLOAT_INIT;	// ���� �ߴ��� (�̱������ ��, ���� ���� ��, ���� ���� ��)
	DHOST_TYPE_FLOAT result_value = kFLOAT_INIT;		// what, who, when �� �հ�

	if ((OwnerActionType & BURST_ACTION_TYPE_SHOT_FGM_FAIL) == BURST_ACTION_TYPE_SHOT_FGM_FAIL)
	{
		pActionOwner->ClearBurstConsecutiveScore();
	}

	burst_gauge_what = CalcBurstGaugeWhat(OwnerActionType);

//#ifdef TEXT_LOG_FILE
//	LOGGER->Log("[BURST] UpdateCharacterBurstGauge characterSN : %u, actionType : %u, burst_gauge_what : %f, BurstConsecutiveValue : %f", pEffectedCharacter->GetCharacterInformation()->id, OwnerActionType, burst_gauge_what, BurstConsecutiveValue);
//#endif

	burst_gauge_what += BurstConsecutiveValue;
	
	burst_gauge_when = CalcBurstGaugeWhen(EffectedCharacterScoreStatus);

	EBURST_ACTION_SUBJECT who = EBURST_ACTION_SUBJECT::NONE;

	if (pActionOwner != nullptr && pEffectedCharacter != nullptr)
	{
		if (pActionOwner->GetCharacterInformation()->id == pEffectedCharacter->GetCharacterInformation()->id)
		{
			who = EBURST_ACTION_SUBJECT::MY;
		}
		else if (pActionOwner->GetCharacterInformation()->team == pEffectedCharacter->GetCharacterInformation()->team)
		{
			who = EBURST_ACTION_SUBJECT::OUR_TEAM;
		}
		else
		{
			who = EBURST_ACTION_SUBJECT::OPPONENT_TEAM;
		}

		burst_gauge_who = CalcBurstGaugeWho(who);

		DHOST_TYPE_FLOAT time_elapsed_one_minute = m_pBalanceData->GetValue("VBURST_EARLY");

		result_value = burst_gauge_what * burst_gauge_who * burst_gauge_when;

		// ���ӽð��� 1�� �̸��̶��
		if (timeWeights && time_elapsed_one_minute > kFLOAT_INIT)
		{
			result_value *= time_elapsed_one_minute;
		}
		//result_value = (burst_gauge_what * 5) * burst_gauge_who * burst_gauge_when;

		DHOST_TYPE_FLOAT currentBurstGauge = pEffectedCharacter->GetCharacterBurstGauge();
		DHOST_TYPE_FLOAT sum_value = currentBurstGauge + result_value;

		DHOST_TYPE_FLOAT BurstMin = m_pBalanceData->GetValue("VBurst_Min");
		DHOST_TYPE_FLOAT BurstMax = m_pBalanceData->GetValue("VBurst_Max");
		DHOST_TYPE_FLOAT OnFire1_Standard = m_pBalanceData->GetValue("VBurst_OnFire1_Standard");
		DHOST_TYPE_FLOAT OnFire2_Standard = m_pBalanceData->GetValue("VBurst_OnFire2_Standard");

		if (sum_value < BurstMin)
		{
			sum_value = BurstMin;
		}

		if (sum_value > BurstMax)
		{
			sum_value = BurstMax;
		}

//#ifdef TEXT_LOG_FILE
//		LOGGER->Log("[BURST] UpdateCharacterBurstGauge OnFire1_Standard : %f, OnFire2_Standard : %f", OnFire1_Standard, OnFire2_Standard);
//		LOGGER->Log("[BURST] UpdateCharacterBurstGauge characterSN : %u, actionType : %u, burst_gauge_what : %f, burst_gauge_who : %f, burst_gauge_when : %f", pEffectedCharacter->GetCharacterInformation()->id, OwnerActionType, burst_gauge_what, burst_gauge_who, burst_gauge_when);
//		LOGGER->Log("[BURST] UpdateCharacterBurstGauge characterSN : %u, currentBurstGauge : %f, result_value : %f", pEffectedCharacter->GetCharacterInformation()->id, currentBurstGauge, result_value);
//		LOGGER->Log("[BURST] UpdateCharacterBurstGauge characterSN : %u, sum_value : %f, EffectedCharacterScoreStatus : %d", pEffectedCharacter->GetCharacterInformation()->id, sum_value, EffectedCharacterScoreStatus);
//#endif

		DHOST_TYPE_BOOL pushVecData = false;
		DHOST_TYPE_FLOAT OnFireValue = kFLOAT_INIT;
		EBURST_ON_FIRE_MODE_STEP character_current_burst_step = pEffectedCharacter->GetCharacterBurstOnFireModeStep();
		EBURST_ON_FIRE_MODE_STEP character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::NONE;

		switch (character_current_burst_step)
		{
			case EBURST_ON_FIRE_MODE_STEP::NONE:
			{
				if (sum_value >= OnFire1_Standard)
				{
					// �����̾� ��� �ߵ�
					pushVecData = true;
					OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
					character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_ONE;
					pEffectedCharacter->SetBurstActionReleaseTriggerTime(kFLOAT_INIT);
				}
			}
			break;
			case EBURST_ON_FIRE_MODE_STEP::STEP_ONE:
			{
				if (sum_value < OnFire1_Standard)
				{
					if (pEffectedCharacter->GetBurstActionReleaseTriggerTime() == kFLOAT_INIT)
					{
						pEffectedCharacter->SetBurstActionReleaseTriggerTime(OnwerActionTime);
					}
				}
				else
				{
					if (sum_value >= OnFire2_Standard)
					{
						// �����̾� ��� �ߵ�
						pushVecData = true;
						OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire2_Value");
						character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_TWO;
						pEffectedCharacter->SetBurstActionReleaseTriggerTime(kFLOAT_INIT);
					}
				}
			}
			break;
			case EBURST_ON_FIRE_MODE_STEP::STEP_TWO:
			{
				if (sum_value < OnFire2_Standard)
				{
					if (pEffectedCharacter->GetBurstActionReleaseTriggerTime() == kFLOAT_INIT)
					{
						pEffectedCharacter->SetBurstActionReleaseTriggerTime(OnwerActionTime);
					}
				}
			}
			break;
			default:
				break;
		}

		pEffectedCharacter->SetBurstGauge(sum_value);
		
		if (pushVecData)
		{
			UpdateCharacterOnFireModeStatBuff(pEffectedCharacter, OnFireValue);
			PushSendOnFireModeCharacter(pEffectedCharacter->GetCharacterInformation()->id);
			pEffectedCharacter->SetCharacterBurstOnFireModeStep(character_update_burst_step);
		}
	}
}

// ĳ������ �нú�� ������ ����Ʈ �������� ������ų ��
void CCharacterManager::UpdateCharacterBurstGaugeWithPassiveActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest)
{
	// �׼��� ��ü
	CCharacter* pActionOwner = GetCharacter(characterSN);

	if (pActionOwner == nullptr)
	{
		return;
	}

	DHOST_TYPE_FLOAT value = kFLOAT_INIT;

	switch (burstRequest)
	{
	case F4PACKET::BURST_REQUEST::burstRequest_passiveWang:
		value = m_pBalanceData->GetValue("VPassive_Burst_Wang");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveMei:
		value = m_pBalanceData->GetValue("VPassive_Burst_Mei");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveGola:
		value = m_pBalanceData->GetValue("VPassive_Burst_Gola");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveDiego:
		value = m_pBalanceData->GetValue("VPassive_Burst_Diego");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveKido:
		{
			for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
			{
				if (iter->second != nullptr)
				{
					if (iter->second->GetCharacterInformation()->characterid != characterSN) continue;

					if (iter->second->GetCharacterInformation()->specialcharacterlevel == 1)
					{
						DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_Burst_Kido_Special");
						DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_Burst_Kido");
						DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
						value = b + (a - b) * c;
					}
					else
					if (iter->second->GetCharacterInformation()->specialcharacterlevel == 2)
					{
						value = m_pBalanceData->GetValue("VPassive_Burst_Kido_Special");
					}
					else
					{
						value = m_pBalanceData->GetValue("VPassive_Burst_Kido");
					}
				}
			}
		}

		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveWuko:
		value = m_pBalanceData->GetValue("VPassive_Burst_Wuko");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveHayato:
		{
			for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
			{
				if (iter->second != nullptr)
				{
					if (iter->second->GetCharacterInformation()->characterid != characterSN) continue;

					if (iter->second->GetCharacterInformation()->specialcharacterlevel == 1)
					{
						DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_Burst_Hayato_Special");
						DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_Burst_Hayato");
						DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
						value = b + (a - b) * c;
					}
					else
					if (iter->second->GetCharacterInformation()->specialcharacterlevel == 2)
					{
						value = m_pBalanceData->GetValue("VPassive_Burst_Hayato_Special");
					}
					else
					{
						value = m_pBalanceData->GetValue("VPassive_Burst_Hayato");
					}
				}
			}
			//value = m_pBalanceData->GetValue("VPassive_Burst_Hayato");
		}
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveAsami:
	case F4PACKET::BURST_REQUEST::burstRequest_passiveSarang:
		value = m_pBalanceData->GetValue("VBurst_Max");
		break;
	case F4PACKET::BURST_REQUEST::burstRequest_passiveJacky:
	{
		DHOST_TYPE_FLOAT valueB = kFLOAT_INIT;

		for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
		{
			if (iter->second != nullptr)
			{
				if (iter->second->GetCharacterInformation()->characterid != characterSN) continue;

				if (iter->second->GetCharacterInformation()->specialcharacterlevel == 1)
				{
					DHOST_TYPE_FLOAT a = m_pBalanceData->GetValue("VPassive_Burst_Jacky_Penalty_Special");
					DHOST_TYPE_FLOAT b = m_pBalanceData->GetValue("VPassive_Burst_Jacky_Penalty");
					DHOST_TYPE_FLOAT c = m_pBalanceData->GetValue("VPassive_Special_2");
					valueB = b + (a - b) * c;
				}
				else
				if (iter->second->GetCharacterInformation()->specialcharacterlevel == 2)
				{
					valueB = m_pBalanceData->GetValue("VPassive_Burst_Jacky_Penalty_Special");
				}
				else
				{
					valueB = m_pBalanceData->GetValue("VPassive_Burst_Jacky_Penalty");
				}
			}
		}

		value = m_pBalanceData->GetValue("VPassive_Burst_Jacky_Benefit");
		CharacterBurstGaugeWithAll(pActionOwner, value, valueB, burstRequest);
		return;
	}
	default:
		break;
	}

	if (value == kFLOAT_INIT)
	{
		return;
	}

	CharacterBurstGaugeWith(pActionOwner, value, burstRequest);
}

// ĳ������ �޴޷� ������ ����Ʈ �������� ������ų ��
void CCharacterManager::UpdateCharacterBurstGaugeWithMedalActivation(DHOST_TYPE_CHARACTER_SN characterSN, BURST_REQUEST burstRequest)
{
	// �׼��� ��ü
	CCharacter* pActionOwner = GetCharacter(characterSN);

	if (pActionOwner == nullptr)
	{
		return;
	}

	DHOST_TYPE_FLOAT value = kFLOAT_INIT;

	switch (burstRequest)
	{
	case F4PACKET::BURST_REQUEST::burstRequest_medalDefensiveLeader:
	{
		for (const auto& medalInfo : pActionOwner->GetCharacterInformation()->medals)
		{
			if (medalInfo.index() != F4PACKET::MEDAL_INDEX::defensiveLeader)
				continue;

			value = m_pBalanceData->GetValue("VBurst_Block_Medal_DefensiveLeader") * medalInfo.value();
		}
	}
	break;
	default:
		break;
	}

	if (value == kFLOAT_INIT)
	{
		return;
	}

	CharacterBurstGaugeWith(pActionOwner, value, burstRequest);
}

// ������ ����Ʈ �������� ������ų ��
void CCharacterManager::CharacterBurstGaugeWith(CCharacter* pActionOwner, DHOST_TYPE_FLOAT burstValue, BURST_REQUEST burstRequest)
{
	// �׼��� ��ü
	if (pActionOwner == nullptr)
	{
		return;
	}

	if (burstValue == kFLOAT_INIT)
	{
		return;
	}

	DHOST_TYPE_FLOAT BurstMin = m_pBalanceData->GetValue("VBurst_Min");
	DHOST_TYPE_FLOAT BurstMax = m_pBalanceData->GetValue("VBurst_Max");
	DHOST_TYPE_FLOAT OnFire1_Standard = m_pBalanceData->GetValue("VBurst_OnFire1_Standard");
	DHOST_TYPE_FLOAT OnFire2_Standard = m_pBalanceData->GetValue("VBurst_OnFire2_Standard");

	DHOST_TYPE_FLOAT OnFireValue = kFLOAT_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			DHOST_TYPE_BOOL bCheck = false;
			
			if (burstRequest == F4PACKET::BURST_REQUEST::burstRequest_passiveWuko)
			{
				if (pActionOwner->GetCharacterInformation()->id == iter->second->GetCharacterInformation()->id)
				{
					bCheck = true;
				}
			}
			else if (burstRequest == F4PACKET::BURST_REQUEST::burstRequest_passiveAsami)
			{
				if (pActionOwner->GetCharacterInformation()->id == iter->second->GetCharacterInformation()->id)
				{
					bCheck = true;
				}
			}
			else if (burstRequest == F4PACKET::BURST_REQUEST::burstRequest_passiveSarang)
			{
				if (pActionOwner->GetCharacterInformation()->id == iter->second->GetCharacterInformation()->id)
				{
					bCheck = true;
				}
			}
			else
			{
				if (pActionOwner->GetCharacterInformation()->team == iter->second->GetCharacterInformation()->team)
				{
					bCheck = true;
				}
			}

			if (bCheck)
			{
				DHOST_TYPE_FLOAT currentBurstGauge = iter->second->GetCharacterBurstGauge();
				DHOST_TYPE_FLOAT sum_value = currentBurstGauge + burstValue;

				DHOST_TYPE_BOOL pushVecData = false;
				EBURST_ON_FIRE_MODE_STEP character_current_burst_step = iter->second->GetCharacterBurstOnFireModeStep();
				EBURST_ON_FIRE_MODE_STEP character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::NONE;

				switch (character_current_burst_step)
				{
					case EBURST_ON_FIRE_MODE_STEP::NONE:
					{
						if (sum_value >= OnFire1_Standard)
						{
							// �����̾� ��� �ߵ�
							pushVecData = true;
							OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
							character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_ONE;
						}
					}
					break;
					case EBURST_ON_FIRE_MODE_STEP::STEP_ONE:
					{
						if (sum_value >= OnFire2_Standard)
						{
							// �����̾� ��� �ߵ�
							pushVecData = true;
							OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire2_Value");
							character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_TWO;
						}
					}
					break;
					case EBURST_ON_FIRE_MODE_STEP::STEP_TWO:
					{

					}
					break;
					default:
						break;
				}

				iter->second->SetBurstGauge(sum_value);

				if (pushVecData)
				{
					UpdateCharacterOnFireModeStatBuff(iter->second, OnFireValue);
					PushSendOnFireModeCharacter(iter->second->GetCharacterInformation()->id);
					iter->second->SetCharacterBurstOnFireModeStep(character_update_burst_step);
				}
			}
		}
	}
}

// ����� ����Ʈ �������� ������ų ��
void CCharacterManager::CharacterBurstGaugeWithAll(CCharacter* pActionOwner, DHOST_TYPE_FLOAT burstValueTeamA, DHOST_TYPE_FLOAT burstValueTeamB, BURST_REQUEST burstRequest)
{
	// �׼��� ��ü
	if (pActionOwner == nullptr)
	{
		return;
	}

	if (burstValueTeamA == kFLOAT_INIT && burstValueTeamB == kFLOAT_INIT)
	{
		return;
	}

	DHOST_TYPE_FLOAT BurstMin = m_pBalanceData->GetValue("VBurst_Min");
	DHOST_TYPE_FLOAT BurstMax = m_pBalanceData->GetValue("VBurst_Max");
	DHOST_TYPE_FLOAT OnFire1_Standard = m_pBalanceData->GetValue("VBurst_OnFire1_Standard");
	DHOST_TYPE_FLOAT OnFire2_Standard = m_pBalanceData->GetValue("VBurst_OnFire2_Standard");

	DHOST_TYPE_FLOAT OnFireValue = kFLOAT_INIT;

	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second == nullptr)
			return;

		DHOST_TYPE_FLOAT burstValue = kFLOAT_INIT;
		if (pActionOwner->GetCharacterInformation()->team == iter->second->GetCharacterInformation()->team)
			burstValue = burstValueTeamA;
		else
			burstValue = burstValueTeamB;

		DHOST_TYPE_FLOAT currentBurstGauge = iter->second->GetCharacterBurstGauge();
		DHOST_TYPE_FLOAT sum_value = currentBurstGauge + burstValue;

		EBURST_ON_FIRE_MODE_STEP character_current_burst_step = iter->second->GetCharacterBurstOnFireModeStep();
		EBURST_ON_FIRE_MODE_STEP character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::NONE;


		if (sum_value >= OnFire2_Standard)
		{
			// �����̾� ��� 2
			character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_TWO;

			if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::NONE)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
				OnFireValue += m_pBalanceData->GetValue("VBurst_OnFire2_Value");
			}
			else if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::STEP_ONE)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire2_Value");
			}
		}
		else if (sum_value >= OnFire1_Standard)
		{
			// �����̾� ��� 1
			character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::STEP_ONE;

			if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::NONE)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
			}
			else if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::STEP_TWO)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire2_Value");
				OnFireValue *= -1.0f;
			}
		}
		else
		{
			// �����̾� ��� ����
			character_update_burst_step = EBURST_ON_FIRE_MODE_STEP::NONE;

			if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::STEP_ONE)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
				OnFireValue *= -1.0f;
			}
			else if (character_current_burst_step == EBURST_ON_FIRE_MODE_STEP::STEP_TWO)
			{
				OnFireValue = m_pBalanceData->GetValue("VBurst_OnFire1_Value");
				OnFireValue += m_pBalanceData->GetValue("VBurst_OnFire2_Value");
				OnFireValue *= -1.0f;
			}
		}

		iter->second->SetBurstGauge(sum_value);

		if (character_current_burst_step != character_update_burst_step)
		{
			UpdateCharacterOnFireModeStatBuff(iter->second, OnFireValue);
			PushSendOnFireModeCharacter(iter->second->GetCharacterInformation()->id);
			iter->second->SetCharacterBurstOnFireModeStep(character_update_burst_step);
		}
	}
}

DHOST_TYPE_FLOAT CCharacterManager::CalcBurstGaugeWhat(uint64_t ActionType)
{
	DHOST_TYPE_FLOAT value = kFLOAT_INIT;

	if (ActionType & BURST_ACTION_TYPE_SHOT)
	{
		if ((ActionType & BURST_ACTION_TYPE_SHOT_TWO) == BURST_ACTION_TYPE_SHOT_TWO)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_TWO");			// 2�� ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_TWO_TIE) == BURST_ACTION_TYPE_SHOT_TWO_TIE)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_TWO_TIE");		// 2���� �����Ͽ� ������ ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_REVERSAL) == BURST_ACTION_TYPE_SHOT_REVERSAL)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_TWO_REVERSAL");	// 2���� �����Ͽ� ������ ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_FAIL) == BURST_ACTION_TYPE_SHOT_FAIL)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_TWO_FAIL");		// 2�� ���� ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_DUNK) == BURST_ACTION_TYPE_SHOT_DUNK)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_TWO_DUNK");		// ��ũ�� ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_THREE) == BURST_ACTION_TYPE_SHOT_THREE)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_THREE");		// 3�� ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_THREE_TIE) == BURST_ACTION_TYPE_SHOT_THREE_TIE)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_THREE_TIE");	// 3���� �����Ͽ� ������ ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_THREE_REVERSAL) == BURST_ACTION_TYPE_SHOT_THREE_REVERSAL)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_THREE_REVERSAL");	// 3���� �����Ͽ� ������ ����
		}

		if ((ActionType & BURST_ACTION_TYPE_SHOT_THREE_FAIL) == BURST_ACTION_TYPE_SHOT_THREE_FAIL)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_SCORE_THREE_FAIL");	// 3�� ���� ����
		}

		if ((ActionType & BURST_ACTION_TYPE_BLOCKED) == BURST_ACTION_TYPE_BLOCKED)
		{
			value += m_pBalanceData->GetValue("VBURST_WHAT_BLOCKED");		// ���� ����
		}
	}
	else if ((ActionType & BURST_ACTION_TYPE_REBOUND) == BURST_ACTION_TYPE_REBOUND)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_REBOUND");		// ���ٿ�� ����
	}
	else if ((ActionType & BURST_ACTION_TYPE_STEAL) == BURST_ACTION_TYPE_STEAL)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_STEAL");			// ��ƿ ����
	}
	else if ((ActionType & BURST_ACTION_TYPE_BLOCK) == BURST_ACTION_TYPE_BLOCK)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_BLOCK");			// ���� ����
	}
	else if ((ActionType & BURST_ACTION_TYPE_LOOSE_BALL) == BURST_ACTION_TYPE_LOOSE_BALL)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_LOOSE_BALL");		// ��� ȹ��
	}
	else if ((ActionType & BURST_ACTION_TYPE_TURN_OVER) == BURST_ACTION_TYPE_TURN_OVER)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_TURN_OVER");		// �Ͽ����� �߻���Ŵ
	}
	else if ((ActionType & BURST_ACTION_TYPE_ANKLE_BREAK) == BURST_ACTION_TYPE_ANKLE_BREAK)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_ANKLE_BREAK");	// ��Ŭ �극��Ŀ ����
	}
	else if ((ActionType & BURST_ACTION_TYPE_ANKLE_BREAKED) == BURST_ACTION_TYPE_ANKLE_BREAKED)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_ANKLE_BREAKED");	// ��Ŭ �극��Ŀ ����
	}
	else if ((ActionType & BURST_ACTION_TYPE_ASSIST) == BURST_ACTION_TYPE_ASSIST)
	{
		value += m_pBalanceData->GetValue("VBURST_WHAT_ASSIST");	// ��Ŭ �극��Ŀ ����
	}

	return value;
}

DHOST_TYPE_FLOAT CCharacterManager::CalcBurstGaugeWho(EBURST_ACTION_SUBJECT who)
{
	DHOST_TYPE_FLOAT value = kFLOAT_INIT;

	if (who == EBURST_ACTION_SUBJECT::MY)
	{
		value = m_pBalanceData->GetValue("VBURST_WHO_MY");
	}
	else if (who == EBURST_ACTION_SUBJECT::OUR_TEAM)
	{
		value = m_pBalanceData->GetValue("VBURST_WHO_OURTEAM");
	}
	else if (who == EBURST_ACTION_SUBJECT::OPPONENT_TEAM)
	{
		value = m_pBalanceData->GetValue("VBURST_WHO_OPPONENTTEAM");
	}

	return value;
}

DHOST_TYPE_FLOAT CCharacterManager::CalcBurstGaugeWhen(EGAME_SCORE_STATUS when)
{
	DHOST_TYPE_FLOAT value = kFLOAT_INIT;

	if (when == EGAME_SCORE_STATUS::TIE_IN_SCORE)
	{
		value = m_pBalanceData->GetValue("VBURST_WHEN_TIE_IN_SCORE");
	}
	else if (when == EGAME_SCORE_STATUS::WINNING_IN_SCORE)
	{
		value = m_pBalanceData->GetValue("VBURST_WHEN_WINNING_IN_SCORE");
	}
	else if (when == EGAME_SCORE_STATUS::LOOSING_IN_SCORE)
	{
		value = m_pBalanceData->GetValue("VBURST_WHEN_LOOSING_IN_SCORE");
	}

	return value;
}

void CCharacterManager::UpdateCharacterOnFireModeStatBuff(CCharacter* pCharacter, DHOST_TYPE_FLOAT OnFireValue, DHOST_TYPE_BOOL Update)
{
	if (pCharacter == nullptr)
	{
		return;
	}

	CAbility* pTargetAbility = pCharacter->GetAbility();

	if (pTargetAbility == nullptr)
	{
		return;
	}

	if (Update)
	{
		F4PACKET::CHARACTER_PASSIVE_TYPE passive_type = pCharacter->GetCharacterInformation()->passivetype;

		DHOST_TYPE_FLOAT value = kFLOAT_INIT;

		for (int i = static_cast<int>(F4PACKET::ABILITY_TYPE::MIN); i <= static_cast<int>(F4PACKET::ABILITY_TYPE::MAX); ++i)
		{
			F4PACKET::ABILITY_TYPE type = static_cast<F4PACKET::ABILITY_TYPE>(i);

			value = pTargetAbility->GetModificationAbility(type) + OnFireValue;

			pTargetAbility->SetModificationAbility(type, value);
		}

		RemoveSendOnFireModeCharacterReservation(pCharacter->GetCharacterInformation()->id);
	}
	else
	{
		//! ����
		PushSendOnFireModeCharacterReservation(pCharacter->GetCharacterInformation()->id, OnFireValue);
	}
}

DHOST_TYPE_UINT64 CCharacterManager::GetBurstActionTypeWithRecordType(ECHARACTER_RECORD_TYPE recordType)
{
	DHOST_TYPE_UINT64 burst_action_type = BURST_ACTION_TYPE;

	switch (recordType)
	{
		case ECHARACTER_RECORD_TYPE::BLOCK:
		{
			burst_action_type |= BURST_ACTION_TYPE_BLOCK;
		}
		break;
		case ECHARACTER_RECORD_TYPE::LOOSE_BALL:
		{
			burst_action_type |= BURST_ACTION_TYPE_LOOSE_BALL;
		}
		break;
		case ECHARACTER_RECORD_TYPE::REBOUND:
		{
			burst_action_type |= BURST_ACTION_TYPE_REBOUND;
		}
		break;
		case ECHARACTER_RECORD_TYPE::STEAL:
		{
			burst_action_type |= BURST_ACTION_TYPE_STEAL;
		}
		break;
		case ECHARACTER_RECORD_TYPE::TURN_OVER:
		{
			burst_action_type |= BURST_ACTION_TYPE_TURN_OVER;
		}
		case ECHARACTER_RECORD_TYPE::ASSIST:
		{
			burst_action_type |= BURST_ACTION_TYPE_ASSIST;
		}
		break;
		default:
			break;
	}
	
	return burst_action_type;
}

void CCharacterManager::CheckCharacterOnFireModeRelease(CCharacter* pCharacter, DHOST_TYPE_FLOAT gameTime)
{
	if (pCharacter == nullptr)
	{
		return;
	}

	if (pCharacter->GetCharacterBurstGauge() >= m_pBalanceData->GetValue("VBurst_Subtract_Standard"))
	{
		if (pCharacter->GetBurstActionSubtractTime() == kFLOAT_INIT)
		{
			pCharacter->SetBurstActionSubtractTime(gameTime);
		}
		
		if (pCharacter->GetBurstActionSubtractTime() - gameTime > 1.f)
		{
			DHOST_TYPE_FLOAT subtract = pCharacter->GetCharacterBurstGauge() - m_pBalanceData->GetValue("VBurst_Subtract_Value");
			pCharacter->SetBurstGauge(subtract);
			pCharacter->SetBurstActionSubtractTime(gameTime);

			EBURST_ON_FIRE_MODE_STEP character_current_burst_step = pCharacter->GetCharacterBurstOnFireModeStep();

			switch (character_current_burst_step)
			{
				case EBURST_ON_FIRE_MODE_STEP::STEP_ONE:
				{
					if (pCharacter->GetCharacterBurstGauge() < m_pBalanceData->GetValue("VBurst_OnFire1_Standard") && pCharacter->GetBurstActionReleaseTriggerTime() == kFLOAT_INIT)
					{
						pCharacter->SetBurstActionReleaseTriggerTime(gameTime);
					}
				}
				break;
				case EBURST_ON_FIRE_MODE_STEP::STEP_TWO:
				{
					if (pCharacter->GetCharacterBurstGauge() < m_pBalanceData->GetValue("VBurst_OnFire2_Standard") && pCharacter->GetBurstActionReleaseTriggerTime() == kFLOAT_INIT)
					{
						pCharacter->SetBurstActionReleaseTriggerTime(gameTime);
					}
				}
				break;
				default:
					break;
			}
		}
	}

	if (pCharacter->GetBurstActionReleaseTriggerTime() != kFLOAT_INIT && pCharacter->GetCharacterBurstOnFireModeStep() > EBURST_ON_FIRE_MODE_STEP::NONE)
	{
		if (pCharacter->GetBurstActionReleaseTriggerTime() - gameTime > m_pBalanceData->GetValue("VBurst_OnFire_Time"))
		{
			//if (pCharacter->GetCharacterBurstGauge() < m_pBalanceData->GetValue("VBurst_OnFire1_Standard"))
			if(pCharacter->GetCharacterBurstOnFireModeStep() == EBURST_ON_FIRE_MODE_STEP::STEP_ONE)
			{
				UpdateCharacterOnFireModeStatBuff(pCharacter, -m_pBalanceData->GetValue("VBurst_OnFire1_Value"));
				pCharacter->SetCharacterBurstOnFireModeStep(EBURST_ON_FIRE_MODE_STEP::NONE);
				PushSendOnFireModeCharacter(pCharacter->GetCharacterInformation()->id);
				pCharacter->SetBurstActionReleaseTriggerTime(kFLOAT_INIT);
			}
			else
			{
				UpdateCharacterOnFireModeStatBuff(pCharacter, -m_pBalanceData->GetValue("VBurst_OnFire2_Value"));
				pCharacter->SetCharacterBurstOnFireModeStep(EBURST_ON_FIRE_MODE_STEP::STEP_ONE);
				PushSendOnFireModeCharacter(pCharacter->GetCharacterInformation()->id);
				pCharacter->SetBurstActionReleaseTriggerTime(kFLOAT_INIT);
			}
		}
	}
}

void CCharacterManager::PushSendOnFireModeCharacter(DHOST_TYPE_CHARACTER_SN characterSN)
{
	m_vecSendOnFireModeCharacter.push_back(characterSN);

	if (m_CallbackOnFireMode != nullptr)
	{
		m_CallbackOnFireMode();
	}
}

void CCharacterManager::PushSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapSendOnFireModeCharacterReservation.find(characterSN);

	if (iter == m_mapSendOnFireModeCharacterReservation.end())
	{
		m_mapSendOnFireModeCharacterReservation.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>(characterSN, value));
	}
}

void CCharacterManager::ClearSendOnFireModeCharacter()
{
	m_vecSendOnFireModeCharacter.clear();
}

void CCharacterManager::ClearSendOnFireModeCharacterReservation()
{
	m_mapSendOnFireModeCharacterReservation.clear();
}

void CCharacterManager::RemoveSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapSendOnFireModeCharacterReservation.find(characterSN);

	if (iter == m_mapSendOnFireModeCharacterReservation.end())
		return;

	m_mapSendOnFireModeCharacterReservation.erase(iter);
}

std::vector<DHOST_TYPE_CHARACTER_SN> CCharacterManager::GetSendOnFireModeCharacter()
{
	return m_vecSendOnFireModeCharacter;
}

std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT> CCharacterManager::GetSendOnFireModeCharacterReservation()
{
	return m_mapSendOnFireModeCharacterReservation;
}

DHOST_TYPE_FLOAT CCharacterManager::FindSendOnFireModeCharacterReservation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>::iterator iter = m_mapSendOnFireModeCharacterReservation.find(characterSN);

	if (iter != m_mapSendOnFireModeCharacterReservation.end())
	{
		result = iter->second;
	}

	return result;
}

DHOST_TYPE_BOOL CCharacterManager::CheckCharacterActionPenetrateType(F4PACKET::ACTION_TYPE type)
{
	DHOST_TYPE_BOOL return_value = false;

	switch (type)
	{
		case F4PACKET::ACTION_TYPE::action_penetrate:
		case F4PACKET::ACTION_TYPE::action_penetratePostUp:
		case F4PACKET::ACTION_TYPE::action_penetrateReady:
			return true;
	}

	return return_value;
}

void CCharacterManager::GetAllCharacterBurstGauge(std::map<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>& map)
{
	for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
	{
		if (iter->second != nullptr)
		{
			map.insert(std::pair<DHOST_TYPE_CHARACTER_SN, DHOST_TYPE_FLOAT>(iter->first, iter->second->GetCharacterBurstGauge()));
		}
	}
}

DHOST_TYPE_FLOAT CCharacterManager::PushAndCheckBurstConsecutive(CCharacter* pCharacter, DHOST_TYPE_GAME_TIME_F time, uint64_t actionType)
{
	DHOST_TYPE_FLOAT result_value = kFLOAT_INIT;

	if (pCharacter == nullptr)
	{
		return result_value;
	}

	if ((actionType & BURST_ACTION_TYPE_SHOT_FGM) == BURST_ACTION_TYPE_SHOT_FGM)
	{
#ifdef TEXT_LOG_FILE
		//LOGGER->Log("[BURST] PushAndCheckBurstConsecutive BURST_ACTION_TYPE_SHOT_FGM characterSN : %u, time : %f", pCharacter->GetCharacterInformation()->id, time);
#endif

		result_value = pCharacter->PushAndCheckBurstConsecutiveScore(time);

		if (result_value == 1)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_SCORE_STRAIGHT_TWICE");
		}
		else if (result_value == 2)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_SCORE_STRAIGHT_THRICE");
		}
	}
	else if ((actionType & BURST_ACTION_TYPE_STEAL) == BURST_ACTION_TYPE_STEAL)
	{
#ifdef TEXT_LOG_FILE
		//LOGGER->Log("[BURST] PushAndCheckBurstConsecutive BURST_ACTION_TYPE_STEAL characterSN : %u, time : %f", pCharacter->GetCharacterInformation()->id, time);
#endif

		result_value = pCharacter->PushAndCheckBurstConsecutiveSteal(time);

		if (result_value == 1)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_STEAL_TWICE");
		}
		else if (result_value == 2)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_STEAL_THRICE");
		}
	}
	else if ((actionType & BURST_ACTION_TYPE_BLOCK) == BURST_ACTION_TYPE_BLOCK)
	{
//#ifdef TEXT_LOG_FILE
//		LOGGER->Log("[BURST] PushAndCheckBurstConsecutive BURST_ACTION_TYPE_BLOCK characterSN : %u, time : %f", pCharacter->GetCharacterInformation()->id, time);
//#endif

		result_value = pCharacter->PushAndCheckBurstConsecutiveBlock(time);

		if (result_value == 1)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_BLOCK_TWICE");
		}
		else if (result_value == 2)
		{
			result_value = m_pBalanceData->GetValue("VBURST_WHAT_BLOCK_THRICE");
		}
	}
		
	return result_value;
}

void CCharacterManager::IncreaseLatencyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseLatencyCount(value);
	}
}

void CCharacterManager::SetAvgPingLatency(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetAvgPingLatency(value);
	}
}

void CCharacterManager::GetBadMannerBalanceData(SBadMannerBalanceData& data)
{
	data.IntentionalBallHogPlayRemainGameTime	= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_REMAIN_GAME_TIME");
	data.IntentionalBallHogPlayOwnBall			= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_OWN_BALL");
	data.IntentionalBallHogPlayCount1			= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_COUNT_1");
	data.IntentionalBallHogPlayCount2			= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_COUNT_2");
	data.IntentionalBallHogPlayCount3			= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_COUNT_3");
	data.IntentionalBallHogPlayCount4			= m_pBalanceData->GetValue("VBAD_MANNER_INTENTIONAL_BALL_HOG_PLAY_COUNT_4");

	data.AwayFromKeyboardOwnBall				= m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_OWN_BALL");
	
	/* // 유저별로 다르게 적용해야 하므로 캐릭터로 옮김 
	if (m_pHost->GetModeType() == EMODE_TYPE::TRIO || m_pHost->GetModeType() == EMODE_TYPE::TRIO_PVP_MIXED)
	{
		data.AwayFromKeyboardNothing = 3.0f;
	}
	else
	{
		data.AwayFromKeyboardNothing = m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_NOTHING");
	}
	*/

	data.AwayFromKeyboardCount1					= m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_COUNT_1");
	data.AwayFromKeyboardCount2					= m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_COUNT_2");
	data.AwayFromKeyboardCount3					= m_pBalanceData->GetValue("VBAD_MANNER_AWAY_FROM_KEYBOARD_COUNT_3");

	data.ImpossibleShootingShotClock			= m_pBalanceData->GetValue("VBAD_MANNER_IMPOSSIBLE_SHOOTING_SHOT_CLOCK");
	data.ImpossibleShootingCount1				= m_pBalanceData->GetValue("VBAD_MANNER_IMPOSSIBLE_SHOOTING_COUNT_1");
	data.ImpossibleShootingCount2				= m_pBalanceData->GetValue("VBAD_MANNER_IMPOSSIBLE_SHOOTING_COUNT_2");
	data.ImpossibleShootingCount3				= m_pBalanceData->GetValue("VBAD_MANNER_IMPOSSIBLE_SHOOTING_COUNT_3");
	data.ImpossibleShootingCount4				= m_pBalanceData->GetValue("VBAD_MANNER_IMPOSSIBLE_SHOOTING_COUNT_4");
}

void CCharacterManager::RegistCallbackFuncBadManner(BadMannerCallback pFunc)
{
	m_CallbackBadManner = std::move(pFunc);
}

void CCharacterManager::IncreaseBadMannerCount(DHOST_TYPE_CHARACTER_SN characterSN, EBAD_MANNER_TYPE type)
{
	DHOST_TYPE_CHARACTER_SN update_character_sn = kCHARACTER_SN_INIT;
	DHOST_TYPE_UINT16 type_count = kUINT16_INIT;

	CCharacter* pCharacter = GetCharacter(characterSN);	// ��ųʸ� ����Ų ĳ����

	if (pCharacter != nullptr)
	{
		if (pCharacter->GetPlayerNumber() == 2 || pCharacter->GetPlayerNumber() == 3)	// Ʈ���� ����� ���� 1p, 2p, 3p �� �����Ѵ�. (2,3p�� ������ �÷����ϴ� ai ĳ����)
		{
			// ���� ���� 1p �� ã��
			CCharacter* pTargetCharacter = GetCharacterTeamWithFirstPlayerNumber(characterSN);

			if (pTargetCharacter != nullptr)
			{
				pTargetCharacter->IncreaseBadMannerCount(type);

				update_character_sn = pTargetCharacter->GetCharacterInformation()->id;
				type_count = pTargetCharacter->GetBadMannerTypeCount(type);
			}
		}
		else
		{
			pCharacter->IncreaseBadMannerCount(type);

			update_character_sn = pCharacter->GetCharacterInformation()->id;
			type_count = pCharacter->GetBadMannerTypeCount(type);
		}

		if (m_CallbackBadManner != nullptr)
		{
			m_CallbackBadManner(update_character_sn, (uint16_t)type, type_count);
		}
	}
}

void CCharacterManager::UpdateAwayFromKeyboardTimeAccumulate(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	DHOST_TYPE_FLOAT AwayFromKeyboardGameTime = kFLOAT_INIT;

	if (pCharacter != nullptr)
	{
		AwayFromKeyboardGameTime = pCharacter->GetAwayFromKeyboardGameTime();

		pCharacter->UpdateAwayFromKeyboardTimeAccumulate(AwayFromKeyboardGameTime - gameTime);

		pCharacter->SetAwayFromKeyboardGameTime(gameTime);

		pCharacter->SetAI(false);
	}
}

void CCharacterManager::UpdateAwayFromKeyboardGameTime(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetAwayFromKeyboardGameTime(gameTime);
	}
}

void CCharacterManager::InitMannerManagerData(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo, DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->InitMannerManagerData(gameTime, ballNo, characterSN);

		if (characterSN == kCHARACTER_SN_INIT)
		{
			for (std::map<DHOST_TYPE_CHARACTER_SN, CCharacter*>::iterator iter = m_mapCharacter.begin(); iter != m_mapCharacter.end(); ++iter)
			{
				if (iter->second != nullptr)
				{
					m_pMannerManager->UpdateAwayFromKeyboard(iter->first, gameTime);
				}
			}
		}
	}
}

void CCharacterManager::ComebackAwayFromKeyboard(DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_USER_ID UserID, DHOST_TYPE_CHARACTER_SN characterSN)
{
	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->ComebackAwayFromKeyboard(gameTime, UserID, characterSN);
	}
}

void CCharacterManager::UpdateIntentionalBallHogPlay(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime, DHOST_TYPE_BALL_NUMBER ballNo)
{
	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->UpdateIntentionalBallHogPlay(characterSN, gameTime, ballNo);
	}
}

void CCharacterManager::UpdateAwayFromKeyboard(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT gameTime)
{
	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->UpdateAwayFromKeyboard(characterSN, gameTime);
	}
}

void CCharacterManager::BadMannerDetectImpossibleShooting(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT shotClock, DHOST_TYPE_FLOAT value)
{
	if (m_pMannerManager != nullptr)
	{
		m_pMannerManager->BadMannerDetectImpossibleShooting(characterSN, shotClock, value);
	}
}

void CCharacterManager::RegistCallbackFuncOnFireMode(OnFireModeCallback pFunc)
{
	m_CallbackOnFireMode = std::move(pFunc);
}

void CCharacterManager::IncreaseEmojiCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseEmojiCount(value);
	}
}

void CCharacterManager::IncreaseCeremonyCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseCeremonyCount(value);
	}
}

void CCharacterManager::IncreaseSkillSucCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_UINT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseSkillSucCount(value);
	}
}

void CCharacterManager::IncreaseUseSkillCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseUseSkillCount();
	}
}

void CCharacterManager::IncreaseUseSignatureCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseUseSignatureCount();
	}
}

void CCharacterManager::IncreaseUseQuickChatCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseUseQuickChatCount();
	}
}

void CCharacterManager::IncreaseUseSwitchCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseUseSwitchCount();
	}
}

void CCharacterManager::IncreaseActionGreatDefenseCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseActionGreatDefenseCount();
	}
}

void CCharacterManager::IncreaseActionLayUpGoalCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseActionLayUpGoalCount();
	}
}

void CCharacterManager::IncreaseActionDunkGoalCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseActionDunkGoalCount();
	}
}

void CCharacterManager::IncreaseActionAnkleBreakeCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseActionAnkleBreakeCount();
	}
}

void CCharacterManager::IncreaseIllegalScreenCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseIllegalScreenCount();
	}
}

void CCharacterManager::IncreaseNotSamePositionCount(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseNotSamePositionCount();
	}
}

void CCharacterManager::SetSpeedHackClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetSpeedHackClientjudgmentCount(value);
	}
}

void CCharacterManager::IncreaseHackImpossibleAction(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreaseHackImpossibleAction();
	}
}

void CCharacterManager::SetMemoryTamperClientjudgmentCount(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_INT32 value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetMemoryTamperClientjudgmentCount(value);
	}
}

void CCharacterManager::IncreasePacketTamper(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->IncreasePacketTamper();
	}
}

void CCharacterManager::AddTeamIllegalScreen(DHOST_TYPE_INT32 teamIndex)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_TeamIllegalScreen.find(teamIndex);

	if (iter == m_TeamIllegalScreen.end())
	{
		m_TeamIllegalScreen.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>(teamIndex, false));
	}
}

void CCharacterManager::InitTeamIllegalScreen()
{
	for (std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_TeamIllegalScreen.begin(); iter != m_TeamIllegalScreen.end(); ++iter)
	{
		iter->second = false;
	}
}

DHOST_TYPE_BOOL	CCharacterManager::GetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN)
{
	F4PACKET::SPlayerInformationT* pInfo = GetCharacterInformation(characterSN);

	if (pInfo != nullptr)
	{
		std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_TeamIllegalScreen.find(pInfo->team);

		if (iter != m_TeamIllegalScreen.end())
		{
			return iter->second;
		}
	}

	return false;
}

DHOST_TYPE_BOOL	CCharacterManager::CheckTeamIllegalScreen()
{
	for (std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_TeamIllegalScreen.begin(); iter != m_TeamIllegalScreen.end(); ++iter)
	{
		if (iter->second == true)
		{
			return true;
		}
	}

	return false;
}

DHOST_TYPE_BOOL CCharacterManager::SetTeamIllegalScreen(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value, DHOST_TYPE_INT32 team)
{
	DHOST_TYPE_BOOL result = false;

	F4PACKET::SPlayerInformationT* pInfo = GetCharacterInformation(characterSN);

	if (pInfo != nullptr)
	{
		std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_TeamIllegalScreen.find(pInfo->team);

		if (iter != m_TeamIllegalScreen.end() && iter->second == false && pInfo->team == team)
		{
			iter->second = value;
			result = true;

			CCharacter* pCharacter = GetCharacter(characterSN);
			if (pCharacter != nullptr)
			{
				pCharacter->IncreaseIllegalScreenCount();
			}
		}
	}

	return result;
}

void CCharacterManager::UseJoystick(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->UseJoystick();
	}
}

void CCharacterManager::AddTeamSurrenderCheck(DHOST_TYPE_INT32 teamIndex)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderCheck.find(teamIndex);

	if (iter == m_mapTeamSurrenderCheck.end())
	{
		m_mapTeamSurrenderCheck.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>(teamIndex, kTEAM_SURRENDER_CHECK_INIT));
	}
}

void CCharacterManager::AddTeamSurrenderGameEndTime(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_FLOAT value)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderGameEndTime.find(teamIndex);

	if (iter == m_mapTeamSurrenderGameEndTime.end())
	{
		m_mapTeamSurrenderGameEndTime.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>(teamIndex, value));
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetTeamSurrenderCheckTime(DHOST_TYPE_INT32 teamIndex)
{
	DHOST_TYPE_FLOAT result = kTEAM_SURRENDER_CHECK_INIT;

	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderCheck.find(teamIndex);

	if (iter != m_mapTeamSurrenderCheck.end())
	{
		result = iter->second;
	}

	return result;
}

DHOST_TYPE_FLOAT CCharacterManager::GetTeamSurrenderGameEndTime(DHOST_TYPE_INT32 teamIndex)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderGameEndTime.find(teamIndex);

	if (iter != m_mapTeamSurrenderGameEndTime.end())
	{
		result = iter->second;
	}

	return result;
}

void CCharacterManager::SetTeamSurrenderCheckTime(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_FLOAT value)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderCheck.find(teamIndex);

	if (iter != m_mapTeamSurrenderCheck.end())
	{
		iter->second = value;
	}
}

std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT> CCharacterManager::GetTeamSurrenderCheck()
{
	return m_mapTeamSurrenderCheck;
}

DHOST_TYPE_BOOL CCharacterManager::GetTeamSurrenderActivation(DHOST_TYPE_CHARACTER_SN characterSN)
{
	DHOST_TYPE_BOOL result = false;

	F4PACKET::SPlayerInformationT* pInfo = GetCharacterInformation(characterSN);

	if (pInfo != nullptr)
	{
		std::map<DHOST_TYPE_INT32, DHOST_TYPE_FLOAT>::iterator iter = m_mapTeamSurrenderCheck.find(pInfo->team);

		if (iter != m_mapTeamSurrenderCheck.end())
		{
			if (iter->second > kTEAM_SURRENDER_CHECK_EXPIRE)
			{
				result = true;
			}
		}
	}

	return result;
}

DHOST_TYPE_BOOL CCharacterManager::GetTeamSurrenderCheckVoteFail(DHOST_TYPE_INT32 teamIndex)
{
	DHOST_TYPE_BOOL result = false;

	std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_mapTeamSurrenderVoteFail.find(teamIndex);

	if (iter != m_mapTeamSurrenderVoteFail.end())
	{
		result = iter->second;
	}

	return result;
}

void CCharacterManager::SetTeamSurrenderCheckVoteFail(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL value)
{
	std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_mapTeamSurrenderVoteFail.find(teamIndex);

	if (iter != m_mapTeamSurrenderVoteFail.end())
	{
		iter->second = value;
	}
}

void CCharacterManager::AddTeamSurrenderVote(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BOOL value)
{
	DHOST_TYPE_BOOL insert = true;

	F4PACKET::SPlayerInformationT* pInfo = GetCharacterInformation(characterSN);

	if (pInfo != nullptr)
	{
		for (auto iter = m_mapTeamSurrenderVoteInfo.begin(); iter != m_mapTeamSurrenderVoteInfo.end(); ++iter)
		{
			SSurrenderVoteInfo sInfo = iter->second;

			if (sInfo.CharacterSN == characterSN)
			{
				insert = false;
				break;
			}
		}

		if (insert)
		{
			SSurrenderVoteInfo sInfo;
			sInfo.CharacterSN = characterSN;
			sInfo.vote = value;

			m_mapTeamSurrenderVoteInfo.insert(std::pair<DHOST_TYPE_INT32, SSurrenderVoteInfo>(pInfo->team, sInfo));
		}	
	}
}

DHOST_TYPE_INT32 CCharacterManager::GetTeamSurrenderVoteCount(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_BOOL value)
{
	DHOST_TYPE_INT32 count = kINT32_INIT;

	auto rangeIter = m_mapTeamSurrenderVoteInfo.equal_range(teamIndex);

	for (auto iter = rangeIter.first; iter != rangeIter.second; ++iter)
	{
		SSurrenderVoteInfo sInfo = iter->second;

		if (sInfo.vote == value)
		{
			++count;
		}
	}

	return count;
}

/*
0은 기권이 발생하지 않고 게임 정상종료
1은 기권이 발생해서 패배했고  기권을 선택한 유저
2는 기권이 빌생해서 패배했고  기권을 선택하지 않은 유저
*/
void CCharacterManager::SaveSurrenderVoteInfo(DHOST_TYPE_INT32 teamIndex)
{
	if (m_mapCharacter.empty() == false)
	{
		for (auto& it : m_mapCharacter)
		{
			auto pCharacter = it.second;
			SCharacterRecord* record = pCharacter->GetCharacterRecord();
			
			// 1. 일단 항복한 팀은 모두 2 
			SPlayerInformationT* info = GetCharacterInformation(it.first);
			if (info)
			{
				if (info->team == teamIndex)
				{
					
					record->SurrenderType = 2;
				}
			}
			
			// 2. 기권을한 당사자인 경우에는 1로 바꿈 
			for (auto iter = m_mapTeamSurrenderVoteInfo.begin(); iter != m_mapTeamSurrenderVoteInfo.end(); ++iter)
			{
				SSurrenderVoteInfo sInfo = iter->second;
				if (it.first == sInfo.CharacterSN)
				{
					if (sInfo.vote)
						record->SurrenderType = 1; // 기권을한 당사자인 경우에는 1로 바꿈 

				}
			}
		}
	}
}


void CCharacterManager::AddTeamSurrenderVoteFail(DHOST_TYPE_CHARACTER_SN characterSN)
{
	F4PACKET::SPlayerInformationT* pInfo = GetCharacterInformation(characterSN);

	if (pInfo != nullptr)
	{
		std::map<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>::iterator iter = m_mapTeamSurrenderVoteFail.find(pInfo->team);

		if (iter == m_mapTeamSurrenderVoteFail.end())
		{
			m_mapTeamSurrenderVoteFail.insert(std::pair<DHOST_TYPE_INT32, DHOST_TYPE_BOOL>(pInfo->team, false));
		}
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetCharacterDisconnectTime(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterDisconnectTime();
	}

	return kFLOAT_INIT;
}

void CCharacterManager::SetCharacterDisconnectTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetCharacterDisconnectTime(value);
	}
}

DHOST_TYPE_FLOAT CCharacterManager::GetCharacterDiconnectComebackTime(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterDiconnectComebackTime();
	}

	return kFLOAT_INIT;
}

void CCharacterManager::SetCharacterDiconnectComebackTime(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_FLOAT value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetCharacterDiconnectComebackTime(value);
	}
}

DHOST_TYPE_UINT32 CCharacterManager::GetPlayerNumber(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetPlayerNumber();
	}

	return kUINT32_INIT;
}

DHOST_TYPE_BALL_NUMBER CCharacterManager::GetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterForceBallEventFail();
	}

	return kBALL_NUMBER_INIT;
}

void CCharacterManager::SetCharacterForceBallEventFail(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetCharacterForceBallEventFail(value);
	}
}

DHOST_TYPE_BALL_NUMBER CCharacterManager::GetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterBallEventSuccess();
	}

	return kBALL_NUMBER_INIT;
}

void CCharacterManager::SetCharacterBallEventSuccess(DHOST_TYPE_CHARACTER_SN characterSN, DHOST_TYPE_BALL_NUMBER value)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		pCharacter->SetCharacterBallEventSuccess(value);
	}
}

DHOST_TYPE_BOOL CCharacterManager::InitialPassiveDataSet(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassiveDataSet_data* pInfo)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->InitialPassiveDataSet(pInfo);
	}

	return false;
}

DHOST_TYPE_BOOL CCharacterManager::SetPacketCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN, const F4PACKET::play_c2s_playerPassive_data* pInfo)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->SetPacketCharacterPassive(pInfo);
	}

	return false;
}

SCharacterPassive* CCharacterManager::GetCharacterPassive(DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	if (pCharacter != nullptr)
	{
		return pCharacter->GetCharacterPassive();
	}

	return nullptr;
}


DHOST_TYPE_BOOL CCharacterManager::GetPotentialInfo(DHOST_TYPE_CHARACTER_SN id, ACTION_TYPE actionType, SHOT_TYPE shotType, SKILL_INDEX skillIndex, SPotentialInfo& sInfo)
{
	DHOST_TYPE_BOOL result = false;

	POTENTIAL_INDEX potential_index = POTENTIAL_INDEX::none;

	F4PACKET::SPlayerInformationT* pCharacterInfo = GetCharacterInformation(id);

	if (pCharacterInfo != nullptr)
	{
		switch (actionType)
		{
			case ACTION_TYPE::action_shot:
			case ACTION_TYPE::action_shotDunk:
			case ACTION_TYPE::action_shotJump:
			case ACTION_TYPE::action_shotLayUp:
			case ACTION_TYPE::action_shotPost:
			{
				switch (shotType)
				{
					case SHOT_TYPE::shotType_closePostUnstready:
					case SHOT_TYPE::shotType_postUnstready:
					case SHOT_TYPE::shotType_hook:
					case SHOT_TYPE::shotType_post:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_hookShot:
							case SKILL_INDEX::skill_hookShotSky:
							case SKILL_INDEX::skill_hookShotPost:
							case SKILL_INDEX::skill_hookShotPostSky:
							case SKILL_INDEX::skill_turningShot:
							case SKILL_INDEX::jumpHook:
							{
								potential_index = POTENTIAL_INDEX::smooth_shot_touch_post;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					case SHOT_TYPE::shotType_middle:
					case SHOT_TYPE::shotType_middleUnstready:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_spotUpShot:
							case SKILL_INDEX::skill_fadeAway:
							case SKILL_INDEX::skill_pullUpJumper:
							case SKILL_INDEX::skill_stepBackJumper:
							case SKILL_INDEX::skill_turnAroundFadeAway:
							case SKILL_INDEX::skill_oneLegedFadeAway:
							case SKILL_INDEX::skill_aimShot:
							case SKILL_INDEX::skill_runner:
							case SKILL_INDEX::skill_rainbowShot:
							case SKILL_INDEX::skill_kneeKick:
							case SKILL_INDEX::catchAndShot:
							case SKILL_INDEX::oneDribblePullUpJumper:
							case SKILL_INDEX::sideStepJumper:
							{
								potential_index = POTENTIAL_INDEX::smooth_shot_touch_midle;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					case SHOT_TYPE::shotType_threePoint:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_spotUpShot3:
							case SKILL_INDEX::skill_fadeAway3:
							case SKILL_INDEX::skill_pullUpJumper:
							case SKILL_INDEX::skill_stepBackJumper:
							case SKILL_INDEX::skill_turnAroundFadeAway:
							case SKILL_INDEX::skill_oneLegedFadeAway:
							case SKILL_INDEX::skill_aimShot:
							case SKILL_INDEX::skill_rainbowShot:
							case SKILL_INDEX::skill_stepBackThreePointShot:
							case SKILL_INDEX::skill_kneeKick:
							case SKILL_INDEX::catchAndShot:
							case SKILL_INDEX::oneDribblePullUpJumper:
							case SKILL_INDEX::sideStepJumper:
							{
								potential_index = POTENTIAL_INDEX::smooth_shot_touch_three;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					case SHOT_TYPE::shotType_layUpDriveIn:
					case SHOT_TYPE::shotType_layUpFingerRoll:
					case SHOT_TYPE::shotType_layUpPost:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_doubleCluch:
							case SKILL_INDEX::skill_reverseLayUp:
							case SKILL_INDEX::skill_selfAlleyOop:
							case SKILL_INDEX::skill_jellyLayUp:
							case SKILL_INDEX::skill_closeReverseLayUp:
							case SKILL_INDEX::skill_putback:
							case SKILL_INDEX::skill_scoopShot:
							case SKILL_INDEX::skill_floater:
							case SKILL_INDEX::skill_euroStepLayUp:
							case SKILL_INDEX::quickLayUp:
							case SKILL_INDEX::skill_spinMove: // Added by findfun , 스핀무브 덩크 블럭이 안된다고 해서 2024/07/22
							case SKILL_INDEX::skill_hopStep: // Added by findfun , 홉스텝 덩크 블럭이 안된다고 해서 2024/07/22
							case SKILL_INDEX::passFake:            // 
							case SKILL_INDEX::skill_shakeAndBake:  // 2024/08/09
							{
								potential_index = POTENTIAL_INDEX::sensuous_layup;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					case SHOT_TYPE::shotType_dunkDriveIn:
					case SHOT_TYPE::shotType_dunkFingerRoll:
					case SHOT_TYPE::shotType_dunkPost:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_reverseDunk:
							case SKILL_INDEX::skill_selfAlleyOop:
							case SKILL_INDEX::skill_closeReverseDunk:
							case SKILL_INDEX::skill_putback:
							case SKILL_INDEX::skill_popUpDunk:
							case SKILL_INDEX::euroStepDunk:
							case SKILL_INDEX::inYourFace:
							case SKILL_INDEX::tomahawkDunk:
							case SKILL_INDEX::skill_spinMove: // Added by findfun , 스핀무브 덩크 블럭이 안된다고 해서 2024/07/22
							case SKILL_INDEX::skill_hopStep: // Added by findfun , 홉스텝 덩크 블럭이 안된다고 해서 2024/07/22
							case SKILL_INDEX::passFake: // Added by findfun ,  2024/08/09
							case SKILL_INDEX::skill_shakeAndBake:  // 2024/08/09
							{
								potential_index = POTENTIAL_INDEX::destructive_dunk;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					default:
					{

					}
					break;
				}
			}
			break;
			//! ������ ����
			//! ����� �� �ڵ鸵
			//! â������ �н�
			case ACTION_TYPE::action_pass:
			{
				switch (skillIndex)
				{
					case SKILL_INDEX::skill_none:
					case SKILL_INDEX::skill_layUpPass:
					case SKILL_INDEX::skill_jumpShotPass:
					case SKILL_INDEX::skill_postUpPass:
					case SKILL_INDEX::skill_penetratePass:
					case SKILL_INDEX::skill_bulletPass:
					case SKILL_INDEX::skill_handOff:
					case SKILL_INDEX::skill_skipPass:
					case SKILL_INDEX::skill_hookShotPass:
					case SKILL_INDEX::skill_jumpingPass:
					case SKILL_INDEX::skill_lobPass:
					case SKILL_INDEX::passFake:
					case SKILL_INDEX::skill_shakeAndBake:  // 2025/03/18
					{
						potential_index = POTENTIAL_INDEX::creative_pass;
					}
					break;
					default:
					{

					}
					break;
				}
			}
			break;
			//! ������ ��� ����
			case ACTION_TYPE::action_block:
			{
				switch (shotType)
				{
					case SHOT_TYPE::shotType_closePostUnstready:
					case SHOT_TYPE::shotType_postUnstready:
					case SHOT_TYPE::shotType_hook:
					case SHOT_TYPE::shotType_post:
					case SHOT_TYPE::shotType_layUpDriveIn:
					case SHOT_TYPE::shotType_layUpFingerRoll:
					case SHOT_TYPE::shotType_layUpPost:
					case SHOT_TYPE::shotType_dunkDriveIn:
					case SHOT_TYPE::shotType_dunkFingerRoll:
					case SHOT_TYPE::shotType_dunkPost:
					{
						switch (skillIndex)
						{
							case SKILL_INDEX::skill_none:
							case SKILL_INDEX::skill_powerBlock:
							case SKILL_INDEX::skill_chasedownBlock:
							case SKILL_INDEX::skill_chasedownCatch:
							case SKILL_INDEX::skill_stretchBlock:
							case SKILL_INDEX::skill_snatchBlock:
							{
								potential_index = POTENTIAL_INDEX::persistent_post_def;
							}
							break;
							default:
							{

							}
							break;
						}
					}
					break;
					default:
					{

					}
					break;
				}
			}
			break;
			//! �㸦 ��� ��ƿ
			//! �� ���� ���� �ܰ� ����
			//! źź�� ���� ����
			default:
			{

			}
			break;
		}
		
		if (potential_index != POTENTIAL_INDEX::none)
		{
			result = GetCharacterPotentialInfo(id, potential_index, sInfo);
		}
	}

	return result;
}

DHOST_TYPE_BOOL CCharacterManager::GetCharacterPotentialInfo(DHOST_TYPE_CHARACTER_SN id, F4PACKET::POTENTIAL_INDEX value, SPotentialInfo& sInfo)
{
	DHOST_TYPE_BOOL result = false;

	CCharacter* pCharacter = GetCharacter(id);

	if (pCharacter != nullptr)
	{
		result = pCharacter->GetCharacterPotentialInfo(value, sInfo);
	}

	return result;
}

DHOST_TYPE_FLOAT CCharacterManager::GetCharacterPotentialValueSum(DHOST_TYPE_CHARACTER_SN id)
{
	DHOST_TYPE_FLOAT result = kFLOAT_INIT;

	CCharacter* pCharacter = GetCharacter(id);

	if (pCharacter != nullptr)
	{
		result = pCharacter->GetCharacterPotentialValueSum();
	}

	return result;
}

void CCharacterManager::GetMyTeamsCharacterVector(std::vector<CCharacter*>& vecInfo, DHOST_TYPE_INT32 teamIndex)
{
	//vecInfo = m_vecTeamsCharacter[teamIndex];
	if (teamIndex >= 0 && teamIndex < m_vecTeamsCharacter.size())
	{
		vecInfo = m_vecTeamsCharacter[teamIndex];
	}
	else
	{
		vecInfo.clear();
	}
}


void CCharacterManager::GetTeamsCharacterVector(std::vector<std::vector<CCharacter*>>& vecInfo)
{
	vecInfo = m_vecTeamsCharacter;
}

void CCharacterManager::GetOppenentCharacterVector(std::vector<CCharacter*>& vecInfo, DHOST_TYPE_CHARACTER_SN characterSN)
{
	CCharacter* pCharacter = GetCharacter(characterSN);

	F4PACKET::SPlayerInformationT* pInformation = pCharacter->GetCharacterInformation();

	DHOST_TYPE_INT32 oppenetTeamIndex = 0;
	
	if (pInformation->team == 0)
	{
		oppenetTeamIndex = 1;
	}
	else
	if (pInformation->team == 1)
	{
		oppenetTeamIndex = 0;
	}
	else
	{
		oppenetTeamIndex = 1;
	}

	if (oppenetTeamIndex >= 0 && oppenetTeamIndex < m_vecTeamsCharacter.size())
	{
		vecInfo = m_vecTeamsCharacter[oppenetTeamIndex];
	}
	else
	{
		vecInfo.clear();
	}
}

DHOST_TYPE_BOOL CCharacterManager::CheckBoxOutedBy(CCharacter* actionCharacter, DHOST_TYPE_CHARACTER_SN& boxoutCharacter)
{

	std::vector<CCharacter*> vecOppenetTeamsCharacter;
	vecOppenetTeamsCharacter.clear();
	GetOppenentCharacterVector(vecOppenetTeamsCharacter, actionCharacter->GetCharacterInformation()->id);

	for (int i = 0; i < vecOppenetTeamsCharacter.size(); i++)
	{
		std::list<DHOST_TYPE_CHARACTER_SN> list = vecOppenetTeamsCharacter[i]->GetBoxOutedList();

		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (*it == actionCharacter->GetCharacterInformation()->id)
			{
				//string _log = "*** Try BoxOuted, Rebound Fail **** Character  : " + std::to_string(actionCharacter->GetCharacterInformation()->id);
				//actionCharacter->GetHost()->ToLog(_log.c_str());
				boxoutCharacter = vecOppenetTeamsCharacter[i]->GetCharacterInformation()->id;

				return true;
			}
		}
	}

	boxoutCharacter = 0;

	return false;
}

/*
if (std::find(vecOppenetTeamsCharacter[i]->GetBoxOutedList().begin(), vecOppenetTeamsCharacter[i]->GetBoxOutedList().end(), actionCharacter->GetCharacterInformation()->id)

	!= vecOppenetTeamsCharacter[i]->GetBoxOutedList().end())
{
	string _log = "*** Try BoxOuted, Rebond Fail **** Character  : " + std::to_string(vecOppenetTeamsCharacter[i]->GetCharacterInformation()->id);
	actionCharacter->GetHost()->ToLog(_log.c_str());
	return true;
}
*/

void CCharacterManager::CheckBoxOutedClear()
{	
	map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();

	for (auto it = characters.begin(); it != characters.end(); ++it)
	{
		CCharacter* character = it->second;
		if (character)
			character->GetBoxOutedList().clear();
	}
}

void CCharacterManager::CheckBoxOut(CCharacter* actionCharacter, F4PACKET::MOVE_MODE moveMode)
{
	TB::SVector3 positionRim;
	positionRim.mutate_x(RIM_POS_X);
	positionRim.mutate_y(kFLOAT_INIT);
	positionRim.mutate_z(RIM_POS_Z());

	TB::SVector3 axis;
	axis.mutate_x(kFLOAT_INIT);
	axis.mutate_y(1.0f);
	axis.mutate_z(kFLOAT_INIT);

	std::vector<CCharacter*> vecOppenetTeamsCharacter;
	vecOppenetTeamsCharacter.clear();
	GetOppenentCharacterVector(vecOppenetTeamsCharacter, actionCharacter->GetCharacterInformation()->id);

	for (int i = 0; i < vecOppenetTeamsCharacter.size(); i++)
	{
		actionCharacter->GetPosition();

		TB::SVector3 myPosition = CommonFunction::ConvertJoVectorToTBVector(actionCharacter->GetPosition());
		TB::SVector3 enemyPosition = CommonFunction::ConvertJoVectorToTBVector(vecOppenetTeamsCharacter[i]->GetPosition());

		TB::SVector3 frontDirection = CommonFunction::SVectorSub(myPosition, positionRim);
		TB::SVector3 defenseDirection = CommonFunction::SVectorSub(enemyPosition, myPosition);

		DHOST_TYPE_FLOAT angle = CommonFunction::SVectorSignedAngle(defenseDirection, frontDirection, axis);
		DHOST_TYPE_FLOAT boxOutAngle = 0.0f;
		DHOST_TYPE_FLOAT boxOutRadius = 0.0f;

		if (moveMode == F4PACKET::MOVE_MODE::enhancedBoxOut)
		{
			boxOutAngle = actionCharacter->GetHost()->GetVerifyManager()->GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::ENHANCED_BOXOUT_ANGLE, actionCharacter);
			boxOutRadius = actionCharacter->GetHost()->GetVerifyManager()->GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::ENHANCED_BOXOUT_RADIUS, actionCharacter);
		}
		else
		{
			boxOutAngle = actionCharacter->GetHost()->GetVerifyManager()->GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BOXOUT_ANGLE, actionCharacter);
			boxOutRadius = actionCharacter->GetHost()->GetVerifyManager()->GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE::BOXOUT_RADIUS, actionCharacter);
		}

		float distance = (actionCharacter->GetPosition() - vecOppenetTeamsCharacter[i]->GetPosition()).Length();

		boxOutRadius += m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_Radius_Medal_PostCleaner") * m_pHost->CheckDoYouHaveMedal(vecOppenetTeamsCharacter[i]->GetCharacterInformation(), F4PACKET::MEDAL_INDEX::medal_postCleaner);

		boxOutRadius += m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_Radius_Medal_PostCleaner") * m_pHost->CheckDoYouHaveMedal(vecOppenetTeamsCharacter[i]->GetCharacterInformation(), F4PACKET::MEDAL_INDEX::postCleaner);

		boxOutRadius += m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_Radius_Medal_PostCleaner2") * m_pHost->CheckDoYouHaveMedal(vecOppenetTeamsCharacter[i]->GetCharacterInformation(), F4PACKET::MEDAL_INDEX::postCleaner2);

		boxOutRadius += m_pHost->GetBalanceTable()->GetValue("VBoxOutCondition_Disturb_Radius_Medal_PaintZoneRuler") * m_pHost->CheckDoYouHaveMedal(vecOppenetTeamsCharacter[i]->GetCharacterInformation(), F4PACKET::MEDAL_INDEX::paintZoneRuler);

		//string _log = "*** Try BoxOut **** distance : " + std::to_string(distance) + " boxOutRadius: "
			//+ std::to_string(boxOutRadius) + " angle: " + std::to_string(angle) + " boxOutAngle: " + std::to_string(boxOutAngle);
		//actionCharacter->GetHost()->ToLog(_log.c_str());

		//EHOST_CHARACTER_STATE GetCharacterCurrentState() const { return m_CharacterCurrentState; }

		bool checkBoxOUt = false;

		if (boxOutAngle > abs(angle) && boxOutRadius > distance)
		{
			//string _log = "*** Inlcude BoxOut **** ";

			if (   vecOppenetTeamsCharacter[i]->GetCharacterCurrentState() == EHOST_CHARACTER_STATE::MOVE 
				|| vecOppenetTeamsCharacter[i]->GetCharacterCurrentState() == EHOST_CHARACTER_STATE::STAND 
				/*|| vecOppenetTeamsCharacter[i]->GetCharacterCurrentState() == EHOST_CHARACTER_STATE::REBOUND*/)
			{
				checkBoxOUt = true;
				//return true;
			}
		}

		if (checkBoxOUt)
		{
			actionCharacter->AddBoxOutedCharacter(vecOppenetTeamsCharacter[i]->GetCharacterInformation()->id, moveMode);
		}
		else
		{
			actionCharacter->RemoveBoxOutedCharacter(vecOppenetTeamsCharacter[i]->GetCharacterInformation()->id, moveMode);
		}


	}
}




