#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CSearchIDForm.h"
#include "json/json.h"

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
	CString strName, strBirth, strPhone;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_SEARCHID_NAME, strName);
	GetDlgItemText(IDC_SEARCHID_BIRTH, strBirth);
	GetDlgItemText(IDC_EDIT_SEARCHID_PHONE, strPhone);

	if (strName.GetLength() == 0)
		AfxMessageBox(_T("�̸��� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strBirth.GetLength() == 0)
		AfxMessageBox(_T("��������� �Է��ϼ���!"), MB_ICONSTOP);
	else if (strPhone.GetLength() == 0)
		AfxMessageBox(_T("��ȭ��ȣ�� �Է��ϼ���!"), MB_ICONSTOP);
	else {
		root["action"] = "searchid";
		root["name"] = std::string(CT2CA(strName));
		root["birth"] = std::string(CT2CA(strBirth));
		root["phone"] = std::string(CT2CA(strPhone));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();

		m_pDlg->m_pClient->m_SendData();

		SetDlgItemText(IDC_EDIT_SEARCHID_NAME, _T(""));
		SetDlgItemText(IDC_EDIT_SEARCHID_BIRTH, _T(""));
		SetDlgItemText(IDC_EDIT_SEARCHID_PHONE, _T(""));
	}
}


void CSearchIDForm::OnBnClickedButtonSearchIDCancel()
{
	m_pDlg->m_ShowForm(0);
}


BOOL CSearchIDForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonSearchIDOK();
		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}
