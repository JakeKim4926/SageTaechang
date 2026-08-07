#include "pch.h"
#include "app/ui/panels/SageWorkspacePanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageWorkspacePanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TCN_SELCHANGE, ID_TAECHANG_TASK_TABS, &SageWorkspacePanel::OnTabChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_TABLE_CHANGED, &SageWorkspacePanel::OnResultTableChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_SELECTION_CHANGED, &SageWorkspacePanel::OnResultSelectionChanged)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_RUN_REQUESTED, &SageWorkspacePanel::OnWorkflowRunRequested)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_INPUT_RESET, &SageWorkspacePanel::OnWorkflowInputReset)
END_MESSAGE_MAP()

SageWorkspacePanel::SageWorkspacePanel()
	: m_pHandler(NULL)
	, m_nCurrentWorkflow(TAECHANG_WORKFLOW_DELIVERY)
	, m_nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT) {
}

BOOL SageWorkspacePanel::Create(CWnd* pParent, UINT nId) {
	CRect rectEmpty(0, 0, 0, 0);
	return CWnd::CreateEx(0, AfxRegisterWndClass(0), NULL, WS_CHILD | WS_CLIPCHILDREN, rectEmpty, pParent, nId);
}

int SageWorkspacePanel::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r(0, 0, 0, 0);
	m_wndTaskTabs.Create(WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH, r, this, ID_TAECHANG_TASK_TABS);
	m_wndTaskTabs.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));

	m_panelPriceManage.Create(this, ID_PRICE_MANAGE_PANEL);
	m_panelPriceCalc.Create(this, ID_CALC_PANEL);
	m_panelWorkflowInput.Create(this, ID_TAECHANG_WORKFLOW_INPUT_PANEL);
	m_panelWorkflowResult.Create(this, ID_TAECHANG_WORKFLOW_RESULT_PANEL);
	m_panelWorkflowHistory.Create(this, ID_TAECHANG_WORKFLOW_HISTORY_PANEL);
	m_panelCompanyOrder.Create(this, ID_TAECHANG_COMPANY_ORDER_PANEL);
	return 0;
}

void SageWorkspacePanel::EnableFileDrop() {
	m_panelWorkflowInput.EnableFileDrop();
	m_panelWorkflowResult.EnableFileDrop();
}

SageWorkflowInputPanel& SageWorkspacePanel::GetInputPanel() {
	return m_panelWorkflowInput;
}

SageWorkflowResultPanel& SageWorkspacePanel::GetResultPanel() {
	return m_panelWorkflowResult;
}

SageWorkflowHistoryPanel& SageWorkspacePanel::GetHistoryPanel() {
	return m_panelWorkflowHistory;
}

SagePriceManagePanel& SageWorkspacePanel::GetPriceManagePanel() {
	return m_panelPriceManage;
}

SagePriceCalcPanel& SageWorkspacePanel::GetPriceCalcPanel() {
	return m_panelPriceCalc;
}

SageCompanyOrderPanel& SageWorkspacePanel::GetCompanyOrderPanel() {
	return m_panelCompanyOrder;
}

BOOL SageWorkspacePanel::IsPriceWorkflow() const {
	return IsPriceWorkflowType(m_nCurrentWorkflow);
}

int SageWorkspacePanel::GetSelectedTab() const {
	return m_nSelectedTaskTab;
}

BOOL SageWorkspacePanel::IsInputTabSelected() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_INPUT) ? TRUE : FALSE;
}

BOOL SageWorkspacePanel::IsResultTab() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_RESULT) ? TRUE : FALSE;
}

BOOL SageWorkspacePanel::IsDetailTab() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_HISTORY) ? TRUE : FALSE;
}

BOOL SageWorkspacePanel::IsDataManageTab() const {
	return (m_nCurrentWorkflow == TAECHANG_WORKFLOW_RECEIVABLES &&
		m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_DATA_MANAGE) ? TRUE : FALSE;
}

