// CChangePWDlg.cpp: 구현 파일
//

#include "pch.h"
#include "afxdialogex.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CChangePWDlg.h"
#include "json/json.h"

// CChangePWDlg 대화 상자

IMPLEMENT_DYNAMIC(CChangePWDlg, CDialogEx)

CChangePWDlg::CChangePWDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CHANGEPW, pParent)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CChangePWDlg::~CChangePWDlg()
{
}

void CChangePWDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChangePWDlg, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_CHAGNEPW_OK, &CChangePWDlg::OnBnClickedButtonChagnepwOk)
END_MESSAGE_MAP()


// CChangePWDlg 메시지 처리기


BOOL CChangePWDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	this->GetDlgItem(IDC_EDIT_CHANGEPW_NEWPW)->SetFocus();

	return FALSE;
}


void CChangePWDlg::OnBnClickedButtonChagnepwOk()
{
	CString strNewPW, strNewPWOK;
	
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_CHANGEPW_NEWPW, strNewPW);
	GetDlgItemText(IDC_EDIT_CHANGEPW_NEWPWOK, strNewPWOK);

	if (strNewPW.GetLength() == 0)
		AfxMessageBox(_T("비밀번호를 입력하세요!"), MB_ICONSTOP);
	else if(strNewPWOK.GetLength() == 0)
		AfxMessageBox(_T("비밀번호를 입력하세요!"), MB_ICONSTOP);
	else if (strNewPW != strNewPWOK)
		AfxMessageBox(_T("비밀번호가 다릅니다!"), MB_ICONSTOP);
	else
	{
		string pw;
		pw = std::string(CT2CA(strNewPW));

		cout << pw << endl;

		if (m_pDlg->pw_check(pw))
		{
			pw = m_pDlg->pw_salting(pw);
			cout << pw << endl;
			pw = m_pDlg->sha256(pw);
			cout << pw << endl;
		}
		else
			AfxMessageBox(_T("비밀번호 정책 오류입니다!"), MB_ICONSTOP);

		cout << "test" << endl;

		root["action"] = "setnewpw";
		if (m_pDlg == NULL) 
			m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
		root["id"] = m_pDlg->MultiByteToUtf8(m_pDlg->m_pClient->m_getID());
		root["pw"] = pw;

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());
		m_pDlg->m_pClient->m_SendData();
	}
	
}
