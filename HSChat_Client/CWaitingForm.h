#pragma once
#include <afxext.h>
class CWaitingForm : public CFormView
{
	DECLARE_DYNAMIC(CWaitingForm)

public:
	CWaitingForm();
	virtual ~CWaitingForm();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_WAITING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void OnInitialUpdate();
	CHSChatDlg* m_pDlg;

	afx_msg void OnBnClickedButtonWatingLogout();	
	afx_msg void OnBnClickedButtonWatingExit();
	afx_msg void OnBnClickedButtonWatingMakeroom();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonWatingMyinfo();
	CListCtrl m_roomlist;
	afx_msg void OnNMDblclkListWatingRoom(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl m_friendslist;
	afx_msg void OnBnClickedButtonWatingFriends();

	BOOL m_bAscending;
	struct SORTPARAM
	{
		int iSortColumn;
		bool bSortDirect;
		CListCtrl* pList;
		int flag = -1;		// 클릭한 header에 따라서 정렬할 값이 다르기 때문에 구분해주기위한 변수
	};

	afx_msg void OnHdnItemclickList(NMHDR* pNMHDR, LRESULT* pResult);
	static int CALLBACK CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

};