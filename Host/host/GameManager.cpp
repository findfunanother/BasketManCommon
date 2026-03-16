#include "GameManager.h"
#include "Host.h"
#include "DataManagerBalance.h"

CGameManager::CGameManager(CHost* pHost, int option) : m_Option(option)
{
	m_pHost = pHost;
	m_StageID = kSTAGE_INIT;
	m_BackBoardState = F4PACKET::BB_STATE::firstState;
	m_fBackBoardHP = kBACK_BOARD_DURABILITY;
	m_ModeType = EMODE_TYPE::NONE;
	m_isNormalGame = false;
	m_ScoredTime = kFLOAT_INIT;
	m_bLoadComplete = false;
	m_fReplayPoint = kFLOAT_INIT;
	m_bReplayAssistCheck = false;
	m_OffenseTeam = -1;
	m_WinnerTeam = 0;
	m_AbNormalEndUserTeam = -1;
	m_bAbnormalExpireRoom = false;
	m_bRedisLoadValue = false;
	m_fRedisLoadRoomElapsedTime = kFLOAT_INIT;
	m_bRedisSaveMatchResult = false;
	m_vecScores.clear();
	m_SpeedHackRestrictionStepOne = kHACK_CHECK_RESTRICTION_STEP_ONE;
	m_SpeedHackRestrictionStepTwo = kHACK_CHECK_RESTRICTION_STEP_TWO;
	m_SwitchToAiLevel = kKICK_SWTICH_TO_AI_LEVEL;
	m_JumpBallStartTime = kFLOAT_INIT;
	m_ExitType = EEXIT_TYPE::NONE;
	m_bLogShotRate = false;
	m_SceneIndex = HOST_OPTION_STAGE_RANDOM;
	m_bBuzzerBeater = false;
	m_fShotRoomElapsedTime = kFLOAT_INIT;
	m_OverTimeCount = kINT32_INIT;
	m_nHandicapRulePoint = 0;
	m_bIsDebug = false;
	m_bNoTieMode = false;

	m_pMinigameCustom = nullptr;

	if (m_Option & HOST_OPTION_LOG_SHOT_CHECK)
	{
		m_bLogShotRate = true;
	}
}

CGameManager::~CGameManager()
{
	if (m_pMinigameCustom)
	{
		SAFE_DELETE(m_pMinigameCustom);
	}
	m_pMinigameCustom = nullptr;
}

void CGameManager::UpdateGameManager(DHOST_TYPE_FLOAT timeDelta)
{

}

DHOST_TYPE_BOOL CGameManager::CheckOption(DHOST_TYPE_UINT64 flag) 
{ 
	return (flag & m_Option); 
}

