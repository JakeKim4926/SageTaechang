
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "app/ui/frame/SageTaechangDoc.h"
#include "app/ui/view/SageTaechangView.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "app/ui/dialogs/TaechangPasswordChangeDlg.h"
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CString BuildDroppedPathList(HDROP hDropInfo) {
	CString strPaths;
	UINT nFileCount = DragQueryFileW(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (UINT i = 0; i < nFileCount; ++i) {
		UINT nLength = DragQueryFileW(hDropInfo, i, NULL, 0);
		if (nLength == 0)
			continue;
		CString strPath;
		LPWSTR pszPath = strPath.GetBuffer(static_cast<int>(nLength) + 1);
		DragQueryFileW(hDropInfo, i, pszPath, nLength + 1);
		strPath.ReleaseBuffer();
		if (!strPaths.IsEmpty())
			strPaths += L"\r\n";
		strPaths += strPath;
	}
	return strPaths;
}

void CSageTaechangView::EnableFileDropForWindow(CWnd& wnd) {
	HWND hWnd = wnd.GetSafeHwnd();
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	::DragAcceptFiles(hWnd, TRUE);
	::ChangeWindowMessageFilterEx(hWnd, WM_DROPFILES, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, WM_COPYDATA, MSGFLT_ALLOW, NULL);
	::ChangeWindowMessageFilterEx(hWnd, 0x0049, MSGFLT_ALLOW, NULL);
}

IMPLEMENT_DYNCREATE(CSageTaechangView, CView)

BEGIN_MESSAGE_MAP(CSageTaechangView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TVN_SELCHANGED, ID_TAECHANG_SIDEBAR_TREE, &CSageTaechangView::OnSidebarSelectionChanged)
	ON_BN_CLICKED(ID_TAECHANG_LOGIN_BTN, &CSageTaechangView::OnLogin)
	ON_BN_CLICKED(ID_TAECHANG_LOGOUT_BTN, &CSageTaechangView::OnLogout)
	ON_MESSAGE(WM_TAECHANG_WORKSPACE_TAB_CHANGED, &CSageTaechangView::OnWorkspaceTabChanged)
	ON_MESSAGE(WM_TAECHANG_WORKSPACE_STATUS, &CSageTaechangView::OnWorkspaceStatus)
	ON_MESSAGE(WM_TAECHANG_WORKSPACE_STATE_CHANGED, &CSageTaechangView::OnWorkspaceStateChanged)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
	: m_nCurrentWorkflow(TAECHANG_WORKFLOW_DELIVERY)
	, m_hLastWorkflowItem(NULL)
	, m_colorHeaderStatus(TAECHANG_COLOR_SECONDARY_TEXT)
	, m_nHeaderStatusBgRole(SAGE_BG_APP)
	, m_nAuthDividerX(0) {
	m_brushListHeader.CreateSolidBrush(TAECHANG_COLOR_LIST_HEADER);
}

CSageTaechangView::~CSageTaechangView() {}

BOOL CSageTaechangView::PreCreateWindow(CREATESTRUCT& cs) {
	cs.style |= WS_CLIPCHILDREN;
	return CView::PreCreateWindow(cs);
}

BOOL CSageTaechangView::PreTranslateMessage(MSG* pMsg) {
	if (pMsg && pMsg->message == WM_DROPFILES) {
		OnDropFiles(reinterpret_cast<HDROP>(pMsg->wParam));
		return TRUE;
	}
	return CView::PreTranslateMessage(pMsg);
}

int CSageTaechangView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateChildControls();
	EnableFileDropForWindow(*this);
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		EnableFileDropForWindow(*pFrame);
	SetStatusText(TAECHANG_UI_READY);
	return 0;
}

