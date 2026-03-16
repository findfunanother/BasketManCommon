#include "FileLog.h"
#include "Util.h"
#ifdef TEXT_LOG_FILE

const string CLogger::m_sFileName = "Log.txt";
CLogger* CLogger::m_pThis = NULL;
ofstream CLogger::m_Logfile;

CLogger::CLogger()
{

}
CLogger* CLogger::GetLogger()
{
    if (m_pThis == NULL)
    {
        m_pThis = new CLogger();
        m_Logfile.open(m_sFileName.c_str(), ios::out | ios::app);
    }
    return m_pThis;
}

void CLogger::Log(const char* format, ...)
{
    if (m_Logfile.is_open() == false)
    {
        m_Logfile.open(m_sFileName.c_str(), ios::out | ios::app);
    }

    char* sMessage = NULL;
    int nLength = 0;
    va_list args;
    va_start(args, format);
    nLength = _vscprintf(format, args) + 1;
    sMessage = new char[nLength];
    vsprintf_s(sMessage, nLength, format, args);
    m_Logfile << Util::CurrentDateTime() << "  ";
    m_Logfile << sMessage << "\n";
    va_end(args);
    delete[] sMessage;

    m_Logfile.close();
}

void CLogger::Log(const string& sMessage)
{
    if (m_Logfile.is_open() == false)
    {
        m_Logfile.open(m_sFileName.c_str(), ios::out | ios::app);
    }

    m_Logfile << Util::CurrentDateTime() << "  ";
    m_Logfile << sMessage << "\n";

    m_Logfile.close();
}
#endif // TEXT_LOG_FILE