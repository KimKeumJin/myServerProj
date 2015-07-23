#pragma once
class CUser;
class CUserManager
{
private:
	std::vector<CUser*>	m_vecUser;

public:
	bool CreateUser(int nCount);
	bool SetAcceptEx(SOCKET listenSock);
	void Release();


public:
	CUserManager();
	~CUserManager();
};

