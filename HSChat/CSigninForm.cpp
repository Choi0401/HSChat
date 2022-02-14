#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSigninForm.h"

// CSigninForm 대화 상자

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
END_MESSAGE_MAP()


// CMyForm 메시지 처리기


BOOL CSigninForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSigninForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
}


void CSigninForm::OnBnClickedButtonSigninSignup()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CSigninForm::OnBnClickedButtonSigninSignin()
{
	// 서버에 로그인 메시지 전송
	CString strID, strPW;
	GetDlgItemText(IDC_EDIT_SIGNIN_ID, strID);
	GetDlgItemText(IDC_EDIT_SIGNIN_PW, strPW);


	// 로그인 실패

	// 로그인 성공
	m_pDlg->ShowForm(1);	
	SetDlgItemText(IDC_EDIT_SIGNIN_ID, _T(""));
	SetDlgItemText(IDC_EDIT_SIGNIN_PW, _T(""));
		
}


void CSigninForm::OnBnClickedButtonSigninSearchID()
{
	m_pDlg->ShowForm(2);
}
