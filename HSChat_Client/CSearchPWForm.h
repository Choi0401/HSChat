#pragma once
#include <afxext.h>
class CSearchPWForm : public CFormView
{
	DECLARE_DYNAMIC(CSearchPWForm)

public:
	CSearchPWForm();
	CSearchPWForm(UINT nIDTemplate);
	virtual ~CSearchPWForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_SEARCHPW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	

	CHSChatDlg* m_pDlg;
	afx_msg void OnBnClickedButtonSearchPWOK();
	afx_msg void OnBnClickedButtonSearchPWCancel();
};

