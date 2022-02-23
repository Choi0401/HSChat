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
    m_ismaster = false;
    m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

void CClient::m_setID(string id)
{
    m_ID = id;
}
void CClient::m_setNickname(string nickname)
{
    m_nickname = nickname;
    m_nickname = m_pDlg->UTF8ToANSI(m_nickname.c_str());
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
        m_pDlg->FileLog("HSChat_Log.txt", "WSAStartup Error ");
    }

    if ((m_socket = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        m_pDlg->FileLog("HSChat_Log.txt", "socket Error ");
    }
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    //m_addr.sin_addr.s_addr = inet_addr("192.168.1.115");
    m_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    m_addr.sin_port = htons(8282);

}

void CClient::m_OpenConnection()
{
    if (connect(m_socket, (SOCKADDR*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
    {
        //m_pDlg->FileLog("HSChat_Log.txt", "Connect Error ");
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
    //cout << "Data Body Size : " << m_pDlg->m_pClient->m_data.size << endl;
    if (m_connstate == CLIENT_DISCONNECTED || (ret_HeadWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_data.size, sizeof(int))) <= 0)
    {
        m_pDlg->FileLog("HSChat_Log.txt", "SSL_write(Head) Error ");
        AfxMessageBox(_T("서버에 연결할 수 없습니다."));
    }
    else
    {
        int ret_BodyWrite = 0;

        if (m_connstate == CLIENT_DISCONNECTED || (ret_BodyWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_data.msg[0], m_data.size)) <= 0)
        {
            m_pDlg->FileLog("HSChat_Log.txt", "SSL_write(Body) Error ");
            AfxMessageBox(_T("서버에 연결할 수 없습니다."));
        }
    }
    m_InitData();
}

void CClient::m_RequestAllList()
{
    Json::Value root;
    Json::StyledWriter writer;

    string nickname;
    nickname = m_pDlg->MultiByteToUtf8(m_getNickname());

    root["action"] = "alllist";
    root["nickname"] = nickname;

    m_data.msg = writer.write(root);
    m_data.size = static_cast<int>(m_data.msg.size());

    m_SendData();
    
}

void CClient::m_LogOut()
{
    Json::Value root;
    Json::StyledWriter writer;

    string nickname;
    nickname = m_pDlg->MultiByteToUtf8(m_getNickname());

    root["action"] = "logout";
    root["nickname"] = nickname;


    m_data.msg = writer.write(root);
    m_data.size = static_cast<int>(m_data.msg.size());

    m_SendData();
}

