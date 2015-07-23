#include "stdafx.h"
#include "Network.h"
#include "../NetLib/Packet.h"

/*
#include <Ws2tcpip.h>
#ifdef _WIN32
const char* _inet_ntop(int af, const void* src, char* dst, socklen_t cnt)
{
	if (af == AF_INET) {

		struct sockaddr_in  in = { 0, };

		in.sin_family = AF_INET;

		::memcpy(&in.sin_addr, src, sizeof(struct in_addr));

		::getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);

		return dst;
	}
	else if (af == AF_INET6) {

		struct sockaddr_in6 in = { 0, };

		in.sin6_family = AF_INET6;

		::memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));

		::getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);

		return dst;
	}

	return NULL;
};

int _inet_pton(int af, const char* src, void* dst)
{
	struct addrinfo hints = { 0, }, *res = NULL, *old = NULL;

	hints.ai_family = af;


	if (::getaddrinfo(src, NULL, &hints, &res) != 0) {

		return -1;
	}

	old = res;

	while (res) {

		::memcpy(dst, res->ai_addr, res->ai_addrlen);

		res = res->ai_next;
	}

	::freeaddrinfo(old);

	return 0;
};
#endif
*/

unsigned WINAPI NetworkThreadCallback(LPVOID pObject)
{
	CNetwork* pNetwork = (CNetwork*)pObject;

	pNetwork->NetworkThreadCallback();

	return 0;
}

unsigned WINAPI RecvThreadCallback(LPVOID pObject)
{
	CNetwork* pNetwork = (CNetwork*)pObject;

	pNetwork->RecvThreadCallback();

	return 0;
}

unsigned WINAPI SendThreadCallback(LPVOID pObject)
{
	CNetwork* pNetwork = (CNetwork*)pObject;

	pNetwork->SendThreadCallback();

	return 0;
}


CNetwork::CNetwork()
	:m_handle(NULL),
	m_connectionTry(0),
	m_sock(NULL),
	m_bConnected(false),
	m_handleRecv(NULL),
	m_handleSend(NULL),
	m_offset(0)
{
	InitializeCriticalSection(&m_recvCS);
	InitializeCriticalSection(&m_sendCS);
}


CNetwork::~CNetwork()
{
	if (m_sock)
		closesocket(m_sock);

	DeleteCriticalSection(&m_recvCS);
	DeleteCriticalSection(&m_sendCS);

	while (!m_recvPacketQueue.empty())
	{
		SAFE_DELETE(m_recvPacketQueue.front());
		m_recvPacketQueue.pop();
	}
	
}

int CNetwork::NetworkThreadCallback()
{
	while (m_connectionTry < 3)
	{
		if (Connect("127.0.0.1", 9000) == true)
		{
			// 연결 성공시 클라이언트 로그인 팝업 창이 떠야한다.

			// recvthread
			m_handleRecv = (HANDLE)_beginthreadex(NULL, 0, ::RecvThreadCallback, this, 0, NULL);

			// sendthread
			m_handleSend = (HANDLE)_beginthreadex(NULL, 0, ::SendThreadCallback, this, 0, NULL);
			break;
		}
	}

	return 0;
}

int CNetwork::RecvThreadCallback()
{
	int recvBytes;
	CPacket packet;

	while (true)
	{
		recvBytes = recv(m_sock, m_buffer, 4096, 0);

		if (recvBytes == 0)
		{
			m_bConnected = false;
			break;
		}
			

		// 버퍼에 저장
		memcpy(m_saveBuffer + m_offset, m_buffer, recvBytes);
		m_offset += recvBytes;

		// 패킷 파싱
		if (m_offset < HEADER_SIZE)
			return false;

		int len = m_offset;
		if (len > READBUFFER_MAX)
			len = 4096;

		memcpy(packet.GetBuffer(), m_saveBuffer, len);

		int totalSize = packet.GetSize();
		if (totalSize < len)
			return false;

		memmove(m_saveBuffer, m_saveBuffer + totalSize, m_offset - totalSize);
		m_offset -= totalSize;

		CPacket* pPacket = new CPacket(packet);

		EnterCriticalSection(&m_recvCS);
		m_recvPacketQueue.push(pPacket);
		LeaveCriticalSection(&m_recvCS);

	}
	
	return 0;
}

int CNetwork::SendThreadCallback()
{
	while (true)
	{
		WaitForSingleObject(m_SendEvent, INFINITE);

		if (m_bConnected == false)
			break;

		while (m_sendPacketQueue.size() > 0)
		{
			CPacket* pPacket = m_sendPacketQueue.front();

			EnterCriticalSection(&m_sendCS);
			m_sendPacketQueue.pop();
			LeaveCriticalSection(&m_sendCS);

			char* buffer = pPacket->GetBuffer();
			int size = pPacket->GetSize();

			int sendBytes = 0;
			int sendedBytes = 0;
			int left = size;
			while (left > 0)
			{
				sendBytes = send(m_sock, buffer + sendedBytes, left, 0);
				left -= sendBytes;
				sendedBytes += sendBytes;
			}
		}
	}

	return 0;
}


bool CNetwork::Init()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET)
		return false;

	m_SendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 통신용 쓰레드 생성
	// 쓰레드 내에서 통신과 관계된 이벤트 처리
	// connect/disconnect 에 대해 이벤트 발생으로 처리한다.
	m_handle = (HANDLE)_beginthreadex(NULL, 0, ::NetworkThreadCallback, this, 0, NULL);
	
	return true;
}

bool CNetwork::Connect(const char* ipAddress, short port)
{
	++m_connectionTry;

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	//inet_pton(AF_INET, ipAddress, &addr.sin_addr.s_addr);
	addr.sin_addr.s_addr = inet_addr(ipAddress);
	addr.sin_port = htons(port);

	if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		DWORD error = WSAGetLastError();
		return false;
	}

	m_bConnected = true;

	return true;
}

bool CNetwork::GetPacket(CPacket** pPacket)
{
	if (m_recvPacketQueue.size() < 1)
		return false;

	CPacket* packet = m_recvPacketQueue.front();
	*pPacket = packet;

	EnterCriticalSection(&m_recvCS);
	m_recvPacketQueue.pop();
	LeaveCriticalSection(&m_recvCS);

	return true;
}

void CNetwork::SendPacket(CPacket* pPacket)
{
	if (m_bConnected == false)
		return;

	EnterCriticalSection(&m_sendCS);
	m_sendPacketQueue.push(pPacket);
	LeaveCriticalSection(&m_sendCS);

	SetEvent(m_SendEvent);	
}

