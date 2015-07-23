#include "stdafx.h"
#include "Room.h"


CRoom::CRoom()
	:m_isLock(FALSE),
	m_limitCnt(ROOM_LIMIT_CNT)

{
	memset(m_szName, 0, sizeof(m_szName));
	memset(m_szPassword, 0, sizeof(m_szPassword));
}


CRoom::~CRoom()
{
}


bool CRoom::AddUser(CUser* user)
{
	return true;
}

bool CRoom::RemoveUser(CUser* user)
{
	return true;
}