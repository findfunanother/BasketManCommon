#pragma once
//! 20200821 Host ����ü ���� - by thinkingpig

//#include <enet/enet.h>

#include "HostDefine.h"
#include <vector>
#include <map>


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
    bool TagCore;
};

struct SRedisMatchCharacterSkillInfo
{
    uint32_t SkillSlot;
    uint32_t SkillIndex;
    uint32_t SkillLevel;
    bool     ForceAwaken;
};

struct SRedisMatchCharacterSignatureInfo
{
    uint32_t SignatureIndex;
};

struct SRedisMatchCharacterEmojiInfo
{
    uint32_t EmojiSlot;
    uint32_t EmojiIndex;
};

struct SRedisMatchCharacterPotentialInfo
{
    uint32_t PotentialIndex;
    uint32_t PotentialLevel;
    float PotentialValue;
    bool PotentialAwaken;
    uint32_t PotentialBloomRateLevel;
    float PotentialBloomRateValue;
    uint32_t PotentialBloomBuffLevel;
    float PotentialBloomBuffValue;
};


struct SRedisMatchTeamAttributesInfo
{
    float FinishingAtPost;
    float PerimeterShooting;
    float Passing;
    float BallHandling;

    float PostDefense;
    float PerimeterDefense;
    float Deflections;
    float ShotContesting;

    float Clutch;
    float StartingMomentum;
    float Tenacity;
    float Concentration;

};

struct SRedisMatchCharacterJukeBoxInfo
{
    uint32_t musicid;
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
    bool TeamControllUser;
    uint32_t AutoPlayUser;
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
    std::vector<SRedisMatchCharacterEmojiInfo*> vEmojiInfo;
    std::vector<SRedisMatchCharacterPotentialInfo*> vPotentialInfo;
    std::vector<SRedisMatchCharacterJukeBoxInfo*> vJukeBoxInfo;
    SRedisMatchTeamAttributesInfo TeamAttributesInfo;
    uint32_t specialCharacterLevel;
    uint32_t Ceremony1;
    uint32_t Ceremony2;
    uint32_t Ceremony3;
    uint32_t CeremonyWin;
    uint32_t CeremonyLose;
    uint32_t CeremonyDraw;
    uint32_t CeremonyMVP;
    uint32_t PassiveType;
    uint32_t elo;
    uint32_t playerNumber; 
    uint32_t intBurstValue; // 초기 ?��?가 받아?�는 버스??�?
    uint32_t effectPassiveID; // ?��? 결제 ?�시�??�펙??
    uint32_t cloakID; 
};

struct SRedisHandicapRule
{
    DHOST_TYPE_UINT32        HandicapTeam;  // weak team number 
    DHOST_TYPE_UINT32        HandicapScore; // ( if HandicapScore > 0 ) Apply HandicapRule 
};

struct SMinigameCustomInfo
{
    bool    isHomeAttackRetained;
    float   addShotBuffRate;
    std::vector<DHOST_TYPE_UINT32> vShotZonerTryID;
    std::vector<DHOST_TYPE_UINT32> vActionCountID;
    std::vector<DHOST_TYPE_UINT32> vRecordCountID;
};

struct SPacketGuardRule
{
    uint16_t magic;
    uint32_t watermarkMask;
    uint32_t watermarkResult;

    SPacketGuardRule()
        : magic(0xF4F4)
        , watermarkMask(0xA1C23BF2u)
        , watermarkResult(0x00820190u)
    {
    }
};

struct SRedisMatchInfo
{
	uint32_t RoomID;
    uint32_t StageID;
    DHOST_TYPE_FLOAT PlayTime;
	EMODE_TYPE ModeType;
    SMinigameCustomInfo MiniGameCustomInfo;
    EEXIT_TYPE ExitType;
	std::string StartDate;
    uint32_t AiHostUserID;
    DHOST_TYPE_BOOL isNormalGame;
    DHOST_TYPE_FLOAT SpeedHackResStepOne;
    DHOST_TYPE_FLOAT SpeedHackResStepTwo;
    uint32_t SwitchToAiLevel;
    DHOST_TYPE_INT32 SceneIndex;
    SRedisHandicapRule HandiCapRule;
    std::vector<SRedisMatchUserInfo*> vUserInfo;
    DHOST_TYPE_BOOL isDebug;
    DHOST_TYPE_BOOL NoTieMode;
    SPacketGuardRule packetGuardRule;
};

