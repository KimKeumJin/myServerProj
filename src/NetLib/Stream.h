#pragma once
#define STREAM_BUFFERSIZE 4096
class CStream
{
private:
	char			m_buffer[STREAM_BUFFERSIZE];
	unsigned short  m_offset;

public:
	char*			GetBuffer() { return m_buffer; }
	unsigned short  GetSize() { return m_offset; }

public:
	bool			SetStr(char* pStr);
	bool			SetWStr(wchar_t* pWStr);
	bool			SetUShort(unsigned short value);
	bool			SetInt(int value);
	bool			SetUInt(unsigned int value);

public:
	char*			GetStr();
	wchar_t*		GetWStr();
	char			GetChar();
	unsigned char	GetByte();
	short			GetShort();
	unsigned short	GetUShort();
	int				GetInt();
	unsigned int	GetUInt();
	long			GetLong();
	unsigned long	GetUlong();
	float			GetFloat();
	double			GetDouble();

public:
	CStream();
	CStream(char* buffer, unsigned short size);
	virtual ~CStream();
};

