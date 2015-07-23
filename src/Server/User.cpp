#include "stdafx.h"
#include "User.h"


CUser::CUser(int i)
	:m_serial(i)
{
}


CUser::~CUser()
{
}

int CUser::ParsePacket(unsigned short* pid, char* buffer, unsigned short* len)
{
	if (m_offset < PACKET_HEADER_SIZE)
		return 0;

	PacketHeader header;

	memcpy(&header.m_dataSize, m_saveBuffer, 2);
	memcpy(&header.m_pid, m_saveBuffer + 2, 2);

	if (header.m_dataSize < 0 || header.m_dataSize > SAVEBUFFER_MAX)
		return -1;	
	
	/*
	// uncompress
	byte uncompress_buffer[4096] = { 0, };
	uLongf uncompress_size;
	
	int result = uncompress(uncompress_buffer, &uncompress_size, (byte*)(m_saveBuffer + PACKET_HEADER_SIZE), header.m_dataSize);
	

	memcpy(buffer, uncompress_buffer, uncompress_size);
	*pid = header.m_pid;
	*len = (unsigned short)uncompress_size;
	*/

	memcpy(buffer, m_saveBuffer + 4, header.m_dataSize);
	*pid = header.m_pid;
	*len = header.m_dataSize;

	int totalSize = PACKET_HEADER_SIZE + header.m_dataSize;

	memmove(m_saveBuffer, m_saveBuffer + totalSize, m_offset - totalSize);
	m_offset -= totalSize;

	return 1;
}

bool CUser::Send(unsigned short pid, char* buffer, int size)
{
	char _buffer[4096] = { 0, };


	// 패킷 헤더 구성 하기
	// 1. checksum

	// 2. encrypt

	/*
	// 3. compress
	byte compress_buffer[4096] = { 0, };
	uLongf compress_size;
	int result = compress(compress_buffer, &compress_size, (byte*)(buffer), size);

	PacketHeader header;
	header.m_dataSize = (unsigned short)compress_size;
	header.m_pid = pid;

	memcpy(_buffer, &header, PACKET_HEADER_SIZE);
	memcpy(_buffer + PACKET_HEADER_SIZE, compress_buffer, compress_size);

	return CNetClient::Send(_buffer, compress_size + PACKET_HEADER_SIZE);
	*/

	PacketHeader header;
	header.m_dataSize = (unsigned short)size;
	header.m_pid = pid;

	memcpy(_buffer, &header, PACKET_HEADER_SIZE);
	memcpy(_buffer + PACKET_HEADER_SIZE, buffer, size);

	return CNetClient::Send(_buffer, size + PACKET_HEADER_SIZE);
	
}
