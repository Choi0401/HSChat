#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CWaitingForm.h"
#include "CMakeRoomDlg.h"

// CSigninForm 대화 상자

IMPLEMENT_DYNAMIC(CWaitingForm, CFormView)

CWaitingForm::CWaitingForm()
	: CFormView(IDD_FORMVIEW_WAITING)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CWaitingForm::CWaitingForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CWaitingForm::~CWaitingForm()
{
}

void CWaitingForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWaitingForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CWaitingForm::OnBnClickedButtonLogout)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CWaitingForm::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_MAKEROOM, &CWaitingForm::OnBnClickedButtonMakeroom)
END_MESSAGE_MAP()


// CMyForm 메시지 처리기


BOOL CWaitingForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CWaitingForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


}



void CWaitingForm::OnBnClickedButtonLogout()
{
	// 서버에 로그아웃하는 메시지 전송 

	// 로그아웃 실패

	// 로그아웃 성공
	m_pDlg->ShowForm(0);
}


void CWaitingForm::OnBnClickedButtonExit()
{
	// 소켓 닫기




	/* 
	* OnClose() : 메모리 충돌 오류 메세지 발생
	* exit(0) : 비정상적인 terminated, 메모리 누수 발생
	*/
	::PostQuitMessage(WM_QUIT);
}


void CWaitingForm::OnBnClickedButtonMakeroom()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CMakeRoomDlg mr_dlg;
	mr_dlg.DoModal();
}



BOOL CWaitingForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{

		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}
