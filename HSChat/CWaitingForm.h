#pragma once
#include <afxext.h>
class CWaitingForm : public CFormView
{
	DECLARE_DYNAMIC(CWaitingForm)

public:
	CWaitingForm();
	CWaitingForm(UINT nIDTemplate);
	virtual ~CWaitingForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_WAITING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	afx_msg void OnBnClickedButtonLogout();

	CHSChatDlg* m_pDlg;
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonMakeroom();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};