#include "stdafx.h"
#include "MyClient.h"


CMyClient::CMyClient()
{
}


CMyClient::~CMyClient()
{
}

bool CMyClient::Init()
{
	if (CClientNetwork::Init() == false)
		return false;

	m_hConnect = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hConnectFail = CreateEvent(NULL, FALSE, FALSE, NULL);

	char szName[128] = { 0, };
	int result = gethostname(szName, sizeof(szName));

	hostent* remoteHost = gethostbyname(szName);
	if (remoteHost == NULL)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != 0)
		{
			if (dwError == WSAHOST_NOT_FOUND)
				LOG(_T("Host not found\n"));
			else if (dwError == WSANO_DATA)
				LOG(_T("No data record found\n"));
			else
				LOG(_T("Function failed with error: %ld\n"), dwError);

			return false;
		}
	}

	printf("\tOfficial name: %s\n", remoteHost->h_name);
	
	char** pAlias;
	int i = 0;
	for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++)
	{
		printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
	}

	printf("\tAddress type: ");
	switch (remoteHost->h_addrtype)
	{
	case AF_INET:
		printf("AF_INET\n");
		break;
	case AF_NETBIOS:
		printf("AF_NETBIOS\n");
		break;
	default:
		printf(" %d\n", remoteHost->h_addrtype);
		break;
	}
	printf("\tAddress length: %d\n", remoteHost->h_length);

	i = 0;
	in_addr addr;
	if (remoteHost->h_addrtype == AF_INET)
	{
		while (remoteHost->h_addr_list[i] != 0)
		{
			addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
			printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
		}
	}
	else if (remoteHost->h_addrtype == AF_NETBIOS)
	{
		printf("NETBIOS address was returned\n");
	}

	return true;

}

void CMyClient::WaitConnect()
{
	HANDLE events[2] = { m_hConnect, m_hConnectFail };

	DWORD result = WaitForMultipleObjects(2, events, FALSE, INFINITE);

	cout << "connect event : " << result << endl;
}

void CMyClient::ConnectFailed()
{
	cout << "connected to server failed (error:" << WSAGetLastError() << ")" << endl;

	SetEvent(m_hConnectFail);
}

void CMyClient::Connected()
{
	cout << "connected to server" << endl;

	SetEvent(m_hConnect);
}

void CMyClient::Disconnected()
{
	cout << "disconnected from server" << endl;
}

void CMyClient::Received()
{
	cout << "received from server" << endl;

	CClientNetwork::Received();

	// 패킷 파싱
	unsigned short pid = 0;
	unsigned short len = 0;

	char buffer[4096] = { 0, };

	while (true)
	{
		int result = ParsePacket(&pid, buffer, &len);

		if (result > 0)
		{
			LOG(_T("packet parsing complete\n"));

			CStream stream(buffer, len);

			Protocol(pid, &stream);
		}
		else if (result == 0)
		{
			LOG(_T("packet parsing incomplete\n"));
			return;
		}
		else
		{
			LOG(_T("packet parsing error\n"));
			return;
		}
	}
}

int CMyClient::ParsePacket(unsigned short* pid, char* buffer, unsigned short* len)
{
	if (m_offset < PACKET_HEADER_SIZE)
		return 0;

	PacketHeader header;

	memcpy(&header.m_dataSize, m_saveBuffer, 2);
	memcpy(&header.m_pid, m_saveBuffer + 2, 2);

	if (header.m_dataSize < 0 || header.m_dataSize > SAVEBUFFER_MAX)
		return -1;

	// uncompress
	byte uncompress_buffer[4096] = { 0, };
	uLongf uncompress_size;

	int result = uncompress(uncompress_buffer, &uncompress_size, (byte*)(m_saveBuffer + PACKET_HEADER_SIZE), header.m_dataSize);

	memcpy(buffer, uncompress_buffer, uncompress_size);
	*pid = header.m_pid;
	*len = (unsigned short)uncompress_size;

	int totalSize = PACKET_HEADER_SIZE + header.m_dataSize;


	memmove(m_saveBuffer, m_saveBuffer + totalSize, m_offset - totalSize);
	m_offset -= totalSize;

	return 1;
}

int  CMyClient::Protocol(unsigned short pid, CStream* pStream)
{
	switch (pid)
	{
		case PID_CHAT_MESSAGE:
		{
			int serial = pStream->GetInt();
			char* pStr = pStream->GetStr();
			printf("message from server : [serial:%d] %s\n", serial, pStr);
		}
		break;
	}

	return 0;
}


void CMyClient::Sended()
{
	cout << "sended to server" << endl;
}

void CMyClient::Send(unsigned short pid, char* buffer, int size)
{
	
	char _buffer[4096] = { 0, };
	
	
	// 패킷 헤더 구성 하기
	// 1. checksum

	// 2. encrypt

	// 3. compress
	byte compress_buffer[4096] = { 0, };
	uLongf compress_size;
	int result = compress(compress_buffer, &compress_size, (byte*)(buffer), size);
	

	PacketHeader header;
	header.m_dataSize = (unsigned short)compress_size;
	header.m_pid = pid;

	memcpy(_buffer, &header, PACKET_HEADER_SIZE);
	memcpy(_buffer + PACKET_HEADER_SIZE, compress_buffer, compress_size);


	CClientNetwork::Send(_buffer, compress_size + PACKET_HEADER_SIZE);

}

