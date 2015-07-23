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
	// login server 에 connection 되었다면
	// ID,PW 입력하는 창을 띄운다.
	if (m_bConnected)
	{
		m_bAuth = true;
	}

	// 인증에 성공하면 다음 스테이지로 넘어간다.
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
