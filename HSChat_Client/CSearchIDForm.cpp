#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSearchIDForm.h"

IMPLEMENT_DYNAMIC(CSearchIDForm, CFormView)

CSearchIDForm::CSearchIDForm()
	: CFormView(IDD_FORMVIEW_SEARCHID)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CSearchIDForm::CSearchIDForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CSearchIDForm::~CSearchIDForm()
{
}

void CSearchIDForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSearchIDForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_SEARCHID_OK, &CSearchIDForm::OnBnClickedButtonSearchIDOK)
	ON_BN_CLICKED(IDC_BUTTON_SEARCHID_CANCEL, &CSearchIDForm::OnBnClickedButtonSearchIDCancel)
END_MESSAGE_MAP()


// CMyForm �޽��� ó����


BOOL CSearchIDForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSearchIDForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}

/*
	* OnClose() : �޸� �浹 ���� �޼��� �߻�
	* exit(0) : ���������� terminated, �޸� ���� �߻�
	*/


void CSearchIDForm::OnBnClickedButtonSearchIDOK()
{
	CEdit* pEdit1;
	CEdit* pEdit2;

	CString name;
	CString birth;

	GetDlgItemText(IDC_EDIT_SEARCHID_NAME, name);
	GetDlgItemText(IDC_EDIT_SEARCHID_BIRTH, birth);

	if (name.GetLength() == 0)
		AfxMessageBox(_T("�̸��� �Է��ϼ���!"), MB_ICONSTOP);
	else if (birth.GetLength() == 0)
		AfxMessageBox(_T("��������� �Է��ϼ���!"), MB_ICONSTOP);
	else {
		AfxMessageBox(_T("ȸ������ ���̵�� ??? �Դϴ�!"), MB_ICONINFORMATION);
		pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_SEARCHID_NAME);
		pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_SEARCHID_BIRTH);

		pEdit1->SetSel(0, -1);
		pEdit1->Clear();
		pEdit2->SetSel(0, -1);
		pEdit2->Clear();

		m_pDlg->m_ShowForm(0);
	}
}


void CSearchIDForm::OnBnClickedButtonSearchIDCancel()
{
	m_pDlg->m_ShowForm(0);
}
