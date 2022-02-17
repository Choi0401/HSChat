#include "pch.h"
#include "HSChat.h"
#include "HSChatDlg.h"
#include "CMyInfoForm.h"
#include "json/json.h"

IMPLEMENT_DYNAMIC(CMyInfoForm, CFormView)

CMyInfoForm::CMyInfoForm()
	: CFormView(IDD_FORMVIEW_MYINFO)
{
	m_pDlg = (CHSChatDlg*)::AfxGetMainWnd();
}

CMyInfoForm::CMyInfoForm(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{

}

CMyInfoForm::~CMyInfoForm()
{
}

void CMyInfoForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyInfoForm, CFormView)
	
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_OK, &CMyInfoForm::OnBnClickedButtonMyInfoOK)
	ON_BN_CLICKED(IDC_BUTTON_MYINFO_CANCEL, &CMyInfoForm::OnBnClickedButtonMyInfoCancel)
END_MESSAGE_MAP()



BOOL CMyInfoForm::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CMyInfoForm::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.


}

BOOL CMyInfoForm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN))
	{
		OnBnClickedButtonMyInfoOK();
		return TRUE;
	}
	return CFormView::PreTranslateMessage(pMsg);
}


void CMyInfoForm::OnBnClickedButtonMyInfoOK()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
}


void CMyInfoForm::OnBnClickedButtonMyInfoCancel()
{
	m_pDlg->m_ShowForm(4);
}
