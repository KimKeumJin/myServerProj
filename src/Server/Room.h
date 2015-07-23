#pragma once

#define ROOM_NAME_MAX 30
#define ROOM_PASSWORD_MAX 30
#define ROOM_LIMIT_CNT 100

class CUser;
class CRoom
{
private:
	char				m_szName[ROOM_NAME_MAX];
	BOOL				m_isLock;
	char				m_szPassword[ROOM_PASSWORD_MAX];
	USHORT				m_limitCnt;
	std::vector<CUser*>	m_vecUser;


public:
	bool AddUser(CUser* user);
	bool RemoveUser(CUser* user);

public:
	CRoom();
	~CRoom();
};

