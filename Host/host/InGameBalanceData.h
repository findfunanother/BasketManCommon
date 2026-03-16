#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "ResourceStruct.h"
#include "CSVLoader.h"

namespace Resource
{
	class CInGameBalance
	{
	public:
		CInGameBalance();
		~CInGameBalance();

		bool Load(int ascendStep, std::string path);
		bool UnLoad();

		bool CheckData();

		const InGameBalanceData& GetData() { return m_data; }

	private:
		InGameBalanceData m_data;
	};
}  // namespace F3Resource