void CSageTaechangView::CreateChildControls() {
	CRect rectEmpty(0, 0, 0, 0);
	m_wndSidebarTitle.Create(TAECHANG_UI_SIDEBAR_TITLE, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, rectEmpty, this);
	m_wndSidebarTree.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP | TVS_NOSCROLL, rectEmpty, this, ID_TAECHANG_SIDEBAR_TREE);
	SetWindowTheme(m_wndSidebarTree.GetSafeHwnd(), L"", L"");
	m_wndSidebarTree.SetBkColor(TAECHANG_COLOR_SIDEBAR);
	m_wndSidebarTree.SetTextColor(TAECHANG_COLOR_SIDEBAR_TEXT);
	m_wndSidebarTree.SetItemHeight(TAECHANG_SIDEBAR_ITEM_HEIGHT);
	m_wndHeaderTitle.Create(TAECHANG_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, rectEmpty, this);
	m_wndHeaderStatus.Create(TAECHANG_UI_READY, WS_CHILD | SS_RIGHT, rectEmpty, this);
	m_wndTitle.Create(TAECHANG_UI_APP_TITLE, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);

	m_wndLoginBtn.Create(TAECHANG_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGIN_BTN);
	m_wndLogoutBtn.Create(TAECHANG_UI_LOGOUT_BTN, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGOUT_BTN);
	m_wndUserLabel.Create(L"", WS_CHILD | SS_CENTERIMAGE | SS_NOPREFIX, rectEmpty, this, ID_TAECHANG_USER_LABEL);

	m_panelWorkspace.Create(this, ID_TAECHANG_WORKSPACE_PANEL);
	m_panelWorkspace.EnableFileDrop();
	m_panelWorkspace.ShowWindow(SW_SHOW);

	ApplyControlFonts();
	ApplyLabelRoles();
	m_panelWorkspace.SetWorkflow(m_nCurrentWorkflow, FindCurrentHandler());
	BuildSidebarTree();
}

void CSageTaechangView::BuildSidebarTree() {
	HTREEITEM hDocument = m_wndSidebarTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_DOCUMENT, TVI_ROOT, TVI_LAST);
	m_wndSidebarTree.SetItemData(hDocument, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hReceivables = m_wndSidebarTree.InsertItem(TAECHANG_UI_RECEIVABLES_NAME, hDocument, TVI_LAST);
	m_wndSidebarTree.SetItemData(hReceivables, TAECHANG_WORKFLOW_RECEIVABLES);
	HTREEITEM hDelivery = m_wndSidebarTree.InsertItem(TAECHANG_UI_DELIVERY_NAME, hDocument, TVI_LAST);
	m_wndSidebarTree.SetItemData(hDelivery, TAECHANG_WORKFLOW_DELIVERY);
	HTREEITEM hEstimate = m_wndSidebarTree.InsertItem(TAECHANG_UI_ESTIMATE_NAME, hDocument, TVI_LAST);
	m_wndSidebarTree.SetItemData(hEstimate, TAECHANG_WORKFLOW_ESTIMATE);

	HTREEITEM hPrice = m_wndSidebarTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_PRICE, TVI_ROOT, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPrice, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hPriceManage = m_wndSidebarTree.InsertItem(TAECHANG_UI_PRICE_MANAGE_NAME, hPrice, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPriceManage, TAECHANG_WORKFLOW_PRICE_MANAGE);
	HTREEITEM hPriceCalc = m_wndSidebarTree.InsertItem(TAECHANG_UI_PRICE_CALC_NAME, hPrice, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPriceCalc, TAECHANG_WORKFLOW_PRICE_CALC);

	HTREEITEM hEtc = m_wndSidebarTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_ETC, TVI_ROOT, TVI_LAST);
	m_wndSidebarTree.SetItemData(hEtc, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hChangePassword = m_wndSidebarTree.InsertItem(TAECHANG_UI_CHANGE_PW_MENU, hEtc, TVI_LAST);
	m_wndSidebarTree.SetItemData(hChangePassword, TAECHANG_SIDEBAR_ACTION_CHANGE_PASSWORD);

	m_wndSidebarTree.Expand(hDocument, TVE_EXPAND);
	m_wndSidebarTree.Expand(hPrice, TVE_EXPAND);
	m_wndSidebarTree.Expand(hEtc, TVE_EXPAND);

	m_hLastWorkflowItem = hDelivery;
	m_wndSidebarTree.SelectItem(hDelivery);
}

void CSageTaechangView::ApplyControlFonts() {
	m_wndSidebarTree.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTROL));

	m_wndHeaderStatus.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLoginBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLogoutBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

void CSageTaechangView::ApplyLabelRoles() {
	m_wndTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR);
	m_wndTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndTitle.SetFontRole(SAGE_FONT_LOGO);

	m_wndSidebarTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR_CATEGORY);
	m_wndSidebarTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndSidebarTitle.SetFontRole(SAGE_FONT_CONTROL);

	m_wndHeaderTitle.SetTextColorRole(SAGE_TEXT_DEFAULT);
	m_wndHeaderTitle.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndHeaderTitle.SetFontRole(SAGE_FONT_TITLE);

	m_wndUserLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndUserLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndUserLabel.SetFontRole(SAGE_FONT_CONTENT);
}

