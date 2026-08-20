#include "pch.h"
#include "app/ui/panels/SageHeaderPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/dialogs/SageLoginDlg.h"
#include "app/core/auth/SageAuthSession.h"
#include "SageDefine.h"

BEGIN_MESSAGE_MAP(SageHeaderPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(ID_SAGE_LOGIN_BTN, &SageHeaderPanel::OnLogin)
	ON_BN_CLICKED(ID_SAGE_LOGOUT_BTN, &SageHeaderPanel::OnLogout)
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
	m_wndTitle.Create(SAGE_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCategory.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndUserLabel.Create(L"", WS_CHILD | SS_RIGHT | SS_CENTERIMAGE | SS_NOPREFIX, r, this, ID_SAGE_USER_LABEL);
	m_wndRoleBadge.Create(L"", WS_CHILD | SS_OWNERDRAW, r, this, ID_SAGE_ROLE_BADGE);
	m_wndLoginBtn.Create(SAGE_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, ID_SAGE_LOGIN_BTN);
	m_wndLogoutBtn.Create(SAGE_UI_LOGOUT_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_SAGE_LOGOUT_BTN);

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
	BOOL bLoggedIn = sageAuth.IsLoggedIn();

	m_wndLoginBtn.ShowWindow(bLoggedIn ? SW_HIDE : SW_SHOW);
	m_wndLogoutBtn.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);
	m_wndUserLabel.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);
	m_wndRoleBadge.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);

	if (bLoggedIn) {
		const SageUserDto& user = sageAuth.GetCurrentUser();
		m_wndUserLabel.SetWindowTextW(user.strLoginId);
		m_wndRoleBadge.SetBadge(
			(user.nRole == USER_ROLE_ADMIN) ? SAGE_UI_ROLE_ADMIN : SAGE_UI_ROLE_USER,
			SAGE_COLOR_LIST_HEADER,
			SAGE_COLOR_LIST_HEADER_BORDER,
			SAGE_COLOR_PRIMARY);
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

	int nRowTop = (rectClient.Height() - SAGE_BUTTON_HEIGHT) / 2;
	int nRight = rectClient.right - SAGE_CONTENT_PAD_X;

	int nButtonLeft = nRight - SAGE_LOGIN_BTN_WIDTH;
	m_wndLoginBtn.MoveWindow(nButtonLeft, nRowTop, SAGE_LOGIN_BTN_WIDTH, SAGE_BUTTON_HEIGHT);
	m_wndLogoutBtn.MoveWindow(nButtonLeft, nRowTop, SAGE_LOGIN_BTN_WIDTH, SAGE_BUTTON_HEIGHT);
	nRight = nButtonLeft - SAGE_HEADER_GAP;

	if (sageAuth.IsLoggedIn()) {
		int nBadgeWidth = m_wndRoleBadge.GetContentWidth();
		m_wndRoleBadge.MoveWindow(nRight - nBadgeWidth, nRowTop, nBadgeWidth, SAGE_BUTTON_HEIGHT);
		nRight -= nBadgeWidth + SAGE_HEADER_GAP;
		m_wndUserLabel.MoveWindow(
			nRight - SAGE_USER_LABEL_WIDTH, nRowTop,
			SAGE_USER_LABEL_WIDTH, SAGE_BUTTON_HEIGHT);
		nRight -= SAGE_USER_LABEL_WIDTH + SAGE_HEADER_GAP;
	}

	int nTitleLeft = rectClient.left + SAGE_CONTENT_PAD_X;
	int nTitleWidth = GetTitleWidth();
	if (nTitleLeft + nTitleWidth > nRight)
		nTitleWidth = nRight - nTitleLeft;
	if (nTitleWidth < 0)
		nTitleWidth = 0;

	m_wndTitle.MoveWindow(nTitleLeft, nRowTop, nTitleWidth, SAGE_BUTTON_HEIGHT);
	m_wndCategory.MoveWindow(
		nTitleLeft + nTitleWidth + SAGE_HEADER_TITLE_GAP, nRowTop,
		SAGE_HEADER_CATEGORY_WIDTH, SAGE_BUTTON_HEIGHT);
}

BOOL SageHeaderPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_PANEL);
	pDC->FillSolidRect(
		0, rectClient.bottom - SAGE_BORDER_THICKNESS, rectClient.Width(),
		SAGE_BORDER_THICKNESS, SAGE_COLOR_BORDER);
	return TRUE;
}

void SageHeaderPanel::OnLogin() {
	SageLoginDlg dlg(this);
	if (dlg.DoModal() == IDOK)
		UpdateAuthState();
}

void SageHeaderPanel::OnLogout() {
	sageAuth.Logout();
	UpdateAuthState();
}
