#pragma once
#include <afxext.h>
class CMyInfoForm : public CFormView
{
	DECLARE_DYNAMIC(CMyInfoForm)

public:
	CMyInfoForm();
	CMyInfoForm(UINT nIDTemplate);
	virtual ~CMyInfoForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_MYINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();

	CHSChatDlg* m_pDlg;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonMyInfoOK();
	afx_msg void OnBnClickedButtonMyInfoCancel();
	afx_msg void OnBnClickedButtonMyInfoDelete();
};


