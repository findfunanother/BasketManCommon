#include "ResourceCppHeader.h"
#include "InGameBalanceData.h"

namespace Resource
{
	CInGameBalance::CInGameBalance()
	{
	}

	CInGameBalance::~CInGameBalance()
	{
	}

	bool CInGameBalance::Load(int ascendStep, std::string path)
	{
//		std::string filename = "BalanceTable.csv";
//		std::string fullname;
//
//		if (path == "")
//		{
//			char buff[FILENAME_MAX]; //create string buffer to hold path
//
//			std::string filename = "BalanceTable.csv";
//
//			std::string currentpath = GetCurrentDir(buff, sizeof(buff));
//			currentpath += "\\";
//
//			std::vector<std::string> piece = Split(currentpath, "\\");
//
//			std::string fullname;
//
//			for (int i = 0; i < piece.size() - ascendStep; i++)
//			{
//				fullname += piece[i] + "\\";
//			}
//		}
//		
//		fullname += path + filename;
//
//#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
//#else
//		std::string LinuxPath = "./game/cservice/Resource/";
//		fullname = LinuxPath + filename;
//#endif
//		CsvParser* csvparser = CsvParser_new(fullname.c_str(), ",", 1);
//
//		if (nullptr == csvparser)
//		{
//			return false;
//		}
//
//		CsvRow* row;
//		const CsvRow* header = CsvParser_getHeader(csvparser);
//
//		if (header == nullptr)
//		{
//			const char* errormsg = CsvParser_getErrorMessage(csvparser);
//			std::cout << "CInGameBalance load faile fullname : " << fullname.c_str() << ", errormsg : " << errormsg << std::endl;
//			return false;
//		}
//
//		std::string key;
//		 
//		int keyIndex = 0;
//		int valueIndex = 1;
//
//		while ((row = CsvParser_getRow(csvparser)))
//		{
//			const char** rowFields = CsvParser_getFields(row);
//			// int count = CsvParser_getNumFields(row);
//
//			key = GETFIELD_STRING(keyIndex);
//
//			this->m_data.BalanceData.insert(std::pair<std::string, float>(key, GETFIELD_FLOAT(valueIndex)));
//
//			key.clear();
//
//			CsvParser_destroy_row(row);
//		}
//
//		CsvParser_destroy(csvparser);
		return true;
	}

	bool CInGameBalance::UnLoad()
	{
		return true;
	}

	bool CInGameBalance::CheckData()
	{
		bool revalue = true;

		return revalue;
	}
}  // namespace Resource
