#include "stdafx.h"
#include "Stream.h"


CStream::CStream()
	:m_offset(0)
{
}

CStream::CStream(char* buffer, unsigned short size)
	: m_offset(0)
{
	memcpy(m_buffer, buffer, size);
}


CStream::~CStream()
{
}

bool CStream::SetStr(char* pStr)
{
	if (pStr == nullptr)
	{
		return SetUShort(sizeof(char));
	}

	int len = strlen(pStr) + sizeof(char);	// null 문자 추가

	if (SetUShort(len) == false)
		return false;

	if (m_offset + len > STREAM_BUFFERSIZE)
		return false;

	memcpy(m_buffer + m_offset, pStr, len);
	m_offset += len;

	return true;
}

bool CStream::SetWStr(wchar_t* pWStr)
{
	if (pWStr == nullptr)
	{
		return SetUShort(sizeof(wchar_t));
	}

	int len = wcslen(pWStr) * sizeof(wchar_t) + sizeof(wchar_t);

	if (SetUShort(len) == false)
		return false;

	if (m_offset + len > STREAM_BUFFERSIZE)
		return false;

	memcpy(m_buffer + m_offset, pWStr, len);

	m_offset += len;

	return true;
}

bool CStream::SetUShort(unsigned short value)
{
	if (m_offset + sizeof(unsigned short) > STREAM_BUFFERSIZE)
		return false;

	memcpy(m_buffer + m_offset, &value, sizeof(unsigned short));
	m_offset += sizeof(unsigned short);

	return true;
}

bool CStream::SetInt(int value)
{
	if (m_offset + sizeof(int) > STREAM_BUFFERSIZE)
		return false;

	memcpy(m_buffer + m_offset, &value, sizeof(int));
	m_offset += sizeof(int);

	return true;
}

bool CStream::SetUInt(unsigned int value)
{
	if (m_offset + sizeof(unsigned int) > STREAM_BUFFERSIZE)
		return false;

	memcpy(m_buffer + m_offset, &value, sizeof(unsigned int));
	m_offset += sizeof(unsigned int);

	return true;
}

unsigned short CStream::GetUShort()
{
	unsigned short value;

	memcpy(&value, m_buffer + m_offset, sizeof(unsigned short));

	m_offset += sizeof(unsigned short);

	return value;
}


char* CStream::GetStr()
{
	unsigned short len = GetUShort();

	char* pStr = m_buffer + m_offset;

	m_offset += len;

	return pStr;
}

int	CStream::GetInt()
{
	int value;

	memcpy(&value, m_buffer + m_offset, sizeof(int));

	m_offset += sizeof(int);

	return value;
}

unsigned int CStream::GetUInt()
{
	unsigned int value;

	memcpy(&value, m_buffer + m_offset, sizeof(unsigned int));

	m_offset += sizeof(unsigned int);

	return value;
}

//wchar_t* CStream::GetWStr()
//{
//
//}
//char CStream::GetChar()
//{
//}
//unsigned char CStream::GetByte()
//{
//
//}
//short CStream::GetShort()
//{
//
//}
//unsigned short CStream::GetUShort()
//{
//
//}
//int CStream::GetInt()
//{
//
//}
//unsigned int CStream::GetUint()
//{
//
//}
//long CStream::GetLong()
//{
//
//}
//unsigned long CStream::GetUlong()
//{
//
//}
//float CStream::GetFloat()
//{
//
//}
//double CStream::GetDouble()
//{
//
//}