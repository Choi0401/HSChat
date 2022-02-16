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


// CMyForm 메시지 처리기


BOOL CSearchPWForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSearchPWForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


}

/*
	* OnClose() : 메모리 충돌 오류 메세지 발생
	* exit(0) : 비정상적인 terminated, 메모리 누수 발생
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
		AfxMessageBox(_T("이름을 입력하세요!"), MB_ICONSTOP);
	else if (birth.GetLength() == 0)
		AfxMessageBox(_T("생년월일을 입력하세요!"), MB_ICONSTOP);
	else if (ID.GetLength() == 0)
		AfxMessageBox(_T("ID를 입력하세요!"), MB_ICONSTOP);
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
