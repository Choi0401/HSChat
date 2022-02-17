#pragma once

#include <string>
#include "TSQueue.cpp"

using namespace std;

#define CLIENT_CONNECTED	1
#define CLIENT_DISCONNECTED	-1

typedef struct data
{
	int size;
	string msg;
}Data;

class CClient
{
private:

public:
	SOCKET m_socket;
	SOCKADDR_IN m_addr;
	WSADATA m_wsaData;


	Data m_data;
	TsQueue <string> m_queue;
	//queue <string> m_queue;

	//string m_recvmsg;	
	int m_connstate;


	CClient();
	void m_OpenConnection();
	void m_ErrorHandling(CString str);
	void m_InitData();
};

