#pragma once

class CPacket;
class CNetClient
{
protected:
	SOCKET			m_sock;
	OVERLAPPED_EX	m_overlappedex_conn;
	OVERLAPPED_EX	m_overlappedex_recv;
	OVERLAPPED_EX	m_overlappedex_send;
	OVERLAPPED_EX	m_overlappedex_reconn;

	TCHAR			m_szAddr[32];
	ULONG			m_uAddr;
	USHORT			m_port;
	bool			m_bConnected;
	bool			m_isReuse;

	CHAR			m_readBuffer[READBUFFER_MAX];
	CHAR			m_saveBuffer[SAVEBUFFER_MAX];
	DWORD			m_offset;

public:
	void Release();
	void Reset();
	bool CreateSocket();
	const SOCKET& GetSocket() {return m_sock;}
	bool SetAcceptEx(SOCKET listenSock);
	void SetRemoteAddress();
	bool BeginReceive();
	void SetConnectStatus(bool bConnected) {m_bConnected = bConnected;}
	bool GetConnectStatus() { return m_bConnected; }
	bool CopyToBuffer(DWORD dwLength);
	const USHORT& GetPort() { return m_port; }
	const TCHAR* GetAddr() { return m_szAddr; }
	const ULONG& GetUAddr() { return m_uAddr; }
	bool IsReuse() { return m_isReuse; }
	OVERLAPPED_EX* GetReUseOverlap() { return &m_overlappedex_reconn; }

public:
	bool Send(char* buffer, int size);


public:
	CNetClient(void);
	virtual ~CNetClient(void);
};

