#pragma once

// DB������ �����Ͽ� ������ ó���ϴ� Ŭ����
// 1.���������� Ǯ�� ���� ����ؾ� �Ѵ�.
// 2.��������� ����ؾ� �Ѵ�.
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

	// �׽�Ʈ������ Query�� �ٷ� �����Ѵ�.
	void ExecuteDirectSQL(TCHAR* szSQL);

	// Query Class �� �̿��Ͽ� parameter �� bind �Ͽ� ����Ѵ�.
	// Stored Procedure Return Value ���� �� �ֵ��� ��������!!
	void Excute(CQuery* pQuery);
	void Fetch(CQuery* pQuery);

	

private:
	void MakeConnString(const TCHAR* szHostName, const TCHAR* szDBName,
		const TCHAR* szID, const TCHAR* szPass, TCHAR* OUT szDSN, int size);

public:
	CODBC();
	~CODBC();
};

