#include "stdafx.h"
#include "StageManager.h"
#include "Logo.h"
#include "Login.h"
#include "Lobby.h"
#include "World.h"
#include "Network.h"


CStageManager::CStageManager()
	:m_iStage(STAGE_NONE),
	m_iNextStage(STAGE_NONE),
	m_pStage(NULL)
{
}


CStageManager::~CStageManager()
{
	SAFE_DELETE(m_pStage);
}

bool CStageManager::Init(int iStage)
{
	return ChangeStage(iStage, true);
}

bool CStageManager::ChangeStage(int iStage, bool bInit)
{
	if (m_iStage == iStage)
		return false;

	if (iStage < STAGE_BEGIN || iStage > STAGE_END)
		return false;
	
	m_iStage = iStage;
	if (bInit)
		m_iNextStage = m_iStage;

	SAFE_DELETE(m_pStage);

	switch (iStage)
	{
	case STAGE_LOGO:
		m_pStage = new CLogo;
		break;
	case STAGE_LOGIN:
		m_pStage = new CLogin;
		break;
	case STAGE_LOBBY:
		m_pStage = new CLobby;
		break;
	case STAGE_WORLD:
		m_pStage = new CWorld;
		break;
	default:
		return false;
	}

	m_pStage->Init();

	return true;
}

int CStageManager::UpdateFrame()
{
	if (m_iStage != m_iNextStage)
		ChangeStage(m_iNextStage);

	CPacket* pPacket = NULL;
	while (CNetwork::GetInstance().GetPacket(&pPacket))
	{
		m_pStage->PacketProcess(pPacket);
	}
	

	m_iNextStage = m_pStage->UpdateFrame();

	return 0;
}

int CStageManager::Render()
{
	m_pStage->Render();
	return 0;
}