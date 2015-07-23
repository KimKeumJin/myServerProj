#pragma once
#include "Stage.h"
class CWorld :
	public CStage
{
public:
	virtual bool Init();
	virtual int UpdateFrame();
	virtual int Render();
	virtual int PacketProcess(CPacket* pPacket);

public:
	CWorld();
	virtual ~CWorld();
};

