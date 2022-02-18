#pragma once
#include "afxdialogex.h"
#include "CFriendListForm.h"
#include "HSChatDlg.h"

// CFriendsListDlg 대화 상자

class CFriendsListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFriendsListDlg)

public:
	CFriendsListDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CFriendsListDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FRIENDSLIST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	CFriendListForm* m_pFriendsListForm;
	CHSChatDlg* m_pDlg;
	void m_ShowForm(int idx);
	void m_AllocForm();
	
	virtual BOOL OnInitDialog();
	void CFriendsListDlg::OnSysCommand(UINT nID, LPARAM lParam);
	virtual void PostNcDestroy();
	afx_msg void OnClose();
};

