#include "stdafx.h"
#include "Logo.h"


CLogo::CLogo()
{
}


CLogo::~CLogo()
{
}

bool CLogo::Init()
{
	return true;
}

int CLogo::UpdateFrame()
{
	return STAGE_LOGIN;
}

int CLogo::Render()
{
	return 0;
}

int CLogo::PacketProcess(CPacket* pPacket)
{
	return 0;
}
