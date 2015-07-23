// SampleRelayIOCP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUF_SIZE 1024
#define READ  3
#define WRITE  5
#define MAXNUMCPU 16

typedef struct // 소켓 정보
{
	SOCKET hServSock;
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;
typedef struct // 버퍼 정보 overlapped구조체가 맨위에 와야함.
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode; // 읽기(3)인가 쓰기(5)인가
	LPPER_HANDLE_DATA handleInfo;
} PER_IO_DATA, *LPPER_IO_DATA;

unsigned int WINAPI EchoThreadMain(void *CompletionPortIO);
void ErrorHandling(char *message);

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	HANDLE hComPort;
	HANDLE hWorkerThread[MAXNUMCPU];
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	SOCKET hServSock;
	SOCKADDR_IN servAdr;
	int recvBytes, flags = 0;
	DWORD i;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// CPU코어수만큼의 스레드갯수에서 접근가능한 IOCP를 생성.
	GetSystemInfo(&sysInfo);
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, sysInfo.dwNumberOfProcessors);

	// 서버소켓을 넌블러킹 UDP 소켓으로 생성.
	hServSock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// 서버소켓에 주소할당.
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi("1670"));    // 포트번호 1670.
	bind(hServSock, (SOCKADDR *)&servAdr, sizeof(servAdr));

	int addrLen = sizeof(SOCKADDR_IN);

	// 서버소캣과 IOCP를 연결하고 IO쓰레드에서 사용할 핸들정보를 인수로 넘김.
	CreateIoCompletionPort((HANDLE)hServSock, hComPort, NULL, 0);

	// CPU코어수 만큼 워커스레드 시작.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		hWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// wsaBuf와 IO쓰레드에서 필요한 rwMode를 세트한후 WSARecv를 수행하여 개별클라이언트의 최초데이터를 수신.
	ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
	ioInfo->handleInfo->hServSock = hServSock;
	ioInfo->wsaBuf.len = BUF_SIZE;
	ioInfo->wsaBuf.buf = ioInfo->buffer;
	ioInfo->rwMode = READ;

	// 최초메세지를 넌블로킹 상태로 대기.
	printf("first recv...\n");
	WSARecvFrom(ioInfo->handleInfo->hServSock, &(ioInfo->wsaBuf), 1, (LPDWORD)&recvBytes,
		(LPDWORD)&flags, (sockaddr *)&ioInfo->handleInfo->clntAdr, (LPINT)&ioInfo->wsaBuf.len, &(ioInfo->overlapped), NULL);

	// 워커스레드들이 모두 끝날때까지 메인스레드는 대기.
	WaitForMultipleObjects(sysInfo.dwNumberOfProcessors, hWorkerThread, TRUE, INFINITE);
	system("PAUSE");
	return 0;
}

unsigned int WINAPI EchoThreadMain(void *CompletionPortIO)
{
	HANDLE hComPort = (HANDLE)CompletionPortIO;
	SOCKET sock;
	SOCKADDR_IN clntAdr;
	DWORD bytesTrans;
	LPPER_HANDLE_DATA handleInfo;
	LPPER_IO_DATA ioInfo;
	while (1)
	{
		// WSARecv/WSASend함수호출후 IO전송, 내부 큐에 송신 혹은 수신이 완료된 것이 있으면 아래 함수가 반환된다. 그전까지는 대기.
		// 아래함수의 3번째 인수는 메인루프에서 CreateIoCompletionPort함수호출시 3번째 인수로 주었던 것으로,
		// 클라이언트정보를 가지는 구조체이며 4번째 인수는 WSARecv/WSASend함수호출시 6번째인수로 준 IO정보이다.
		// 이 3, 4번째 인수는 IO대기큐에 저장되어 있던 포인터들을 불러오는 것이다.
		printf("get queued completion status...\n");
		if (GetQueuedCompletionStatus(hComPort, &bytesTrans, (PULONG_PTR)&handleInfo, (LPOVERLAPPED *)&ioInfo, INFINITE) == FALSE)
		{
			printf("break.\n");
			break;
		}

		sock = ioInfo->handleInfo->hServSock;
		memcpy(&clntAdr, &ioInfo->handleInfo->clntAdr, sizeof(SOCKADDR_IN));
		if (ioInfo->rwMode == READ)
		{
			ioInfo->buffer[bytesTrans - 1] = 0;
			printf("message [%s] received!\n", ioInfo->buffer);
		
			// EOF를 받았으면, 소켓을 닫고 정보구조체들을 할당해제후에 워커스레드를 종료.
			if (bytesTrans == 0)
			{
				printf("num of transmitted bytes : 0\n");
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				break;
			}
			
			// 받은 메세지를 다시 클라이언트에게 보냄.
			// 주고받는 메세지가 짧은메세지이므로 보내는 IO완료상태가 IO완료 대기큐에 먼저등록됨.
			printf("send echo message...\n");
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			int bytesSent;
			WSASendTo(sock, &(ioInfo->wsaBuf), 1, (LPDWORD)&bytesSent, 0, (sockaddr *)&ioInfo->handleInfo->clntAdr,
				sizeof(ioInfo->handleInfo->clntAdr), &(ioInfo->overlapped), NULL);

			// 넌블러킹상태로 메세지 수신을 대기.
			printf("recv...\n");
			ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			ioInfo->handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
			ioInfo->handleInfo->hServSock = sock;
			int bytesRecv, flags = 0;
			if (WSARecvFrom(sock, &(ioInfo->wsaBuf), 1, (LPDWORD)&bytesRecv, (LPDWORD)&flags,
				(sockaddr *)&ioInfo->handleInfo->clntAdr, (LPINT)&ioInfo->wsaBuf.len,
				&(ioInfo->overlapped), NULL) == SOCKET_ERROR) {
				int error;
				if ((error = WSAGetLastError()) == WSA_IO_PENDING) {
					printf("recv io pending.\n");
				}
				else {
					printf("recv error.\n");
					if (error == WSAEFAULT)
						printf("WSAEFAULT\n");
					break;
				}
			}
		}
		else
		{
			// ioInfo->rwMode가 WRITE이면 메세지보냄을 알리고 io정보를 할당해제함.
			puts("message sent!");
			free(ioInfo->handleInfo);
			free(ioInfo);
		}
	}
	return 0;
}
void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	system("PAUSE");
	exit(1);
}

