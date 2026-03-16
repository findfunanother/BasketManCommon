#include "State_JumpBall.h"
#include "DataManagerBalance.h"
#include "BallController.h"

void CState_JumpBall::OnEnter(void* pData)
{
    string log_message = "CState_JumpBall::OnEnter RoomElapsedTime : " + std::to_string(m_pHost->GetRoomElapsedTime());
    m_pHost->ToLog(log_message.c_str());

    // 비매너 감지 초기화
    m_pHost->InitMannerManagerData(m_pHost->GetGameTime(), m_pHost->BallNumberGet());

    MedalCheckAndApply();

    CBallController* pBallController = m_pHost->BallControllerGet();
    if (pBallController != nullptr)
    {
        //pBallController->RemoveCurrentBallData();
        if(pBallController->GameBallDataGet())
            pBallController->GameBallDataGet()->ballState = F4PACKET::BALL_STATE::ballState_none;
    }

    CREATE_BUILDER(builder)
    CREATE_FBPACKET(builder, system_s2c_readyToJumpBall, message, send_data);
    send_data.add_ballnumber(m_pHost->BallNumberGet());
    send_data.add_team1(0);
    send_data.add_team2(1);
    send_data.add_id11(m_pHost->GetCharacterSN(0, 0));
    send_data.add_id12(m_pHost->GetCharacterSN(0, 1));
    send_data.add_id13(m_pHost->GetCharacterSN(0, 2));
    send_data.add_id21(m_pHost->GetCharacterSN(1, 0));
    send_data.add_id22(m_pHost->GetCharacterSN(1, 1));
    send_data.add_id23(m_pHost->GetCharacterSN(1, 2));
    send_data.add_id31(m_pHost->GetCharacterSN(2, 0));
    send_data.add_id32(m_pHost->GetCharacterSN(2, 1));
    send_data.add_id33(m_pHost->GetCharacterSN(2, 2));
    send_data.add_overtime(m_pHost->GetOverTime());
    send_data.add_time(m_pHost->GetGameTimeInit());
    STORE_FBPACKET(builder, message, send_data)

    timeElapsed = 0.0f;
    
    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}

void CState_JumpBall::OnUpdate(float timeDelta)
{
    CState::OnUpdate(timeDelta);

    timeElapsed += timeDelta;

    if (timeWait <= timeElapsed)
    {
        m_pHost->BallNumberAdd(1);

        float velocityX = rand() / 2 == 0 ? 0.0f : (((float)rand() / RAND_MAX) * 0.3f - 0.2f);
        float velocityY = 5.5f + Util::GetRandom(1.5f);
        float velocityZ = Util::GetRandom(1.0f) - 0.5f;

        auto datavelocity = TB::SVector3(velocityX, velocityY, velocityZ);

        CREATE_BUILDER(builder)
        CREATE_FBPACKET(builder, system_s2c_jumpBall, message, send_data);
        send_data.add_ballnumber(m_pHost->BallNumberGet());
        send_data.add_velocity(&datavelocity);
        STORE_FBPACKET(builder, message, send_data)

        m_pHost->BroadcastPacket(message, kUSER_ID_INIT);

        m_pHost->ChangeState(EHOST_STATE::PLAY);
    }
}

DHOST_TYPE_BOOL CState_JumpBall::OnPacket(CPacketBase* pPacket, void* peer, const DHOST_TYPE_GAME_TIME_F& recv_time)
{
    if (nullptr == pPacket)
        return false;

    return CState::OnPacket(pPacket, peer, recv_time);
}


