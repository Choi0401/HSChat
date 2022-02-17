#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSigninForm.h"
#include "json/json.h"

using namespace std;


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
	ON_BN_CLICKED(IDC_BUTTON_SIGNIN_SEARCHPW, &CSigninForm::OnBnClickedButtonSigninSearchPW)
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
	// 서버에 로그인 메시지 전송
	CString strID, strPW;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_SIGNIN_ID, strID);
	GetDlgItemText(IDC_EDIT_SIGNIN_PW, strPW);

	if (strID.GetLength() == 0) 
		AfxMessageBox(_T("아이디를 입력하세요!"), MB_ICONSTOP);
	else if (strPW.GetLength() == 0)
		AfxMessageBox(_T("비밀번호를 입력하세요!"), MB_ICONSTOP);
	else
	{
		root["action"] = "signin";
		root["id"] = std::string(CT2CA(strID));
		root["id"] = std::string(CT2CA(strID));
		root["pw"] = std::string(CT2CA(strPW));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();
		int ret_HeadWrite = 0;
		if (m_pDlg->m_pOpenssl->m_pSSL == NULL || (ret_HeadWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_pDlg->m_pClient->m_data.size, sizeof(int))) <= 0)
		{
			AfxMessageBox(_T("서버에 연결할 수 없습니다."));
		}
		else
		{	
			int ret_BodyWrite = 0;
			if ((ret_BodyWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_pDlg->m_pClient->m_data.msg[0], m_pDlg->m_pClient->m_data.size)) <= 0)
			{
				AfxMessageBox(_T("서버에 연결할 수 없습니다."));
			}

			SetDlgItemText(IDC_EDIT_SIGNIN_ID, _T(""));
			SetDlgItemText(IDC_EDIT_SIGNIN_PW, _T(""));
		}
		m_pDlg->m_pClient->m_InitData();
	}	
}

BOOL CSigninForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonSigninSignin();
		return TRUE;
	}


	return CFormView::PreTranslateMessage(pMsg);

}