F4PACKET::EGAME_MODE CGameManager::GetConvertGameMode()
{
	F4PACKET::EGAME_MODE result_type = F4PACKET::EGAME_MODE::eGameMode_none;

	switch (m_ModeType)
	{
		case EMODE_TYPE::THREE_ON_THREE:
			result_type = F4PACKET::EGAME_MODE::eGameMode_three_on_three;
			break;
		case EMODE_TYPE::TWO_ON_TWO:
			result_type = F4PACKET::EGAME_MODE::eGameMode_two_on_two;
			break;
		case EMODE_TYPE::ONE_ON_ONE:
			result_type = F4PACKET::EGAME_MODE::eGameMode_one_on_one;
			break;
		case EMODE_TYPE::AI:
			result_type = F4PACKET::EGAME_MODE::eGameMode_ai;
			break;
		case EMODE_TYPE::TRAINING:
			result_type = F4PACKET::EGAME_MODE::eGameMode_training;
			break;
		case EMODE_TYPE::TRAINING_REBOUND:
			result_type = F4PACKET::EGAME_MODE::eGameMode_training_rebound;
			break;
		case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_training_jumpshotBlock;
			break;
		case EMODE_TYPE::TRAINING_RIMATTACKBLOCK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_training_rimAttackBlock;
			break;
		case EMODE_TYPE::TRAINING_OFFBALLMOVE:
			result_type = F4PACKET::EGAME_MODE::eGameMode_training_offBallMove;
			break;
		case EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE:
			result_type = F4PACKET::EGAME_MODE::eGameMode_challenge_off_ball_move;
			break;
		case EMODE_TYPE::SKILL_CHALLENGE_REBOUND:
			result_type = F4PACKET::EGAME_MODE::eGameMode_challenge_rebound;
			break;
		case EMODE_TYPE::SKILL_CHALLENGE_PASS:
			result_type = F4PACKET::EGAME_MODE::eGameMode_challenge_pass;
			break;
		case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_challenge_jump_shot_block;
			break;
		case EMODE_TYPE::CUSTOM:
			result_type = F4PACKET::EGAME_MODE::eGameMode_custom;
			break;
		case EMODE_TYPE::TRIO:
			result_type = F4PACKET::EGAME_MODE::eGameMode_trio;
			break;
		case EMODE_TYPE::TRIO_PVP_MIXED:
			result_type = F4PACKET::EGAME_MODE::eGameMode_trio_pvp_mixed;
			break;
		case EMODE_TYPE::CONTINUOUS:
			result_type = F4PACKET::EGAME_MODE::eGameMode_continuous;
			break;
		case EMODE_TYPE::TUTORIAL_BASIC:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_basic;
			break;
		case EMODE_TYPE::TUTORIAL_PASS:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_pass;
			break;
		case EMODE_TYPE::TUTORIAL_STEAL:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_steal;
			break;
		case EMODE_TYPE::TUTORIAL_DIVINGCATCH:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_divingCatch;
			break;
		case EMODE_TYPE::TUTORIAL_JUMPSHOT:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_jumpShot;
			break;
		case EMODE_TYPE::TUTORIAL_RIMATTACK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_rimAttack;
			break;
		case EMODE_TYPE::TUTORIAL_JUMPSHOTBLOCK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_jumpShotBlock;
			break;
		case EMODE_TYPE::TUTORIAL_REBOUND:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_rebound;
			break;
		case EMODE_TYPE::TUTORIAL_REBOUND_BEGINNER:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_reboundBeginner;
			break;
		case EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_blockBeginner;
			break;
		case EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_rimAttackBlock;
			break;
		case EMODE_TYPE::TUTORIAL_BOXOUT:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_boxout;
			break;
		case EMODE_TYPE::TUTORIAL_PENETRATE:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_penetrate;
			break;
		case EMODE_TYPE::TUTORIAL_SHOOTINGDISTURB:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_shootingDisturb;
			break;
		case EMODE_TYPE::TUTORIAL_MINIGAME_CUSTOM:
			result_type = F4PACKET::EGAME_MODE::eGameMode_tutorial_minigame_custom;
			break;
		default:
			result_type = F4PACKET::EGAME_MODE::eGameMode_none;
			break;
	}

	return result_type;
}

F4PACKET::EXIT_TYPE CGameManager::GetConvertExitType()
{
	F4PACKET::EXIT_TYPE type = F4PACKET::EXIT_TYPE::none;

	switch (m_ExitType)
	{
		case EEXIT_TYPE::LOGIN:
			type = F4PACKET::EXIT_TYPE::login;
			break;
		case EEXIT_TYPE::LOBBY:
			type = F4PACKET::EXIT_TYPE::lobby;
			break;
	}

	return type;
}

DHOST_TYPE_INT32 CGameManager::GetScore(DHOST_TYPE_INT32 teamIndex)
{
	if (teamIndex < m_vecScores.size())
	{
		return m_vecScores[teamIndex];
	}

	return 0;
}

void CGameManager::SetScore(DHOST_TYPE_INT32 teamIndex, DHOST_TYPE_INT32 value)
{
	if (teamIndex < kINT32_INIT)
	{
		return;
	}

	while (m_vecScores.size() <= teamIndex)
	{
		m_vecScores.push_back(0);
	}

	if (0 <= teamIndex && teamIndex < m_vecScores.size())
	{
		m_vecScores[teamIndex] = value;
	}
}

void CGameManager::CheckBackBoardState(DHOST_TYPE_FLOAT value)
{
	m_fBackBoardHP -= Util::GetRandom(value);

	if (m_fBackBoardHP > 50.f)
	{
		m_BackBoardState = F4PACKET::BB_STATE::firstState;
	}
	else if (m_fBackBoardHP > 20.f)
	{
		m_BackBoardState = F4PACKET::BB_STATE::secondState;
	}
	else if (m_fBackBoardHP > 10.f)
	{
		m_BackBoardState = F4PACKET::BB_STATE::thirdState;
	}
	else if (m_fBackBoardHP > 5.f)
	{
		m_BackBoardState = F4PACKET::BB_STATE::forthState;
	}
	else if (m_fBackBoardHP > 0.f)
	{
		m_BackBoardState = F4PACKET::BB_STATE::fifthState;
	}
	else
	{
		m_BackBoardState = F4PACKET::BB_STATE::brokenState;
	}
}

