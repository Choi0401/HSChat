#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CClient.h"
#include "json/json.h"

using namespace std;

CClient::CClient()
{
    m_socket = INVALID_SOCKET;
    m_connstate = CLIENT_DISCONNECTED;        
    m_data.size = 0;   
    m_data.msg.clear();
    memset(&m_addr, 0, sizeof(m_addr));
    m_roomnum = 0;
    m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
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
    // ���� �ʱ�ȭ
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

void CClient::m_SendData()
{
    int ret_HeadWrite = 0;
    if (m_pDlg == NULL) m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
    if (m_connstate == CLIENT_DISCONNECTED || (ret_HeadWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_data.size, sizeof(int))) <= 0)
    {
        AfxMessageBox(_T("������ ������ �� �����ϴ�."));
    }
    else
    {
        int ret_BodyWrite = 0;
        if (m_connstate == CLIENT_DISCONNECTED || (ret_BodyWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_data.msg[0], m_data.size)) <= 0)
        {
            AfxMessageBox(_T("������ ������ �� �����ϴ�."));
        }
    }
    m_InitData();
}

void CClient::m_RequestAllList()
{
    Json::Value root;
    Json::StyledWriter writer;

    root["action"] = "alllist";
    root["nickname"] = m_getNickname();

    m_data.msg = writer.write(root);
    m_data.size = m_data.msg.size();

    m_SendData();
    
}

void CClient::m_LogOut()
{
    Json::Value root;
    Json::StyledWriter writer;

    root["action"] = "logout";
    root["nickname"] = m_getNickname();

    m_data.msg = writer.write(root);
    m_data.size = m_data.msg.size();

    m_SendData();
}