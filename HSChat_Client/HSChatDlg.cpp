#pragma once
// HSChatDlg.cpp: 구현 파일
//
#include "pch.h"
#include "framework.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "json/json.h"
#include <chrono>
#include <thread>
#include <locale.h>

using std::this_thread::sleep_for;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	
// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHSChatDlg 대화 상자



CHSChatDlg::CHSChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HSCHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pSigninForm = NULL;
	m_pSignupForm = NULL;
	m_pSearchIDForm = NULL;
	m_pSearchPWForm = NULL;
	m_pWatingForm = NULL;	
	m_pMyInfomForm = NULL;
	m_pChatRoomForm = NULL;
	m_pClient = new CClient();
	m_pOpenssl = new COpenSSL();
}

void CHSChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);	
}

BEGIN_MESSAGE_MAP(CHSChatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()	
	ON_WM_SIZE()
	ON_MESSAGE(MESSAGE_SET_STATE, &CHSChatDlg::m_SetState)
	ON_MESSAGE(MESSAGE_PROC, &CHSChatDlg::m_Proc)
END_MESSAGE_MAP()


// CHSChatDlg 메시지 처리기

BOOL CHSChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	setlocale(LC_ALL, "");

	CWinThread* pRecvthread = NULL;
	m_pClient->m_InitSocket();
	m_pOpenssl->m_InitCTX();
	if (m_pOpenssl->m_CheckCertKey() == 0)
		AfxMessageBox(_T("m_CheckCertKey() Error"));

	pRecvthread = AfxBeginThread(m_RecvThread, this);

	if (pRecvthread == NULL)
	{
		AfxMessageBox(_T("AfxBeginThread() Error"));
		exit(1);
	}


	m_AllocForm();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CHSChatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE)
	{
		if (AfxMessageBox(_T("프로그램을 종료하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
		{

			//TODO: 종료하기전에 서버에 종료 메시지 보내줘야함
			m_pClient->m_CloseSocket();
			ExitProcess(0);
			
		}
		else 
		{
			;
		}
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CHSChatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CHSChatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHSChatDlg::m_AllocForm()
{
	CCreateContext context;
	ZeroMemory(&context, sizeof(context));

	CRect rectOfPanelArea;

	GetDlgItem(IDC_PICTURE_CONTROL)->GetWindowRect(&rectOfPanelArea);
	ScreenToClient(&rectOfPanelArea);
	m_pSigninForm = new CSigninForm();
	m_pSigninForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_SIGNIN, &context);
	m_pSigninForm->OnInitialUpdate();
	m_pSigninForm->ShowWindow(SW_SHOW);

	m_pSignupForm = new CSignupForm();
	m_pSignupForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_SIGNUP, &context);
	m_pSignupForm->OnInitialUpdate();
	m_pSignupForm->ShowWindow(SW_HIDE);

	m_pSearchIDForm = new CSearchIDForm();
	m_pSearchIDForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_SEARCHID, &context);
	m_pSearchIDForm->OnInitialUpdate();
	m_pSearchIDForm->ShowWindow(SW_HIDE);

	m_pSearchPWForm = new CSearchPWForm();
	m_pSearchPWForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_SEARCHPW, &context);
	m_pSearchPWForm->OnInitialUpdate();
	m_pSearchPWForm->ShowWindow(SW_HIDE);

	m_pWatingForm = new CWaitingForm();
	m_pWatingForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_WAITING, &context);
	m_pWatingForm->OnInitialUpdate();
	m_pWatingForm->ShowWindow(SW_HIDE);

	m_pMyInfomForm = new CMyInfoForm();
	m_pMyInfomForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_MYINFO, &context);
	m_pMyInfomForm->OnInitialUpdate();
	m_pMyInfomForm->ShowWindow(SW_HIDE);

	m_pChatRoomForm = new CChatRoomForm();
	m_pChatRoomForm->Create(NULL, NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL, rectOfPanelArea, this, IDD_FORMVIEW_CHATROOM, &context);
	m_pChatRoomForm->OnInitialUpdate();
	m_pChatRoomForm->ShowWindow(SW_HIDE);

	



	GetDlgItem(IDC_PICTURE_CONTROL)->DestroyWindow();
}


