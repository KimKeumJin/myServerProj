#pragma once
#include "../NetLib/Packet.h"
class CStage
{
public:
	virtual bool Init() = 0;
	virtual int UpdateFrame() = 0;
	virtual int Render() = 0;
	virtual int PacketProcess(CPacket* pPacket) = 0;

public:
	CStage();
	~CStage();
};

