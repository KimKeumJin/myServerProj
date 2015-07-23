#pragma once
#include "Lock.h"

class CNetClient;
class CSessionManager;
class CPacket;
class CIocpServer
{
	friend unsigned WINAPI IO_ThreadCallback(LPVOID pObject);

private:
	SOCKET					m_sock;
	SOCKADDR_IN				m_addr;
	HANDLE					m_hCompletionPort;
	std::vector<HANDLE>		m_hThreads;
	bool					m_isRelease;

	LPFN_DISCONNECTEX		m_pfnDisconnectEx;

private:
	bool LoadDisconnectEx();
	int  IO_ThreadCallback();
	void RegisterToIOCP(CNetClient* pNetClient);
	void OnConnect(void* pObject);
	void OnDisconnect(void* pObject);
	void OnReceive(void* pObject, DWORD dwLength);
	void OnSend(void* pObject, DWORD dwLength);
	void OnWaitReuse(void* pObject);

	/* 
	// - ���̺귯��ȭ �� �� �ʿ��� ����
	1. IOCP �� �θ� Ŭ������ ����� �� ������ �����Լ�
	2. AcceptEx �޸� Ǯ ó�� (SessionManager)
	3. Queing ����ȭ ���� vs �񵿱� ����
	*/

public:
	const SOCKET& GetListenSock() const { return m_sock; }
	void Disconnect(CNetClient* pNetClient);
	

public:
	virtual bool Init();
	virtual void Release();
	virtual bool BeginTCPServer();
	virtual bool BeginUDPServer();
	virtual void Connected(void* pObject) = 0;
	virtual void Disconnected(void* pObject) = 0;
	virtual bool Received(void* pObject, DWORD dwLength) = 0;
	virtual void Sended(void* pObject, DWORD dwLength) = 0;

public:


public:
	CIocpServer(void);
	virtual ~CIocpServer(void);
};

