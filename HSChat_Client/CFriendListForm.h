#pragma once
#include <afxext.h>
class CFriendListForm : public CFormView
{
	DECLARE_DYNAMIC(CFriendListForm)
public:
	CFriendListForm();
	CFriendListForm(UINT nIDTemplate);
	virtual ~CFriendListForm();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_FRIENDS_LIST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();

	CHSChatDlg* m_pDlg;

	CListCtrl m_friendslist;
	afx_msg void OnBnClickedButtonFriendsAdd();
	afx_msg void OnBnClickedButtonFriendsDelete();

	BOOL m_bAscending;
	struct SORTPARAM
	{
		int iSortColumn;
		bool bSortDirect;
		CListCtrl* pList;
		int flag = -1;		// Ŭ���� header�� ���� ������ ���� �ٸ��� ������ �������ֱ����� ����
	};

	afx_msg void OnHdnItemclickList(NMHDR* pNMHDR, LRESULT* pResult);
	static int CALLBACK CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


};