void CState_JumpBall::MedalCheckAndApply()
{
    // 처음 게임 시작할 때 하는 점프볼에서만 실행. 이외의 연장전 또는 다른 점프볼에서의 추가적용 방지.
    if (justOneApply == false)
        justOneApply = true;
    else
        return;

    m_pHost->SetJumpBallStartTime(m_pHost->GetRoomElapsedTime());

    map<DHOST_TYPE_CHARACTER_SN, CCharacter*> characters = m_pHost->GetCharacterManager()->GetCharacters();
    for (auto it = characters.begin(); it != characters.end(); ++it)
    {
        CCharacter* character = it->second;

        if (character == nullptr) continue;

        SPlayerInformationT* characterInformation = character->GetCharacterInformation();

        //if (characterInformation == nullptr)
        //    continue;

        // 캐릭터가 착용중인 모든 메달 확인
        for (int k = 0; k < characterInformation->medals.size(); k++)
        {
            F4PACKET::SMedalInfo medalInfo = characterInformation->medals[k];

            float medalBalanceValue = 0;
            ABILITY_TYPE abilityTypeA = ABILITY_TYPE::abilityType_none;
            ABILITY_TYPE abilityTypeB = ABILITY_TYPE::abilityType_none;

            // 발동 조건 확인
            switch (medalInfo.index())
            {
            case F4PACKET::MEDAL_INDEX::medal_twoBigGameBallHandler:
            {
                // 메달 발동 조건 확인
                int count_C = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_center);
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);

                if (count_PF + count_C < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoBigGame_BallHandler");
                abilityTypeA = ABILITY_TYPE::abilityType_threePointShot;
                abilityTypeB = ABILITY_TYPE::abilityType_dribble;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoBigGameScorer:
            {
                // 메달 발동 조건 확인
                int count_C = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_center);
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);

                if (count_PF + count_C < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoBigGame_Scorer");
                abilityTypeA = ABILITY_TYPE::abilityType_layUp;
                abilityTypeB = ABILITY_TYPE::abilityType_dunk;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoBigGameBigman:
            {
                // 메달 발동 조건 확인
                int count_C = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_center);
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);

                if (count_PF + count_C < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoBigGame_Bigman");
                abilityTypeA = ABILITY_TYPE::abilityType_postDefense;
                abilityTypeB = ABILITY_TYPE::abilityType_block;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoGuardGameBallHandler:
            {
                // 메달 발동 조건 확인
                int count_PG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_point);
                int count_SG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_shooting);

                if (count_PG + count_SG < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoGuardGame_BallHandler");
                abilityTypeA = ABILITY_TYPE::abilityType_threePointShot;
                abilityTypeB = ABILITY_TYPE::abilityType_pass;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoGuardGameScorer:
            {
                // 메달 발동 조건 확인
                int count_PG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_point);
                int count_SG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_shooting);

                if (count_PG + count_SG < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoGuardGame_Scorer");
                abilityTypeA = ABILITY_TYPE::abilityType_midRangeShot;
                abilityTypeB = ABILITY_TYPE::abilityType_threePointShot;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoGuardGameBigman:
            {
                // 메달 발동 조건 확인
                int count_PG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_point);
                int count_SG = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_guard_shooting);

                if (count_PG + count_SG < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoGuardGame_Bigman");
                abilityTypeA = ABILITY_TYPE::abilityType_rebound;
                abilityTypeB = ABILITY_TYPE::abilityType_pass;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoForwardGameBallHandler:
            {
                // 메달 발동 조건 확인
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);
                int count_SF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_small);

                if (count_SF + count_PF < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoForwardGame_BallHandler");
                abilityTypeA = ABILITY_TYPE::abilityType_pass;
                abilityTypeB = ABILITY_TYPE::abilityType_dribble;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoForwardGameScorer:
            {
                // 메달 발동 조건 확인
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);
                int count_SF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_small);

                if (count_SF + count_PF < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoForwardGame_Scorer");
                abilityTypeA = ABILITY_TYPE::abilityType_midRangeShot;
                abilityTypeB = ABILITY_TYPE::abilityType_layUp;
            }
            break;
            case F4PACKET::MEDAL_INDEX::medal_twoForwardGameBigman:
            {
                // 메달 발동 조건 확인
                int count_PF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_power);
                int count_SF = m_pHost->GetCharacterManager()->GetRoleCount(characterInformation->team, F4PACKET::ROLE::role_foward_small);

                if (count_SF + count_PF < 2)
                    continue;

                // 메달 효과 값 세팅
                medalBalanceValue = m_pHost->GetBalanceTable()->GetValue("VMedal_TwoForwardGame_Bigman");
                abilityTypeA = ABILITY_TYPE::abilityType_postDefense;
                abilityTypeB = ABILITY_TYPE::abilityType_rebound;
            }
            break;
            default:
                continue;
            }

            // 메달 효과 적용
            float medalAddValue = medalInfo.value() * medalBalanceValue;

            float abilityValueA = medalAddValue + character->GetAbility()->GetModificationAbility(abilityTypeA);
            float abilityValueB = medalAddValue + character->GetAbility()->GetModificationAbility(abilityTypeB);

            character->GetAbility()->SetModificationAbility(abilityTypeA, abilityValueA);
            character->GetAbility()->SetModificationAbility(abilityTypeB, abilityValueB);


            // 능력치 변경 패킷 전송
            SendModificationAbilitiesMedalTwoRoleGame
            (
                characterInformation->id,
                abilityTypeA,
                abilityTypeB,
                abilityValueA,
                abilityValueB
            );


            // 메달 디스플레이 패킷 전송
            SendMedalUIDisplay(characterInformation->id, medalInfo.index());
        }
    }
}

void CState_JumpBall::SendModificationAbilitiesMedalTwoRoleGame(int32_t id, ABILITY_TYPE typeA, ABILITY_TYPE typeB, float valueA, float valueB)
{
    // 빌더 생성
    CREATE_BUILDER(builder);
    std::vector< flatbuffers::Offset<F4PACKET::SPlayerModificationAbility>> vecModificationAbilities;

    // 빌더에 변경될 능력치 정보 추가
    F4PACKET::SPlayerModificationAbilityBuilder abilityBuilder_A(builder);
    abilityBuilder_A.add_abilitytype(typeA);
    abilityBuilder_A.add_value(valueA);
    vecModificationAbilities.push_back(abilityBuilder_A.Finish());

    F4PACKET::SPlayerModificationAbilityBuilder abilityBuilder_B(builder);
    abilityBuilder_B.add_abilitytype(typeB);
    abilityBuilder_B.add_value(valueB);
    vecModificationAbilities.push_back(abilityBuilder_B.Finish());

    auto offsetcountinfo = builder.CreateVector(vecModificationAbilities);

    // 패킷 생성 및 전송
    CREATE_FBPACKET(builder, play_s2c_playerModificationAbility, message, databuilder);
    databuilder.add_playerid(id);
    databuilder.add_playermodificationabilities(offsetcountinfo);

    STORE_FBPACKET(builder, message, databuilder);

    m_pHost->BroadcastPacket(message, kUSER_ID_INIT);
}