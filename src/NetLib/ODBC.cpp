#include "stdafx.h"
#include "ODBC.h"
#include "ODBCUtil.h"



CODBC::CODBC()
	:m_hEnv(NULL),
	m_hDbc(NULL),
	m_hStmt(NULL),
	m_bConnected(FALSE)
{
}


CODBC::~CODBC()
{
	Release();
}

void CODBC::Release()
{
	if (m_hStmt)
	{ 
		SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
		m_hStmt = NULL;
	}

	if (m_hDbc)
	{
		SQLDisconnect(m_hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
		m_hDbc = NULL;
	}

	if (m_hEnv)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = NULL;
	}

	m_bConnected = FALSE;
}

bool CODBC::Connect(const TCHAR* szHostName, const TCHAR* szDBName,
	const TCHAR* szID, const TCHAR* szPass)
{
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv) == SQL_ERROR)
	{
		LOG(_T("Unable to allocate an environment handle\n"));
		return false;
	}

	RETCODE rc;
	
	rc = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if (rc == SQL_ERROR)
	{
		Release();
		return false;
	}

	rc = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
	if (rc == SQL_ERROR)
	{
		Release();
		return false;
	}


	SQLSetConnectAttr(m_hDbc, 5, (SQLPOINTER)SQL_LOGIN_TIMEOUT, 0);

	// DSN 栏肺 立加窍绰 规过档 贸府秦具 茄促. (SQLConnect)

	// SQLDriverConnect规过
	TCHAR szConnString[1024] = { 0, };
	MakeConnString(szHostName, szDBName, szID, szPass, szConnString, sizeof(TCHAR) * 1024);

	rc = SQLDriverConnect(m_hDbc,
			NULL/*GetDesktopWindow()*/,
			szConnString,
			SQL_NTS,
			NULL,
			0,
			NULL,
			SQL_DRIVER_COMPLETE);

	if (rc == SQL_ERROR)
	{
		Release();
		return false;
	}

	m_bConnected = TRUE;

	return true;
}

void CODBC::MakeConnString(const TCHAR* szHostName, const TCHAR* szDBName,
	const TCHAR* szID, const TCHAR* szPass, TCHAR* OUT szDSN, int size)
{
	_stprintf_s(szDSN, size, _T("DRIVER={SQL Server};SERVER=%s;DATABASE=%s;UID=%s;PWD=%s"),
		szHostName, szDBName, szID, szPass);
}

void CODBC::ExecuteDirectSQL(TCHAR* szSQL)
{
	if (m_bConnected == FALSE)
		return;

	RETCODE     RetCode;
	SQLSMALLINT sNumResults;

	RetCode = SQLExecDirect(m_hStmt, szSQL, SQL_NTS);

	switch (RetCode)
	{
	case SQL_SUCCESS_WITH_INFO:
	{
		HandleDiagnosticRecord(m_hStmt, SQL_HANDLE_STMT, RetCode);
		// fall through
	}
	case SQL_SUCCESS:
	{
		// If this is a row-returning query, display
		// results
		TRYODBC(m_hStmt,
			SQL_HANDLE_STMT,
			SQLNumResultCols(m_hStmt, &sNumResults));

		if (sNumResults > 0)
		{
			DisplayResults(m_hStmt, sNumResults);
		}
		else
		{
			SQLLEN cRowCount;

			TRYODBC(m_hStmt,
				SQL_HANDLE_STMT,
				SQLRowCount(m_hStmt, &cRowCount));

			if (cRowCount >= 0)
			{
				wprintf(L"%Id %s affected\n",
					cRowCount,
					cRowCount == 1 ? L"row" : L"rows");
			}
		}
		break;
	}

	case SQL_ERROR:
	{
		HandleDiagnosticRecord(m_hStmt, SQL_HANDLE_STMT, RetCode);
		break;
	}

	default:
		fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);

	}

	TRYODBC(m_hStmt,
		SQL_HANDLE_STMT,
		SQLFreeStmt(m_hStmt, SQL_CLOSE));

	return;

	
Exit:
	Release();
	
}
