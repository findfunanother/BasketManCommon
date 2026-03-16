#pragma once
#include <stdint.h>
#include <cmath>
#include <sstream>
#include <string>
#include <set>

#include "ResourceBase.h"
#include "ResourceStruct.h"
#include "InGameBalanceData.h"
#include "ShotSolution.h"

namespace Resource
{
	class CResourceManager
	{
	public:

		CResourceManager();
		~CResourceManager();

		bool Initialize();
		bool Finalize();
		bool Verify();

		static CResourceManager* GetInstance();

		static void Release();

		bool Load(int ascendStep, std::string path);
		bool UnLoad();

	private:
		static CResourceManager* m_pInstance;


	private:
		//std::set<CResourceBase*> resources_;
		CInGameBalance m_InGameBalanceData;

	public:
		const Resource::_InGameBalanceData& GetResourceInGameBalanceData() { return m_InGameBalanceData.GetData(); }
		const float GetValue(std::string key);
	};
}

#ifndef ResourceInst
#define ResourceInst Resource::CResourceManager::GetInstance()
#endif