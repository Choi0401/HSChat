#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSigninForm.h"
#include "json/json.h"

using namespace std;


// CSigninForm ��ȭ ����

IMPLEMENT_DYNAMIC(CSigninForm, CFormView)

CSigninForm::CSigninForm()
	: CFormView(IDD_FORMVIEW_SIGNIN)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();	
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
	Json::Value root;
	Json::StyledWriter writer;

	string id;
	string pw;
	GetDlgItemText(IDC_EDIT_SIGNIN_ID, strID);
	GetDlgItemText(IDC_EDIT_SIGNIN_PW, strPW);

	if (strID.GetLength() == 0) 
		AfxMessageBox(_T("���̵� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPW.GetLength() == 0)
		AfxMessageBox(_T("��й�ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else
	{
		id = std::string(CT2CA(strID));
		pw = std::string(CT2CA(strPW));
		//pw = m_pDlg->sha256(pw);

		root["action"] = "signin";
		root["id"] = id;
		root["pw"] = pw;

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());
		
		m_pDlg->m_pClient->m_SendData();
		
		SetDlgItemText(IDC_EDIT_SIGNIN_ID, _T(""));
		SetDlgItemText(IDC_EDIT_SIGNIN_PW, _T(""));
	}	
}

BOOL CSigninForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonSigninSignin();
		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}
