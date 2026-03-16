#include "Ability.h"

CAbility::CAbility(F4PACKET::SPlayerInformationT* pInfo)
{
	pDefault = pInfo->ability.get();
	pModification = pInfo->abilitymodification.get();
	pResult = new F4PACKET::SPlayerAbilityT();

    refreshAllResult();
}

CAbility::~CAbility()
{
    if (pResult)
    {
        SAFE_DELETE(pResult);
    }
    pResult = nullptr;
}

float CAbility::GetAbility(F4PACKET::ABILITY_TYPE type)
{
	return GetValueOfPlayerAbilityForType(pResult, type);
}

F4PACKET::SPlayerAbilityT* CAbility::GetAbilityResult()
{
    return pResult;
}

float CAbility::GetDefaultAbility(F4PACKET::ABILITY_TYPE type)
{
	return GetValueOfPlayerAbilityForType(pDefault, type);
}

float CAbility::GetModificationAbility(F4PACKET::ABILITY_TYPE type)
{
	return GetValueOfPlayerAbilityForType(pModification, type);
}

F4PACKET::SPlayerAbilityT* CAbility::GetModificationAbility()
{
    return pModification;
}

void CAbility::SetDefaultAbility(F4PACKET::ABILITY_TYPE type, float setValue)
{
    SetValueOfPlayerAbilityForType(pDefault, type, setValue);
    refreshAllResult();
}

void CAbility::SetModificationAbility(F4PACKET::ABILITY_TYPE type, float setValue)
{
    SetValueOfPlayerAbilityForType(pModification, type, setValue);
    refreshAllResult();
}

void CAbility::SetModificationAbilityAll(float setValue)
{
    for (int i = static_cast<int>(F4PACKET::ABILITY_TYPE::MIN); i <= static_cast<int>(F4PACKET::ABILITY_TYPE::MAX); ++i)
    {
        SetValueOfPlayerAbilityForType(pModification, static_cast<F4PACKET::ABILITY_TYPE>(i), setValue);
    }

    refreshAllResult();
}


float CAbility::GetValueOfPlayerAbilityForType(F4PACKET::SPlayerAbilityT* playerAbility, F4PACKET::ABILITY_TYPE type)
{
    switch (type)
    {
    case F4PACKET::ABILITY_TYPE::abilityType_postShot:
        return playerAbility->postshot;
    case F4PACKET::ABILITY_TYPE::abilityType_midRangeShot:
        return playerAbility->midrangeshot;
    case F4PACKET::ABILITY_TYPE::abilityType_threePointShot:
        return playerAbility->threepointshot;
    case F4PACKET::ABILITY_TYPE::abilityType_dunk:
        return playerAbility->dunk;
    case F4PACKET::ABILITY_TYPE::abilityType_layUp:
        return playerAbility->layup;
    case F4PACKET::ABILITY_TYPE::abilityType_pass:
        return playerAbility->pass;
    case F4PACKET::ABILITY_TYPE::abilityType_dribble:
        return playerAbility->dribble;
    case F4PACKET::ABILITY_TYPE::abilityType_postDefense:
        return playerAbility->postdefense;
    case F4PACKET::ABILITY_TYPE::abilityType_perimeterDefense:
        return playerAbility->perimeterdefense;
    case F4PACKET::ABILITY_TYPE::abilityType_block:
        return playerAbility->block;
    case F4PACKET::ABILITY_TYPE::abilityType_rebound:
        return playerAbility->rebound;
    case F4PACKET::ABILITY_TYPE::abilityType_steal:
        return playerAbility->steal;
    case F4PACKET::ABILITY_TYPE::abilityType_strength:
        return playerAbility->strength;
    case F4PACKET::ABILITY_TYPE::abilityType_vertical:
        return playerAbility->vertical;
    case F4PACKET::ABILITY_TYPE::abilityType_speed:
        return playerAbility->speed;
    case F4PACKET::ABILITY_TYPE::abilityType_postMove:
        return playerAbility->postmove;
    default:
        return 0;
    }
}

void CAbility::SetValueOfPlayerAbilityForType(F4PACKET::SPlayerAbilityT* playerAbility, F4PACKET::ABILITY_TYPE type, float setValue)
{
    switch (type)
    {
    case F4PACKET::ABILITY_TYPE::abilityType_postShot:
        playerAbility->postshot = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_midRangeShot:
        playerAbility->midrangeshot = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_threePointShot:
        playerAbility->threepointshot = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_dunk:
        playerAbility->dunk = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_layUp:
        playerAbility->layup = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_pass:
        playerAbility->pass = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_dribble:
        playerAbility->dribble = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_postDefense:
        playerAbility->postdefense = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_perimeterDefense:
        playerAbility->perimeterdefense = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_block:
        playerAbility->block = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_rebound:
        playerAbility->rebound = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_steal:
        playerAbility->steal = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_strength:
        playerAbility->strength = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_vertical:
        playerAbility->vertical = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_speed:
        playerAbility->speed = setValue;
        break;
    case F4PACKET::ABILITY_TYPE::abilityType_postMove:
        playerAbility->postmove = setValue;
        break;
    }
}


void CAbility::refreshAllResult()
{
    pResult->block              = resulting(pDefault->block,            pModification->block);
    pResult->dribble            = resulting(pDefault->dribble,          pModification->dribble);
    pResult->dunk               = resulting(pDefault->dunk,             pModification->dunk);
    pResult->layup              = resulting(pDefault->layup,            pModification->layup);
    pResult->midrangeshot       = resulting(pDefault->midrangeshot,     pModification->midrangeshot);
    pResult->pass               = resulting(pDefault->pass,             pModification->pass);
    pResult->perimeterdefense   = resulting(pDefault->perimeterdefense, pModification->perimeterdefense);
    pResult->postdefense        = resulting(pDefault->postdefense,      pModification->postdefense);
    pResult->postmove           = resulting(pDefault->postmove,         pModification->postmove);
    pResult->postshot           = resulting(pDefault->postshot,         pModification->postshot);
    pResult->rebound            = resulting(pDefault->rebound,          pModification->rebound);
    pResult->speed              = resulting(pDefault->speed,            pModification->speed);
    pResult->steal              = resulting(pDefault->steal,            pModification->steal);
    pResult->strength           = resulting(pDefault->strength,         pModification->strength);
    pResult->threepointshot     = resulting(pDefault->threepointshot,   pModification->threepointshot);
    pResult->vertical           = resulting(pDefault->vertical,         pModification->vertical);
}


float CAbility::resulting(float defaultValue, float modificationValue)
{
    return defaultValue + modificationValue;
}