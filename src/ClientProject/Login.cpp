#include "stdafx.h"
#include "Login.h"
#include "Network.h"

CLogin::CLogin()
	:m_bConnected(false),
	m_bAuth(false)
{
}


CLogin::~CLogin()
{
}

bool CLogin::Init()
{
	if (CNetwork::GetInstance().GetConnectStatus() == false)
	{
		CNetwork::GetInstance().Init();
	}

	return true;
}

int CLogin::UpdateFrame()
{
	// login server �� connection �Ǿ��ٸ�
	// ID,PW �Է��ϴ� â�� ����.
	if (m_bConnected)
	{
		m_bAuth = true;
	}

	// ������ �����ϸ� ���� ���������� �Ѿ��.
	if (m_bAuth)
		return STAGE_LOBBY;

	return STAGE_LOGIN;
}

int CLogin::Render()
{
	return 0;
}

int CLogin::PacketProcess(CPacket* pPacket)
{
	return 0;
}
