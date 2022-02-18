#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CWaitingForm.h"
#include "CMakeRoomDlg.h"
#include "json/json.h"

// CSigninForm ��ȭ ����

IMPLEMENT_DYNAMIC(CWaitingForm, CFormView)

CWaitingForm::CWaitingForm()
	: CFormView(IDD_FORMVIEW_WAITING)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CWaitingForm::CWaitingForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CWaitingForm::~CWaitingForm()
{
}

void CWaitingForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWaitingForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_WATING_LOGOUT, &CWaitingForm::OnBnClickedButtonWatingLogout)
	ON_BN_CLICKED(IDC_BUTTON_WATING_EXIT, &CWaitingForm::OnBnClickedButtonWatingExit)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MAKEROOM, &CWaitingForm::OnBnClickedButtonWatingMakeroom)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MYINFO, &CWaitingForm::OnBnClickedButtonWatingMyinfo)
END_MESSAGE_MAP()


// CMyForm �޽��� ó����


BOOL CWaitingForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CWaitingForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.


}



void CWaitingForm::OnBnClickedButtonWatingLogout()
{
	// ������ �α׾ƿ��ϴ� �޽��� ���� 

	// �α׾ƿ� ����

	// �α׾ƿ� ����
	m_pDlg->m_ShowForm(0);
}


void CWaitingForm::OnBnClickedButtonWatingExit()
{	
	if (AfxMessageBox(_T("���α׷��� �����Ͻðڽ��ϱ�?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		//TODO: �����ϱ����� ������ ���� �޽��� ���������
		m_pDlg->m_pClient->m_CloseSocket();
		ExitProcess(0);

	}
}


void CWaitingForm::OnBnClickedButtonWatingMakeroom()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CMakeRoomDlg mr_dlg;
	mr_dlg.DoModal();
}



BOOL CWaitingForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{

		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}


void CWaitingForm::OnBnClickedButtonWatingMyinfo()
{

	Json::Value root;
	Json::StyledWriter writer;
	
	root["action"] = "showmyinfo";
	root["nickname"] = m_pDlg->m_pClient->m_getNickname();

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
	}
	m_pDlg->m_pClient->m_InitData();
}
