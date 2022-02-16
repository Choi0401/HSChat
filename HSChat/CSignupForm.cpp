#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSignupForm.h"



IMPLEMENT_DYNAMIC(CSignupForm, CFormView)

CSignupForm::CSignupForm()
	: CFormView(IDD_FORMVIEW_SIGNUP)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CSignupForm::CSignupForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CSignupForm::~CSignupForm()
{
}

void CSignupForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSignupForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_SIGNUP_OK, &CSignupForm::OnBnClickedButtonSignupOK)
	ON_BN_CLICKED(IDC_BUTTON_SIGNUP_CANCEL, &CSignupForm::OnBnClickedButtonSignupCancel)
END_MESSAGE_MAP()


// CMyForm 메시지 처리기


BOOL CSignupForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSignupForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


}


void CSignupForm::OnBnClickedButtonSignupOK()
{
	CEdit* pEdit1;
	CEdit* pEdit2;
	CEdit* pEdit3;
	CEdit* pEdit4;
	CEdit* pEdit5;
	CEdit* pEdit6;

	CString name;
	CString birth;
	CString id;
	CString nickname;
	CString pw;
	CString checkpw;

	GetDlgItemText(IDC_EDIT_SIGNUP_NAME, name);
	GetDlgItemText(IDC_EDIT_SIGNUP_BIRTH, birth);
	GetDlgItemText(IDC_EDIT_SIGNUP_ID, id);
	GetDlgItemText(IDC_EDIT_SIGNUP_NICKNAME, nickname);
	GetDlgItemText(IDC_EDIT_SIGNUP_PW, pw);
	GetDlgItemText(IDC_EDIT_SIGNUP_PWOK, checkpw);

	if (name.GetLength() == 0)
		AfxMessageBox(_T("이름을 입력하세요!"), MB_ICONSTOP);
	else if (birth.GetLength() == 0)
		AfxMessageBox(_T("생년월일을 입력하세요!"), MB_ICONSTOP);
	else if (id.GetLength() == 0)
		AfxMessageBox(_T("아이디를 입력하세요!"), MB_ICONSTOP);
	else if (nickname.GetLength() == 0)
		AfxMessageBox(_T("닉네임을 입력하세요!"), MB_ICONSTOP);
	else if (pw.GetLength() == 0)
		AfxMessageBox(_T("비밀번호를 입력하세요!"), MB_ICONSTOP);
	else if (pw != checkpw)
		AfxMessageBox(_T("비밀번호를 확인하세요!"), MB_ICONSTOP);
	else {
		AfxMessageBox(_T("회원님의 아이디는 " + id + " 입니다!\n환영합니다!"), MB_ICONINFORMATION);
		pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_NAME);
		pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_BIRTH);
		pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_ID);
		pEdit4 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_NICKNAME);
		pEdit5 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_PW);
		pEdit6 = (CEdit*)GetDlgItem(IDC_EDIT_SIGNUP_PWOK);


		pEdit1->SetSel(0, -1);
		pEdit1->Clear();
		pEdit2->SetSel(0, -1);
		pEdit2->Clear();
		pEdit3->SetSel(0, -1);
		pEdit3->Clear();
		pEdit4->SetSel(0, -1);
		pEdit4->Clear();
		pEdit5->SetSel(0, -1);
		pEdit5->Clear();
		pEdit6->SetSel(0, -1);
		pEdit6->Clear();
		m_pDlg->ShowForm(0);
	}
	//AfxMessageBox(_T("회원가입 완료입니다!\n반갑습니다!"), MB_ICONINFORMATION);
	//m_pDlg->ShowForm(0);
	//MessageBox(_T(""), _T("회원가입"), MB_OK); //내용에 CString값 표시 가능
}


void CSignupForm::OnBnClickedButtonSignupCancel()
{
	m_pDlg->ShowForm(0);
}