struct SCharacterRecordShootInfo
{
    float PositionX;
    float PositionZ;
    uint32_t Zone;  // ��ȹ������ ������ �������� 1 ~ 14 �������� �ִ�. �ڼ��� ������ ��ȹ�� ���� https://docs.google.com/document/d/10aw0uCs0XabDD3kImcTAdHFi8L1DocQavGKqzhrDH_I/edit#
    bool Goal;  // false =  ���? true = ����
};

struct SCharacterRecord
{
    DHOST_TYPE_USER_ID  UserID;
    DHOST_TYPE_CHARACTER_SN CharacterSN;   // �濡 �ִ� ĳ���� ������ȣ(�ε���)
    DHOST_TYPE_CHARACTER_ID CharacterID;    // ĳ���� ���ҽ� ID
    DHOST_TYPE_BOOL SwitchToAi;             // ������ ���� �� Ai �������� Ȯ��
    DHOST_TYPE_UINT32 SpeedHackLevel;       // ���ǵ��� ���?����
    DHOST_TYPE_FLOAT SpeedHackTimeAccumulate;   // ���ǵ��� ���?�����ð�
    DHOST_TYPE_FLOAT AwayFromKeyboardTimeAccumulate;   // �ڸ����?�����ð�
    DHOST_TYPE_FLOAT DisconnectTimeAccumulate;   // ��Ʈ��ũ ���� �����ð�
    ECHARACTER_KICK_TYPE CharacterKickType; // ű ���� ����
    DHOST_TYPE_UINT32 LatencyExceed100;     // �� �����Ͻð� 100ms �̻��ΰ��?
    DHOST_TYPE_UINT32 LatencyExceed200;     // �� �����Ͻð� 200ms �̻��ΰ��?
    DHOST_TYPE_FLOAT AvgPingLatency;        // ���?�� �����Ͻ�
    DHOST_TYPE_BOOL UseJoystick;            // ���̽�ƽ���� �÷��� �ߴ��� ����
    std::string UserName;
    std::string Team;
    DHOST_TYPE_UINT16 TotalScore;           // ���� ����
    DHOST_TYPE_UINT16 RegularTotalScore;           // ���Խð� ���� ����
    DHOST_TYPE_UINT16 Try2PointCount;       // 2���� �õ� Ƚ��
    DHOST_TYPE_UINT16 RegularTry2PointCount;       // ���Խð� 2���� �õ� Ƚ��
    DHOST_TYPE_UINT16 Suc2Pointcount;       // 2���� ���� Ƚ��
    DHOST_TYPE_UINT16 RegularSuc2Pointcount;       // ���Խð� 2���� ���� Ƚ��
    DHOST_TYPE_UINT16 Try3PointCount;       // 3���� �õ� Ƚ��
    DHOST_TYPE_UINT16 RegularTry3PointCount;       // ���Խð� 3���� �õ� Ƚ��
    DHOST_TYPE_UINT16 Suc3Pointcount;       // 3���� ���� Ƚ��
    DHOST_TYPE_UINT16 RegularSuc3Pointcount;       // ���Խð� 3���� ���� Ƚ��
    DHOST_TYPE_UINT16 AssistCount;          // ��ý��?
    DHOST_TYPE_UINT16 RegularAssistCount;          // ���Խð� ��ý��?
    DHOST_TYPE_UINT16 ReboundCount;         // ���ٿ��?
    DHOST_TYPE_UINT16 RegularReboundCount;         // ���Խð� ���ٿ��?
    DHOST_TYPE_UINT16 BlockCount;           // ����
    DHOST_TYPE_UINT16 RegularBlockCount;           // ���Խð� ����
    DHOST_TYPE_UINT16 StealCount;           // ��ƿ
    DHOST_TYPE_UINT16 RegularStealCount;           // ���Խð� ��ƿ
    DHOST_TYPE_UINT16 LooseBallCount;       // ���
    DHOST_TYPE_UINT16 RegularLooseBallCount;       // ���Խð� ���
    DHOST_TYPE_UINT16 TurnOverCount;        // �Ͽ��� �߻���Ų Ƚ��
    DHOST_TYPE_UINT16 RegularTurnOverCount;        // ���Խð� �Ͽ��� �߻���Ų Ƚ��
    DHOST_TYPE_UINT16 PassCount;            // �н��� �õ��� Ƚ��
    DHOST_TYPE_UINT16 NicePassCount;        // ���̽��н��� �� Ƚ��
    DHOST_TYPE_UINT16 RegularNicePassCount;
    DHOST_TYPE_UINT16 DivingCatchCount;     // ���̺�ĳġ�� ������ Ƚ��
    DHOST_TYPE_UINT16 OverPassCount;        // �����н��� �� Ƚ��
    DHOST_TYPE_UINT16 JumpShot2PointCount;  // 2�� ������
    DHOST_TYPE_UINT16 RegularJumpShot2PointCount;  // ���Խð� 2�� ������
    DHOST_TYPE_UINT16 MVPScore;             // MVP ����
    
