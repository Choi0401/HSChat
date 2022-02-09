#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSigninForm.h"

// CSigninForm ��ȭ ����

IMPLEMENT_DYNAMIC(CSigninForm, CFormView)

CSigninForm::CSigninForm()
	: CFormView(IDD_FORMVIEW_SIGNIN)
{

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
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN, &CSigninForm::OnBnClickedButtonSignin)
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



void CSigninForm::OnBnClickedButtonSignin()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString strID, strPW;
	GetDlgItemText(IDC_EDIT_ID, strID);
	GetDlgItemText(IDC_EDIT_PW, strPW);
	// �ٸ� Ŭ�������� ���� ���̾�α��� ������ ���
	CHSChatDlg* pDlg = (CHSChatDlg*)::AfxGetMainWnd();

	pDlg->ShowForm(1);
		
}
