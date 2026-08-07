#include "pch.h"
#include "app/ui/panels/SageSidebarPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "TaechangDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SageSidebarPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TVN_SELCHANGED, ID_TAECHANG_SIDEBAR_TREE, &SageSidebarPanel::OnSelectionChanged)
END_MESSAGE_MAP()

SageSidebarPanel::SageSidebarPanel()
	: m_hLastWorkflowItem(NULL)
	, m_nSelectedWorkflow(TAECHANG_WORKFLOW_DELIVERY) {
}

BOOL SageSidebarPanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageSidebarPanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();
	return 0;
}

void SageSidebarPanel::CreateControls() {
	CRect rectEmpty(0, 0, 0, 0);
	m_wndTitle.Create(TAECHANG_UI_APP_TITLE, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
	m_wndMenuLabel.Create(TAECHANG_UI_SIDEBAR_TITLE, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, rectEmpty, this);
	m_wndTree.Create(
		WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS
		| TVS_DISABLEDRAGDROP | TVS_NOSCROLL,
		rectEmpty, this, ID_TAECHANG_SIDEBAR_TREE);
	SetWindowTheme(m_wndTree.GetSafeHwnd(), L"", L"");
	m_wndTree.SetBkColor(TAECHANG_COLOR_SIDEBAR);
	m_wndTree.SetTextColor(TAECHANG_COLOR_SIDEBAR_TEXT);
	m_wndTree.SetItemHeight(TAECHANG_SIDEBAR_ITEM_HEIGHT);
	m_wndTree.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTROL));

	m_wndTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR);
	m_wndTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndTitle.SetFontRole(SAGE_FONT_LOGO);

	m_wndMenuLabel.SetTextColorRole(SAGE_TEXT_SIDEBAR_CATEGORY);
	m_wndMenuLabel.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndMenuLabel.SetFontRole(SAGE_FONT_CONTROL);
}

void SageSidebarPanel::BuildTree() {
	HTREEITEM hDocument = m_wndTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_DOCUMENT, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hDocument, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hReceivables = m_wndTree.InsertItem(TAECHANG_UI_RECEIVABLES_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hReceivables, TAECHANG_WORKFLOW_RECEIVABLES);
	HTREEITEM hDelivery = m_wndTree.InsertItem(TAECHANG_UI_DELIVERY_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hDelivery, TAECHANG_WORKFLOW_DELIVERY);
	HTREEITEM hEstimate = m_wndTree.InsertItem(TAECHANG_UI_ESTIMATE_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hEstimate, TAECHANG_WORKFLOW_ESTIMATE);

	HTREEITEM hPrice = m_wndTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_PRICE, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hPrice, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hPriceManage = m_wndTree.InsertItem(TAECHANG_UI_PRICE_MANAGE_NAME, hPrice, TVI_LAST);
	m_wndTree.SetItemData(hPriceManage, TAECHANG_WORKFLOW_PRICE_MANAGE);
	HTREEITEM hPriceCalc = m_wndTree.InsertItem(TAECHANG_UI_PRICE_CALC_NAME, hPrice, TVI_LAST);
	m_wndTree.SetItemData(hPriceCalc, TAECHANG_WORKFLOW_PRICE_CALC);

	HTREEITEM hEtc = m_wndTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_ETC, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hEtc, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hChangePassword = m_wndTree.InsertItem(TAECHANG_UI_CHANGE_PW_MENU, hEtc, TVI_LAST);
	m_wndTree.SetItemData(hChangePassword, TAECHANG_SIDEBAR_ACTION_CHANGE_PASSWORD);

	m_wndTree.Expand(hDocument, TVE_EXPAND);
	m_wndTree.Expand(hPrice, TVE_EXPAND);
	m_wndTree.Expand(hEtc, TVE_EXPAND);

	m_hLastWorkflowItem = hDelivery;
	m_wndTree.SelectItem(hDelivery);
}