int SageWorkspacePanel::GetTabVisualIndex(int nSemanticTabIndex) const {
	if (m_pHandler == NULL)
		return TAECHANG_TAB_INDEX_INPUT;
	int nTabCount = m_pHandler->GetTabCount();
	for (int nVisualTabIndex = 0; nVisualTabIndex < nTabCount; ++nVisualTabIndex) {
		if (m_pHandler->GetTab(nVisualTabIndex).nSemanticIndex == nSemanticTabIndex)
			return nVisualTabIndex;
	}
	return TAECHANG_TAB_INDEX_INPUT;
}

int SageWorkspacePanel::GetTabSemanticIndex(int nVisualTabIndex) const {
	if (m_pHandler == NULL)
		return TAECHANG_TAB_INDEX_INPUT;
	if (nVisualTabIndex < 0 || nVisualTabIndex >= m_pHandler->GetTabCount())
		return TAECHANG_TAB_INDEX_INPUT;
	return m_pHandler->GetTab(nVisualTabIndex).nSemanticIndex;
}

void SageWorkspacePanel::SetWorkflow(int nWorkflowType, ISageWorkflowHandler* pHandler) {
	m_nCurrentWorkflow = nWorkflowType;
	m_pHandler = pHandler;
	if (!::IsWindow(GetSafeHwnd()))
		return;

	Invalidate();
	if (!::IsWindow(m_wndTaskTabs.GetSafeHwnd()))
		return;

	m_wndTaskTabs.DeleteAllItems();
	if (pHandler == NULL)
		return;

	int nTabCount = pHandler->GetTabCount();
	for (int nVisualTabIndex = 0; nVisualTabIndex < nTabCount; ++nVisualTabIndex)
		m_wndTaskTabs.InsertItem(nVisualTabIndex, pHandler->GetTab(nVisualTabIndex).pszLabel);
	m_wndTaskTabs.ApplyTabHeight();
	m_wndTaskTabs.SetCurSel(GetTabVisualIndex(m_nSelectedTaskTab));
}

void SageWorkspacePanel::SelectTab(int nSemanticTabIndex) {
	m_nSelectedTaskTab = nSemanticTabIndex;
	if (!::IsWindow(m_wndTaskTabs.GetSafeHwnd()))
		return;
	m_wndTaskTabs.SetCurSel(GetTabVisualIndex(m_nSelectedTaskTab));
	if (IsDataManageTab())
		m_panelCompanyOrder.RefreshList();
}

CRect SageWorkspacePanel::GetContentRect() const {
	CRect rectClient;
	GetClientRect(&rectClient);

	int nTop = IsPriceWorkflow() ? 0 : TAECHANG_TAB_HEIGHT;
	return CRect(
		TAECHANG_CONTENT_PAD_X,
		nTop + TAECHANG_CONTENT_PAD_Y,
		rectClient.Width() - TAECHANG_CONTENT_PAD_X,
		rectClient.Height() - TAECHANG_CONTENT_PAD_Y);
}

void SageWorkspacePanel::Layout(const CRect& rectPanel) {
	MoveWindow(rectPanel);
	LayoutTabRow();
	LayoutActivePanel();
}

void SageWorkspacePanel::LayoutTabRow() {
	if (!::IsWindow(m_wndTaskTabs.GetSafeHwnd()))
		return;
	if (IsPriceWorkflow()) {
		m_wndTaskTabs.ShowWindow(SW_HIDE);
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);
	m_wndTaskTabs.ShowWindow(SW_SHOW);
	m_wndTaskTabs.MoveWindow(
		TAECHANG_CONTENT_PAD_X,
		0,
		rectClient.Width() - TAECHANG_CONTENT_PAD_X * 2,
		TAECHANG_TAB_HEIGHT);
}

