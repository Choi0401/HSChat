#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSignupForm.h"
#include "json/json.h"



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


// CMyForm �޽��� ó����


BOOL CSignupForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CSignupForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}


void CSignupForm::OnBnClickedButtonSignupOK()
{	
	CString strName, strBirth, strPhone, strID, strNickname, strPW, strPWOK;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_SIGNUP_NAME, strName);
	GetDlgItemText(IDC_DATETIMEPICKER_BIRTH, strBirth);
	GetDlgItemText(IDC_EDIT_SIGNUP_PHONE, strPhone);
	GetDlgItemText(IDC_EDIT_SIGNUP_ID, strID);
	GetDlgItemText(IDC_EDIT_SIGNUP_NICKNAME, strNickname);
	GetDlgItemText(IDC_EDIT_SIGNUP_PW, strPW);
	GetDlgItemText(IDC_EDIT_SIGNUP_PWOK, strPWOK);

	if (strName.GetLength() == 0)
		AfxMessageBox(_T("�̸��� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strBirth.GetLength() == 0)
		AfxMessageBox(_T("��������� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPhone.GetLength() == 0)
		AfxMessageBox(_T("��ȭ��ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strID.GetLength() == 0)
		AfxMessageBox(_T("���̵� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strNickname.GetLength() == 0)
		AfxMessageBox(_T("�г����� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPW.GetLength() == 0)
		AfxMessageBox(_T("��й�ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPW != strPWOK)
		AfxMessageBox(_T("��й�ȣ�� Ȯ���ϼ���!"), MB_ICONSTOP);
	else {				
		root["action"] = "signup";
		root["name"] = std::string(CT2CA(strName));
		root["birth"] = std::string(CT2CA(strBirth));
		root["phone"] = std::string(CT2CA(strPhone));
		root["id"] = std::string(CT2CA(strID));
		root["nickname"] = std::string(CT2CA(strNickname));
		root["pw"] = std::string(CT2CA(strPW));


		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();
		
		m_pDlg->m_pClient->m_SendData();

	}	
}


void CSignupForm::OnBnClickedButtonSignupCancel()
{
	m_pDlg->m_ShowForm(0);
}


BOOL CSignupForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonSignupOK();
		return TRUE;
	}
	return CFormView::PreTranslateMessage(pMsg);
}
