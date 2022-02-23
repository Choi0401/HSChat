#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CWaitingForm.h"
#include "CMakeRoomDlg.h"
#include "json/json.h"

// CSigninForm ��ȭ ����

IMPLEMENT_DYNAMIC(CWaitingForm, CFormView)

CWaitingForm::CWaitingForm() : CFormView(IDD_FORMVIEW_WAITING)
{
	m_bAscending = false;
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();	
}


CWaitingForm::~CWaitingForm()
{
}

void CWaitingForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_WATING_ROOM, m_roomlist);
	DDX_Control(pDX, IDC_LIST_WAITING_FRIENDS, m_friendslist);
}


BEGIN_MESSAGE_MAP(CWaitingForm, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_WATING_LOGOUT, &CWaitingForm::OnBnClickedButtonWatingLogout)
	ON_BN_CLICKED(IDC_BUTTON_WATING_EXIT, &CWaitingForm::OnBnClickedButtonWatingExit)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MAKEROOM, &CWaitingForm::OnBnClickedButtonWatingMakeroom)
	ON_BN_CLICKED(IDC_BUTTON_WATING_MYINFO, &CWaitingForm::OnBnClickedButtonWatingMyinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_WATING_ROOM, &CWaitingForm::OnNMDblclkListWatingRoom)
	ON_BN_CLICKED(IDC_BUTTON_WATING_FRIENDS, &CWaitingForm::OnBnClickedButtonWatingFriends)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CWaitingForm::OnHdnItemclickList)

END_MESSAGE_MAP()


// CMyForm �޽��� ó����


BOOL CWaitingForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CWaitingForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.		
	CRect rect;
	m_roomlist.GetClientRect(&rect);
	m_roomlist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_roomlist.InsertColumn(0, _T("��ȣ"), LVCFMT_CENTER, 45);
	m_roomlist.InsertColumn(1, _T("�̸�"), LVCFMT_LEFT, 330);
	m_roomlist.InsertColumn(2, _T("�ο�"), LVCFMT_RIGHT, rect.Width() - 375);
	//m_roomlist.GetHeaderCtrl()->EnableWindow(false);

	m_friendslist.GetClientRect(&rect);
	m_friendslist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_friendslist.InsertColumn(0, _T("�¶���"), LVCFMT_LEFT, rect.Width());
	//m_friendslist.GetHeaderCtrl()->EnableWindow(false);

}



void CWaitingForm::OnBnClickedButtonWatingLogout()
{
	m_pDlg->m_pClient->m_LogOut();
	m_pDlg->m_ShowForm(0);
}


void CWaitingForm::OnBnClickedButtonWatingExit()
{	
	if (AfxMessageBox(_T("���α׷��� �����Ͻðڽ��ϱ�?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		//TODO: �����ϱ����� ������ ���� �޽��� ���������
		m_pDlg->m_pClient->m_CloseSocket();
		ExitProcess(0);

	}
}


void CWaitingForm::OnBnClickedButtonWatingMakeroom()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CMakeRoomDlg mr_dlg;
	mr_dlg.DoModal();
}



BOOL CWaitingForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{

		return TRUE;
	}

	return CFormView::PreTranslateMessage(pMsg);
}


void CWaitingForm::OnBnClickedButtonWatingMyinfo()
{

	Json::Value root;
	Json::StyledWriter writer;
	
	root["action"] = "showmyinfo";
	root["nickname"] = m_pDlg->m_pClient->m_getNickname();

	m_pDlg->m_pClient->m_data.msg = writer.write(root);
	m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());

	m_pDlg->m_pClient->m_SendData();

}


void CWaitingForm::OnNMDblclkListWatingRoom(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (pNMItemActivate->iItem != -1)
	{
		CString strroomnum = m_roomlist.GetItemText(pNMItemActivate->iItem, 0);
		Json::Value root;
		Json::StyledWriter writer;
		int roomnum = _ttoi(strroomnum);

		root["action"] = "enterroom";
		root["nickname"] = m_pDlg->MultiByteToUtf8(m_pDlg->m_pClient->m_getNickname());
		root["roomnum"] = roomnum;

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());

		m_pDlg->m_pClient->m_SendData();
	}
	*pResult = 0;
}


