#pragma once
#include "Stage.h"
class CLobby :
	public CStage
{
public:
	virtual bool Init();
	virtual int UpdateFrame();
	virtual int Render();
	virtual int PacketProcess(CPacket* pPacket);

public:
	CLobby();
	virtual ~CLobby();
};

