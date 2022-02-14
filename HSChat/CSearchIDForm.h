#pragma once
#include <afxext.h>
class CSearchIDForm : public CFormView
{
	DECLARE_DYNAMIC(CSearchIDForm)

public:
	CSearchIDForm();
	CSearchIDForm(UINT nIDTemplate);
	virtual ~CSearchIDForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_SEARCHIDs };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();


	CHSChatDlg* m_pDlg;

	afx_msg void OnBnClickedButtonSearchIDOK();
	afx_msg void OnBnClickedButtonSearchIDCancel();
};

