#pragma once
#include "FlatBufProtocol.h"

class CAbility
{

public:

    CAbility(F4PACKET::SPlayerInformationT* pInfo);
    ~CAbility();

    /// <summary>
    /// 지정한 타입의 최종값 리턴.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    float GetAbility(F4PACKET::ABILITY_TYPE type);
    F4PACKET::SPlayerAbilityT* GetAbilityResult();


    /// <summary>
    /// 지정한 타입의 고유값 리턴.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    float GetDefaultAbility(F4PACKET::ABILITY_TYPE type);

    /// <summary>
    /// 지정한 타입의 가감값 리턴.
    /// </summary>
    /// <param name="type"></param>
    /// <returns></returns>
    float GetModificationAbility(F4PACKET::ABILITY_TYPE type);
    F4PACKET::SPlayerAbilityT* GetModificationAbility();

    /// <summary>
    /// 지정한 타입의 고유값 세팅
    /// </summary>
    /// <param name="type"></param>
    /// <param name="setValue"></param>
    void SetDefaultAbility(F4PACKET::ABILITY_TYPE type, float setValue);

    /// <summary>
    /// 지정한 타입의 가감값 세팅
    /// </summary>
    /// <param name="type"></param>
    /// <param name="setValue"></param>
    void SetModificationAbility(F4PACKET::ABILITY_TYPE type, float setValue);

    void SetModificationAbilityAll(float setValue);

private:

    F4PACKET::SPlayerAbilityT* pDefault;        // [기본 능력치]
    F4PACKET::SPlayerAbilityT* pModification;   // [가감되는 능력치]
    F4PACKET::SPlayerAbilityT* pResult;         // 최종 능력치 = [기본 능력치] + [가감되는 능력치]


    // 요청한 타입에 맞는 값을 리턴.
    float GetValueOfPlayerAbilityForType(F4PACKET::SPlayerAbilityT* playerAbility, F4PACKET::ABILITY_TYPE type);

    // 요청한 타입의 값을 세팅.
    void SetValueOfPlayerAbilityForType(F4PACKET::SPlayerAbilityT* playerAbility, F4PACKET::ABILITY_TYPE type, float setValue);

    // 최종 ability result 값을 모두 재계산.
    void refreshAllResult();

    // 공식을 적용하여 계산한 값을 리턴.
    float resulting(float defaultValue, float modificationValue);
};