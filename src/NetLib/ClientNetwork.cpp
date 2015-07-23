#include "stdafx.h"
#include "ClientNetwork.h"


unsigned WINAPI EventSelectThreadCallback(LPVOID pObject)
{
	CClientNetwork* pNetwork = (CClientNetwork*)pObject;

	pNetwork->EventSelectThreadCallback();

	return 0;
}



CClientNetwork::CClientNetwork()
	:m_sock(INVALID_SOCKET),
	m_offset(0)
{
	memset(&m_overlappedex_send, 0, sizeof(m_overlappedex_send));

	m_overlappedex_send.operation = IO_SEND;
	m_overlappedex_send.object = this;
}


CClientNetwork::~CClientNetwork()
{
}


int CClientNetwork::EventSelectThreadCallback()
{
	WSANETWORKEVENTS	NetworkEvents;
	DWORD				EventID = 0;
	DWORD				Result = 0;
	HANDLE				ThreadEvents[2] = { m_hDestroy, m_hSelect };

	SetEvent(m_hStartup);

	while (TRUE)
	{
		EventID = ::WaitForMultipleObjects(2, ThreadEvents, FALSE, INFINITE);

		switch (EventID)
		{
		case WAIT_OBJECT_0:
			return 0;

		case WAIT_OBJECT_0 + 1:
			EventID = WSAEnumNetworkEvents(m_sock, m_hSelect, &NetworkEvents);

			if (EventID == SOCKET_ERROR)
			{
				return -1;
			}
			else
			{
				if (NetworkEvents.lNetworkEvents & FD_CONNECT)
				{
					if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
					{
						ConnectFailed();
					}
					else
					{
						Connected();
					}
				}
				else if (NetworkEvents.lNetworkEvents & FD_WRITE)
				{
					Sended();
				}
				else if (NetworkEvents.lNetworkEvents & FD_READ)
				{
					Received();
				}
				else if (NetworkEvents.lNetworkEvents & FD_CLOSE)
				{
					Disconnected();

					return 0;
				}
			}
			break;
		default:
			return 0;
		}
	}

	return 0;
}

bool CClientNetwork::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	m_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sock == INVALID_SOCKET)
		return false;

	m_hSelect = WSACreateEvent();
	if (m_hSelect == WSA_INVALID_EVENT)
		return false;

	int result = WSAEventSelect(m_sock, m_hSelect, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
	if (result == SOCKET_ERROR)
		return false;

	// 시작 이벤트
	m_hStartup = CreateEvent(0, FALSE, FALSE, 0);
	if (m_hStartup == NULL)
		return false;

	// 종료 이벤트
	m_hDestroy = CreateEvent(0, FALSE, FALSE, 0);
	if (m_hDestroy == NULL)
		return false;

	m_hSelectThread = (HANDLE)_beginthreadex(NULL, 0, ::EventSelectThreadCallback, this, 0, NULL);
	if (m_hSelectThread == NULL)
		return false;

	WaitForSingleObject(m_hStartup, INFINITE);

	return true;
}


bool CClientNetwork::Connect(const char* ipAddress, short port)
{
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_addr.s_addr = inet_addr(ipAddress);
	addr.sin_port = htons(port);

	if (WSAConnect(m_sock, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			return false;
	}

	return true;
}

bool CClientNetwork::Disconnect()
{
	if (m_sock)
	{
		shutdown(m_sock, SD_BOTH);
		closesocket(m_sock);
	}

	return true;
}

bool CClientNetwork::Send(char* buffer, int size)
{
	WSABUF	WsaBuf;
	DWORD	WriteBytes = 0;
	DWORD	WriteFlag = 0;


	WsaBuf.buf = (CHAR*)buffer;
	WsaBuf.len = size;

	int	result = WSASend(m_sock,
		&WsaBuf,
		1,
		&WriteBytes,
		WriteFlag,
		&m_overlappedex_send.overlapped,
		NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
		return false;

	return true;
}


void CClientNetwork::Received()
{
	DWORD recvBytes = recv(m_sock, m_recvBuffer, 4096, 0);

	memcpy(m_saveBuffer, m_recvBuffer, recvBytes);

	m_offset += recvBytes;
}

