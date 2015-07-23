#include "stdafx.h"
#include "Lock.h"


CLock::CLock()
{
	InitializeCriticalSection(&m_cs);

	//InitializeCriticalSectionAndSpinCount(&m_cs, 2000);
}


CLock::~CLock()
{
	DeleteCriticalSection(&m_cs);
}

void CLock::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CLock::Unlock()
{
	LeaveCriticalSection(&m_cs);
}


