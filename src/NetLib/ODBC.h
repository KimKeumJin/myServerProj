#pragma once

// DB서버에 접속하여 쿼리를 처리하는 클래스
// 1.서버에서는 풀을 만들어서 사용해야 한다.
// 2.백업쿼리를 사용해야 한다.
class CQuery;
class CODBC
{
private:
	SQLHENV     m_hEnv ;
	SQLHDBC     m_hDbc;
	SQLHSTMT    m_hStmt;
	BOOL		m_bConnected;

public:
	bool Connect(const TCHAR* szHostName, const TCHAR* szDBName, const TCHAR* szID, const TCHAR* szPass);
	void Release();

	// 테스트용으로 Query를 바로 실행한다.
	void ExecuteDirectSQL(TCHAR* szSQL);

	// Query Class 를 이용하여 parameter 를 bind 하여 사용한다.
	// Stored Procedure Return Value 얻을 수 있도록 구성하자!!
	void Excute(CQuery* pQuery);
	void Fetch(CQuery* pQuery);

	

private:
	void MakeConnString(const TCHAR* szHostName, const TCHAR* szDBName,
		const TCHAR* szID, const TCHAR* szPass, TCHAR* OUT szDSN, int size);

public:
	CODBC();
	~CODBC();
};

