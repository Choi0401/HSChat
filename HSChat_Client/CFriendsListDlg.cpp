// CFriendsListDlg.cpp: 구현 파일
//

#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "CFriendsListDlg.h"


// CFriendsListDlg 대화 상자

IMPLEMENT_DYNAMIC(CFriendsListDlg, CDialogEx)

CFriendsListDlg::CFriendsListDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FRIENDSLIST, pParent)
{
	m_pFriendsListForm = NULL;
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CFriendsListDlg::~CFriendsListDlg()
{
}

void CFriendsListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFriendsListDlg, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CFriendsListDlg 메시지 처리기

void CFriendsListDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_CLOSE)
	{	
		DestroyWindow();
		delete m_pDlg->m_pFriendslistDlg;
		m_pDlg->m_pFriendslistDlg = NULL;
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



BOOL CFriendsListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_AllocForm();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CFriendsListDlg::m_ShowForm(int idx)
{
	switch (idx)
	{
	case 0:		// 친구 추가화면
		m_pFriendsListForm->ShowWindow(SW_SHOW);

		break;

	case 1:		// 친구 초대화면
		m_pFriendsListForm->ShowWindow(SW_HIDE);

		break;
	}
}
void CFriendsListDlg::m_AllocForm()
{
		CCreateContext context;
		ZeroMemory(&context, sizeof(context));

		CRect rectOfPanelArea;

		GetDlgItem(IDC_PICTURE_FRIENDSLIST)->GetWindowRect(&rectOfPanelArea);
		ScreenToClient(&rectOfPanelArea);
		m_pFriendsListForm = new CFriendListForm();
		m_pFriendsListForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_SIGNIN, &context);
		m_pFriendsListForm->OnInitialUpdate();
		m_pFriendsListForm->ShowWindow(SW_SHOW);
}

void CFriendsListDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	DestroyWindow();
	CDialogEx::OnClose();
}


void CFriendsListDlg::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pDlg->m_pFriendslistDlg = NULL;
	delete m_pDlg->m_pFriendslistDlg;
	CDialogEx::PostNcDestroy();
}


