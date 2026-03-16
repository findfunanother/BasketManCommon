#pragma once
//! 20200821 Host 구조체 정의 - by thinkingpig

//#include <enet/enet.h>

#include "HostDefine.h"
#include <vector>


#define kCONNECT_SIZE 150
struct Svector3
{
    float x;
    float y;
    float z;
};

struct SRedisMatchCharacterTagInfo
{
    uint32_t TagSlot;
    uint32_t TagIndex;
    uint32_t TagGrade;
    uint32_t TagLevel;
    uint32_t TagValue;
};

struct SRedisMatchCharacterSkillInfo
{
    uint32_t SkillSlot;
    uint32_t SkillIndex;
    uint32_t SkillLevel;
};

struct SRedisMatchCharacterSignatureInfo
{
    uint32_t SignatureIndex;
};

struct SRedisMatchUserInfo
{
    uint32_t UserID;
    uint32_t AiLevel;
    std::string Team;
    std::string UserName;
    uint32_t CharacterIDX;
    uint32_t CharacterID;   // ResourceID
    ECHARACTER_ROLE CharacterRole;
    uint32_t AbilitySpeed;
    uint32_t AbilityVertical;
    uint32_t AbilityStrength;
    uint32_t AbilityPass;
    uint32_t AbilityDribble;
    uint32_t AbilityRebound;
    uint32_t AbilityBlock;
    uint32_t AbilitySteal;
    uint32_t AbilityMidRangeShot;
    uint32_t AbilityThreePointShot;
    uint32_t AbilityPostShot;
    uint32_t AbilityDunk;
    uint32_t AbilityLayUp;
    uint32_t AbilityPostDefense;
    uint32_t AbilityPerimeterDefense;
    uint32_t AbilityWinspan;
    uint32_t AbilityHeight;
    std::string AbilityHandedness;
    uint32_t AbilityPostMove;
    uint32_t AbilityGender;
    std::vector<SRedisMatchCharacterSkillInfo*> vSkillInfo;
    std::vector<SRedisMatchCharacterTagInfo*> vTagInfo;
    std::vector<SRedisMatchCharacterSignatureInfo*> vSignatureInfo;
    uint32_t Ceremony1;
    uint32_t Ceremony2;
    uint32_t Ceremony3;
    uint32_t CeremonyWin;
    uint32_t CeremonyLose;
    uint32_t CeremonyDraw;
    uint32_t CeremonyMVP;
    uint32_t PassiveType;
    uint32_t TendencyShotMind;
    uint32_t TendencyPassMind;
    uint32_t TendencyPenetrationMind;
    uint32_t TendencyHelpMind;
    uint32_t TendencyStealMind;
    uint32_t TendencyReboundMind;
    uint32_t TendencyBlockMind;
    uint32_t TendencyAceMind;
};

struct SRedisMatchInfo
{
	uint32_t RoomID;
    uint32_t StageID;
	EMODE_TYPE ModeType;
	std::string StartDate;
    uint32_t AiHostUserID;
    std::vector<SRedisMatchUserInfo*> vUserInfo;
};

struct SCharacterRecordShootInfo
{
    float PositionX;
    float PositionZ;
    uint32_t Zone;  // 기획서에서 정의한 구역정보 1 ~ 14 구역까지 있다. 자세한 내용은 기획서 참조 https://docs.google.com/document/d/10aw0uCs0XabDD3kImcTAdHFi8L1DocQavGKqzhrDH_I/edit#
    bool Goal;  // false =  노골, true = 골인
};

struct SCharacterRecord
{
    DHOST_TYPE_USER_ID  UserID;
    DHOST_TYPE_CHARACTER_SN CharacterSN;   // 방에 있는 캐릭터 고유번호(인덱스)
    DHOST_TYPE_CHARACTER_ID CharacterID;    // 캐릭터 리소스 ID
    std::string UserName;
    std::string Team;
    DHOST_TYPE_UINT16 TotalScore;           // 누적 득점
    DHOST_TYPE_UINT16 Try2PointCount;       // 2득점 시도 횟수
    DHOST_TYPE_UINT16 Suc2Pointcount;       // 2득점 성공 횟수
    DHOST_TYPE_UINT16 Try3PointCount;       // 3득점 시도 횟수
    DHOST_TYPE_UINT16 Suc3Pointcount;       // 3득점 성공 횟수
    DHOST_TYPE_UINT16 AssistCount;          // 어시스트
    DHOST_TYPE_UINT16 ReboundCount;         // 리바운드
    DHOST_TYPE_UINT16 BlockCount;           // 블럭
    DHOST_TYPE_UINT16 StealCount;           // 스틸
    DHOST_TYPE_UINT16 LooseBallCount;       // 루즈볼
    DHOST_TYPE_UINT16 TurnOverCount;        // 턴오버 발생시킨 횟수
    DHOST_TYPE_UINT16 MVPScore;             // MVP 점수
    std::vector<SCharacterRecordShootInfo> vShootInfo;  // 슛 정보 (위치랑 골인 여부)
};

struct SRedisMatchResult
{
    uint32_t RoomID;
    EMODE_TYPE ModeType;
    int SkillChallengeScore;
    std::string EndTime;
    DHOST_TYPE_CHARACTER_SN MVP;
    std::vector<SCharacterRecord*> vCharacterRecord;
    std::vector<DHOST_TYPE_UINT32> vTeamScore;
};

struct SBallActionInfo
{
    DHOST_TYPE_BALL_NUMBER BallNumber;
    DHOST_TYPE_CHARACTER_SN BallOwner;
    DHOST_TYPE_STR BallOwnerTeam;
    EBALL_STATE Ballstate;
    DHOST_TYPE_BOOL bAssist;
    DHOST_TYPE_GAME_TIME_F fGameTime;
};

struct MatchInfo
{
    int id;
    int target;
};