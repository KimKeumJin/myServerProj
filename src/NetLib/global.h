#pragma once

#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <Windows.h>
#include <mswsock.h>
#include <process.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <locale.h>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <string>
#include <crtdbg.h>
#include <assert.h>
#include <strsafe.h>
#include <dbghelp.h>

#include <sql.h>
#include <sqlext.h>
#include <conio.h>
#include <sal.h>

enum IO_TYPE
{
	IO_CONNECT,
	IO_RECV,
	IO_SEND,
	IO_WAIT_REUSE,
};

enum LOG_LEVEL
{
	LOG_NORMAL,
	LOG_WARNING,
	LOG_ERROR
};

typedef struct _OVERLAPPEDEX
{
	OVERLAPPED	 overlapped;
	IO_TYPE		 operation;
	void*		 object;
}OVERLAPPED_EX, *POVERLAPPED_EX;

#define SAFE_DELETE(p) if(p) {delete p; p = NULL;}
#define SAFE_RELEASE(p) if(p) {p->Release(); p = NULL;}
#define READBUFFER_MAX 4096
#define SAVEBUFFER_MAX READBUFFER_MAX*3


// vs11 double check lock not need
template <typename T>
class CSingleTon
{
	typedef T& reference;

public:
	static reference GetInstance()
	{
		static T instance;
		return instance;
	}
	
protected:
	CSingleTon() {};
	virtual ~CSingleTon() {};
};

