#include "StdAfx.h"
#include "IocpServer.h"
#include "NetClient.h"
#include "Packet.h"

unsigned WINAPI IO_ThreadCallback(LPVOID pObject)
{
	CIocpServer* pIocpServer = (CIocpServer*)pObject;

	pIocpServer->IO_ThreadCallback();

	return 0;
}

CIocpServer::CIocpServer(void)
	:m_sock(NULL),
	m_hCompletionPort(NULL),
	m_isRelease(false),
	m_pfnDisconnectEx(NULL)
{
}

CIocpServer::~CIocpServer(void)
{
	Release();
}

void CIocpServer::Release()
{
	if (m_isRelease == true)
		return;

	for (DWORD i = 0; i < m_hThreads.size(); ++i)
		PostQueuedCompletionStatus(m_hCompletionPort, 0, NULL, NULL);

	for (DWORD i = 0; i < m_hThreads.size(); ++i)
	{
		WaitForSingleObject(m_hThreads[i], INFINITE);

		CloseHandle(m_hThreads[i]);
	}

	m_hThreads.clear();

	if (m_hCompletionPort)
		CloseHandle(m_hCompletionPort);

	if (m_sock)
		closesocket(m_sock);


	WSACleanup();

	m_isRelease = true;
}

int CIocpServer::IO_ThreadCallback()
{
	DWORD bytesTransferred = 0;
	VOID* CompletionKey = NULL;
	OVERLAPPED_EX* pOverlappedEx = NULL;
	BOOL ret;

	LOG(_T("IO_Thread %u - Begin\n"), GetCurrentThreadId());

	while(true)
	{
		ret = GetQueuedCompletionStatus(
			m_hCompletionPort,
			&bytesTransferred,
			(PULONG_PTR)&CompletionKey,
			(LPOVERLAPPED*)&pOverlappedEx,
			INFINITE);
		

		// PostQueuedCompletionStatus 로 종료시
		if(CompletionKey == NULL)
		{
			LOG(_T("IO_Thread %u - Exit\n"), GetCurrentThreadId());
			return 0;
		}

		LOG(_T("IO_Thread %u - GQCS returned\n"), GetCurrentThreadId());

		if (!ret || (ret && !bytesTransferred))
		{
			if (pOverlappedEx == nullptr)
			{
				OnDisconnect(CompletionKey);
				continue;
			}

			if (pOverlappedEx->operation == IO_CONNECT)
				OnConnect(pOverlappedEx->object);
			else if (pOverlappedEx->operation == IO_WAIT_REUSE)
				OnWaitReuse(pOverlappedEx->object);
			else
				OnDisconnect(pOverlappedEx->object);

			continue;
		}

		switch(pOverlappedEx->operation)
		{
		case IO_RECV:
			OnReceive(pOverlappedEx->object, bytesTransferred);
			break;
		case IO_SEND:
			OnSend(pOverlappedEx->object, bytesTransferred);
			break;
		default:
			break;
		}
	}

	return 0;
}

void CIocpServer::RegisterToIOCP( CNetClient* pNetClient )
{
	m_hCompletionPort = CreateIoCompletionPort((HANDLE)pNetClient->GetSocket(), m_hCompletionPort, (ULONG_PTR)pNetClient, 0);
}


void CIocpServer::OnConnect( void* pObject )
{
	CNetClient* pNetClient = reinterpret_cast<CNetClient*>(pObject);

	pNetClient->SetRemoteAddress();

	// 재사용된 소켓은 IOCP 에 이미 등록되어 있어서 재등록 안해도 된다.
	if(pNetClient->IsReuse() == false)
		RegisterToIOCP(pNetClient);	
	
	pNetClient->SetConnectStatus(true);

	// override
	Connected(pNetClient);

	if(pNetClient->BeginReceive() == false)
		Disconnect(pNetClient);

}

