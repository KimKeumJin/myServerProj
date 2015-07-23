#pragma once

class CLock
{
private:
	CRITICAL_SECTION	m_cs;

public:
	void Lock();
	void Unlock();

public:
	CLock();
	virtual ~CLock();
};

class CGuard
{
private:
	CLock*	m_pLock;

public:
	CGuard(CLock* lock)
		:m_pLock(lock)
	{
		lock->Lock();
		
	}
	~CGuard()
	{
		m_pLock->Unlock();
	}
};

