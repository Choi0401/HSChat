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


// CMyForm 메시지 처리기


BOOL CSearchIDForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSearchIDForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


}

/*
	* OnClose() : 메모리 충돌 오류 메세지 발생
	* exit(0) : 비정상적인 terminated, 메모리 누수 발생
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
		AfxMessageBox(_T("이름을 입력하세요!"), MB_ICONSTOP);
	else if (birth.GetLength() == 0)
		AfxMessageBox(_T("생년월일을 입력하세요!"), MB_ICONSTOP);
	else {
		AfxMessageBox(_T("회원님의 아이디는 ??? 입니다!"), MB_ICONINFORMATION);
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
