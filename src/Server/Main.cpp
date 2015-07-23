// Server.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "MyServer.h"
#include "ServiceInstaller.h"
#include "SampleService.h"
#include "Dump.h"

CMyServer* g_pServer;

// 
// Settings of the service
// 

// Internal name of the service
#define SERVICE_NAME             L"CppWindowsService"

// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"CppWindowsService Sample Service"

// Service start options.
#define SERVICE_START_TYPE       SERVICE_DEMAND_START

// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""

// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"

// The password to the service account name
#define SERVICE_PASSWORD         NULL


int _tmain(int argc, _TCHAR* argv[])
{
#ifdef	_DEBUG
	//_CrtSetBreakAlloc(0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CDump::Begin();

#ifdef _SERVICE
	// service install, uninstall
	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
	{
		if (_wcsicmp(L"install", argv[1] + 1) == 0)
		{
			// Install the service when the command is 
			// "-install" or "/install".
			InstallService(
				SERVICE_NAME,               // Name of service
				SERVICE_DISPLAY_NAME,       // Name to display
				SERVICE_START_TYPE,         // Service start type
				SERVICE_DEPENDENCIES,       // Dependencies
				SERVICE_ACCOUNT,            // Service running account
				SERVICE_PASSWORD            // Password of the account
				);
		}
		else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
		{
			// Uninstall the service when the command is 
			// "-remove" or "/remove".
			UninstallService(SERVICE_NAME);
		}
	}
	else
	{
		// service runnning
		CSampleService service(SERVICE_NAME);
		CServiceBase::Run(service);
		
	}

#else

	LOG(_T("<Program Start>\n"));

	g_pServer = new CMyServer;

	if (g_pServer->Init() == false)
	{
		LOG(_T("IOCP Server Init failed\n"));
		return 0;
	}

	if (g_pServer->BeginTCPServer() == false)
	{
		LOG(_T("TCP Server Begin Failed\n"));
		return 0;
	}

	g_pServer->Run();

	g_pServer->WaitEnd();

	SAFE_DELETE(g_pServer);

	LOG(_T("<Program End>\n"));

#endif

	CDump::End();

	
	return 0;
}

