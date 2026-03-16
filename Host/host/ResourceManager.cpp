#include "HostDefine.h"
#include "ResourceManager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;
namespace Resource
{
	CResourceManager* CResourceManager::m_pInstance = nullptr;

	CResourceManager* CResourceManager::GetInstance()
	{
		if (m_pInstance == nullptr)
			m_pInstance = new CResourceManager();

		return m_pInstance;
	}

	void CResourceManager::Release()
	{
		if (nullptr == m_pInstance)
			return;

		SAFE_DELETE(m_pInstance);
	}

	CResourceManager::CResourceManager()
	{
		string path = "";

		char buff[FILENAME_MAX]; //create string buffer to hold path
		std::string currentpath = GetCurrentDir(buff, sizeof(buff));

		string iniPath = currentpath + "\\resourcePath.ini";

		ifstream inFile;
		inFile.open(iniPath);

		if (inFile)
		{
			getline(inFile, path);
		}
		
		inFile.close();

		this->Load(0, path);
	}

	CResourceManager::~CResourceManager()
	{
		UnLoad();
	}

	bool CResourceManager::Load(int ascentStep, std::string path)
	{
		/*
		if (false == m_InGameBalanceData.Load(ascentStep, path))
		{
			return false;
		}

		if (false == m_ShotSolution.Load(ascentStep, path))
		{
			return false;
		}
		*/

		return true;
	}

	bool CResourceManager::UnLoad()
	{
		this->m_InGameBalanceData.UnLoad();
		return true;
	}

	bool CResourceManager::Initialize()
	{
		return true;
	}

	bool CResourceManager::Finalize()
	{
		return true;
	}

	bool CResourceManager::Verify()
	{
		return true;
	}

	const float CResourceManager::GetValue(std::string key)
	{
		auto map = m_InGameBalanceData.GetData().BalanceData;
		auto iter = map.find(key);
		
		return iter == map.end() ? 0.0f : iter->second;
	}

}  // namespace Resource
