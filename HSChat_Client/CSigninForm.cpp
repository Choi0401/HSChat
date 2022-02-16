#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSigninForm.h"

// CSigninForm ��ȭ ����

IMPLEMENT_DYNAMIC(CSigninForm, CFormView)

CSigninForm::CSigninForm()
	: CFormView(IDD_FORMVIEW_SIGNIN)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();	
}

CSigninForm::CSigninForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CSigninForm::~CSigninForm()
{
}

void CSigninForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSigninForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN_SIGNUP, &CSigninForm::OnBnClickedButtonSigninSignup)
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN_SIGNIN, &CSigninForm::OnBnClickedButtonSigninSignin)
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN_SEARCHID, &CSigninForm::OnBnClickedButtonSigninSearchID)
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN_SEARCHPW, &CSigninForm::OnBnClickedButtonSigninSearchPW)
END_MESSAGE_MAP()


// CMyForm �޽��� ó����


BOOL CSigninForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSigninForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.	
}


void CSigninForm::OnBnClickedButtonSigninSignup()  {
	m_pDlg->m_ShowForm(1);

}


void CSigninForm::OnBnClickedButtonSigninSearchID()
{
	m_pDlg->m_ShowForm(2);
}


void CSigninForm::OnBnClickedButtonSigninSearchPW()
{
	m_pDlg->m_ShowForm(3);
}


void CSigninForm::OnBnClickedButtonSigninSignin()
{
	// ������ �α��� �޽��� ����
	CString strID, strPW;
	GetDlgItemText(IDC_EDIT_SIGNIN_ID, strID);
	GetDlgItemText(IDC_EDIT_SIGNIN_PW, strPW);


	// �α��� ����

	// �α��� ����
	m_pDlg->m_ShowForm(4);
	SetDlgItemText(IDC_EDIT_SIGNIN_ID, _T(""));
	SetDlgItemText(IDC_EDIT_SIGNIN_PW, _T(""));

}