void CHSChatDlg::m_ShowForm(int idx)
{
	switch (idx)
	{
	case 0:		// 로그인 화면
		m_pSigninForm->ShowWindow(SW_SHOW);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_HIDE);
		m_pSigninForm->GetDlgItem(IDC_EDIT_SIGNIN_ID)->SetFocus();
		break;

	case 1:		// 회원가입 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_SHOW);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_HIDE);		
		m_pSignupForm->GetDlgItem(IDC_EDIT_SIGNUP_NAME)->SetFocus();
		break;


	case 2:		// ID찾기 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_SHOW);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->GetDlgItem(IDC_EDIT_SEARCHID_NAME)->SetFocus();
		break;

	case 3:		// PW찾기 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_SHOW);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->GetDlgItem(IDC_EDIT_SEARCHPW_NAME)->SetFocus();
		break;

	case 4:		// 대기실 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_SHOW);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_HIDE);
		m_pWatingForm->GetDlgItem(IDC_BUTTON_WATING_MAKEROOM)->SetFocus();
		break;

	case 5:		// 내정보 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_SHOW);
		m_pChatRoomForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->GetDlgItem(IDC_EDIT_MYINFO_PHONE)->SetFocus();
		break;

	case 6:		// 채팅방 화면
		m_pSigninForm->ShowWindow(SW_HIDE);
		m_pSignupForm->ShowWindow(SW_HIDE);
		m_pSearchIDForm->ShowWindow(SW_HIDE);
		m_pSearchPWForm->ShowWindow(SW_HIDE);
		m_pWatingForm->ShowWindow(SW_HIDE);
		m_pMyInfomForm->ShowWindow(SW_HIDE);
		m_pChatRoomForm->ShowWindow(SW_SHOW);
		m_pChatRoomForm->GetDlgItem(IDC_EDIT_CHATROOM_SENDMSG)->SetFocus();
		/*CEdit* p_EditSend = NULL;
		if ((p_EditSend = (CEdit*)m_pChatRoomForm->GetDlgItem(IDC_EDIT_CHATROOM_SENDMSG)) == NULL)
		{
			AfxMessageBox(_T("ERROR[GetDlgItem()] : Failed to get IDC_EDIT_CHATROOM_SENDMSG"));
		}
		p_EditSend->SetSel(-1);
		p_EditSend->SetFocus();*/
		break;

	}

}

UINT CHSChatDlg::m_RecvThread(LPVOID _mothod)
{
	CHSChatDlg* fir = (CHSChatDlg*)_mothod;	
	while (1)
	{				
		// Connect 시도
		if (fir->m_pClient->m_connstate == CLIENT_DISCONNECTED) {
			fir->m_pClient->m_OpenConnection();
			
			if (fir->m_pClient->m_connstate == CLIENT_CONNECTED)
			{
				fir->m_pOpenssl->m_pSSL = SSL_new(fir->m_pOpenssl->m_pCTX);
				SSL_set_fd(fir->m_pOpenssl->m_pSSL, fir->m_pClient->m_socket);
				if (SSL_connect(fir->m_pOpenssl->m_pSSL) == -1)
				{
					fir->m_pClient->m_connstate = CLIENT_DISCONNECTED;
					//AfxMessageBox(_T("SSL_connect() Error"));
				}
				else
				{
					::PostMessage(fir->m_hWnd, MESSAGE_SET_STATE, NULL, NULL);
				}
			}
		}
		else 
		{									
			// Success Connect 			
			int ret_HeadRead = 0;			
			ret_HeadRead = SSL_read(fir->m_pOpenssl->m_pSSL, &fir->m_pClient->m_data.size, sizeof(int));
						
			if (ret_HeadRead > 0)
			{
				fir->m_pClient->m_data.msg.resize(fir->m_pClient->m_data.size);
				int ret_BodyRead = SSL_read(fir->m_pOpenssl->m_pSSL,&fir->m_pClient->m_data.msg[0], fir->m_pClient->m_data.size);

				//TODO : 큐가 최대일 때 처리, Locking
				fir->m_pClient->m_queue.push(fir->m_pClient->m_data.msg);
				::PostMessage(fir->m_hWnd, MESSAGE_PROC, NULL, NULL);					
			}
			else {								
				fir->m_pOpenssl->m_pSSL = NULL;
				fir->m_pClient->m_CloseSocket();
				fir->m_pClient->m_InitSocket();
				fir->m_pOpenssl->m_InitCTX();
				//AfxMessageBox(_T("서버와 연결이 끊어졌습니다."), MB_ICONERROR);				
				::PostMessage(fir->m_hWnd, MESSAGE_SET_STATE, NULL, NULL);
				fir->m_ShowForm(0);

			}			
			fir->m_pClient->m_InitData();
		}
	}	
}

