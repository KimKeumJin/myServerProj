// MyClient.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "MyClient.h"


int _tmain(int argc, _TCHAR* argv[])
{
	
	CMyClient client;

	client.Init();

	client.Connect("127.0.0.1", 9000);

	client.WaitConnect();

	string str;
	char input[1024] = { 0, };

	while (true)
	{
		memset(input, 0, sizeof(input));

		cin >> input;

		if (strcmp(input, "exit") == 0)
		{
			client.Disconnect();
			break;
		}


		CStream stream;
		stream.SetStr(input);
		
		client.Send(PID_CHAT_MESSAGE, stream.GetBuffer(), stream.GetSize());

	}

	return 0;
}

