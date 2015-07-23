#pragma once
#include "Stage.h"
class CLogo :
	public CStage
{
public:
	virtual bool Init();
	virtual int UpdateFrame();
	virtual int Render();
	virtual int PacketProcess(CPacket* pPacket);

public:
	CLogo();
	virtual ~CLogo();
};

