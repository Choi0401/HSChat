#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSearchPWForm.h"
#include "json/json.h"


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
	CString strName, strBirth, strID, strPhone;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_SEARCHPW_NAME, strName);
	GetDlgItemText(IDC_EDIT_SEARCHPW_BIRTH, strBirth);
	GetDlgItemText(IDC_EDIT_SEARCHPW_ID, strID);
	GetDlgItemText(IDC_EDIT_SEARCHPW_PHONE, strPhone);

	if (strName.GetLength() == 0)
		AfxMessageBox(_T("�̸��� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strBirth.GetLength() == 0)
		AfxMessageBox(_T("��������� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPhone.GetLength() == 0)
		AfxMessageBox(_T("��ȭ��ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strID.GetLength() == 0)
		AfxMessageBox(_T("ID�� �Է��ϼ���!"), MB_ICONSTOP);
	else {
		root["action"] = "searchpw";
		root["name"] = std::string(CT2CA(strName));
		root["birth"] = std::string(CT2CA(strBirth));
		root["phone"] = std::string(CT2CA(strPhone));
		root["id"] = std::string(CT2CA(strID));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();

		int ret_HeadWrite = 0;
		if (m_pDlg->m_pOpenssl->m_pSSL == NULL || (ret_HeadWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_pDlg->m_pClient->m_data.size, sizeof(int))) <= 0)
		{
			AfxMessageBox(_T("������ ������ �� �����ϴ�."));
		}
		else
		{
			int ret_BodyWrite = 0;
			if ((ret_BodyWrite = SSL_write(m_pDlg->m_pOpenssl->m_pSSL, &m_pDlg->m_pClient->m_data.msg[0], m_pDlg->m_pClient->m_data.size)) <= 0)
			{
				AfxMessageBox(_T("������ ������ �� �����ϴ�."));
			}
			SetDlgItemText(IDC_EDIT_SEARCHPW_NAME, _T(""));
			SetDlgItemText(IDC_EDIT_SEARCHPW_BIRTH, _T(""));
			SetDlgItemText(IDC_EDIT_SEARCHPW_PHONE, _T(""));
			SetDlgItemText(IDC_EDIT_SEARCHPW_ID, _T(""));
		}


		m_pDlg->m_pClient->m_InitData();
	}
}


void CSearchPWForm::OnBnClickedButtonSearchPWCancel()
{
	m_pDlg->m_ShowForm(0);
}
