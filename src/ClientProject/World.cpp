#include "stdafx.h"
#include "World.h"


CWorld::CWorld()
{
}


CWorld::~CWorld()
{
}


bool CWorld::Init()
{
	return true;
}

int CWorld::UpdateFrame()
{
	return STAGE_WORLD;
}

int CWorld::Render()
{
	return 0;
}

int CWorld::PacketProcess(CPacket* pPacket)
{
	return 0;
}
