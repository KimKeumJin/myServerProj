#pragma once
#include "../NetLib/NetClient.h"

class CUser : public CNetClient
{
private:
	int		m_serial;

public:
	int ParsePacket(unsigned short* pid, char* buffer, unsigned short* len);
	int GetSerial() { return m_serial; }

public:
	bool Send(unsigned short pid, char* buffer, int size);


public:
	CUser(int i);
	virtual ~CUser();
};

