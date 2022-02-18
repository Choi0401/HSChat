// CFriendAddDlg.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "CFriendAddDlg.h"
#include "json/json.h"


// CFriendAddDlg 대화 상자

IMPLEMENT_DYNAMIC(CFriendAddDlg, CDialogEx)

CFriendAddDlg::CFriendAddDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FRIENDADD, pParent)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CFriendAddDlg::~CFriendAddDlg()
{
}

void CFriendAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFriendAddDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_FRIENDADD_OK, &CFriendAddDlg::OnBnClickedButtonFriendAddOK)
	ON_BN_CLICKED(IDC_BUTTON_FRIENDADD_CANCEL, &CFriendAddDlg::OnBnClickedButtonFriendaddCancel)
END_MESSAGE_MAP()


// CFriendAddDlg 메시지 처리기


BOOL CFriendAddDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	this->GetDlgItem(IDC_EDIT_FRIENDADD_NICKNAME)->SetFocus();

	return FALSE;
}



void CFriendAddDlg::OnBnClickedButtonFriendAddOK()
{
	CString strFNickname;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_FRIENDADD_NICKNAME, strFNickname);
	
	if (strFNickname.GetLength() == 0 )
		AfxMessageBox(_T("친구 닉네임을 입력해주세요."));
	else
	{
		root["action"] = "addfriend";
		root["nickname"] = m_pDlg->m_pClient->m_getNickname();
		root["fnickname"] = std::string(CT2CA(strFNickname));

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();

		m_pDlg->m_pClient->m_SendData();
		EndDialog(IDOK);
	}

}




void CFriendAddDlg::OnBnClickedButtonFriendaddCancel()
{
	EndDialog(IDCANCEL);
}