void CSageTaechangView::OnSize(UINT nType, int cx, int cy) {
	CView::OnSize(nType, cx, cy);
	LayoutChildControls();
}

void CSageTaechangView::LayoutChildControls() {
	if (!::IsWindow(m_wndSidebarTree.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	int nSidebarLeft = 0;
	int nSidebarHeight = rectClient.Height();
	int nSidebarInnerWidth = TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2);
	int nContentLeft = TAECHANG_SIDEBAR_WIDTH + TAECHANG_CONTENT_PAD_X;
	int nContentWidth = rectClient.Width() - nContentLeft - TAECHANG_CONTENT_PAD_X;

	m_wndTitle.MoveWindow(TAECHANG_MARGIN, 0, nSidebarInnerWidth, TAECHANG_HEADER_HEIGHT);
	m_wndSidebarTitle.MoveWindow(TAECHANG_MARGIN, TAECHANG_HEADER_HEIGHT, nSidebarInnerWidth, TAECHANG_SIDEBAR_TITLE_HEIGHT);
	m_wndSidebarTree.MoveWindow(
		TAECHANG_MARGIN,
		TAECHANG_HEADER_HEIGHT + TAECHANG_SIDEBAR_TITLE_HEIGHT,
		nSidebarInnerWidth,
		nSidebarHeight - TAECHANG_HEADER_HEIGHT - TAECHANG_SIDEBAR_TITLE_HEIGHT - TAECHANG_MARGIN);

	int nHeaderRowTop = (TAECHANG_HEADER_HEIGHT - TAECHANG_BUTTON_HEIGHT) / 2;
	{
		int nLoginBtnTop = nHeaderRowTop;
		int nLoginBtnRight = rectClient.Width() - TAECHANG_CONTENT_PAD_X;
		int nLoginBtnLeft = nLoginBtnRight - TAECHANG_LOGIN_BTN_WIDTH;
		int nUserLabelLeft = nLoginBtnLeft - TAECHANG_USER_LABEL_WIDTH - TAECHANG_ROW_GAP;
		m_nAuthDividerX = nUserLabelLeft - TAECHANG_ROW_GAP;

		m_wndLoginBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndLogoutBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndUserLabel.MoveWindow(nUserLabelLeft, nLoginBtnTop + TAECHANG_BUTTON_TEXT_TOP_OFFSET, TAECHANG_USER_LABEL_WIDTH, TAECHANG_BUTTON_HEIGHT);
	}

	int nHeaderTitleWidth = nContentWidth - TAECHANG_LOGIN_BTN_WIDTH - TAECHANG_USER_LABEL_WIDTH - TAECHANG_ROW_GAP * 2 - TAECHANG_MARGIN;
	m_wndHeaderTitle.MoveWindow(nContentLeft, nHeaderRowTop, nHeaderTitleWidth, TAECHANG_BUTTON_HEIGHT);
	m_wndHeaderStatus.MoveWindow(0, 0, 0, 0);

	InvalidateContentArea();

	m_panelWorkspace.Layout(CRect(
		TAECHANG_SIDEBAR_WIDTH + TAECHANG_BORDER_THICKNESS,
		TAECHANG_HEADER_HEIGHT + TAECHANG_BORDER_THICKNESS,
		rectClient.Width(),
		rectClient.Height()));
	m_panelWorkspace.RefreshVisibility();

	UNREFERENCED_PARAMETER(nSidebarLeft);
}

void CSageTaechangView::OnDraw(CDC* pDC) {
	CSageTaechangDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
	pDC->FillSolidRect(0, TAECHANG_HEADER_HEIGHT, TAECHANG_SIDEBAR_WIDTH, TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_SIDEBAR_DIVIDER);
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
	DrawShellBands(pDC, rectClient);
	if (taechangAuth.IsLoggedIn() && m_nAuthDividerX > 0) {
		int nDivTop = (TAECHANG_HEADER_HEIGHT - TAECHANG_BUTTON_HEIGHT) / 2;
		pDC->FillSolidRect(m_nAuthDividerX, nDivTop, 1, TAECHANG_BUTTON_HEIGHT, TAECHANG_COLOR_BORDER);
	}
}

void CSageTaechangView::DrawShellBands(CDC* pDC, const CRect& rectClient) {
	int nLeft = TAECHANG_SIDEBAR_WIDTH + TAECHANG_BORDER_THICKNESS;
	int nWidth = rectClient.Width() - nLeft;
	int nHeaderBottom = TAECHANG_HEADER_HEIGHT;

	pDC->FillSolidRect(nLeft, 0, nWidth, nHeaderBottom, TAECHANG_COLOR_PANEL);
	pDC->FillSolidRect(nLeft, nHeaderBottom, nWidth, TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_BORDER);
}

void CSageTaechangView::InvalidateContentArea() {
	CRect rectContent;
	GetClientRect(&rectContent);
	rectContent.left += TAECHANG_SIDEBAR_WIDTH;
	InvalidateRect(rectContent, TRUE);
}


int CSageTaechangView::GetSelectedWorkflow() const {
	return m_nCurrentWorkflow;
}

ISageWorkflowHandler* CSageTaechangView::FindCurrentHandler() const {
	return SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
}

void CSageTaechangView::OnWorkflowChanged() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	m_panelWorkspace.SetWorkflow(m_nCurrentWorkflow, pHandler);
	m_panelWorkspace.RestoreWorkflowState(m_nCurrentWorkflow);

	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_wndHeaderTitle.SetWindowTextW(
			m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE
			? TAECHANG_UI_PRICE_MANAGE_NAME
			: TAECHANG_UI_PRICE_CALC_NAME
		);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE)
			m_panelWorkspace.GetPriceManagePanel().RefreshCompanyList();
		else
			m_panelWorkspace.GetPriceCalcPanel().RefreshCompanyCombo();
		LayoutChildControls();
		Invalidate(FALSE);
		SetStatusText(TAECHANG_UI_READY);
		return;
	}

	if (pHandler != NULL) {
		m_wndHeaderTitle.SetWindowTextW(pHandler->GetHeaderTitle());
		m_panelWorkspace.ApplyWorkflowLabels(pHandler);
	}
	m_panelWorkspace.RebuildResultTable();
	LayoutChildControls();
	if (!m_panelWorkspace.IsRunning())
		SetStatusText(TAECHANG_UI_READY);
}

