#pragma once

class CPacket;
class CNetwork : public CSingleTon<CNetwork>
{
	friend unsigned WINAPI NetworkThreadCallback(LPVOID pObject);
	friend unsigned WINAPI RecvThreadCallback(LPVOID pObject);
	friend unsigned WINAPI SendThreadCallback(LPVOID pObject);
	friend class CSingleTon<CNetwork>;

private:
	HANDLE		m_handle;
	DWORD		m_connectionTry;
	SOCKET		m_sock;
	bool		m_bConnected;

	HANDLE		m_handleRecv;
	HANDLE		m_handleSend;
	HANDLE		m_SendEvent;
	

	char						m_buffer[4096];
	char						m_saveBuffer[4096 * 3];
	int							m_offset;
	std::queue<CPacket*>		m_recvPacketQueue;
	std::queue<CPacket*>		m_sendPacketQueue;

	CRITICAL_SECTION			m_recvCS;
	CRITICAL_SECTION			m_sendCS;

private:
	int NetworkThreadCallback();
	int RecvThreadCallback();
	int SendThreadCallback();

public:
	bool Init();
	bool Connect(const char* ipAddress, short port);
	const bool& GetConnectStatus() const { return m_bConnected; }
	bool GetPacket(CPacket** pPacket);
	void SendPacket(CPacket* pPacket);

private:
	CNetwork();
	virtual ~CNetwork();
};

