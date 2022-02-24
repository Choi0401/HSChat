#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CMyInfoForm.h"
#include "json/json.h"

IMPLEMENT_DYNAMIC(CMyInfoForm, CFormView)

CMyInfoForm::CMyInfoForm()
	: CFormView(IDD_FORMVIEW_MYINFO)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CMyInfoForm::CMyInfoForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CMyInfoForm::~CMyInfoForm()
{
}

void CMyInfoForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyInfoForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_OK, &CMyInfoForm::OnBnClickedButtonMyInfoOK)
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_CANCEL, &CMyInfoForm::OnBnClickedButtonMyInfoCancel)
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_DELETE, &CMyInfoForm::OnBnClickedButtonMyInfoDelete)
END_MESSAGE_MAP()



BOOL CMyInfoForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CMyInfoForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}

BOOL CMyInfoForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonMyInfoOK();
		return TRUE;
	}
	return CFormView::PreTranslateMessage(pMsg);
}


void CMyInfoForm::OnBnClickedButtonMyInfoOK()
{
	CString strPhone, strNickname, strPW, strPWOK;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_MYINFO_PHONE, strPhone);
	GetDlgItemText(IDC_EDIT_MYINFO_NICKNAME, strNickname);
	GetDlgItemText(IDC_EDIT_MYINFO_PW, strPW);
	GetDlgItemText(IDC_EDIT_MYINFO_PWOK, strPWOK);

	if (strPhone.GetLength() == 0 || strPhone.GetLength() > 20)
		AfxMessageBox(_T("��ȭ��ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strNickname.GetLength() == 0 || strNickname.GetLength() > 20)
		AfxMessageBox(_T("�г����� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPW.GetLength() == 0)
		AfxMessageBox(_T("��й�ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPWOK.GetLength() == 0)
		AfxMessageBox(_T("��й�ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPW != strPWOK)
		AfxMessageBox(_T("��й�ȣ�� Ȯ���ϼ���!"), MB_ICONSTOP);
	else 
	{
		root["action"] = "changemyinfo";
		root["phone"] = std::string(CT2CA(strPhone));
		root["nickname"] = m_pDlg->MultiByteToUtf8((std::string(CT2CA(strNickname))));
		root["pw"] = std::string(CT2CA(strPW));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());

		m_pDlg->m_pClient->m_SendData();

		SetDlgItemText(IDC_EDIT_MYINFO_PHONE, _T(""));
		SetDlgItemText(IDC_EDIT_MYINFO_NICKNAME, _T(""));
		SetDlgItemText(IDC_EDIT_MYINFO_PW, _T(""));
		SetDlgItemText(IDC_EDIT_MYINFO_PWOK, _T(""));
		
	}

	
}

void CMyInfoForm::OnBnClickedButtonMyInfoDelete()
{
	if (AfxMessageBox(_T("������ Ż���Ͻðڽ��ϱ�?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		Json::Value root;
		Json::StyledWriter writer;

		root["action"] = "deleteaccount";
		root["nickname"] = m_pDlg->MultiByteToUtf8(m_pDlg->m_pClient->m_getNickname());

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());

		m_pDlg->m_pClient->m_SendData();
	}
	else
	{
		;
	}	
}



void CMyInfoForm::OnBnClickedButtonMyInfoCancel()
{

	m_pDlg->m_pClient->m_RequestAllList();
	m_pDlg->m_ShowForm(4);
}


