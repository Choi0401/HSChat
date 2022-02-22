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
	int maxnum = 0;
	Json::Value root;
	Json::StyledWriter writer;

	GetDlgItemText(IDC_EDIT_MAKEROOM_NAME, strRoomName);
	maxnum = GetDlgItemInt(IDC_EDIT_MAKEROOM_NUM);

	if (strRoomName.GetLength() == 0 || maxnum == 0)
	{
		if(strRoomName.GetLength() == 0)
			AfxMessageBox(_T("방 이름을 입력해주세요."));
		else if(maxnum == 0)
			AfxMessageBox(_T("방 인원을 입력해주세요."));
	}
	if (strRoomName.GetLength() > 30 || maxnum > 50)
	{
		if (strRoomName.GetLength() > 30)
			AfxMessageBox(_T("방 이름을 30자 이하로 적어주세요"));
		else if (maxnum > 50)
			AfxMessageBox(_T("방 최대인원은 50명입니다"));
	}
	else
	{
		//TODO : 방 만드는 메시지 서버에 전송
		root["action"] = "createroom";
		root["master"] = m_pDlg->m_pClient->m_getNickname();
		root["roomname"] = std::string(CT2CA(strRoomName));
		root["maxnum"] = maxnum;
		if (m_pRB_public->GetCheck())
			root["roomtype"] = "public";
		else
			root["roomtype"] = "private";

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());
		
		m_pDlg->m_pClient->m_SendData();

		m_pDlg->m_pChatRoomForm->SetDlgItemText(IDC_EDIT_CHATROOM_RECVMSG, _T(""));

		EndDialog(IDOK);
		
	}

}

void CMakeRoomDlg::OnBnClickedButtonMakeroomCancel()
{
	EndDialog(IDCANCEL);
}