LRESULT CHSChatDlg::m_Proc(WPARAM wParam, LPARAM lParam)
{
	if (!m_pClient->m_queue.empty())
	{
		string recvstr;
		//recvstr = m_pClient->m_queue.front();
		recvstr = m_pClient->m_queue.pop();
		Json::Value recvroot;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse(recvstr, recvroot);
		if (parsingSuccessful == false)
		{
			//AfxMessageBox(_T("Failed to parse configuration")); // reader.getFormatedErrorMessages();
			exit(1);
		}

		else {
			string action = recvroot["action"].asString();
			// 로그인 
			if (action == "signin")
			{
				// parse json
				string result = recvroot["result"].asString();
				string nickname = recvroot["nickname"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					m_ShowForm(4);
					//fir->m_pSigninForm->SetDlgItemText(IDC_EDIT_SIGNIN_ID, _T(""));
					//fir->m_pSigninForm->SetDlgItemText(IDC_EDIT_SIGNIN_PW, _T(""));
					AfxMessageBox(cstr, MB_ICONINFORMATION);
					//TODO: 클라이언트의 이름, 아이디를 클래스에 저장해야함
					m_pClient->m_setNickname(nickname);
					//TODO: 서버에 방, 친구 목록 요청해야함
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// 회원가입 
			else if (action == "signup")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					m_ShowForm(0);
					AfxMessageBox(cstr, MB_ICONINFORMATION);
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// ID 찾기 
			else if (action == "searchid")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					string id = recvroot["id"].asString();
					m_ShowForm(0);
					AfxMessageBox(cstr, MB_ICONINFORMATION);
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// PW 찾기 
			else if (action == "searchpw")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					string pw = recvroot["pw"].asString();
					m_ShowForm(0);
					AfxMessageBox(cstr, MB_ICONINFORMATION);
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			else if (action == "showmyinfo")
			{
				// parse json
				string result = recvroot["result"].asString();				
				// 성공
				if (result == "true")
				{
					CString cname, cid, cbirth;
					string name = recvroot["name"].asString();
					string id = recvroot["id"].asString();
					string birth = recvroot["birth"].asString();
					cname = name.c_str();
					cid = id.c_str();
					cbirth = birth.c_str();
					m_pMyInfomForm->SetDlgItemText(IDC_EDIT_MYINFO_NAME, cname);
					m_pMyInfomForm->SetDlgItemText(IDC_EDIT_MYINFO_ID, cid);
					m_pMyInfomForm->SetDlgItemText(IDC_EDIT_MYINFO_BIRTH, cbirth);
					m_ShowForm(5);

					
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			else if (action == "changemyinfo")
			{
				// parse json
				string result = recvroot["result"].asString();				
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{					
					m_ShowForm(4);
					AfxMessageBox(cstr, MB_ICONINFORMATION);					
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			else if (action == "deleteaccount")
			{
			// parse json
			string result = recvroot["result"].asString();
			string msg = recvroot["msg"].asString();
			CString cstr;
			cstr = msg.c_str();
			// 성공
			if (result == "true")
			{
				m_ShowForm(0);
				AfxMessageBox(cstr, MB_ICONINFORMATION);
			}
			// 실패
			else if (result == "false")
			{

			}
			}
			// 채팅방 만들기 
			else if (action == "createroom")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					int roomnum = recvroot["roomnum"].asInt();
					m_pClient->m_roomnum = roomnum;
					CString str;
					str.Format(_T("[%d번]채팅방"), roomnum);
					m_pChatRoomForm->GetDlgItem(IDC_STATIC_CHATROOM)->SetWindowText(str);
					m_ShowForm(6);
					//AfxMessageBox(cstr, MB_ICONINFORMATION);
				}
				// 실패
				else if (result == "false")
				{

				}
			}


		}		
	}

	return 0;
}


LRESULT CHSChatDlg::m_SetState(WPARAM wParam, LPARAM lParam)
{
	if (m_pClient->m_connstate == CLIENT_DISCONNECTED)
	{
		AfxMessageBox(_T("서버와 연결이 끊어졌습니다."), MB_ICONERROR);
		m_pSigninForm->GetDlgItem(IDC_STATIC_SIGNIN_STATE)->SetWindowText(_T("서버에 접속중입니다..."));
	}
	else
		m_pSigninForm->GetDlgItem(IDC_STATIC_SIGNIN_STATE)->SetWindowText(_T("환영합니다. 서버에 접속하셨습니다."));

	return 0;
}


void CHSChatDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 창 크기 변화할 때 호출

}