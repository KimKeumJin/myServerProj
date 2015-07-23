#pragma once

#include "Stream.h"
#define DEFAUT_PACKET_BUFFER_SIZE 4096

#pragma pack(push, 1)

// �⺻ ��Ŷ Ŭ����
// ť���ؼ� ó���ϱ� ���ؼ�, client serial ������ �����Ѵ�.
class CPacket
{
private:
	unsigned short  m_pid;			// ��Ŷ ���̵�
	int				m_serial;		// ��Ŷ Owner Serial
	void*			m_net;			// ��Ŷ Owner Pointer

	char*			m_buffer;
	unsigned short  m_buffersize;

public:
	void SetPID(unsigned short pid) { m_pid = pid; }
	void SetSerial(int serial) { m_serial = serial; }
	void SetNetPtr(void* ptr) { m_net = ptr; }
	void SetData(char* buffer, unsigned short size) { memcpy(m_buffer, buffer, size); }
	
	unsigned short GetPID() { return m_pid; }
	int GetSerial() { return m_serial; }
	void* GetNetPtr() { return m_net; }
	
	unsigned short GetSize() { return m_buffersize; }
	char* GetBuffer() { return m_buffer; }
	


public:
	CPacket(unsigned short buffersize = DEFAUT_PACKET_BUFFER_SIZE);
	CPacket(char* buffer, unsigned short buffersize);
	CPacket(CStream& stream);
	~CPacket();
};
#pragma pack(pop)