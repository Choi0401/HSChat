#pragma once
#include "afxdialogex.h"
#include "HSChatDlg.h"

// CChangePWDlg 대화 상자

class CChangePWDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChangePWDlg)

public:
	CChangePWDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CChangePWDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHANGEPW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	CHSChatDlg* m_pDlg;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonChagnepwOk();
};
