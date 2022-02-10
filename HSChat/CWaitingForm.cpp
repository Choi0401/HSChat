#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CWaitingForm.h"
#include "CMakeRoomDlg.h"

// CSigninForm ��ȭ ����

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


// CMyForm �޽��� ó����


BOOL CWaitingForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CWaitingForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}



void CWaitingForm::OnBnClickedButtonLogout()
{
	// ������ �α׾ƿ��ϴ� �޽��� ���� 

	// �α׾ƿ� ����

	// �α׾ƿ� ����
	m_pDlg->ShowForm(0);
}


void CWaitingForm::OnBnClickedButtonExit()
{
	// ���� �ݱ�




	/* 
	* OnClose() : �޸� �浹 ���� �޼��� �߻�
	* exit(0) : ���������� terminated, �޸� ���� �߻�
	*/
	::PostQuitMessage(WM_QUIT);
}


void CWaitingForm::OnBnClickedButtonMakeroom()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CMakeRoomDlg mr_dlg;
	mr_dlg.DoModal();
}



BOOL CWaitingForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{

		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}
