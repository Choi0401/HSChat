﻿
// HSChatDlg.h: 헤더 파일
//
#pragma once
#include "CSigninForm.h"
#include "CSignupForm.h"
#include "CSearchIDForm.h"
#include "CSearchPWForm.h"
#include "CWaitingForm.h"
#include "CChatRoomForm.h"
#include "CClient.h"
#include "COpenSSL.h"

#define MESSAGE_SET_STATE 9999

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
	CChatRoomForm *m_pChatRoomForm;	

	CClient* m_pClient;
	COpenSSL* m_pOpenssl;

	void m_AllocForm();	
	void m_ShowForm(int idx);

	static UINT m_RecvThread(LPVOID _mothod);
	LRESULT m_SetState(WPARAM wParam, LPARAM lParam);


};