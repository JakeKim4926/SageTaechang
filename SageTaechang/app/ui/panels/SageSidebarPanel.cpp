#include "pch.h"
#include "app/ui/panels/SageSidebarPanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "app/core/auth/SageAuthSession.h"
#include "SageDefine.h"
#include <uxtheme.h>

BEGIN_MESSAGE_MAP(SageSidebarPanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TVN_SELCHANGED, ID_SAGE_SIDEBAR_TREE, &SageSidebarPanel::OnSelectionChanged)
END_MESSAGE_MAP()

SageSidebarPanel::SageSidebarPanel()
	: m_hLastWorkflowItem(NULL)
	, m_nSelectedWorkflow(SAGE_WORKFLOW_DELIVERY) {
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
	m_wndTitle.Create(SAGE_UI_APP_TITLE, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
	m_wndTree.Create(
		WS_CHILD | WS_VISIBLE | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS
		| TVS_DISABLEDRAGDROP | TVS_NOSCROLL,
		rectEmpty, this, ID_SAGE_SIDEBAR_TREE);
	SetWindowTheme(m_wndTree.GetSafeHwnd(), L"", L"");
	m_wndTree.SetBkColor(SAGE_COLOR_SIDEBAR);
	m_wndTree.SetTextColor(SAGE_COLOR_SIDEBAR_TEXT);
	m_wndTree.SetItemHeight(SAGE_SIDEBAR_ITEM_HEIGHT);
	m_wndTree.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTROL));

	m_wndTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR);
	m_wndTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndTitle.SetFontRole(SAGE_FONT_LOGO);
}

void SageSidebarPanel::BuildTree() {
	HTREEITEM hDocument = m_wndTree.InsertItem(SAGE_UI_SIDEBAR_GROUP_DOCUMENT, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hDocument, SAGE_SIDEBAR_ACTION_NONE);
	HTREEITEM hReceivables = m_wndTree.InsertItem(SAGE_UI_RECEIVABLES_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hReceivables, SAGE_WORKFLOW_RECEIVABLES);
	HTREEITEM hDelivery = m_wndTree.InsertItem(SAGE_UI_DELIVERY_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hDelivery, SAGE_WORKFLOW_DELIVERY);
	HTREEITEM hEstimate = m_wndTree.InsertItem(SAGE_UI_ESTIMATE_NAME, hDocument, TVI_LAST);
	m_wndTree.SetItemData(hEstimate, SAGE_WORKFLOW_ESTIMATE);

	HTREEITEM hPrice = m_wndTree.InsertItem(SAGE_UI_SIDEBAR_GROUP_PRICE, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hPrice, SAGE_SIDEBAR_ACTION_NONE);
	HTREEITEM hPriceManage = m_wndTree.InsertItem(SAGE_UI_PRICE_MANAGE_NAME, hPrice, TVI_LAST);
	m_wndTree.SetItemData(hPriceManage, SAGE_WORKFLOW_PRICE_MANAGE);
	HTREEITEM hPriceCalc = m_wndTree.InsertItem(SAGE_UI_PRICE_CALC_NAME, hPrice, TVI_LAST);
	m_wndTree.SetItemData(hPriceCalc, SAGE_WORKFLOW_PRICE_CALC);

	HTREEITEM hEtc = m_wndTree.InsertItem(SAGE_UI_SIDEBAR_GROUP_ETC, TVI_ROOT, TVI_LAST);
	m_wndTree.SetItemData(hEtc, SAGE_SIDEBAR_ACTION_NONE);
	HTREEITEM hChangePassword = m_wndTree.InsertItem(SAGE_UI_CHANGE_PW_MENU, hEtc, TVI_LAST);
	m_wndTree.SetItemData(hChangePassword, SAGE_SIDEBAR_ACTION_CHANGE_PASSWORD);

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

	int nTreeTop = SAGE_HEADER_HEIGHT + SAGE_SIDEBAR_TREE_TOP_PAD;
	m_wndTitle.MoveWindow(
		SAGE_SIDEBAR_PAD_X, 0,
		rectClient.Width() - SAGE_SIDEBAR_PAD_X, SAGE_HEADER_HEIGHT);
	m_wndTree.MoveWindow(
		0, nTreeTop, rectClient.Width(), rectClient.Height() - nTreeTop - SAGE_MARGIN);
}

BOOL SageSidebarPanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, SAGE_COLOR_SIDEBAR);
	pDC->FillSolidRect(
		0, SAGE_HEADER_HEIGHT, rectClient.Width(), SAGE_BORDER_THICKNESS,
		SAGE_COLOR_SIDEBAR_DIVIDER);
	return TRUE;
}

BOOL SageSidebarPanel::IsLoginRequired(DWORD_PTR nItemData) const {
	if (nItemData == SAGE_SIDEBAR_ACTION_CHANGE_PASSWORD)
		return TRUE;
	if (nItemData == SAGE_WORKFLOW_RECEIVABLES || nItemData == SAGE_WORKFLOW_PRICE_MANAGE)
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
	if (nItemData == SAGE_SIDEBAR_ACTION_NONE)
		return;

	if (IsLoginRequired(nItemData) && !sageAuth.IsLoggedIn()) {
		ShowSageMessageBox(SAGE_UI_LOGIN_REQUIRED, MB_ICONWARNING);
		RestoreLastSelection();
		return;
	}

	if (nItemData == SAGE_SIDEBAR_ACTION_CHANGE_PASSWORD) {
		NotifyParent(WM_SAGE_SIDEBAR_ACTION, nItemData);
		RestoreLastSelection();
		return;
	}

	int nWorkflowType = static_cast<int>(nItemData);
	m_hLastWorkflowItem = hItem;
	if (nWorkflowType == m_nSelectedWorkflow)
		return;

	m_nSelectedWorkflow = nWorkflowType;
	NotifyParent(WM_SAGE_SIDEBAR_WORKFLOW, static_cast<WPARAM>(nWorkflowType));
}
