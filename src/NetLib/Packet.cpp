#include "StdAfx.h"
#include "Packet.h"


CPacket::CPacket(unsigned short buffersize)
	:m_pid(0),
	m_serial(0),
	m_net(nullptr),
	m_buffer(nullptr),
	m_buffersize(0)
{
	if (buffersize > 0)
		m_buffer = new char[buffersize];

	m_buffersize = buffersize;
}

CPacket::CPacket(char* buffer, unsigned short buffersize)
	:m_pid(0),
	m_serial(0),
	m_net(nullptr)
{

	m_buffer = buffer;
	m_buffersize = buffersize;
}

CPacket::CPacket(CStream& stream)
	:m_pid(0),
	m_serial(0),
	m_net(nullptr)
{
	m_buffersize = stream.GetSize();
	if (m_buffersize > 0)
	{
		m_buffer = new char[m_buffersize];
	}
}

CPacket::~CPacket()
{
	SAFE_DELETE(m_buffer);
}