void CWaitingForm::OnBnClickedButtonWatingFriends()
{
	if (m_pDlg->m_pFriendslistDlg == NULL) {
		Json::Value root;
		Json::StyledWriter writer;
		m_pDlg->m_pFriendslistDlg = new CFriendsListDlg();		
		root["action"] = "friendslist";
		root["nickname"] = m_pDlg->m_pClient->m_getNickname();

		m_pDlg->m_pClient->m_data.msg = writer.write(root);
		m_pDlg->m_pClient->m_data.size = static_cast<int>(m_pDlg->m_pClient->m_data.msg.size());
		m_pDlg->m_pClient->m_SendData();
		m_pDlg->m_pFriendslistDlg->Create(IDD_DIALOG_FRIENDSLIST);

	}
}

// ����Ʈ��Ʈ�� �÷� Ŭ���� ������ ����
void CWaitingForm::OnHdnItemclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);


	if (pNMLV->hdr.hwndFrom == m_roomlist.GetDlgItem(0)->GetSafeHwnd())
	{
		// Ŭ���� �÷��� �ε���
		int nColumn = pNMLV->iItem;

		// �� ����Ʈ ��Ʈ�ѿ� �ִ� ������ �� �ڷ� ������ŭ ����
		for (int i = 0; i < (m_roomlist.GetItemCount()); i++) {
			m_roomlist.SetItemData(i, i);
		}

		// ���� ��� ����
		m_bAscending = !m_bAscending;

		// ���� ���� ����ü ���� ���� �� ������ �ʱ�ȭ
		SORTPARAM sortparams;
		sortparams.pList = &m_roomlist;
		sortparams.iSortColumn = nColumn;
		sortparams.bSortDirect = m_bAscending;

		// |    0	 |     1    | 
		// | Roomnum | Roomname | 

		// Roomnum ����
		if (nColumn == 0)
			sortparams.flag = 0;

		// Roomname�� ���ĺ� ����
		if (nColumn == 1)
			sortparams.flag = 1;

		// ���� �Լ� ȣ��
		m_roomlist.SortItems(&CompareItem, (LPARAM)&sortparams);
	}
	else if(pNMLV->hdr.hwndFrom == m_friendslist.GetDlgItem(0)->GetSafeHwnd())
	{			
		// Ŭ���� �÷��� �ε���
		int nColumn = pNMLV->iItem;

		// �� ����Ʈ ��Ʈ�ѿ� �ִ� ������ �� �ڷ� ������ŭ ����
		for (int i = 0; i < (m_friendslist.GetItemCount()); i++) {
			m_friendslist.SetItemData(i, i);
		}

		// ���� ��� ����
		m_bAscending = !m_bAscending;

		// ���� ���� ����ü ���� ���� �� ������ �ʱ�ȭ
		SORTPARAM sortparams;
		sortparams.pList = &m_friendslist;
		sortparams.iSortColumn = nColumn;
		sortparams.bSortDirect = m_bAscending;

		// |    0	 |
		// | friends |  

		// friends ����
		if (nColumn == 0)
			sortparams.flag = 1;
		// ���� �Լ� ȣ��
		m_friendslist.SortItems(&CompareItem, (LPARAM)&sortparams);
	}
	*pResult = 0;
}

int CWaitingForm::CompareItem(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrl* pList = ((SORTPARAM*)lParamSort)->pList;
	int iSortColumn = ((SORTPARAM*)lParamSort)->iSortColumn;
	bool bSortDirect = ((SORTPARAM*)lParamSort)->bSortDirect;
	int flag = ((SORTPARAM*)lParamSort)->flag;

	LVFINDINFO info1, info2;
	info1.flags = LVFI_PARAM;
	info1.lParam = lParam1;

	info2.flags = LVFI_PARAM;
	info2.lParam = lParam2;

	int irow1 = pList->FindItem(&info1, -1);
	int irow2 = pList->FindItem(&info2, -1);

	CString strItem1 = pList->GetItemText(irow1, iSortColumn);
	CString strItem2 = pList->GetItemText(irow2, iSortColumn);

	//if(pList->m_hWnd == m_roomlist.m_hWnd)
	// ä�ù� ��ȣ ����
	if (flag == 0)
	{
		int iItem1 = _tstoi(strItem1);
		int iItem2 = _tstoi(strItem2);

		if (bSortDirect) {
			return iItem1 == iItem2 ? 0 : iItem1 > iItem2;
		}
		else {
			return iItem1 == iItem2 ? 0 : iItem1 < iItem2;
		}
	}
	// ä�ù� �̸� ����
	else if (flag == 1)
	{
		return	bSortDirect ? strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2))) : -strcmp(LPSTR(LPCTSTR(strItem1)), LPSTR(LPCTSTR(strItem2)));
	}		

	return 0;

}