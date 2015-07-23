// Sample.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

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

in_addr g_privateAddr;
in_addr g_publicAddr;
map<int, clientInfo> g_addrMap;

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


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 5)
	{
		printf("usage [serial] [openport] [serverip] [serverport]");
		return -1;
	}

	int serial = _ttoi(argv[1]);
	int openport = _ttoi(argv[2]);
	int serverport = _ttoi(argv[4]);

	char ipaddress[128] = { 0, };
	int len = WideCharToMultiByte(CP_ACP, 0, argv[3], -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, argv[3], -1, ipaddress, len, NULL, NULL);

	WSAData wsadata;

	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (result != 0)
	{
		printf("wsastartup failed\n");
		return -1;
	}

	if (PrintLocal(&g_privateAddr) == false)
		return -1;

	SOCKADDR_IN myAddr;
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons(openport);

	///////////////////////////////////////////////////////
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ipaddress);
	serverAddr.sin_port = htons(serverport);

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("create socket failed\n");
		return -1;
	}

	bind(sock, (SOCKADDR*)&myAddr, sizeof(myAddr));

	char recvbuffer[4096] = { 0, };
	char sendbuffer[4096] = { 0, };

	int addrlen = sizeof(SOCKADDR_IN);
	int recvBytes;
	int sendBytes;

	int pid;
	int size;
	char message[1024] = { 0, };
	while (true)
	{
		cout << endl;
		cout << "1. confirm public ip" << endl;
		cout << "2. group and update private ip" << endl;
		cout << "3. request other ip infomation" << endl;
		cout << "4. udp holepunching" << endl;
		cout << "5. exit" << endl;

		cin >> message;

		if (strcmp(message, "1") == 0)
		{
			pid = 0;
			size = 8;
			memcpy(sendbuffer, &serial, 4);
			memcpy(sendbuffer + 4, &pid, 4);

			sendBytes = sendto(sock, sendbuffer, size, 0, (SOCKADDR*)&serverAddr, addrlen);
			printf("send to server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), sendBytes);


			recvBytes = recvfrom(sock, recvbuffer, 4096, 0, NULL, NULL);
			printf("recv from server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvBytes);


			SOCKADDR_IN publicAddr;
			memcpy(&publicAddr, recvbuffer, sizeof(SOCKADDR_IN));

			g_publicAddr = publicAddr.sin_addr;
			printf("public ip:%s\n", inet_ntoa(g_publicAddr));

		}
		else if (strcmp(message, "2") == 0)
		{
			pid = 1;
			size = 8 + sizeof(in_addr) + 4;
			memcpy(sendbuffer, &serial, 4);
			memcpy(sendbuffer + 4, &pid, 4);
			memcpy(sendbuffer + 8, &g_privateAddr, sizeof(in_addr));
			memcpy(sendbuffer + 8 + sizeof(in_addr), &openport, 4);

			sendBytes = sendto(sock, sendbuffer, size, 0, (SOCKADDR*)&serverAddr, addrlen);
			printf("send to server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), sendBytes);


			recvBytes = recvfrom(sock, recvbuffer, 4096, 0, NULL, NULL);
			printf("recv from server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvBytes);

			int result;
			memcpy(&result, recvbuffer, 4);

			if (result == 1)
			{
				printf("insert complete");
			}
			else if (result == 2)
			{
				printf("update complete");
			}
			else
			{
				printf("unknown result %d", result);
			}
			

		}
		else if (strcmp(message, "3") == 0)
		{
			pid = 2;
			memcpy(sendbuffer, &serial, 4);
			memcpy(sendbuffer + 4, &pid, 4);

			sendBytes = sendto(sock, sendbuffer, size, 0, (SOCKADDR*)&serverAddr, addrlen);
			printf("send to server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), sendBytes);

			recvBytes = recvfrom(sock, recvbuffer, 4096, 0, NULL, NULL);
			printf("recv from server(%s:%d) %d bytes\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvBytes);

			int count;
			memcpy(&count, recvbuffer, 4);

			if (count > 0)
			{
				printf("<group client info list begin>\n");

				int offset;
				map<int, clientInfo>::iterator iter;
				for (int i = 0; i < count; ++i)
				{
					offset = sizeof(clientInfo) * i + 4;

					clientInfo info;
					memcpy(&info, recvbuffer + offset, sizeof(clientInfo));

					char addr1[128] = { 0, };
					char addr2[128] = { 0, };
					char* szAddr;
					szAddr = inet_ntoa(info.privateIP);
					strcpy_s(addr1, szAddr);

					szAddr = inet_ntoa(info.publicIP);
					strcpy_s(addr2, szAddr);

					printf("serial:%d - private:%s, public:%s, openport:%d\n",
						info.serial,
						addr1,
						addr2,
						info.publicPort);

					iter = g_addrMap.find(info.serial);
					if (iter == g_addrMap.end())
					{
						g_addrMap.insert(make_pair(info.serial, info));
					}
					else
					{
						iter->second = info;
					}					
				}

				printf("<group client info list end>\n");
			}
		}
		else if (strcmp(message, "4") == 0)
		{
			// privateIP == publicIP 이면,
			// 1)랑데뷰 서버와 같은 NAT 이거나,
			// 2)privateIP 도 publicIP 를 사용하고 있다. (ex. twinip)

			// 1. privateIP 로 전송 -> ack 과 오지 않으면 publicIP 로 전환

			// 2. publicIP 로 전송 -> 일정 시간 내 ack 가 오지 않으면 재전송(3~5회)

			// 3. relay 서버를 통한 전송 시작



		}
		else if (strcmp(message, "5") == 0)
		{
			break;
		}
		else
		{			
			continue;
		}
	}


	WSACleanup();

	return 0;
}

