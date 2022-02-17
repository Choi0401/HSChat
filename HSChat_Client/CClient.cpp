#include "pch.h"
#include "CClient.h"


using namespace std;

CClient::CClient()
{
    m_socket = INVALID_SOCKET;
    m_connstate = CLIENT_DISCONNECTED;        
    m_data.size = 0;   
    m_data.msg.clear();
    memset(&m_addr, 0, sizeof(m_addr));
    m_roomnum = 0;
}

void CClient::m_setID(string id)
{
    m_ID = id;
}
void CClient::m_setNickname(string nickname)
{
    m_nickname = nickname;
}
string CClient::m_getID()
{
    return m_ID;
}
string CClient::m_getNickname()
{
    return m_nickname;
}


void CClient::m_InitSocket()
{
    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
    {
        //m_ErrorHandling(_T("WSAStartup() error : %d"));
    }

    if ((m_socket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        //m_ErrorHandling(_T("socket() error : %d"));
    }
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    //m_addr.sin_addr.s_addr = inet_addr("192.168.1.125");
    //m_addr.sin_port = htons(8282);
    m_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    m_addr.sin_port = htons(7777);

}

void CClient::m_OpenConnection()
{
    if (connect(m_socket, (SOCKADDR*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
    {
        //m_ErrorHandling(_T("connect() error : %d"));        
    }
    else
        m_connstate = CLIENT_CONNECTED;
}

void CClient::m_CloseSocket()
{
    closesocket(m_socket);
    WSACleanup();
    m_socket = INVALID_SOCKET;
    m_connstate = CLIENT_DISCONNECTED;
}


void CClient::m_InitData() 
{
    m_data.msg.clear();
    m_data.size = 0;
}



void CClient::m_ErrorHandling(CString str)
{
    CString errstr;
    errstr.Format(str + "%d", WSAGetLastError());
    //AfxMessageBox(errstr);
    //exit(1);
}


