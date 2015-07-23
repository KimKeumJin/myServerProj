#pragma once
#include "Stage.h"
class CLogin :
	public CStage
{
private:
	bool		m_bConnected;
	bool		m_bAuth;

public:
	virtual bool Init();
	virtual int UpdateFrame();
	virtual int Render();
	virtual int PacketProcess(CPacket* pPacket);

public:
	CLogin();
	virtual ~CLogin();
};