DHOST_TYPE_BOOL CGameManager::CheckSingleMode()
{
	switch (m_ModeType)
	{
		case EMODE_TYPE::TRAINING:
		case EMODE_TYPE::TRAINING_REBOUND:
		case EMODE_TYPE::TRAINING_JUMPSHOTBLOCK:
		case EMODE_TYPE::TRAINING_RIMATTACKBLOCK:
		case EMODE_TYPE::TRAINING_OFFBALLMOVE:
		case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
		case EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE:
		case EMODE_TYPE::SKILL_CHALLENGE_PASS:
		case EMODE_TYPE::SKILL_CHALLENGE_REBOUND:

		case EMODE_TYPE::TUTORIAL_BASIC:
		case EMODE_TYPE::TUTORIAL_BOXOUT:
		case EMODE_TYPE::TUTORIAL_DIVINGCATCH:
		case EMODE_TYPE::TUTORIAL_JUMPSHOT:
		case EMODE_TYPE::TUTORIAL_JUMPSHOTBLOCK:
		case EMODE_TYPE::TUTORIAL_PASS:
		case EMODE_TYPE::TUTORIAL_REBOUND:
		case EMODE_TYPE::TUTORIAL_RIMATTACK:
		case EMODE_TYPE::TUTORIAL_RIMATTACKBLOCK:
		case EMODE_TYPE::TUTORIAL_STEAL:
		case EMODE_TYPE::TUTORIAL_PENETRATE:
		case EMODE_TYPE::TUTORIAL_SHOOTINGDISTURB:
		case EMODE_TYPE::TUTORIAL_BLOCK_BEGINNER:
		case EMODE_TYPE::TUTORIAL_REBOUND_BEGINNER:
			return true;
		default:
			return false;
	}
}

DHOST_TYPE_BOOL CGameManager::CheckSkillChallengeMode()
{
	switch (m_ModeType)
	{
		case EMODE_TYPE::SKILL_CHALLENGE_JUMP_SHOT_BLOCK:
		case EMODE_TYPE::SKILL_CHALLENGE_OFF_BALL_MOVE:
		case EMODE_TYPE::SKILL_CHALLENGE_PASS:
		case EMODE_TYPE::SKILL_CHALLENGE_REBOUND:
			return true;
		default:
			return false;
	}
}

void CGameManager::SetWinnerTeam()
{
	// 싱글플레이인데 강종된 경우 패배처리 한다.
	if (GetAbnormalExpireRoom() && GetAbNormalEndUserTeam() != -1)
	{
		if (GetAbNormalEndUserTeam() == 0)
		{
			SetScore(0, 0);

			if (GetScore(1) == 0)
			{
				SetScore(1, 2);
			}
		}
		else if (GetAbNormalEndUserTeam() == 1)
		{
			SetScore(1, 0);

			if (GetScore(0) == 0)
			{
				SetScore(0, 2);
			}
		}
		else
		{
			// 에러 처리
		}
	}
	
	m_WinnerTeam = (GetScore(0) >= GetScore(1)) ? 0 : 1;

	if (m_ModeType == EMODE_TYPE::CONTINUOUS)
	{
		for (int teamIndex = 0; teamIndex < 3; ++teamIndex)
		{
			if (11 <= GetScore(teamIndex))
			{
				m_WinnerTeam = teamIndex;
				break;
			}
		}
	}

	if (GetAbnormalExpireRoom() && GetAbNormalEndUserTeam() == -1)
	{
		m_WinnerTeam = -1;
	}
}

void CGameManager::SetRedisLoadValue(DHOST_TYPE_BOOL value, DHOST_TYPE_FLOAT time)
{
	m_bRedisLoadValue = value;

	m_fRedisLoadRoomElapsedTime = time;
}

DHOST_TYPE_BOOL CGameManager::GetKickReasonSaveMode()
{
	DHOST_TYPE_BOOL result = false;

	switch (m_ModeType)
	{
		case EMODE_TYPE::CONTINUOUS:
		case EMODE_TYPE::CUSTOM:
		case EMODE_TYPE::THREE_ON_THREE:
		case EMODE_TYPE::TRIO:
		case EMODE_TYPE::TRIO_PVP_MIXED:
		{
			result = true;
		}
		break;
		default:
			break;
	}

	return result;
}

DHOST_TYPE_FLOAT CGameManager::GetRescue1ScoreExtend()
{
	return m_pHost->GetBalanceTable()->GetValue("VRescue1Score") + m_nHandicapRulePoint;
}

DHOST_TYPE_FLOAT CGameManager::GetRescue2ScoreExtend()
{
	return m_pHost->GetBalanceTable()->GetValue("VRescue2Score") + m_nHandicapRulePoint;
}