void CIocpServer::OnDisconnect( void* pObject )
{
	CNetClient* pNetClient = reinterpret_cast<CNetClient*>(pObject);

	// override
	Disconnected(pNetClient);	

	// 소켓 재사용
	//pNetClient->Release();

	//shutdown(m_sock, SD_BOTH);
	
	OVERLAPPED_EX* pOverlap = pNetClient->GetReUseOverlap();

	if (m_pfnDisconnectEx(pNetClient->GetSocket(), &pOverlap->overlapped, TF_REUSE_SOCKET, 0) == false)
	{
		DWORD err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			LOG(_T("DisconnectEx Error (errcode:%lu). so, create new socket\n"), err);

			pNetClient->Release();

			pNetClient->CreateSocket();

			pNetClient->SetAcceptEx(m_sock);
		}
	}
}

void CIocpServer::OnWaitReuse(void* pObject)
{
	CNetClient* pNetClient = reinterpret_cast<CNetClient*>(pObject);

	printf("net client socket %d is ready for reuse\n", pNetClient->GetSocket());

	pNetClient->Reset();

	pNetClient->SetAcceptEx(m_sock);
}

void CIocpServer::OnReceive( void* pObject, DWORD dwLength )
{
	CNetClient* pNetClient = reinterpret_cast<CNetClient*>(pObject);

	if(pNetClient->CopyToBuffer(dwLength) == false)
	{
		Disconnect(pNetClient);
		return;
	}

	// override
	if (Received(pNetClient, dwLength) == false)
	{
		Disconnect(pNetClient);
		return;
	}
		
	if (pNetClient->BeginReceive() == false)
	{
		Disconnect(pNetClient);
		return;
	}
}

void CIocpServer::OnSend( void* pObject, DWORD dwLength )
{
	CNetClient* pNetClient = reinterpret_cast<CNetClient*>(pObject);

	Sended(pNetClient, dwLength);
}

bool CIocpServer::LoadDisconnectEx()
{
	if (m_pfnDisconnectEx)
		return true;

	GUID guidDisconnectEx = WSAID_DISCONNECTEX;

	DWORD dwBytes = 0;

	bool ok = true;

	if (0 != WSAIoctl(
		m_sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidDisconnectEx,
		sizeof(GUID),
		&m_pfnDisconnectEx,
		sizeof(void *),
		&dwBytes,
		0,
		0))
	{
		ok = false;
	}

	return ok;
}

bool CIocpServer::Init()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		LOG(_T("WSAStartup Failed\n"));
		return false;
	}	

	return true;
}


bool CIocpServer::BeginTCPServer()
{
	LOG(_T("TCP Server Begin\n"));
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (!m_hCompletionPort)
	{
		LOG(_T("CreateIoCompletionPort Failed\n"));
		return false;
	}

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	for (DWORD i = 0; i < SystemInfo.dwNumberOfProcessors * 2 ; ++i)
	{
		HANDLE WorkerThread = (HANDLE)_beginthreadex(NULL, 0, ::IO_ThreadCallback, this, 0, NULL);
		m_hThreads.push_back(WorkerThread);
	}

	if (m_sock)
	{
		LOG(_T("Invalid Socket\n"));
		return false;
	}

	m_sock	= WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sock == INVALID_SOCKET)
	{
		LOG(_T("WSASocket Failed\n"));
		return false;
	}

	if (LoadDisconnectEx() == false)
		return false;

	//BOOL NoDelay = TRUE;
	//setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));

	//LINGER Linger;
	//Linger.l_onoff = 1;
	//Linger.l_linger = 0;
	//if (setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char*) &Linger, sizeof(LINGER)) == SOCKET_ERROR)
	//	return false;


	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_addr.sin_port = htons(9000);		// atoi(char*)

	bind(m_sock, (SOCKADDR*)&m_addr, sizeof(m_addr));
	listen(m_sock, 5);

	m_hCompletionPort = CreateIoCompletionPort((HANDLE) m_sock, m_hCompletionPort, (ULONG_PTR)this, 0);
	if(!m_hCompletionPort)
	{
		LOG(_T("CreateIoCompletionPort Failed\n"));
		return false;
	}

	return true;
}

bool CIocpServer::BeginUDPServer()
{
	return true;
}

void CIocpServer::Disconnect(CNetClient* pNetClient)
{
	PostQueuedCompletionStatus(m_hCompletionPort, 0, (ULONG_PTR)pNetClient, NULL);
}
