#pragma once

class CMyClient : public CClientNetwork
{
private:
	HANDLE	m_hConnect;
	HANDLE  m_hConnectFail;

public:
	void Send(unsigned short pid, char* buffer, int size);

public:
	void WaitConnect();
	int  ParsePacket(unsigned short* pid, char* buffer, unsigned short* len);
	int  Protocol(unsigned short pid, CStream* pStream);

public:
	virtual bool Init();
	virtual void ConnectFailed();
	virtual void Connected();
	virtual void Disconnected();
	virtual void Received();
	virtual void Sended();

public:
	CMyClient();
	virtual ~CMyClient();
};

