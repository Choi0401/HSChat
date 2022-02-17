#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CMyInfoForm.h"
#include "json/json.h"

IMPLEMENT_DYNAMIC(CMyInfoForm, CFormView)

CMyInfoForm::CMyInfoForm()
	: CFormView(IDD_FORMVIEW_MYINFO)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CMyInfoForm::CMyInfoForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CMyInfoForm::~CMyInfoForm()
{
}

void CMyInfoForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyInfoForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_OK, &CMyInfoForm::OnBnClickedButtonMyInfoOK)
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_CANCEL, &CMyInfoForm::OnBnClickedButtonMyInfoCancel)
END_MESSAGE_MAP()



BOOL CMyInfoForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CMyInfoForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}

BOOL CMyInfoForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonMyInfoOK();
		return TRUE;
	}
	return CFormView::PreTranslateMessage(pMsg);
}


void CMyInfoForm::OnBnClickedButtonMyInfoOK()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	
}


void CMyInfoForm::OnBnClickedButtonMyInfoCancel()
{
	m_pDlg->m_ShowForm(4);
}
