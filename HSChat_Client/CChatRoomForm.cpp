// CChatRoomForm.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CChatRoomForm.h"


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
}

void CChatRoomForm::OnBnClickedButtonChatroomSend()
{
	CString send_msg;
	GetDlgItemText(IDC_EDIT_CHATROOM_SENDMSG, send_msg);


	//TODO : 메시지 서버에 전송
	if (send_msg.GetLength() > 0) {
		AfxMessageBox(send_msg);
	}
	SetDlgItemText(IDC_EDIT_CHATROOM_SENDMSG, _T(""));


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
	// TODO: 서버에 나가는 메시지 전송하고 소켓닫기 
	m_pDlg->m_ShowForm(1);
}
