
// HSChatDlg.h: 헤더 파일
//
#pragma once
#include "CSigninForm.h"
#include "CSignupForm.h"
#include "CSearchIDForm.h"
#include "CSearchPWForm.h"
#include "CWaitingForm.h"
#include "CMyInfoForm.h"
#include "CChatRoomForm.h"
#include "CClient.h"
#include "COpenSSL.h"
#include "CFriendsListDlg.h"
#include "CChangePWDlg.h"
#include "json/json.h"
#include <codecvt>
#include <iostream>
#include <map>

#define MESSAGE_SET_STATE		9999
#define MESSAGE_PROC			9998

#define RECVMSG					5000
#define SIGNUP					5001	
#define SIGNIN					5002
#define SEARCHID				5003
#define SEARCHPW				5004
#define ALLLIST					5005
#define CREATEROOM				5006
#define ENTERROOM				5007
#define UPDATEUSERLIST			5008
#define FRIENDSLIST				5017
#define ADDFRIEND				5010
#define DELETEFRIEND			5011
#define SHOWMYINFO				5012
#define CHANGEMYINFO			5013
#define DELETEACCOUNT			5014
#define SETNEWPW				5015
#define LOGOUT					5016



// CHSChatDlg 대화 상자
class CHSChatDlg : public CDialogEx
{
// 생성입니다.
public:
	CHSChatDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HSCHAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CSigninForm *m_pSigninForm;
	CSignupForm* m_pSignupForm;
	CSearchIDForm* m_pSearchIDForm;
	CSearchPWForm* m_pSearchPWForm;
	CWaitingForm *m_pWatingForm;
	CMyInfoForm* m_pMyInfomForm;
	CChatRoomForm *m_pChatRoomForm;	

	CFriendsListDlg* m_pFriendslistDlg;
	CChangePWDlg m_changpwdlg;

	CClient* m_pClient;
	COpenSSL* m_pOpenssl;

	Json::Value m_recvroot;
	Json::Reader m_reader;


	/* 화면 관련 함수 */
	void m_AllocForm();	
	void m_ShowForm(int idx);
	LRESULT m_SetState(WPARAM wParam, LPARAM lParam);
	/* 쓰레드 관련 함수 */
	static UINT m_RecvThread(LPVOID _mothod);
	LRESULT m_Proc(WPARAM wParam, LPARAM lParam);

	/* 로그 관련 함수 */
	void m_ClearFileLog(const char* pszFileName);
	void m_FileLog(const char* pszFileName, const char* pszLog, ...);

	/* 채팅 관련 함수 */
	afx_msg void m_OnMenuWhisper();
	afx_msg void m_OnMenuAssign();

	void m_Wait(DWORD dwMillisecond);

	/* 인코딩 관련 함수 */
	char* UTF8ToANSI(const char* pszCode);
	string MultiByteToUtf8(string multibyte_str);

	/* 패스워드 암호화 관련 함수 */
	bool pw_check(string pw);
	string sha256(string pw);
	string pw_salting(string pw);

	/* map */
	map<string, int> m_map;
	void m_InitMap();

	/* action 관련 함수 */
	void m_Signup();
	void m_Signin();
	void m_SearchID();
	void m_SearchPW();
	void m_AllList();
	void m_CreateRoom();
	void m_EnterRoom();
	void m_UpdateUser();
	void m_FriendsList();
	void m_AddFriend();
	void m_DeleteFriend();
	void m_ShowMyInfo();
	void m_ChangeMyInfo();
	void m_SetNewPW();
	void m_DeleteAccount();
	void m_RecvMSG();
	void m_Logout();
};
