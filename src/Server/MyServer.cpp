#include "StdAfx.h"
#include "MyServer.h"
#include "UserManager.h"
#include "../NetLib/Packet.h"
#include "User.h"


CMyServer::CMyServer(void)
	:m_pUserManager(NULL),
	m_isUsingQueue(true),
	m_isExit(false)
{
	m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hExitEvent == NULL)
	{
		throw GetLastError();
	}
}


CMyServer::~CMyServer(void)
{
	m_isExit = true;

	// iocp io_thread 를 모두 종료
	CIocpServer::Release();

	SAFE_DELETE(m_pUserManager);
}

bool CMyServer::Init()
{
	if (CIocpServer::Init() == false)
		return false;

	return true;
}

bool CMyServer::BeginTCPServer()
{
	if (CIocpServer::BeginTCPServer() == false)
		return false;

	m_pUserManager = new CUserManager;
	m_pUserManager->CreateUser(100);
	m_pUserManager->SetAcceptEx(GetListenSock());

	return true;
}

bool CMyServer::BeginUDPServer()
{
	if (CIocpServer::BeginUDPServer() == false)
		return false;

	return true;
}

void CMyServer::Connected(void* pObject)
{
	CUser* pUser = reinterpret_cast<CUser*>(pObject);

	if (m_isUsingQueue)
	{
		CPacket* pPacket = new CPacket(0);
		pPacket->SetPID(PID_NET_CONNECT);
		pPacket->SetSerial(pUser->GetSerial());
		pPacket->SetNetPtr(pUser);

		AddPacket(pPacket);
		
	}
	else
	{
		AddUser(pUser->GetSerial(), pUser);

		LOG(_T("Client(serial:%d) Connected - %s:%d\n"), pUser->GetSerial(), pUser->GetAddr(), pUser->GetPort());

	}
	
}

void CMyServer::Disconnected(void* pObject)
{
	CUser* pUser = reinterpret_cast<CUser*>(pObject);

	if (m_isUsingQueue)
	{
		CPacket* pPacket = new CPacket(6);
		pPacket->SetPID(PID_NET_DISCONNECT);
		pPacket->SetSerial(pUser->GetSerial());
		pPacket->SetNetPtr(pUser);

		char buf[6] = { 0, };
		memcpy(buf, &pUser->GetUAddr(), sizeof(ULONG));
		memcpy(buf + 4, &pUser->GetPort(), sizeof(USHORT));
		pPacket->SetData(buf, 6);

		AddPacket(pPacket);

	}
	else
	{
		DelUser(pUser->GetSerial());

		LOG(_T("Client(serial:%d) Disconnected - %s:%d\n"), pUser->GetSerial(), pUser->GetAddr(), pUser->GetPort());
	}
	
}

