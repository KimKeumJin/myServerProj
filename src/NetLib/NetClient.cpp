#include "StdAfx.h"
#include "NetClient.h"
#include "Packet.h"
#include "CommonFunc.h"


CNetClient::CNetClient(void)
	:m_sock(INVALID_SOCKET),
	m_port(0),
	m_bConnected(false),
	m_offset(0),
	m_isReuse(false)
{
	memset(m_szAddr, 0, sizeof(m_szAddr));

	memset(&m_overlappedex_conn, 0, sizeof(m_overlappedex_conn));
	memset(&m_overlappedex_recv, 0, sizeof(m_overlappedex_recv));
	memset(&m_overlappedex_send, 0, sizeof(m_overlappedex_send));
	memset(&m_overlappedex_reconn, 0, sizeof(m_overlappedex_reconn));

	m_overlappedex_conn.operation = IO_CONNECT;
	m_overlappedex_conn.object = this;

	m_overlappedex_recv.operation = IO_RECV;
	m_overlappedex_recv.object = this;

	m_overlappedex_send.operation = IO_SEND;
	m_overlappedex_send.object = this;

	m_overlappedex_reconn.operation = IO_WAIT_REUSE;
	m_overlappedex_reconn.object = this;
}


CNetClient::~CNetClient(void)
{
	Release();
}

void CNetClient::Release()
{
	if (m_sock != INVALID_SOCKET)
	{
		shutdown(m_sock, SD_BOTH);
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

	Reset();
	
}

void CNetClient::Reset()
{
	memset(m_szAddr, 0, sizeof(m_szAddr));

	memset(&m_overlappedex_conn, 0, sizeof(m_overlappedex_conn));
	memset(&m_overlappedex_recv, 0, sizeof(m_overlappedex_recv));
	memset(&m_overlappedex_send, 0, sizeof(m_overlappedex_send));

	m_overlappedex_conn.operation = IO_CONNECT;
	m_overlappedex_conn.object = this;

	m_overlappedex_recv.operation = IO_RECV;
	m_overlappedex_recv.object = this;

	m_overlappedex_send.operation = IO_SEND;
	m_overlappedex_send.object = this;

	m_port = 0;
	m_offset = 0;
	m_bConnected = false;

	m_isReuse = true;
}

bool CNetClient::CreateSocket()
{
	m_sock	= WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sock == INVALID_SOCKET)
		return false;

	return true;
}

bool CNetClient::SetAcceptEx( SOCKET listenSock )
{
	//m_sock	= WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//if (m_sock == INVALID_SOCKET)
	//	return false;

	//BOOL NoDelay = TRUE;
	//setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&NoDelay, sizeof(NoDelay));

	if (!AcceptEx(listenSock, 
		m_sock, 
		m_readBuffer, 
		0, 
		sizeof(sockaddr_in) + 16, 
		sizeof(sockaddr_in) + 16, 
		NULL, 
		&m_overlappedex_conn.overlapped))
	{
		DWORD error = WSAGetLastError();
		if (error != ERROR_IO_PENDING && error != WSAEWOULDBLOCK)
			return false;
	}

	return true;
}

bool CNetClient::BeginReceive()
{
	WSABUF	WsaBuf;
	DWORD	ReadBytes	= 0;
	DWORD	ReadFlag	= 0;

	WsaBuf.buf = m_readBuffer;
	WsaBuf.len = sizeof(m_readBuffer);

	int	result = WSARecv(m_sock,
		&WsaBuf,
		1,
		&ReadBytes,
		&ReadFlag,
		&m_overlappedex_recv.overlapped,
		NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		return false;
	}

	return true;
}


bool CNetClient::CopyToBuffer(DWORD dwLength)
{
	if (m_offset + dwLength > SAVEBUFFER_MAX)
		return false;

	memcpy(m_saveBuffer + m_offset, m_readBuffer, dwLength);

	m_offset += dwLength;

	return true;
}


void CNetClient::SetRemoteAddress()
{
	sockaddr_in		*Local = NULL;
	INT				LocalLength = 0;

	sockaddr_in		*Remote = NULL;
	INT				RemoteLength = 0;

	GetAcceptExSockaddrs(m_readBuffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr **)&Local,
		&LocalLength,
		(sockaddr **)&Remote,
		&RemoteLength);

	CHAR	TempRemoteAddress[32] = { 0, };
	strcpy_s(TempRemoteAddress, 32, inet_ntoa(Remote->sin_addr));

	TCHAR szAddr[32] = { 0, };

	MultiByteToWideChar(CP_ACP,
		0,
		TempRemoteAddress,
		-1,
		m_szAddr,
		32);

	//_tcscpy_s(m_szAddr, 32, szAddr);

	m_uAddr = Remote->sin_addr.S_un.S_addr;
	m_port = ntohs(Remote->sin_port);
}

bool CNetClient::Send(char* buffer, int size)
{
	WSABUF	WsaBuf;
	DWORD	WriteBytes = 0;
	DWORD	WriteFlag = 0;

	WsaBuf.buf = buffer;
	WsaBuf.len = size;

	int	result = WSASend(m_sock,
		&WsaBuf,
		1,
		&WriteBytes,
		WriteFlag,
		&m_overlappedex_send.overlapped,
		NULL);

	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		return false;
	}

	return true;
}