void CSageTaechangView::OnSidebarSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;
	HTREEITEM hItem = m_wndSidebarTree.GetSelectedItem();
	if (hItem == NULL)
		return;
	DWORD_PTR nItemData = m_wndSidebarTree.GetItemData(hItem);
	if (nItemData == TAECHANG_SIDEBAR_ACTION_NONE)
		return;
	if (nItemData == TAECHANG_SIDEBAR_ACTION_CHANGE_PASSWORD) {
		if (!taechangAuth.IsLoggedIn()) {
			AfxMessageBox(TAECHANG_UI_LOGIN_REQUIRED, MB_ICONWARNING);
			if (m_hLastWorkflowItem != NULL)
				m_wndSidebarTree.SelectItem(m_hLastWorkflowItem);
			return;
		}
		TaechangPasswordChangeDlg dlg(this);
		dlg.DoModal();
		if (m_hLastWorkflowItem != NULL)
			m_wndSidebarTree.SelectItem(m_hLastWorkflowItem);
		return;
	}
	if (nItemData == TAECHANG_WORKFLOW_RECEIVABLES || nItemData == TAECHANG_WORKFLOW_PRICE_MANAGE) {
		if (!taechangAuth.IsLoggedIn()) {
			AfxMessageBox(TAECHANG_UI_LOGIN_REQUIRED, MB_ICONWARNING);
			if (m_hLastWorkflowItem != NULL)
				m_wndSidebarTree.SelectItem(m_hLastWorkflowItem);
			return;
		}
	}
	int nNewWorkflow = static_cast<int>(nItemData);
	m_hLastWorkflowItem = hItem;
	if (nNewWorkflow == m_nCurrentWorkflow)
		return;
	m_panelWorkspace.SaveWorkflowState(m_nCurrentWorkflow);
	m_nCurrentWorkflow = nNewWorkflow;
	OnWorkflowChanged();
}

LRESULT CSageTaechangView::OnWorkspaceTabChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	LayoutChildControls();
	Invalidate();
	return 0;
}

LRESULT CSageTaechangView::OnWorkspaceStatus(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	LPCWSTR pszStatus = reinterpret_cast<LPCWSTR>(lParam);
	if (pszStatus != NULL)
		SetStatusText(pszStatus);
	return 0;
}