bool CMyServer::Received(void* pObject, DWORD dwLength)
{

	CUser* pUser = reinterpret_cast<CUser*>(pObject);

	LOG(_T("Client(serial:%d) Received - %s:%d\n"), pUser->GetSerial(), pUser->GetAddr(), pUser->GetPort());

	unsigned short pid = 0;
	unsigned short len = 0;

	char buffer[4096] = { 0, };

	if (m_isUsingQueue)
	{
		// 1. Server Queue 에 Euque 후, Main Thread 에서 패킷 처리
		while (true)
		{
			int result = pUser->ParsePacket(&pid, buffer, &len);

			if (result > 0)
			{
				LOG(_T("packet parsing complete - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());

				CPacket* pPacket = new CPacket(len);
				pPacket->SetPID(pid);
				pPacket->SetSerial(pUser->GetSerial());
				pPacket->SetNetPtr(pUser);
				pPacket->SetData(buffer, len);

				AddPacket(pPacket);
			}
			else if (result == 0)
			{
				LOG(_T("packet parsing incomplete - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());
				return true;
			}
			else
			{
				LOG(_T("packet parsing error - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());
				return false;
			}
		}

	}
	else
	{
		// 2. IO_Thread 에서 바로 Protocol 처리 호출
		while (true)
		{
			int result = pUser->ParsePacket(&pid, buffer, &len);

			if (result > 0)
			{
				LOG(_T("packet parsing complete - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());

				//Protocol(pUser, pid, buffer, len);

				CStream stream(buffer, len);

				Protocol(pUser, pid, &stream);
			}
			else if (result == 0)
			{
				LOG(_T("packet parsing incomplete - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());
				return true;
			}
			else
			{
				LOG(_T("packet parsing error - %s:%d\n"), pUser->GetAddr(), pUser->GetPort());
				return false;
			}
		}
	}

	return true;
}

void CMyServer::Sended(void* pObject, DWORD dwLength)
{
	CUser* pUser = reinterpret_cast<CUser*>(pObject);
}

void CMyServer::AddUser(int serial, CUser* pUser)
{
	CGuard guard(&m_lock);

	m_userMap.insert(UserMap::value_type(serial, pUser));
}

void CMyServer::DelUser(int serial)
{
	CGuard guard(&m_lock);

	m_userMap.erase(serial);
}

void CMyServer::AddPacket(CPacket* pPacket)
{
	//m_Queue.enqueue(pPacket);
	m_packetQueue.push(pPacket);
}


void CMyServer::Run()
{
	if (m_isUsingQueue == false)
		return;

	DWORD start = GetTickCount();

	CPacket* pPacket = NULL;

	while (m_isExit == false)
	{
		//m_profile.LoopBegin();

		// message queue 에서 패킷을 꺼내서 처리한다.

		//while (m_Queue.dequeue(pPacket) == true)
		
		while(m_packetQueue.try_pop(pPacket) == true)
		{
			CUser* pUser = (CUser*)pPacket->GetNetPtr();
			unsigned short pid = pPacket->GetPID();

			CStream stream(pPacket->GetBuffer(), pPacket->GetSize());

			Protocol(pUser, pid = pPacket->GetPID(), &stream);
		}

		Sleep(1);

		//m_profile.LoopEnd();

	}
	
	SetEvent(m_hExitEvent);

	return;
}

void CMyServer::WaitEnd()
{
	WaitForSingleObject(m_hExitEvent, INFINITE);
}

int CMyServer::Protocol(CUser* pUser, unsigned short pid, char* buffer, int len)
{
	switch (pid)
	{
	case PID_CHAT_MESSAGE:

		break;
	}
	return 0;
}

int CMyServer::Protocol(CUser* pUser, unsigned short pid, CStream* pStream)
{
	switch (pid)
	{
	case PID_NET_CONNECT:
		OnConnect(pUser, pid, pStream);
		break;

	case PID_NET_DISCONNECT:
		OnDisconnect(pUser, pid, pStream);
		break;

	case PID_CHAT_MESSAGE:
		OnChatMessage(pUser, pid, pStream);
		break;

	case PID_ROOMLIST:
		OnRoomList(pUser, pid, pStream);
		break;

	case PID_CREATE_ROOM:
		OnCreateRoom(pUser, pid, pStream);
		break;

	case PID_JOIN_ROOM:
		OnJoinRoom(pUser, pid, pStream);
		break;

	case PID_LEAVE_ROOM:
		OnLeaveRoom(pUser, pid, pStream);
		break;
	}

	return 0;
}

void CMyServer::OnConnect(CUser* pUser, unsigned short pid, CStream* pStream)
{
	AddUser(pUser->GetSerial(), pUser);
	_tprintf(_T("Client(serial:%d) Connected - %s:%d\n"), pUser->GetSerial(), pUser->GetAddr(), pUser->GetPort());

	// Room List 를 전달한다.


}
void CMyServer::OnDisconnect(CUser* pUser, unsigned short pid, CStream* pStream)
{
	DelUser(pUser->GetSerial());

	ULONG addr = pStream->GetUInt();
	USHORT port = pStream->GetUShort();

	// Release 타이밍을 예측할 수 없어서 명시적으로 IP, PORT 를 전달하게 수정함
	//_tprintf(_T("Client(serial:%d) Disconnected - %s:%d\n"), pUser->GetSerial(), pUser->GetAddr(), pUser->GetPort());

	_tprintf(_T("Client(serial:%d) Disconnected - %d.%d.%d.%d:%d\n"),
		pUser->GetSerial(),
		addr & 0xff,
		(addr >> 8) & 0xff,
		(addr >> 16) & 0xff,
		(addr >> 24) & 0xff,
		port);
}
void CMyServer::OnRoomList(CUser* pUser, unsigned short pid, CStream* pStream)
{

}
void CMyServer::OnChatMessage(CUser* pUser, unsigned short pid, CStream* pStream)
{
	char* pStr = pStream->GetStr();
	printf("message from user %d : %s\n", pUser->GetSerial(), pStr);

	CStream stream;
	stream.SetInt(pUser->GetSerial());
	stream.SetStr(pStr);

	// broad cast
	for (auto& user : m_userMap)
	{
		user.second->Send(pid, stream.GetBuffer(), stream.GetSize());
	}
}
void CMyServer::OnCreateRoom(CUser* pUser, unsigned short pid, CStream* pStream)
{
	char* roomname = pStream->GetStr();	// 방 이름
	BOOL isLock = pStream->GetInt();	// 방 잠금 여부
}

void CMyServer::OnJoinRoom(CUser* pUser, unsigned short pid, CStream* pStream)
{

}
void CMyServer::OnLeaveRoom(CUser* pUser, unsigned short pid, CStream* pStream)
{

}

