#pragma once
// HSChatDlg.cpp: 구현 파일
//
#include "pch.h"
#include "framework.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "afxdialogex.h"
#include "json/json.h"
#include <locale.h>

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
	m_pFriendslistDlg = NULL;
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
	ON_COMMAND(ID_MENU_WHISPER, &CHSChatDlg::OnMenuWhisper)
	ON_COMMAND(ID_MENU_ASSIGN, &CHSChatDlg::OnMenuAssign)
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

			//TODO: 여기 다시한번확인해야함
			if (m_pClient->m_getNickname().length() != 0)
				m_pClient->m_LogOut();
			m_pClient->m_CloseSocket();
			Sleep(1);
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
				int ret_BodyRead = SSL_read(fir->m_pOpenssl->m_pSSL, &fir->m_pClient->m_data.msg[0], fir->m_pClient->m_data.size);
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
					//TODO: 클라이언트의 이름, 아이디를 클래스에 저장해야함
					m_pClient->m_setNickname(nickname);
					m_pClient->m_RequestAllList();
					AfxMessageBox(cstr, MB_ICONINFORMATION);
					m_ShowForm(4);

				}
				// 실패
				else if (result == "false")
				{
					AfxMessageBox(cstr, MB_ICONERROR);
					m_pSigninForm->GetDlgItem(IDC_EDIT_SIGNIN_ID)->SetFocus();
				}
			}
			// 채팅방 및 친구목록 
			else if (action == "alllist")
			{
				// parse json
				string result = recvroot["result"].asString();
				// 성공
				if (result == "true")
				{
					m_pWatingForm->m_roomlist.DeleteAllItems();
					m_pWatingForm->m_friendslist.DeleteAllItems();
					Json::Value roomlist = recvroot["roomlist"];
					Json::ValueIterator itroom;
					int roomcnt = recvroot["roomlist"].size();
					int i = 0;
					for (itroom = roomlist.begin(), i = 0; itroom != roomlist.end(); itroom++, i++)
					{
						if (itroom->isObject())
						{
							int roomnum = (*itroom)["roomnum"].asInt();
							int usernum = (*itroom)["usernum"].asInt();
							int maxusernum = (*itroom)["maxusernum"].asInt();
							string roomname = (*itroom)["roomname"].asString();
							//string roomtype = (*it)["roomnum"].asString();	타입은 필요없을듯?

							CString strRoomnum, strUsernum, strMaxusernum, strRoomname;
							strRoomnum.Format(_T("%d"), roomnum);
							strUsernum.Format(_T("%d"), usernum);
							strMaxusernum.Format(_T("%d"), maxusernum);
							strRoomname = roomname.c_str();
							m_pWatingForm->m_roomlist.InsertItem(i, strRoomnum);
							m_pWatingForm->m_roomlist.SetItemText(i, 1, strRoomname);
							m_pWatingForm->m_roomlist.SetItemText(i, 2, strUsernum + _T("/") + strMaxusernum);
						}
					}
					Json::Value friendslist = recvroot["friendslist"];
					Json::ValueIterator itfriends;
					int friendscnt = recvroot["friendslist"].size();
					for (itfriends = friendslist.begin(), i = 0; itfriends != friendslist.end(); itfriends++, i++)
					{
						if (itfriends->isObject())
						{
							string nickname = (*itfriends)["nickname"].asString();
							string fstate = (*itfriends)["fstate"].asString();

							CString strNickname;
							strNickname = nickname.c_str();
							if (fstate == "online")
							{
								m_pWatingForm->m_friendslist.InsertItem(i, strNickname);
							}
						}
					}

				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// 친구목록 버튼
			else if (action == "friendslist")
			{
				// parse json
				string result = recvroot["result"].asString();
				// 성공
				if (result == "true")
				{
					m_pFriendslistDlg->m_pFriendsListForm->m_friendslist.DeleteAllItems();
					Json::Value friendslist = recvroot["friendslist"];
					Json::ValueIterator itfriends;
					int i = 0;
					int friendscnt = recvroot["friendslist"].size();
					for (itfriends = friendslist.begin(), i = 0; itfriends != friendslist.end(); itfriends++, i++)
					{
						if (itfriends->isObject())
						{
							string nickname = (*itfriends)["nickname"].asString();
							string fstate = (*itfriends)["fstate"].asString();

							CString strNickname;
							strNickname = nickname.c_str();
							m_pFriendslistDlg->m_pFriendsListForm->m_friendslist.InsertItem(i, strNickname);
						}
					}

				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// 친구추가
			else if (action == "addfriend")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					AfxMessageBox(cstr);
					Json::Value root;
					Json::StyledWriter writer;
					root["action"] = "friendslist";
					root["nickname"] = m_pClient->m_getNickname();

					m_pClient->m_data.msg = writer.write(root);
					m_pClient->m_data.size = m_pClient->m_data.msg.size();
					m_pClient->m_SendData();

				}
				// 실패
				else if (result == "false")
				{

				}
			}

			// 친구삭제
			else if (action == "deletefriends")
			{
				// parse json
				string result = recvroot["result"].asString();
				string msg = recvroot["msg"].asString();
				CString cstr;
				cstr = msg.c_str();
				// 성공
				if (result == "true")
				{
					int nMark = m_pFriendslistDlg->m_pFriendsListForm->m_friendslist.GetSelectionMark();
					m_pFriendslistDlg->m_pFriendsListForm->m_friendslist.DeleteItem(nMark);
					AfxMessageBox(cstr);
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
					SetDlgItemText(IDC_EDIT_SIGNUP_NAME, _T(""));
					SetDlgItemText(IDC_EDIT_SIGNUP_PHONE, _T(""));
					SetDlgItemText(IDC_EDIT_SIGNUP_ID, _T(""));
					SetDlgItemText(IDC_EDIT_SIGNUP_NICKNAME, _T(""));
					SetDlgItemText(IDC_EDIT_SIGNUP_PW, _T(""));
					SetDlgItemText(IDC_EDIT_SIGNUP_PWOK, _T(""));
					AfxMessageBox(cstr, MB_ICONINFORMATION);
				}
				// 실패
				else if (result == "false")
				{
					AfxMessageBox(cstr, MB_ICONERROR);
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
					AfxMessageBox(cstr, MB_ICONERROR);
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
					AfxMessageBox(cstr, MB_ICONERROR);
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
					//TODO : 리스트 지워야함
					m_pClient->m_ismaster = true;
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
					AfxMessageBox(cstr, MB_ICONERROR);
				}
			}
			// 채팅방 입장 
			else if (action == "enterroom")
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
					string master = recvroot["master"].asString();
					m_pClient->m_roomnum = roomnum;
					m_pClient->m_ismaster = false;
					CString str;
					str.Format(_T("[%d번]채팅방"), roomnum);
					m_pChatRoomForm->GetDlgItem(IDC_STATIC_CHATROOM)->SetWindowText(str);
					m_pChatRoomForm->GetDlgItem(IDC_EDIT_CHATROOM_RECVMSG)->SetWindowText(_T("                         --- 채팅에 참여했습니다 ---\r\n\r\n"));

					m_pChatRoomForm->m_roomuserlist.DeleteAllItems();
					Json::Value userlist = recvroot["userlist"];
					Json::ValueIterator ituser;
					int i = 0;
					int usercnt = recvroot["userlist"].size();
					for (ituser = userlist.begin(), i = 0; ituser != userlist.end(); ituser++, i++)
					{
						if (ituser->isObject())
						{
							CString strNickname;
							string nickname = (*ituser)["nickname"].asString();
							if (master == nickname)
							{
								nickname += "(방장)";
								strNickname = nickname.c_str();
								m_pChatRoomForm->m_roomuserlist.InsertItem(0, strNickname);
							}
							else
							{
								strNickname = nickname.c_str();
								m_pChatRoomForm->m_roomuserlist.InsertItem(i, strNickname);
							}
						}
					}

					m_ShowForm(6);
				}
				// 실패
				else if (result == "false")
				{

				}
			}
			// 채팅 출력
			else if (action == "recvmsg")
			{
				// parse json
				string nickname = recvroot["nickname"].asString();
				string time = recvroot["time"].asString();
				string msg = recvroot["msg"].asString();

				string tmpstr = "[" + time + "]" + nickname + " : " + msg + " \r\n";

				// 채팅창 길이
				int chatlength = m_pChatRoomForm->m_chat.GetWindowTextLength();
				// 마지막 줄 선택
				m_pChatRoomForm->m_chat.SetSel(chatlength, chatlength);
				// 선택된 행의 텍스트를 교체
				CString strmsg;
				strmsg = tmpstr.c_str();
				m_pChatRoomForm->m_chat.ReplaceSel(strmsg);

			}
			// 채팅 출력
			else if (action == "assignmaster")
			{
				// parse json
				string result = recvroot["true"].asString();
				string msg = recvroot["msg"].asString();

				CString strmsg;
				strmsg = msg.c_str();
				if (result == "true")
					m_pClient->m_ismaster = false;

				//TODO : 유저리스트 재정렬 and 방장 바뀐것 출력
				AfxMessageBox(strmsg);
				string tmp = "[공지]" + m_pClient->m_getNickname() + "님이 " + msg + "\r\n";
				CString str;
				str = tmp.c_str();
				int chatlength = m_pChatRoomForm->m_chat.GetWindowTextLength();
				m_pChatRoomForm->m_chat.SetSel(chatlength, chatlength);
				m_pChatRoomForm->m_chat.ReplaceSel(str);

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

void CHSChatDlg::OnMenuWhisper()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.	
	//int m_pChatRoomForm->m_cbchat.GetCount();
	int flag = -1;

	for (int i = 0; i <= m_pChatRoomForm->m_cntcb; i++)
	{
		CString tmpstr;
		m_pChatRoomForm->m_cbchat.GetLBText(i, tmpstr);
		if (tmpstr == m_pChatRoomForm->m_selUser)
		{
			m_pChatRoomForm->m_cbchat.SetCurSel(i);
			flag = 0;
			break;
		}
	}
	if (flag == -1)
	{		
		m_pChatRoomForm->m_cbchat.InsertString(m_pChatRoomForm->m_cntcb, m_pChatRoomForm->m_selUser);
		m_pChatRoomForm->m_cbchat.SetCurSel(m_pChatRoomForm->m_cntcb);
		m_pChatRoomForm->m_cntcb++;
	}
}


void CHSChatDlg::OnMenuAssign()
{
	Json::Value root;
	Json::StyledWriter writer;

	root["action"] = "assignmaster";
	root["nickname"] = std::string(CT2CA(m_pChatRoomForm->m_selUser));
	root["roomnum"] = m_pClient->m_roomnum;

	m_pClient->m_data.msg = writer.write(root);
	m_pClient->m_data.size = m_pClient->m_data.msg.size();

	m_pClient->m_SendData();
	
}
