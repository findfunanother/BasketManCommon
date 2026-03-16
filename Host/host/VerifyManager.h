#pragma once
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
#else
#pragma clang diagnostic ignored "-Wswitch"
#endif

#include "Host.h"
#include "HostDefine.h"
#include "HostStruct.h"
#include "F4Packet_generated.h"
#include "FlatBufPacket.h"

using namespace std;
using namespace F4PACKET;
using namespace TB;

class CHost;
class CDataManagerBalance;


class CVerifyManager
{
public:
	CVerifyManager(CHost* host);
	~CVerifyManager();

	DHOST_TYPE_BOOL CheckShotTypeRimAttack(SHOT_TYPE value);
	DHOST_TYPE_FLOAT CorrectionEarlyInTheGame(DHOST_TYPE_CHARACTER_SN id, ECORRECTION_EARLY_TYPE value);
	F4PACKET::ECHARACTER_INDEX GetCharacterIndex(SPlayerInformationT* pInfo);
	DHOST_TYPE_BOOL	CheckHaveSkill(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value, DHOST_TYPE_BOOL bLogCheck = true);
	DHOST_TYPE_FLOAT GetSkillLevel(DHOST_TYPE_CHARACTER_SN id, SKILL_INDEX value);
	DHOST_TYPE_INT32 GetMedalValue(SPlayerInformationT* pInfo, MEDAL_INDEX value);
	DHOST_TYPE_FLOAT GetPlayerVariableTypeValue(EPLAYER_VARIABLE_TYPE type, CCharacter* pCharacter, DHOST_TYPE_FLOAT param = kFLOAT_INIT, DHOST_TYPE_FLOAT paramSecond = kFLOAT_INIT);
	DHOST_TYPE_FLOAT GetRandomValueAlgorithm(DHOST_TYPE_USER_ID userid, DHOST_TYPE_BOOL xorShift, DHOST_TYPE_INT32 randCount = kINT32_INIT);

	//! Block
	DHOST_TYPE_FLOAT GetBlockSuccessRate(const F4PACKET::play_c2s_playerBlock_data* pInfo);
	DHOST_TYPE_FLOAT GetBlockSuccessRateSnatch(const F4PACKET::play_c2s_playerBlock_data* pInfo);
	DHOST_TYPE_FLOAT GetBlockSuccessRateDoubleHandsUp(const F4PACKET::play_c2s_playerBlock_data* pInfo);
	DHOST_TYPE_FLOAT GetBlockSuccessRateWithRimAttacktPotential(DHOST_TYPE_CHARACTER_SN owner, SHOT_TYPE ownerShotType, SKILL_INDEX skillIndex, DHOST_TYPE_BALL_NUMBER ballNo);

	DHOST_TYPE_FLOAT GetBlockTeamAttributeBonus(const F4PACKET::play_c2s_playerBlock_data* pInfo);

	DHOST_TYPE_FLOAT VerifyBlockPotential(F4PACKET::SPlayerInformationT* pCharacterInfo, SHOT_TYPE ownerShotType, SKILL_INDEX skillIndex, DHOST_TYPE_INT32 fxLevel);

	EACTION_VERIFY VerifyBlockShadowBlock(const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid);
	EACTION_VERIFY VerifyBlockSnatchBlock(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid);
	EACTION_VERIFY VerifyBlockDoubleHandUp(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid);
	EACTION_VERIFY VerifyBlockDefault(CCharacter* pCharacter, F4PACKET::SKILL_INDEX skillindex, const F4PACKET::play_c2s_playerBlock_data* pInfo, DHOST_TYPE_USER_ID userid);

	//! Pick
	DHOST_TYPE_INT32 VerifyPickMove(const F4PACKET::play_c2s_playerPick_data* pInfo);
	DHOST_TYPE_INT32 VerifyPickBall(const F4PACKET::play_c2s_playerPick_data* pInfo);

	//! Shot
	DHOST_TYPE_BOOL CheckShotLayUp(const F4PACKET::play_c2s_playerShot_data* pInfo);
	DHOST_TYPE_BOOL CheckShotDunk(const F4PACKET::play_c2s_playerShot_data* pInfo);
	DHOST_TYPE_BOOL CheckShotJumpShot(const F4PACKET::play_c2s_playerShot_data* pInfo);

	// !Move 
	DHOST_TYPE_BOOL CheckMove(const F4PACKET::play_c2s_playerMove_data* pInfo);


	//! PotentialBloom
	EACTION_VERIFY VerifyPotentialFxLevel(DHOST_TYPE_USER_ID userid, DHOST_TYPE_CHARACTER_SN id, DHOST_TYPE_INT32 fxlevel, POTENTIAL_INDEX value);

	// ���θ��� ���� ��ġ�ý����� ĳ���� ��ġ�� üũ�ϱ� ���� ���� ���� ��
	DHOST_TYPE_BOOL VerifyCommonPosition(CCharacter* pCharacter, TB::SVector3 position, string taglog);


	void VerifyLog(LOG_TYPE logType);

private:
	CHost* m_pHost;

public :
	DHOST_TYPE_FLOAT addBlockPotentialBuffDebug;
	DHOST_TYPE_FLOAT subBlockPotentialBuffDebug;

};

