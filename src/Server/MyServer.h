#pragma once

class CUserManager;
class CPacket;
class CUser;
class CMyServer
	: public CIocpServer
{
private:
	CUserManager*			m_pUserManager;
	bool					m_isUsingQueue;

	// tbb concurrent queue 로 교체할 것!
	//MSQueue<CPacket*>				m_Queue;
	tbb::concurrent_queue<CPacket*>	m_packetQueue;


	std::queue<CPacket*>	m_PacketQueue;
	CLock					m_packetLock;
	bool					m_isExit;
	HANDLE					m_hExitEvent;

	
	typedef std::unordered_map<int, CUser*> UserMap;

	CLock				m_lock;
	UserMap				m_userMap;

	CElapsedProfile		m_profile;
	


public:
	virtual bool Init();	
	virtual bool BeginTCPServer();
	virtual bool BeginUDPServer();
	virtual void Connected(void* pObject);
	virtual void Disconnected(void* pObject);
	virtual bool Received(void* pObject, DWORD dwLength);
	virtual void Sended(void* pObject, DWORD dwLength);

private:
	void OnConnect(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnDisconnect(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnRoomList(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnChatMessage(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnCreateRoom(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnJoinRoom(CUser* pUser, unsigned short pid, CStream* pStream);
	void OnLeaveRoom(CUser* pUser, unsigned short pid, CStream* pStream);

public:
	void AddUser(int serial, CUser* pUser);
	void DelUser(int serial);
	void AddPacket(CPacket* pPacket);

public:
	void Run();
	void End() { m_isExit = true; }
	void WaitEnd();

public:
	int Protocol(CUser* pUser, unsigned short pid, char* buffer, int len);
	int Protocol(CUser* pUser, unsigned short pid, CStream* pStream);



public:
	CMyServer(void);
	virtual ~CMyServer(void);
};

