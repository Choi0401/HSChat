#pragma once

#include <string>

using namespace std;

#define CLIENT_CONNECTED	1
#define CLIENT_DISCONNECTED	-1

class CClient
{
private:

public:
	SOCKET m_socket;
	SOCKADDR_IN m_addr;
	WSADATA m_wsaData;

	string m_recvmsg;
	int m_connstate;


	CClient();
	void m_OpenConnection();
	void m_ErrorHandling(CString str);

};

