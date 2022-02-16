#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSearchPWForm.h"



IMPLEMENT_DYNAMIC(CSearchPWForm, CFormView)

CSearchPWForm::CSearchPWForm()
	: CFormView(IDD_FORMVIEW_SEARCHPW)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CSearchPWForm::CSearchPWForm(UINT nIDTemplate) : CFormView(nIDTemplate)
{

}

CSearchPWForm::~CSearchPWForm()
{
}

void CSearchPWForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSearchPWForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_SEARCHPW_OK, &CSearchPWForm::OnBnClickedButtonSearchPWOK)
	ON_BN_CLICKED(IDC_BUTTON_SEARCHPW_CANCEL, &CSearchPWForm::OnBnClickedButtonSearchPWCancel)
END_MESSAGE_MAP()


// CMyForm �޽��� ó����


BOOL CSearchPWForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSearchPWForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}

/*
	* OnClose() : �޸� �浹 ���� �޼��� �߻�
	* exit(0) : ���������� terminated, �޸� ���� �߻�
	*/


void CSearchPWForm::OnBnClickedButtonSearchPWOK()
{
	CEdit* pEdit1;
	CEdit* pEdit2;
	CEdit* pEdit3;


	CString name;
	CString birth;
	CString ID;

	GetDlgItemText(IDC_EDIT_SEARCHPW_NAME, name);
	GetDlgItemText(IDC_EDIT_SEARCHPW_BIRTH, birth);
	GetDlgItemText(IDC_EDIT_SEARCHPW_ID, ID);


	if (name.GetLength() == 0)
		AfxMessageBox(_T("�̸��� �Է��ϼ���!"), MB_ICONSTOP);
	else if (birth.GetLength() == 0)
		AfxMessageBox(_T("��������� �Է��ϼ���!"), MB_ICONSTOP);
	else if (ID.GetLength() == 0)
		AfxMessageBox(_T("ID�� �Է��ϼ���!"), MB_ICONSTOP);
	else {
		AfxMessageBox(_T("P/W : qwer1234!"), MB_ICONINFORMATION);

		pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_SEARCHPW_NAME);
		pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_SEARCHPW_BIRTH);
		pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_SEARCHPW_ID);

		pEdit1->SetSel(0, -1);
		pEdit1->Clear();
		pEdit2->SetSel(0, -1);
		pEdit2->Clear();
		pEdit3->SetSel(0, -1);
		pEdit3->Clear();

		m_pDlg->m_ShowForm(0);
	}
}


void CSearchPWForm::OnBnClickedButtonSearchPWCancel()
{
	m_pDlg->m_ShowForm(0);
}
