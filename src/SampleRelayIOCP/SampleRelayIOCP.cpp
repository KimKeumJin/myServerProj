// SampleRelayIOCP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUF_SIZE 1024
#define READ  3
#define WRITE  5
#define MAXNUMCPU 16

typedef struct // ���� ����
{
	SOCKET hServSock;
	SOCKADDR_IN clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;
typedef struct // ���� ���� overlapped����ü�� ������ �;���.
{
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	char buffer[BUF_SIZE];
	int rwMode; // �б�(3)�ΰ� ����(5)�ΰ�
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

	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// CPU�ھ����ŭ�� �����尹������ ���ٰ����� IOCP�� ����.
	GetSystemInfo(&sysInfo);
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, sysInfo.dwNumberOfProcessors);

	// ���������� �ͺ�ŷ UDP �������� ����.
	hServSock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// �������Ͽ� �ּ��Ҵ�.
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi("1670"));    // ��Ʈ��ȣ 1670.
	bind(hServSock, (SOCKADDR *)&servAdr, sizeof(servAdr));

	int addrLen = sizeof(SOCKADDR_IN);

	// ������Ĺ�� IOCP�� �����ϰ� IO�����忡�� ����� �ڵ������� �μ��� �ѱ�.
	CreateIoCompletionPort((HANDLE)hServSock, hComPort, NULL, 0);

	// CPU�ھ�� ��ŭ ��Ŀ������ ����.
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++)
		hWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

	// wsaBuf�� IO�����忡�� �ʿ��� rwMode�� ��Ʈ���� WSARecv�� �����Ͽ� ����Ŭ���̾�Ʈ�� ���ʵ����͸� ����.
	ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
	ioInfo->handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
	ioInfo->handleInfo->hServSock = hServSock;
	ioInfo->wsaBuf.len = BUF_SIZE;
	ioInfo->wsaBuf.buf = ioInfo->buffer;
	ioInfo->rwMode = READ;

	// ���ʸ޼����� �ͺ��ŷ ���·� ���.
	printf("first recv...\n");
	WSARecvFrom(ioInfo->handleInfo->hServSock, &(ioInfo->wsaBuf), 1, (LPDWORD)&recvBytes,
		(LPDWORD)&flags, (sockaddr *)&ioInfo->handleInfo->clntAdr, (LPINT)&ioInfo->wsaBuf.len, &(ioInfo->overlapped), NULL);

	// ��Ŀ��������� ��� ���������� ���ν������ ���.
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
		// WSARecv/WSASend�Լ�ȣ���� IO����, ���� ť�� �۽� Ȥ�� ������ �Ϸ�� ���� ������ �Ʒ� �Լ��� ��ȯ�ȴ�. ���������� ���.
		// �Ʒ��Լ��� 3��° �μ��� ���η������� CreateIoCompletionPort�Լ�ȣ��� 3��° �μ��� �־��� ������,
		// Ŭ���̾�Ʈ������ ������ ����ü�̸� 4��° �μ��� WSARecv/WSASend�Լ�ȣ��� 6��°�μ��� �� IO�����̴�.
		// �� 3, 4��° �μ��� IO���ť�� ����Ǿ� �ִ� �����͵��� �ҷ����� ���̴�.
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
		
			// EOF�� �޾�����, ������ �ݰ� ��������ü���� �Ҵ������Ŀ� ��Ŀ�����带 ����.
			if (bytesTrans == 0)
			{
				printf("num of transmitted bytes : 0\n");
				closesocket(sock);
				free(handleInfo);
				free(ioInfo);
				break;
			}
			
			// ���� �޼����� �ٽ� Ŭ���̾�Ʈ���� ����.
			// �ְ�޴� �޼����� ª���޼����̹Ƿ� ������ IO�Ϸ���°� IO�Ϸ� ���ť�� ������ϵ�.
			printf("send echo message...\n");
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;
			int bytesSent;
			WSASendTo(sock, &(ioInfo->wsaBuf), 1, (LPDWORD)&bytesSent, 0, (sockaddr *)&ioInfo->handleInfo->clntAdr,
				sizeof(ioInfo->handleInfo->clntAdr), &(ioInfo->overlapped), NULL);

			// �ͺ�ŷ���·� �޼��� ������ ���.
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
			// ioInfo->rwMode�� WRITE�̸� �޼��������� �˸��� io������ �Ҵ�������.
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

