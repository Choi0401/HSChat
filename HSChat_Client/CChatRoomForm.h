#pragma once

class CHSChatDlg;

// CChatRoomForm 폼 보기

class CChatRoomForm : public CFormView
{
	DECLARE_DYNCREATE(CChatRoomForm)

	CChatRoomForm();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CChatRoomForm();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FORMVIEW_CHATROOM };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	void OnInitialUpdate();
	CHSChatDlg* m_pDlg;

	afx_msg void OnBnClickedButtonChatroomSend();	
	afx_msg void OnBnClickedButtonChatroomQuit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CEdit m_chat;
	CEdit m_sendmsg;
	CListCtrl m_roomuserlist;
	CMenu m_menuuser, * m_submenuuser,m_menuchat;
	CString m_selUser;
	int m_cntcb;
	afx_msg void OnNMRClickListChatroomUserlist(NMHDR* pNMHDR, LRESULT* pResult);
	CComboBox m_cbchat;

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


