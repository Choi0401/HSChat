#pragma once
#include <afxext.h>
class CSearchPWForm : public CFormView
{
	DECLARE_DYNAMIC(CSearchPWForm)

public:
	CSearchPWForm();
	CSearchPWForm(UINT nIDTemplate);
	virtual ~CSearchPWForm();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_SEARCHPW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	

	CHSChatDlg* m_pDlg;
	afx_msg void OnBnClickedButtonSearchPWOK();
	afx_msg void OnBnClickedButtonSearchPWCancel();
};

