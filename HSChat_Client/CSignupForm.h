#pragma once
#include <afxext.h>
class CSignupForm : public CFormView
{
	DECLARE_DYNAMIC(CSignupForm)

public:
	CSignupForm();
	CSignupForm(UINT nIDTemplate);
	virtual ~CSignupForm();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_SIGNUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();

	CHSChatDlg* m_pDlg;	
	afx_msg void OnBnClickedButtonSignupOK();
	afx_msg void OnBnClickedButtonSignupCancel();
};

