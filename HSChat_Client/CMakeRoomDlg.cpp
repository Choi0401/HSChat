// CMakeRoomDlg.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "CMakeRoomDlg.h"
#include "json/json.h"

// CMakeRoomDlg 대화 상자

IMPLEMENT_DYNAMIC(CMakeRoomDlg, CDialogEx)

CMakeRoomDlg::CMakeRoomDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MAKEROOM, pParent)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CMakeRoomDlg::~CMakeRoomDlg()
{
}

void CMakeRoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMakeRoomDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_MAKEROOM_CANCEL, &CMakeRoomDlg::OnBnClickedButtonMakeroomCancel)
	ON_BN_CLICKED(IDC_BUTTON_MAKEROOM, &CMakeRoomDlg::OnBnClickedButtonMakeroom)
END_MESSAGE_MAP()


// CMakeRoomDlg 메시지 처리기

BOOL CMakeRoomDlg::OnInitDialog()

{
	m_pRB_public = (CButton*)GetDlgItem(IDC_RADIO_PUBLIC);
	m_pRB_public->SetCheck(TRUE);

	this->GetDlgItem(IDC_EDIT_MAKEROOM_NAME)->SetFocus();


	return FALSE;

}

void CMakeRoomDlg::OnBnClickedButtonMakeroom()
{
	CString strRoomName;
	int userNum = 0;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_MAKEROOM_NAME, strRoomName);
	userNum = GetDlgItemInt(IDC_EDIT_MAKEROOM_NUM);

	if (strRoomName.GetLength() == 0 || userNum == 0)
	{
		if(strRoomName.GetLength() == 0)
			AfxMessageBox(_T("방 이름을 입력해주세요."));
		else if(userNum == 0)
			AfxMessageBox(_T("방 인원을 입력해주세요."));
	}
	else
	{
		//TODO : 방 만드는 메시지 서버에 전송
		root["action"] = "createroom";
		root["master"] = m_pDlg->m_pClient->m_getNickname();
		root["roomname"] = std::string(CT2CA(strRoomName));
		root["usernum"] = userNum;
		if (m_pRB_public->GetCheck())
			root["roomtype"] = "public";
		else
			root["roomtype"] = "private";

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();
		
		m_pDlg->m_pClient->m_SendData();

		EndDialog(IDOK);
	}

}

void CMakeRoomDlg::OnBnClickedButtonMakeroomCancel()
{
	EndDialog(IDCANCEL);
}
