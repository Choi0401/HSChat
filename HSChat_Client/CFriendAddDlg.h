#pragma once
#include "afxdialogex.h"
#include "HSChatDlg.h"

// CFriendAddDlg 대화 상자

class CFriendAddDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFriendAddDlg)

public:
	CFriendAddDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CFriendAddDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FRIENDADD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CHSChatDlg* m_pDlg;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonFriendAddOK();
	afx_msg void OnBnClickedButtonFriendaddCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
