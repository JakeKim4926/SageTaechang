#include "pch.h"
#include "app/ui/panels/SageHeaderPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageHeaderPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(ID_TAECHANG_LOGIN_BTN, &SageHeaderPanel::OnLogin)
	ON_BN_CLICKED(ID_TAECHANG_LOGOUT_BTN, &SageHeaderPanel::OnLogout)
END_MESSAGE_MAP()

BOOL SageHeaderPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageHeaderPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	return 0;
}

void SageHeaderPanel::CreateControls() {
	CRect r(0, 0, 0, 0);
	m_wndTitle.Create(TAECHANG_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCategory.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndUserLabel.Create(L"", WS_CHILD | SS_RIGHT | SS_CENTERIMAGE | SS_NOPREFIX, r, this, ID_TAECHANG_USER_LABEL);
	m_wndRoleBadge.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_TAECHANG_ROLE_BADGE);
	m_wndLoginBtn.Create(TAECHANG_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_TAECHANG_LOGIN_BTN);
	m_wndLogoutBtn.Create(TAECHANG_UI_LOGOUT_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_TAECHANG_LOGOUT_BTN);

	m_wndTitle.SetTextColorRole(SAGE_TEXT_DEFAULT);
	m_wndTitle.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndTitle.SetFontRole(SAGE_FONT_TITLE);

	m_wndCategory.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCategory.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCategory.SetFontRole(SAGE_FONT_CAPTION);

	m_wndUserLabel.SetTextColorRole(SAGE_TEXT_MUTED);
	m_wndUserLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndUserLabel.SetFontRole(SAGE_FONT_CAPTION);

	m_wndLoginBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLogoutBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

void SageHeaderPanel::SetTitle(LPCWSTR pszTitle) {
	m_wndTitle.SetWindowTextW(pszTitle);
}

void SageHeaderPanel::SetCategory(const CString& strCategory) {
	m_wndCategory.SetWindowTextW(strCategory);
	LayoutChildren();
}

int SageHeaderPanel::GetTitleWidth() {
	CString strTitle;
	m_wndTitle.GetWindowTextW(strTitle);
	if (strTitle.IsEmpty())
		return 0;

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(SageUiResources::GetFont(SAGE_FONT_TITLE));
	CSize sizeText = dc.GetTextExtent(strTitle);
	dc.SelectObject(pOldFont);
	return sizeText.cx;
}

void SageHeaderPanel::UpdateAuthState() {
	BOOL bLoggedIn = taechangAuth.IsLoggedIn();

	m_wndLoginBtn.ShowWindow(bLoggedIn ? SW_HIDE : SW_SHOW);
	m_wndLogoutBtn.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);
	m_wndUserLabel.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);
	m_wndRoleBadge.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);

	if (bLoggedIn) {
		const TaechangUserDto& user = taechangAuth.GetCurrentUser();
		m_wndUserLabel.SetWindowTextW(user.strLoginId);
		m_wndRoleBadge.SetBadge(
			(user.nRole == USER_ROLE_ADMIN) ? TAECHANG_UI_ROLE_ADMIN : TAECHANG_UI_ROLE_USER,
			TAECHANG_COLOR_LIST_HEADER,
			TAECHANG_COLOR_LIST_HEADER_BORDER,
			TAECHANG_COLOR_PRIMARY);
	}

	LayoutChildren();
	Invalidate();
}

void SageHeaderPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutChildren();
}

void SageHeaderPanel::LayoutChildren() {
	if (!::IsWindow(m_wndTitle.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	int nRowTop = (rectClient.Height() - TAECHANG_BUTTON_HEIGHT) / 2;
	int nRight = rectClient.right - TAECHANG_CONTENT_PAD_X;

	int nButtonLeft = nRight - TAECHANG_LOGIN_BTN_WIDTH;
	m_wndLoginBtn.MoveWindow(nButtonLeft, nRowTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	m_wndLogoutBtn.MoveWindow(nButtonLeft, nRowTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nRight = nButtonLeft - TAECHANG_HEADER_GAP;

	if (taechangAuth.IsLoggedIn()) {
		int nBadgeWidth = m_wndRoleBadge.GetContentWidth();
		m_wndRoleBadge.MoveWindow(nRight - nBadgeWidth, nRowTop, nBadgeWidth, TAECHANG_BUTTON_HEIGHT);
		nRight -= nBadgeWidth + TAECHANG_HEADER_GAP;
		m_wndUserLabel.MoveWindow(
			nRight - TAECHANG_USER_LABEL_WIDTH, nRowTop,
			TAECHANG_USER_LABEL_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nRight -= TAECHANG_USER_LABEL_WIDTH + TAECHANG_HEADER_GAP;
	}

	int nTitleLeft = rectClient.left + TAECHANG_CONTENT_PAD_X;
	int nTitleWidth = GetTitleWidth();
	if (nTitleLeft + nTitleWidth > nRight)
		nTitleWidth = nRight - nTitleLeft;
	if (nTitleWidth < 0)
		nTitleWidth = 0;

	m_wndTitle.MoveWindow(nTitleLeft, nRowTop, nTitleWidth, TAECHANG_BUTTON_HEIGHT);
	m_wndCategory.MoveWindow(
		nTitleLeft + nTitleWidth + TAECHANG_HEADER_TITLE_GAP, nRowTop,
		TAECHANG_HEADER_CATEGORY_WIDTH, TAECHANG_BUTTON_HEIGHT);
}

BOOL SageHeaderPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_PANEL);
	pDC->FillSolidRect(
		0, rectClient.bottom - TAECHANG_BORDER_THICKNESS, rectClient.Width(),
		TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_BORDER);
	return TRUE;
}

void SageHeaderPanel::OnLogin() {
	TaechangLoginDlg dlg(this);
	if (dlg.DoModal() == IDOK)
		UpdateAuthState();
}

void SageHeaderPanel::OnLogout() {
	taechangAuth.Logout();
	UpdateAuthState();
}
