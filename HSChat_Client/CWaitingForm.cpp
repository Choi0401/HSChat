#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CWaitingForm.h"
#include "CMakeRoomDlg.h"
#include "json/json.h"

// CSigninForm 대화 상자

IMPLEMENT_DYNAMIC(CWaitingForm, CFormView)

CWaitingForm::CWaitingForm() : CFormView(IDD_FORMVIEW_WAITING)
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
	DDX_Control(pDX, IDC_LIST_WATING_ROOM, m_roomlist);
}


BEGIN_MESSAGE_MAP(CWaitingForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_WATING_LOGOUT, &CWaitingForm::OnBnClickedButtonWatingLogout)
	ON_BN_CLICKED(IDC_BUTTON_WATING_EXIT, &CWaitingForm::OnBnClickedButtonWatingExit)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MAKEROOM, &CWaitingForm::OnBnClickedButtonWatingMakeroom)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MYINFO, &CWaitingForm::OnBnClickedButtonWatingMyinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_WATING_ROOM, &CWaitingForm::OnNMDblclkListWatingRoom)
END_MESSAGE_MAP()


// CMyForm 메시지 처리기


BOOL CWaitingForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CWaitingForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.		
	CRect rect;
	m_roomlist.GetClientRect(&rect);
	m_roomlist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_roomlist.InsertColumn(0, _T("번호"), LVCFMT_CENTER, 50);
	m_roomlist.InsertColumn(1, _T("이름"), LVCFMT_LEFT, 300);
	m_roomlist.InsertColumn(2, _T("인원"), LVCFMT_RIGHT, rect.Width() -350);
	m_roomlist.GetHeaderCtrl()->EnableWindow(false);
}



void CWaitingForm::OnBnClickedButtonWatingLogout()
{
	m_pDlg->m_pClient->m_LogOut();
	m_pDlg->m_ShowForm(0);
}


void CWaitingForm::OnBnClickedButtonWatingExit()
{	
	if (AfxMessageBox(_T("프로그램을 종료하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		//TODO: 종료하기전에 서버에 종료 메시지 보내줘야함
		m_pDlg->m_pClient->m_CloseSocket();
		ExitProcess(0);

	}
}


void CWaitingForm::OnBnClickedButtonWatingMakeroom()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CMakeRoomDlg mr_dlg;
	mr_dlg.DoModal();
}



BOOL CWaitingForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
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

	m_pDlg->m_pClient->m_SendData();

}


void CWaitingForm::OnNMDblclkListWatingRoom(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMItemActivate->iItem != -1)
	{
		CString strroomnum = m_roomlist.GetItemText(pNMItemActivate->iItem, 0);
		Json::Value root;
		Json::StyledWriter writer;
		int roomnum = _ttoi(strroomnum);
	


		root["action"] = "enterroom";
		root["nickname"] = m_pDlg->m_pClient->m_getNickname();
		root["roomnum"] = roomnum;
		
		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();
		
		m_pDlg->m_pClient->m_SendData();
	}
	*pResult = 0;
}
