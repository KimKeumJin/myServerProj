#include "stdafx.h"
#include "Lobby.h"


CLobby::CLobby()
{
}


CLobby::~CLobby()
{
}

bool CLobby::Init()
{
	return true;
}

int CLobby::UpdateFrame()
{
	return STAGE_WORLD;
}

int CLobby::Render()
{
	return 0;
}

int CLobby::PacketProcess(CPacket* pPacket)
{
	return 0;
}
