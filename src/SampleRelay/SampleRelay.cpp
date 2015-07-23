// SampleRelay.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;

bool PrintLocal(in_addr* paddr)
{
	char szName[128] = { 0, };
	int result = gethostname(szName, sizeof(szName));
	if (result < 0)
	{
		printf("gethostname failed\n");
		return false;
	}

	hostent* remoteHost = gethostbyname(szName);
	if (remoteHost == NULL)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != 0)
		{
			if (dwError == WSAHOST_NOT_FOUND)
				printf("Host not found\n");
			else if (dwError == WSANO_DATA)
				printf("No data record found\n");
			else
				printf("Function failed with error: %ld\n", dwError);

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

			if (i == 1)
				*paddr = addr;
		}
	}
	else if (remoteHost->h_addrtype == AF_NETBIOS)
	{
		printf("NETBIOS address was returned\n");
	}

	return true;
}

in_addr g_local;

#pragma pack(push, 1)  
struct clientInfo
{
	int serial;

	in_addr privateIP;
	short privatePort;

	in_addr publicIP;
	short publicPort;

};
#pragma pack(pop)

map<int, clientInfo> addrmap;

int _tmain(int argc, _TCHAR* argv[])
{
	WSAData wsadata;

	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (result != 0)
	{
		printf("wsastartup failed\n");
		return -1;
	}

	if (PrintLocal(&g_local) == false)
		return -1;

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9000);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("create socket failed\n");
		return -1;
	}

	//getsockopt(SO_MAX_MSG_SIZE, )

	if (bind(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		printf("bind socket failed\n");
		return -1;
	}

	printf("relay server(%s:%d) start\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	char recvbuffer[1024] = { 0, };
	char sendbuffer[1024] = { 0, };

	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR_IN);
	int recvBytes;
	int sendBytes;
	int serial;
	int pid;

	while (true)
	{
		recvBytes = recvfrom(sock, recvbuffer, 1024, 0, (SOCKADDR*)&clientAddr, &len);

		if (recvBytes < 8)
		{
			printf("recv bytes is too short\n");
			continue;
		}

		memcpy(&serial, recvbuffer, sizeof(int));
		memcpy(&pid, recvbuffer + 4, sizeof(int));

		printf("recv from client(%s:%d) %d bytes - serial:%d pid:%d\n",
			inet_ntoa(clientAddr.sin_addr),
			ntohs(clientAddr.sin_port),
			recvBytes,
			serial,
			pid);

		switch (pid)
		{

		case 0:		// public ip È®ÀÎ
		{

			memcpy(sendbuffer, &clientAddr, sizeof(SOCKADDR_IN));
			sendBytes = sendto(sock, sendbuffer, sizeof(SOCKADDR_IN), 0, (SOCKADDR*)&clientAddr, len);
		}
		break;

		case 1:		// group and update private ip
		{
			in_addr privateAddr;
			int openPort;

			memcpy(&privateAddr, recvbuffer + 8, sizeof(in_addr));
			memcpy(&openPort, recvbuffer + 8 + sizeof(in_addr), 4);

			clientInfo info;
			info.serial = serial;
			info.privateIP = privateAddr;
			info.privatePort = openPort;
			info.publicIP = clientAddr.sin_addr;
			info.publicPort = openPort;


			char addr1[128] = { 0, };
			char addr2[128] = { 0, };
			char* szAddr;
			szAddr = inet_ntoa(info.privateIP);
			strcpy_s(addr1, szAddr);

			szAddr = inet_ntoa(info.publicIP);
			strcpy_s(addr2, szAddr);

			printf("update!! serial:%d, private:%s, public:%s, openport:%d\n",
				info.serial,
				addr1,
				addr2,
				info.publicPort);

			int result;
			map<int, clientInfo>::iterator iter = addrmap.find(serial);
			if (iter == addrmap.end())
			{
				addrmap.insert(make_pair(serial, info));
				result = 1;	// insert
			}
			else
			{
				iter->second = info;
				result = 2;	// update
			}

			memcpy(sendbuffer, &result, 4);
			sendBytes = sendto(sock, sendbuffer, 4, 0, (SOCKADDR*)&clientAddr, len);

			break;
		}

		case 2:		// request other ip infomation
		{
			int offset = 4;
			int count = 0;
			map<int, clientInfo>::iterator iter;
			for (iter = addrmap.begin(); iter != addrmap.end(); ++iter)
			{
				if (iter->first == serial)
					continue;

				clientInfo& info = iter->second;

				memcpy(sendbuffer + offset, &info, sizeof(clientInfo));
				offset += sizeof(clientInfo);

				++count;
			}

			memcpy(sendbuffer, &count, 4);
			sendBytes = sendto(sock, sendbuffer, sizeof(clientInfo) * count + 4, 0, (SOCKADDR*)&clientAddr, len);

			/*
			char* message = recvbuffer + 8 + sizeof(SOCKADDR_IN);
			int length = strlen(message);

			memcpy(sendbuffer, &serial, 4);
			memcpy(sendbuffer + 4, &pid, 4);
			memcpy(sendbuffer + 8, message, length + 1);
			sendBytes = sendto(sock, sendbuffer, 9 + length, 0, (SOCKADDR*)&clientAddr, len);
			*/

			break;
		}

		}

	}


	WSACleanup();

	return 0;
}

