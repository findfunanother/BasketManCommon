#pragma once
#include "HostDefine.h"
#ifdef TEXT_LOG_FILE

#include <fstream>
#include <iostream>
#include <cstdarg>
#include <string>

#define LOGGER CLogger::GetLogger()

using namespace std;

class CLogger
{
public:
    void Log(const std::string& sMessage);
    void Log(const char* format, ...);
    static CLogger* GetLogger();

private:
    CLogger();
    CLogger(const CLogger&) {};
    CLogger& operator=(const CLogger&) { return *this; };
    static const std::string m_sFileName;
    static CLogger* m_pThis;
    static ofstream m_Logfile;
};
#endif // TEXT_LOG_FILE