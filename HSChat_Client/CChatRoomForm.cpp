// CChatRoomForm.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CChatRoomForm.h"
#include "json/json.h"

// CChatRoomForm

IMPLEMENT_DYNCREATE(CChatRoomForm, CFormView)

CChatRoomForm::CChatRoomForm()
	: CFormView(IDD_FORMVIEW_CHATROOM)
{	
}

CChatRoomForm::~CChatRoomForm()
{
}

void CChatRoomForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_CHATROOM_RECVMSG, m_chat);
	DDX_Control(pDX, IDC_EDIT_CHATROOM_SENDMSG, m_sendmsg);
	DDX_Control(pDX, IDC_LIST_CHATROOM_USERLIST, m_roomuserlist);
}

BEGIN_MESSAGE_MAP(CChatRoomForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_CHATROOM_SEND, &CChatRoomForm::OnBnClickedButtonChatroomSend)
	ON_BN_CLICKED(IDC_BUTTON_CHATROOM_QUIT, &CChatRoomForm::OnBnClickedButtonChatroomQuit)
END_MESSAGE_MAP()


// CChatRoomForm 진단

#ifdef _DEBUG
void CChatRoomForm::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CChatRoomForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CChatRoomForm 메시지 처리기

BOOL CChatRoomForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CChatRoomForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
	m_chat.SetLimitText(0);
	m_sendmsg.SetLimitText(0);

	CRect rect;
	m_roomuserlist.GetClientRect(&rect);
	m_roomuserlist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_roomuserlist.InsertColumn(0, _T("참여자"), LVCFMT_LEFT, rect.Width());
	m_roomuserlist.GetHeaderCtrl()->EnableWindow(false);

}

void CChatRoomForm::OnBnClickedButtonChatroomSend()
{
	CString strMsg;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_CHATROOM_SENDMSG, strMsg);
	
	//TODO : 메시지 서버에 전송
	if (strMsg.GetLength() > 0) 
	{
		root["action"] = "sendmsg";
		root["nickname"] = m_pDlg->m_pClient->m_getNickname();
		root["roomnum"] = m_pDlg->m_pClient->m_roomnum;
		root["msg"] = std::string(CT2CA(strMsg));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();

		m_pDlg->m_pClient->m_SendData();

		SetDlgItemText(IDC_EDIT_CHATROOM_SENDMSG, _T(""));

	}
}


BOOL CChatRoomForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonChatroomSend();
		return TRUE;
	}


	return CFormView::PreTranslateMessage(pMsg);

}


void CChatRoomForm::OnBnClickedButtonChatroomQuit()
{
	m_pDlg->m_pClient->m_RequestAllList();
	m_pDlg->m_ShowForm(4);
}
