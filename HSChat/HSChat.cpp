
// HSChat.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "HSChat.h"
#include "HSChatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHSChatApp

BEGIN_MESSAGE_MAP(CHSChatApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHSChatApp 생성

CHSChatApp::CHSChatApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CHSChatApp 개체입니다.

CHSChatApp theApp;


// CHSChatApp 초기화

BOOL CHSChatApp::InitInstance()
{
	CWinApp::InitInstance();

	CHSChatDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}

