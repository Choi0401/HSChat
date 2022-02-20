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
	string m_ID;
	string m_nickname;
public:
	SOCKET m_socket;
	SOCKADDR_IN m_addr;
	WSADATA m_wsaData;

	CHSChatDlg* m_pDlg;

	Data m_data;
	TsQueue <string> m_queue;
	
	int m_roomnum;
	bool m_ismaster;

	//string m_recvmsg;	
	int m_connstate;

	void m_setID(string id);
	void m_setNickname(string nickname);
	string m_getID();
	string m_getNickname();

	CClient();
	void m_InitSocket();
	void m_OpenConnection();
	void m_CloseSocket();
	void m_InitData();

	void m_RequestAllList();
	void m_LogOut();
	void m_SendData();

};

