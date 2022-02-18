#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CFriendListForm.h"
#include "json/json.h"
#include "CFriendAddDlg.h"

IMPLEMENT_DYNAMIC(CFriendListForm, CFormView)

CFriendListForm::CFriendListForm()
	: CFormView(IDD_FORMVIEW_FRIENDS_LIST)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CFriendListForm::CFriendListForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CFriendListForm::~CFriendListForm()
{
}

void CFriendListForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FRIENDLIST_FRIENDS, m_friendslist);
}


BEGIN_MESSAGE_MAP(CFriendListForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_FRIENDS_ADD, &CFriendListForm::OnBnClickedButtonFriendsAdd)
	ON_BN_CLICKED(IDC_BUTTON_FRIENDS_DELETE, &CFriendListForm::OnBnClickedButtonFriendsDelete)
END_MESSAGE_MAP()


// CMyForm 메시지 처리기


BOOL CFriendListForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CFriendListForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CRect rect;
	m_friendslist.GetClientRect(&rect);
	m_friendslist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_friendslist.InsertColumn(0, _T("친구"), LVCFMT_LEFT, rect.Width());
	m_friendslist.GetHeaderCtrl()->EnableWindow(false);

}

void CFriendListForm::OnBnClickedButtonFriendsAdd()
{
	CFriendAddDlg dlg;
	dlg.DoModal();
}


void CFriendListForm::OnBnClickedButtonFriendsDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nMark = m_friendslist.GetSelectionMark();
	if (nMark >= 0)
	{
		if (LVIS_SELECTED == m_friendslist.GetItemState(nMark, LVIS_SELECTED))
		{			
			CString fnickname = m_friendslist.GetItemText(nMark, 0);
			Json::Value root;
			Json::StyledWriter writer;

			root["action"] = "deletefriends";
			root["nickname"] = m_pDlg->m_pClient->m_getNickname();
			root["fnickname"] = std::string(CT2CA(fnickname));

			m_pDlg->m_pClient->m_data.msg = writer.write(root);
			m_pDlg->m_pClient->m_data.size = m_pDlg->m_pClient->m_data.msg.size();

			m_pDlg->m_pClient->m_SendData();

		}		
	}
	else
		;

	



}