    std::vector<SCharacterRecordShootInfo> vShootInfo;  // �� ���� (��ġ�� ���� ����)

    //����Ʈ ����
    std::map<EBAD_MANNER_TYPE, DHOST_TYPE_UINT16> mapBadMannerInfo; // Ÿ�Ժ� ��ų�?���� (Ƚ��)
    std::map<DHOST_TYPE_INT32, DHOST_TYPE_INT32> mapEmojiInfo;      // �̸��� ���?���� (Ƚ��)
    std::map<DHOST_TYPE_INT32, DHOST_TYPE_INT32> mapCeremonyInfo;   // �������?���?���� (Ƚ��)
    std::map<DHOST_TYPE_UINT32, DHOST_TYPE_INT32> mapSKillSucInfo;  // ���� �Ʒ� �ý��ۿ� ����?��ų���� Ƚ��

    DHOST_TYPE_UINT16 UseSkillCount;                                // ��ų ���?����
    DHOST_TYPE_UINT16 UseSignatureCount;                            // �ñ״�ó �ߵ� Ƚ��
    DHOST_TYPE_UINT16 UseQuickChatCount;                            // ��ä�� ���?Ƚ��
    DHOST_TYPE_UINT16 UseSwitchCount;                               // ����ġ ���?����
    DHOST_TYPE_UINT16 ActionGreatDefenseCount;                      // �ְ��� ���� �ߵ� Ƚ��
    DHOST_TYPE_UINT16 RegularActionGreatDefenseCount;
    DHOST_TYPE_UINT16 ActionLayUpGoalCount;                         // ���̾� �� Ƚ��
    DHOST_TYPE_UINT16 RegularActionLayUpGoalCount;
    DHOST_TYPE_UINT16 ActionDunkGoalCount;                          // ��ũ �� Ƚ��
    DHOST_TYPE_UINT16 RegularActionDunkGoalCount;
    DHOST_TYPE_UINT16 ActionAnkleBreakeCount;                       // ��Ŭ �극��Ŀ Ƚ��
    DHOST_TYPE_UINT16 RegularActionAnkleBreakeCount;
    DHOST_TYPE_UINT16 IllegalScreenCount;                           // �ϸ��� ��ũ�� Ƚ��
    //!

    //! ĳ���� ��ġ ����
    DHOST_TYPE_INT32 NotSamePositionCount;          // Ŭ���?������ ĳ���� ��ġ�� ������ n �̻��� ���?

    //! �� ����
    DHOST_TYPE_INT32 HackImpossibleAction;           // ��ȹ�ʿ��� �������� �� �̻��� ���?(��ƿ�Ÿ�, �����Ÿ�, ���ٰŸ�, ���� ���ʽ�)

    //! Ŭ�� �Ǵ��ϴ� �� ���?
    DHOST_TYPE_INT32 SpeedHackClientjudgment;       // Ŭ�� ���ǵ����̶��?�Ǵ��ϴ� ���?
    DHOST_TYPE_INT32 MemoryTamperClientjudgment;    // Ŭ���� �޸� ���� ����
    DHOST_TYPE_INT32 PacketTamperCount;             // ��Ŷ ���� �õ�

    // About Surrender 
    DHOST_TYPE_INT32 SurrenderType;
};

