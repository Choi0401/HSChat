// CMakeRoomDlg.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "CMakeRoomDlg.h"


// CMakeRoomDlg 대화 상자

IMPLEMENT_DYNAMIC(CMakeRoomDlg, CDialogEx)

CMakeRoomDlg::CMakeRoomDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MAKEROOM, pParent)
{

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
	CButton* pRB_public = NULL;
	if ((pRB_public = (CButton*)GetDlgItem(IDC_RADIO_PUBLIC)) == NULL)
	{
		AfxMessageBox(_T("ERROR[GetDlgItem()] : Failed to get IDC_RADIO_PUBLIC"));
		// TODO : 소켓 닫기 필요
		::PostQuitMessage(WM_QUIT);
	}

	pRB_public->SetCheck(TRUE);
	CEdit* pEDC_name = NULL;
	if ((pEDC_name = (CEdit*)GetDlgItem(IDC_EDIT_MAKEROOM_NAME)) == NULL)
	{
		AfxMessageBox(_T("ERROR[GetDlgItem()] : Failed to get IDC_EDIT_MAKEROOM_NAME"));
	}
	pEDC_name->SetSel(-1);
	pEDC_name->SetFocus();

	return FALSE;

}

void CMakeRoomDlg::OnBnClickedButtonMakeroom()
{
	CString strName = _T("");
	int userNum = 0;
	GetDlgItemText(IDC_EDIT_MAKEROOM_NAME, strName);
	userNum = GetDlgItemInt(IDC_EDIT_MAKEROOM_NUM);

	if (strName.GetLength() == 0 || userNum == 0)
	{
		if(strName.GetLength() == 0)
			AfxMessageBox(_T("방 이름을 입력해주세요."));
		else if(userNum == 0)
			AfxMessageBox(_T("방 인원을 입력해주세요."));
	}
	else
	{
		//TODO : 방 만드는 메시지 서버에 전송

		//TODO : 서버로부터 메시지 받기

		EndDialog(IDOK);

		//TODO : 폼 전환(채팅방)		
		CHSChatDlg* pDlg = (CHSChatDlg*)AfxGetMainWnd();
		pDlg->ShowForm(5);	


		//CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDIT_CHATROOM_SENDMSG);
		//if (pedit == NULL)
		//	AfxMessageBox(_T("오류오류"));
		
		
		/*
		CEdit* pedit = (CEdit*)GetDlgItem(IDC_EDIT_CHATROOM_SENDMSG);		
		//pedit->SetFocus();
		pedit->SetSel(-1);

		int nID = GetFocus()->GetDlgCtrlID();
		CString str;
		str.Format(_T("%d"), nID);
		AfxMessageBox(str);
		*/
	
	
	}

}

void CMakeRoomDlg::OnBnClickedButtonMakeroomCancel()
{
	EndDialog(IDCANCEL);
}
