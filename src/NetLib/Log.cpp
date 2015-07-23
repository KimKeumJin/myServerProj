#include "StdAfx.h"
#include "Log.h"
#include <string>


CLog::CLog(void)
{
	GetLocalTime(&m_FileTime);	

	TCHAR moduleFile[1024] = { 0, };
	GetModuleFileName(NULL, moduleFile, 1024);

	m_FileDirectory = moduleFile;
	m_ExeFileName = moduleFile;

	// 실행파일 디렉토리
	std::basic_string <wchar_t>::size_type index1 = m_FileDirectory.find_last_of(L"\\");
	m_FileDirectory.erase(m_FileDirectory.begin() + index1, m_FileDirectory.end());

	// 실행파일명
	std::basic_string <wchar_t>::size_type index2 = m_ExeFileName.find_last_of(L".");
	std::basic_string <wchar_t>::size_type index3 = m_ExeFileName.find_last_of(L"\\");
	m_ExeFileName.erase(m_ExeFileName.begin() + index2, m_ExeFileName.end());
	m_ExeFileName.erase(m_ExeFileName.begin(), m_ExeFileName.begin() + index3 + 1);

	_stprintf_s(m_FileName, MAX_PATH, _T("%s\\%s %04u-%02u-%02u %02u.log"),
		m_FileDirectory.c_str(),
		m_ExeFileName.c_str(),
		m_FileTime.wYear, 
		m_FileTime.wMonth, 
		m_FileTime.wDay, 
		m_FileTime.wHour);

	m_FileStream.imbue(std::locale(""));
	m_FileStream.open(m_FileName, std::ios::app);
}


CLog::~CLog(void)
{
	if(m_FileStream.is_open())
	{
		m_FileStream.flush();
		m_FileStream.close();
	}
}

VOID CLog::WriteLog( TCHAR* logString, ... )
{
	va_list ap;
	TCHAR szLog[1024] = { 0, };

	va_start(ap, logString);
	_vstprintf_s(szLog, logString, ap);
	va_end(ap);

	OutputLog(LOG_NORMAL, szLog);
}

VOID CLog::WriteLog( DWORD logLevel, TCHAR* logString, ... )
{
	va_list ap;
	TCHAR szLog[1024]	= {0,};

	va_start(ap, logString);
	_vstprintf_s(szLog, logString, ap);
	va_end(ap);

	OutputLog(logLevel, szLog);
}

VOID CLog::OutputLog(DWORD logLevel, TCHAR* logString)
{
	SYSTEMTIME current;
	GetLocalTime(&current);

	TCHAR szCurrentTime[16] = {0,};
	_stprintf_s(szCurrentTime, 16, _T("%02u:%02u:%02u"),
		current.wHour,
		current.wMinute,
		current.wSecond);

	TCHAR* szLogLevel = NULL;
	
	switch (logLevel)
	{
	case LOG_NORMAL:
		szLogLevel = L"normal";
		break;

	case LOG_WARNING:
		szLogLevel = L"warning";
		break;

	case LOG_ERROR:
		szLogLevel = L"error";
		break;

	default:
		szLogLevel = L"unknown";
	}


	TCHAR szTotalLog[2048] = {0,};
	_stprintf_s(szTotalLog, 1024, _T("[%s] [%.8s]: %s\n"),
		szCurrentTime, szLogLevel, logString);

#ifdef _DEBUG
	OutputDebugString(szTotalLog);
#endif

#ifdef _CONSOLE
	_tprintf_s(szTotalLog);
#endif

	if(m_FileTime.wYear != current.wYear
		|| m_FileTime.wMonth != current.wMonth
		|| m_FileTime.wDay != current.wDay
		|| m_FileTime.wHour != current.wHour)
	{
		m_FileStream.close();

		_stprintf_s(m_FileName, MAX_PATH, _T("%s%s %04u-%02u-%02u %02u.log"),
			m_FileDirectory.c_str(),
			m_ExeFileName.c_str(),
			m_FileTime.wYear,
			m_FileTime.wMonth,
			m_FileTime.wDay,
			m_FileTime.wHour);

		m_FileStream.open(m_FileName, std::ios::app);

		m_FileTime = current;
	}

	m_FileStream << szTotalLog;;
	m_FileStream.flush();

}


namespace proto
{
	void logging(TCHAR* logString, ...)
	{
		va_list ap;
		TCHAR szLog[1024] = { 0, };

		va_start(ap, logString);
		_vstprintf_s(szLog, logString, ap);
		va_end(ap);

		_tprintf_s(szLog);
	}
}