struct SRedisMatchResult
{
    uint32_t RoomID;
    EMODE_TYPE ModeType;
    int SkillChallengeScore;
    DHOST_TYPE_BOOL TutorialResult;
    std::string EndTime;
    DHOST_TYPE_CHARACTER_SN MVP;
    DHOST_TYPE_FLOAT PlayTime;
    std::vector<SCharacterRecord*> vCharacterRecord;
    std::vector<DHOST_TYPE_UINT32> vTeamScore;
    //! �߱��� ��û���� (���������� �������?�������?
    DHOST_TYPE_BOOL AbnormalExpireRoom;
    DHOST_TYPE_INT32 OverTimeCount; // ������ Ƚ��
};

struct SRedisUserKickReason
{
    uint32_t RoomID;
    DHOST_TYPE_USER_ID  UserID;
    ECHARACTER_KICK_TYPE KickType;
};

struct SBallActionInfo
{
    DHOST_TYPE_BALL_NUMBER BallNumber;
    DHOST_TYPE_CHARACTER_SN BallOwner;
    DHOST_TYPE_STR BallOwnerTeam;
    EBALL_STATE Ballstate;
    DHOST_TYPE_BOOL bAssist;
    DHOST_TYPE_GAME_TIME_F fGameTime;
    DHOST_TYPE_UINT32 SkillIndex;
};

struct MatchInfo
{
    int id;
    int target;
};

struct SPingRecvData
{
    DHOST_TYPE_FLOAT RecvTime;
    DHOST_TYPE_INT32 DataTime;
};

struct SBadMannerBalanceData
{
    DHOST_TYPE_FLOAT IntentionalBallHogPlayRemainGameTime;  // ���ð��� n �� ������ ��
    DHOST_TYPE_FLOAT IntentionalBallHogPlayOwnBall;         // ���� ����ä n �� ������ ��
    DHOST_TYPE_FLOAT IntentionalBallHogPlayCount1;          // ī���� 1 ����
    DHOST_TYPE_FLOAT IntentionalBallHogPlayCount2;          // ī���� 2 ����
    DHOST_TYPE_FLOAT IntentionalBallHogPlayCount3;          // ī���� 3 ����
    DHOST_TYPE_FLOAT IntentionalBallHogPlayCount4;          // ī���� 4 ����

    DHOST_TYPE_FLOAT AwayFromKeyboardOwnBall;               // ���� ����ä n �� ������ ��
    DHOST_TYPE_FLOAT AwayFromKeyboardNothing;               // �� ������ �����ϰ� n �� ������ ��
    DHOST_TYPE_FLOAT AwayFromKeyboardCount1;                // ī���� 1 ����
    DHOST_TYPE_FLOAT AwayFromKeyboardCount2;                // ī���� 2 ����
    DHOST_TYPE_FLOAT AwayFromKeyboardCount3;                // ī���� 3 ����

    DHOST_TYPE_FLOAT ImpossibleShootingShotClock;           // ��Ŭ���� n �� ������ ��
    DHOST_TYPE_FLOAT ImpossibleShootingCount1;              // ī���� 1 ����
    DHOST_TYPE_FLOAT ImpossibleShootingCount2;              // ī���� 2 ����
    DHOST_TYPE_FLOAT ImpossibleShootingCount3;              // ī���� 3 ����
    DHOST_TYPE_FLOAT ImpossibleShootingCount4;              // ī���� 4 ����
};

struct SBadMannerIntentionalBallHogPlay
{
    DHOST_TYPE_CHARACTER_SN CharacterSN;
    DHOST_TYPE_FLOAT OwnBallTime;   // ���� �����ߴ� �ð�
    DHOST_TYPE_BALL_NUMBER BallNo;  // �� ��ȣ
};

struct SSurrenderVoteInfo
{
    DHOST_TYPE_CHARACTER_SN CharacterSN;
    DHOST_TYPE_BOOL vote;   // ture = ����, false = �ݴ�
};

struct SAddAbility
{
    std::string AbilityName;        // "speed", "pass", ....
    int AddValue;
    int TimesType;  //1:?�长s   2:次数
    int Times;
};

struct SGiftFrom
{
    int         Id;
    uint32_t    UserID;
    std::string UserName;
};

struct SGiveGiftInfo
{
    uint32_t UserID;
    std::map<std::string, SAddAbility>Attrs;
    SGiftFrom GiftFrom;
};

