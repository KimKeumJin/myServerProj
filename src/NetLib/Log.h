#pragma once

#if _UNICODE
typedef std::wofstream	OFSTREAM;
typedef std::wifstream	IFSTREAM;
typedef std::wstreampos	STREAMPOS;
#else
typedef std::ofstream	OFSTREAM;
typedef std::ifstream	IFSTREAM;
typedef std::streampos	STREAMPOS;
#endif

class CLog
	: public CSingleTon<CLog>
{
	friend class CSingleTon<CLog>;

private:
	std::wstring		m_FileDirectory;
	std::wstring		m_ExeFileName;

	TCHAR				m_FileName[MAX_PATH];
	OFSTREAM			m_FileStream;
	SYSTEMTIME			m_FileTime;

private:
	VOID OutputLog(DWORD logLevel, TCHAR* logString);

public:
	VOID WriteLog(TCHAR* logString, ...);
	VOID WriteLog(DWORD logLevel, TCHAR* logString, ...);

private:
	CLog(void);
	~CLog(void);
};


// 프로토타입 전용(빠른 개발 목적)
namespace proto
{
	void logging(TCHAR* logString, ...);
}

//#define LOGGING CLog::GetInstance().WriteLog
#define LOG proto::logging
