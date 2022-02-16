#pragma once

#include <string>

class CClient
{
private:

public:
	SOCKET m_socket;
	SOCKADDR_IN m_addr;
	WSADATA m_wsaData;

	CClient();
	void m_OpenConnection();
	void m_ErrorHandling(CString str);

};

