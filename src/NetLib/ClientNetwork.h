#pragma once

class CPacket;
class CClientNetwork
{
	friend unsigned WINAPI EventSelectThreadCallback(LPVOID pObject);

protected:
	SOCKET m_sock;
	
	HANDLE m_hSelectThread;

	HANDLE m_hSelect;
	HANDLE m_hStartup;
	HANDLE m_hDestroy;

	char   m_recvBuffer[4096];
	char   m_saveBuffer[4096 * 3];
	DWORD  m_offset;

	OVERLAPPED_EX m_overlappedex_send;


private:
	int EventSelectThreadCallback();

public:
	virtual bool Init();
	virtual void ConnectFailed() = 0;
	virtual void Connected() = 0;
	virtual void Disconnected() = 0;
	virtual void Sended() = 0;
	virtual void Received();

public:
	
	bool Connect(const char* ipAddress, short port);
	bool Disconnect();
	bool Send(char* buffer, int size);


public:
	CClientNetwork();
	~CClientNetwork();
};

