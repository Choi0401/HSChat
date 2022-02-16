#pragma once

class CHSChatDlg;

class CSigninForm : public CFormView
{
	DECLARE_DYNAMIC(CSigninForm)

public:
	CSigninForm();
	CSigninForm(UINT nIDTemplate);
	virtual ~CSigninForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_SIGNIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();

	CHSChatDlg* m_pDlg;
	afx_msg void OnBnClickedButtonSigninSignup();
	afx_msg void OnBnClickedButtonSigninSignin();
	afx_msg void OnBnClickedButtonSigninSearchID();
	afx_msg void OnBnClickedButtonSigninSearchPW();
};