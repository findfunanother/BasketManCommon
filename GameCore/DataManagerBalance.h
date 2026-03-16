#pragma once
#include <string>
#include <map>
#pragma warning(disable: 4251)

#ifdef _MSC_VER
#ifdef GAMECORE_EXPORT_DLL
#define DECLSPECBALANCE __declspec(dllexport)
#else
#define DECLSPECBALANCE __declspec(dllimport) 
#endif
#define DLLOCAL
#else
#define DECLSPECBALANCE __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))
#endif


#define SHOT_BONUS_PASSIVE_MAX   0.35f
#define SHOT_BONUS_ANKLE_MAX     0.05f
#define SHOT_BONUS_NICEPASS_MAX  0.3f
#define SHOT_BONUS_SIGNATURE_MAX 0.05f
#define REBOUND_MAX_DISTANCE 5.5f

using namespace std;
namespace TB
{
    struct F4BalanceTable;
    struct F4Balance;
}
template<typename> class CDataController;

class DECLSPECBALANCE CDataManagerBalance
{
    
protected:
    CDataController<TB::F4BalanceTable> *m_dataController;
    typedef map<string, const TB::F4Balance*> MAPDATA;
    MAPDATA m_mapData;
    std::map<int, string> m_mapBalanceData;

public:
    CDataManagerBalance();
    virtual ~CDataManagerBalance();
    void CreateDictionary();// ;
    bool HasData();
    float GetValue(string str);
    void GetIndexName(int idx, string& str);

    CDataController<TB::F4BalanceTable>* GetData() { return m_dataController; }

    int GetBalanceRawDataLen() const;
    const unsigned char* GetBalanceRawData() const;
};