void SageWorkspacePanel::LayoutActivePanel() {
	CRect rectContent = GetContentRect();
	if (rectContent.IsRectEmpty())
		return;

	if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) {
		m_panelPriceManage.Layout(rectContent);
		return;
	}
	if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_CALC) {
		m_panelPriceCalc.Layout(rectContent);
		return;
	}

	if (IsDataManageTab()) {
		m_panelCompanyOrder.Layout(rectContent);
		return;
	}

	if (IsInputTabSelected()) {
		m_panelWorkflowInput.Layout(CRect(
			rectContent.left,
			rectContent.top,
			rectContent.right + TAECHANG_EDIT_BORDER_WIDTH,
			rectContent.bottom));
		return;
	}

	int nBodyHeight = max(TAECHANG_RESULT_MIN_HEIGHT, rectContent.Height() - TAECHANG_RESULT_HEADER_HEIGHT);
	if (IsResultTab()) {
		m_panelWorkflowResult.Layout(CRect(
			rectContent.left,
			rectContent.top - m_panelWorkflowResult.GetBandHeight(),
			rectContent.right,
			rectContent.top + TAECHANG_RESULT_HEADER_HEIGHT + nBodyHeight));
		return;
	}
	if (IsDetailTab()) {
		m_panelWorkflowHistory.Layout(CRect(
			rectContent.left,
			rectContent.top,
			rectContent.right,
			rectContent.top + TAECHANG_RESULT_HEADER_HEIGHT + nBodyHeight));
	}
}

void SageWorkspacePanel::UpdateVisibility(const SageWorkspaceVisibility& state) {
	BOOL bPrice = IsPriceWorkflow();
	BOOL bShowInput = (!bPrice && IsInputTabSelected()) ? TRUE : FALSE;
	BOOL bShowResult = (!bPrice && IsResultTab()) ? TRUE : FALSE;
	BOOL bShowDetail = (!bPrice && IsDetailTab()) ? TRUE : FALSE;

	m_panelWorkflowInput.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_panelWorkflowInput.UpdateActionVisibility(state.bInputResetVisible, state.bHasLastResult);
	m_panelWorkflowInput.UpdateInputTableVisibility(
		(bShowInput && state.bInputTableVisible) ? TRUE : FALSE,
		state.bOnePageVisible,
		state.bFilterVisible);

	m_panelWorkflowResult.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
	m_panelWorkflowResult.UpdateResultTableVisibility((bShowResult && state.bFilterVisible) ? TRUE : FALSE);

	m_panelWorkflowHistory.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
	m_panelCompanyOrder.ShowWindow(IsDataManageTab() ? SW_SHOW : SW_HIDE);

	m_panelPriceManage.ShowWindow((m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) ? SW_SHOW : SW_HIDE);
	m_panelPriceCalc.ShowWindow((m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_CALC) ? SW_SHOW : SW_HIDE);
}

BOOL SageWorkspacePanel::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	if (IsPriceWorkflow())
		return TRUE;

	pDC->FillSolidRect(0, 0, rectClient.Width(), TAECHANG_TAB_HEIGHT, TAECHANG_COLOR_PANEL);
	pDC->FillSolidRect(
		0,
		TAECHANG_TAB_HEIGHT - TAECHANG_BORDER_THICKNESS,
		rectClient.Width(),
		TAECHANG_BORDER_THICKNESS,
		TAECHANG_COLOR_BORDER);
	return TRUE;
}

void SageWorkspacePanel::OnTabChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	m_nSelectedTaskTab = GetTabSemanticIndex(m_wndTaskTabs.GetCurSel());
	*pResult = 0;
	if (IsDataManageTab())
		m_panelCompanyOrder.RefreshList();
	ForwardToParent(WM_TAECHANG_WORKSPACE_TAB_CHANGED, 0, 0);
}

LRESULT SageWorkspacePanel::ForwardToParent(UINT nMessage, WPARAM wParam, LPARAM lParam) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return 0;
	return pParent->SendMessage(nMessage, wParam, lParam);
}

LRESULT SageWorkspacePanel::OnResultTableChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_TAECHANG_RESULT_TABLE_CHANGED, wParam, lParam);
}

LRESULT SageWorkspacePanel::OnResultSelectionChanged(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_TAECHANG_RESULT_SELECTION_CHANGED, wParam, lParam);
}

LRESULT SageWorkspacePanel::OnWorkflowRunRequested(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_TAECHANG_WORKFLOW_RUN_REQUESTED, wParam, lParam);
}

LRESULT SageWorkspacePanel::OnWorkflowInputReset(WPARAM wParam, LPARAM lParam) {
	return ForwardToParent(WM_TAECHANG_WORKFLOW_INPUT_RESET, wParam, lParam);
}