int SageSidebarPanel::GetSelectedWorkflow() const {
	return m_nSelectedWorkflow;
}

CString SageSidebarPanel::GetSelectedCategory() const {
	CString strCategory;
	if (m_hLastWorkflowItem == NULL)
		return strCategory;

	HTREEITEM hParent = m_wndTree.GetParentItem(m_hLastWorkflowItem);
	if (hParent == NULL)
		return strCategory;
	return m_wndTree.GetItemText(hParent);
}

void SageSidebarPanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	if (!::IsWindow(m_wndTree.GetSafeHwnd()))
		return;

	CRect rectClient;
	GetClientRect(&rectClient);
	if (rectClient.IsRectEmpty())
		return;

	int nInnerWidth = rectClient.Width() - TAECHANG_MARGIN * 2;
	m_wndTitle.MoveWindow(TAECHANG_MARGIN, 0, nInnerWidth, TAECHANG_HEADER_HEIGHT);
	m_wndMenuLabel.MoveWindow(
		TAECHANG_MARGIN, TAECHANG_HEADER_HEIGHT, nInnerWidth, TAECHANG_SIDEBAR_TITLE_HEIGHT);
	m_wndTree.MoveWindow(
		TAECHANG_MARGIN,
		TAECHANG_HEADER_HEIGHT + TAECHANG_SIDEBAR_TITLE_HEIGHT,
		nInnerWidth,
		rectClient.Height() - TAECHANG_HEADER_HEIGHT - TAECHANG_SIDEBAR_TITLE_HEIGHT - TAECHANG_MARGIN);
}

BOOL SageSidebarPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_SIDEBAR);
	pDC->FillSolidRect(
		0, TAECHANG_HEADER_HEIGHT, rectClient.Width(), TAECHANG_BORDER_THICKNESS,
		TAECHANG_COLOR_SIDEBAR_DIVIDER);
	return TRUE;
}

BOOL SageSidebarPanel::IsLoginRequired(DWORD_PTR nItemData) const {
	if (nItemData == TAECHANG_SIDEBAR_ACTION_CHANGE_PASSWORD)
		return TRUE;
	if (nItemData == TAECHANG_WORKFLOW_RECEIVABLES || nItemData == TAECHANG_WORKFLOW_PRICE_MANAGE)
		return TRUE;
	return FALSE;
}

void SageSidebarPanel::RestoreLastSelection() {
	if (m_hLastWorkflowItem != NULL)
		m_wndTree.SelectItem(m_hLastWorkflowItem);
}

void SageSidebarPanel::NotifyParent(UINT nMessage, WPARAM wParam) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(nMessage, wParam, 0);
}

void SageSidebarPanel::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	*pResult = 0;

	HTREEITEM hItem = m_wndTree.GetSelectedItem();
	if (hItem == NULL)
		return;

	DWORD_PTR nItemData = m_wndTree.GetItemData(hItem);
	if (nItemData == TAECHANG_SIDEBAR_ACTION_NONE)
		return;

	if (IsLoginRequired(nItemData) && !taechangAuth.IsLoggedIn()) {
		AfxMessageBox(TAECHANG_UI_LOGIN_REQUIRED, MB_ICONWARNING);
		RestoreLastSelection();
		return;
	}

	if (nItemData == TAECHANG_SIDEBAR_ACTION_CHANGE_PASSWORD) {
		NotifyParent(WM_TAECHANG_SIDEBAR_ACTION, nItemData);
		RestoreLastSelection();
		return;
	}

	int nWorkflowType = static_cast<int>(nItemData);
	m_hLastWorkflowItem = hItem;
	if (nWorkflowType == m_nSelectedWorkflow)
		return;

	m_nSelectedWorkflow = nWorkflowType;
	NotifyParent(WM_TAECHANG_SIDEBAR_WORKFLOW, static_cast<WPARAM>(nWorkflowType));
}
