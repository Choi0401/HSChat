#pragma once
#include "afxdialogex.h"


// CMakeRoomDlg 대화 상자

class CMakeRoomDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMakeRoomDlg)

public:
	CMakeRoomDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMakeRoomDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MAKEROOM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	CButton* m_pRB_public;
	CHSChatDlg* m_pDlg;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonMakeroomCancel();
	afx_msg void OnBnClickedButtonMakeroom();
};
