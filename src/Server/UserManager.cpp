#include "stdafx.h"
#include "UserManager.h"
#include "User.h"


CUserManager::CUserManager()
{
}


CUserManager::~CUserManager()
{
	Release();
}

void CUserManager::Release()
{
	for (DWORD i = 0; i < m_vecUser.size(); ++i)
	{
		SAFE_DELETE(m_vecUser[i]);
	}

	m_vecUser.clear();
}


bool CUserManager::CreateUser(int nCount)
{
	if (nCount < 1)
		return false;

	for (int i = 0; i < nCount; ++i)
	{
		CUser* pUser = new CUser(i);

		if (pUser->CreateSocket() == false)
		{
			LOG(_T("CreateSocket Error\n"));
			continue;
		}

		m_vecUser.push_back(pUser);
	}
	
	return true;
}


bool CUserManager::SetAcceptEx(SOCKET listenSock)
{
	for (DWORD i = 0; i < m_vecUser.size(); ++i)
	{
		if (m_vecUser[i]->GetSocket() == INVALID_SOCKET)
			continue;

		if (m_vecUser[i]->SetAcceptEx(listenSock) == false)
		{
			Release();
			return false;
		}
	}

	return true;
}