LRESULT CSageTaechangView::OnWorkspaceStateChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	m_panelWorkspace.SaveWorkflowState(m_nCurrentWorkflow);
	return 0;
}

void CSageTaechangView::OnDropFiles(HDROP hDropInfo) {
	CString strPaths = BuildDroppedPathList(hDropInfo);
	DragFinish(hDropInfo);
	m_panelWorkspace.ApplyDroppedInputPaths(strPaths);
}

void CSageTaechangView::UpdateAuthState() {
	BOOL bLoggedIn = taechangAuth.IsLoggedIn();

	m_wndLoginBtn.ShowWindow(bLoggedIn ? SW_HIDE : SW_SHOW);
	m_wndLogoutBtn.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);
	m_wndUserLabel.ShowWindow(bLoggedIn ? SW_SHOW : SW_HIDE);

	if (bLoggedIn) {
		const TaechangUserDto& user = taechangAuth.GetCurrentUser();
		LPCWSTR pszRole = (user.nRole == USER_ROLE_ADMIN) ? TAECHANG_UI_ROLE_ADMIN : TAECHANG_UI_ROLE_USER;
		CString strLabel;
		strLabel.Format(TAECHANG_UI_USER_FORMAT, user.strLoginId.GetString(), pszRole);
		m_wndUserLabel.SetWindowText(strLabel);
	}

	Invalidate();
}

void CSageTaechangView::OnLogin() {
	TaechangLoginDlg dlg(this);
	if (dlg.DoModal() == IDOK)
		UpdateAuthState();
}

void CSageTaechangView::OnLogout() {
	taechangAuth.Logout();
	UpdateAuthState();
}

BOOL CSageTaechangView::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
	pDC->FillSolidRect(0, TAECHANG_HEADER_HEIGHT, TAECHANG_SIDEBAR_WIDTH, TAECHANG_BORDER_THICKNESS, TAECHANG_COLOR_SIDEBAR_DIVIDER);
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
	DrawShellBands(pDC, rectClient);
	return TRUE;
}

HBRUSH CSageTaechangView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CView::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
		return hBrush;
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	if (pWnd->GetSafeHwnd() == m_wndHeaderStatus.GetSafeHwnd()) {
		pDC->SetTextColor(m_colorHeaderStatus);
		pDC->SetBkColor(SageUiResources::GetBackgroundColor(m_nHeaderStatusBgRole));
		return SageUiResources::GetBrush(m_nHeaderStatusBgRole);
	}
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
	}
	return hBrush;
}

void CSageTaechangView::SetStatusText(const CString& strStatus) {
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		pFrame->SetMessageText(strStatus);

	if (::IsWindow(m_wndHeaderStatus.GetSafeHwnd())) {
		m_colorHeaderStatus = ResolveStatusColor(strStatus);
		m_nHeaderStatusBgRole = ResolveStatusBgRole(strStatus);
		m_wndHeaderStatus.SetWindowTextW(strStatus);
		m_wndHeaderStatus.Invalidate();
	}
}

COLORREF CSageTaechangView::ResolveStatusColor(const CString& strStatus) const {
	if (strStatus == TAECHANG_UI_RUNNING)
		return TAECHANG_COLOR_PRIMARY;
	if (strStatus == TAECHANG_UI_COMPLETED)
		return TAECHANG_COLOR_SUCCESS;
	if (strStatus == TAECHANG_UI_FAILED)
		return TAECHANG_COLOR_ERROR;
	return TAECHANG_COLOR_SECONDARY_TEXT;
}

SageBackgroundRole CSageTaechangView::ResolveStatusBgRole(const CString& strStatus) const {
	if (strStatus == TAECHANG_UI_COMPLETED)
		return SAGE_BG_STATUS_SUCCESS;
	if (strStatus == TAECHANG_UI_RUNNING)
		return SAGE_BG_STATUS_WARNING;
	if (strStatus == TAECHANG_UI_FAILED)
		return SAGE_BG_STATUS_ERROR;
	return SAGE_BG_APP;
}


#ifdef _DEBUG
void CSageTaechangView::AssertValid() const {
	CView::AssertValid();
}

void CSageTaechangView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CSageTaechangDoc* CSageTaechangView::GetDocument() const {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSageTaechangDoc)));
	return (CSageTaechangDoc*)m_pDocument;
}
#endif


