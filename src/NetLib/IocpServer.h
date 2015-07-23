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
	// - 라이브러리화 할 때 필요한 사항
	1. IOCP 를 부모 클래스로 사용할 때 정리할 가상함수
	2. AcceptEx 메모리 풀 처리 (SessionManager)
	3. Queing 동기화 버전 vs 비동기 버전
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

