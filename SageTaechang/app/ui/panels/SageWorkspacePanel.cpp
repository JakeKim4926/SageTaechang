#include "pch.h"
#include "app/ui/panels/SageWorkspacePanel.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "app/common/TaechangJson.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageWorkspacePanel, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TCN_SELCHANGE, ID_TAECHANG_TASK_TABS, &SageWorkspacePanel::OnTabChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_TABLE_CHANGED, &SageWorkspacePanel::OnResultTableChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_SELECTION_CHANGED, &SageWorkspacePanel::OnResultSelectionChanged)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_RUN_REQUESTED, &SageWorkspacePanel::OnWorkflowRunRequested)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_INPUT_RESET, &SageWorkspacePanel::OnWorkflowInputReset)
	ON_MESSAGE(WM_TAECHANG_OPEN_OUTPUT_FOLDER, &SageWorkspacePanel::OnOpenOutputFolder)
	ON_MESSAGE(WM_TAECHANG_VIEW_RESULT_TAB, &SageWorkspacePanel::OnViewResultTab)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &SageWorkspacePanel::OnWorkflowComplete)
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

	ClearStatusCard();
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
	m_panelWorkflowInput.UpdateActionVisibility(state.bInputResetVisible);
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
	UNREFERENCED_PARAMETER(lParam);
	RequestRun(static_cast<int>(wParam));
	return 0;
}

LRESULT SageWorkspacePanel::OnWorkflowInputReset(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	ResetInput();
	return 0;
}

LRESULT SageWorkspacePanel::OnWorkflowComplete(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	SageWorkflowResult* pResult = reinterpret_cast<SageWorkflowResult*>(lParam);
	if (pResult != NULL) {
		DisplayResponse(pResult->m_nWorkflowType, pResult->m_nTaskType, pResult->m_strResponseJson);
		delete pResult;
	}
	SetRunningState(FALSE);
	return 0;
}

SageResultTablePanel* SageWorkspacePanel::FindResultTable() {
	if (m_pHandler == NULL)
		return NULL;
	return m_pHandler->UsesInputTable()
		? &m_panelWorkflowInput.GetInputTable()
		: &m_panelWorkflowResult.GetResultTable();
}

BOOL SageWorkspacePanel::IsRunning() const {
	return m_controller.IsRunning();
}

BOOL SageWorkspacePanel::IsInputTableVisible() const {
	if (m_pHandler == NULL || !m_pHandler->UsesInputTable())
		return FALSE;
	if (m_controller.GetLastWorkflowType() != m_pHandler->GetWorkflowType())
		return FALSE;
	return m_pHandler->UsesCustomResultTable(m_controller.GetLastTaskType());
}

BOOL SageWorkspacePanel::IsOnePageOptionVisible() const {
	if (m_pHandler == NULL || !m_pHandler->UsesOnePageOption())
		return FALSE;
	return IsInputTableVisible();
}

BOOL SageWorkspacePanel::IsInputResetVisible() const {
	if (m_controller.IsRunning() || !IsInputTabSelected())
		return FALSE;
	return IsInputTableVisible();
}

BOOL SageWorkspacePanel::IsResultFilterVisible() const {
	if (m_pHandler == NULL)
		return FALSE;
	if (m_controller.GetLastWorkflowType() != m_pHandler->GetWorkflowType())
		return FALSE;
	return m_pHandler->UsesCustomResultTable(m_controller.GetLastTaskType());
}

void SageWorkspacePanel::RefreshVisibility() {
	SageWorkspaceVisibility state;
	state.bInputResetVisible = IsInputResetVisible();
	state.bInputTableVisible = (IsInputTabSelected() && IsInputTableVisible()) ? TRUE : FALSE;
	state.bOnePageVisible = IsOnePageOptionVisible();
	state.bFilterVisible = IsResultFilterVisible();
	UpdateVisibility(state);
}

void SageWorkspacePanel::ApplyWorkflowLabels(ISageWorkflowHandler* pHandler) {
	if (pHandler == NULL)
		return;
	m_panelWorkflowInput.SetSectionLabel(pHandler->GetInputSectionLabel());
	m_panelWorkflowInput.SetActionButtonLabel(pHandler->GetActionButtonLabel());
	m_panelWorkflowInput.SetInputDialogTitle(pHandler->GetInputDialogTitle());
	m_panelWorkflowInput.SetAutoLoadOnInput(pHandler->UsesInputTable());
	m_panelWorkflowHistory.SetSectionLabel(pHandler->GetDetailSectionLabel());
}

void SageWorkspacePanel::ApplyResultTableSchema() {
	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel == NULL)
		return;

	int nLastTaskType = m_controller.GetLastTaskType();
	std::vector<SageWorkflowColumn> arrColumns;
	int nColumnCount = m_pHandler->GetResultColumnCount(nLastTaskType);
	for (int i = 0; i < nColumnCount; ++i)
		arrColumns.push_back(m_pHandler->GetResultColumn(nLastTaskType, i));

	std::vector<SageWorkflowFilterCriteria> arrCriteria;
	int nCriteriaCount = m_pHandler->GetFilterCriteriaCount();
	for (int i = 0; i < nCriteriaCount; ++i)
		arrCriteria.push_back(m_pHandler->GetFilterCriteria(i));

	pPanel->SetColumns(arrColumns, m_pHandler->GetResultStyle(nLastTaskType));
	pPanel->SetFilterCriteria(arrCriteria);
}

void SageWorkspacePanel::SetResultTableRows(const std::vector<TaechangResultRow>& arrRows) {
	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel == NULL)
		return;
	pPanel->SetRows(arrRows);
	UpdateResultSummary();
	UpdateActionButtonState();
}

void SageWorkspacePanel::UpdateResultSummary() {
	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel == NULL)
		return;

	int nLastTaskType = m_controller.GetLastTaskType();
	std::vector<SageResultSummaryItem> arrItems;
	if (m_pHandler->GetWorkflowType() != m_controller.GetLastWorkflowType() ||
		!m_pHandler->BuildResultSummary(nLastTaskType, pPanel->GetVisibleRows(),
			m_controller.GetLastResponseJson(), arrItems)) {
		pPanel->ClearSummary();
		pPanel->ClearTotals();
		return;
	}
	pPanel->SetSummaryItems(arrItems);

	std::vector<SageResultTotalCell> arrTotalCells;
	if (!m_pHandler->BuildResultTotals(nLastTaskType, pPanel->GetVisibleRows(), arrTotalCells)) {
		pPanel->ClearTotals();
		return;
	}
	pPanel->SetTotalCells(arrTotalCells);
}

void SageWorkspacePanel::UpdateActionButtonState() {
	if (m_pHandler == NULL)
		return;
	ApplyActionButtonState(m_pHandler->UsesInputTable()
		? m_panelWorkflowInput.GetInputTable().GetCheckedRowCount()
		: 0);
}

void SageWorkspacePanel::ApplyActionButtonState(int nSelectedCount) {
	if (m_pHandler == NULL)
		return;

	BOOL bEnable = m_controller.IsRunning() ? FALSE : TRUE;
	if (bEnable && m_pHandler->UsesInputTable())
		bEnable = (nSelectedCount > 0) ? TRUE : FALSE;
	m_panelWorkflowInput.EnableGenerateButton(bEnable);
}

void SageWorkspacePanel::SetRunningState(BOOL bRunning) {
	m_panelWorkflowInput.SetRunningState(bRunning);
	UpdateActionButtonState();
	RefreshVisibility();
	LayoutActivePanel();
	if (bRunning)
		NotifyStatus(TAECHANG_UI_RUNNING);
}

BOOL SageWorkspacePanel::ValidateInputPath(CString& strInputPath) const {
	strInputPath = m_panelWorkflowInput.GetInputPath();
	strInputPath.Trim();
	if (!strInputPath.IsEmpty())
		return TRUE;
	AfxMessageBox(TAECHANG_UI_INPUT_REQUIRED, MB_ICONWARNING);
	return FALSE;
}

BOOL SageWorkspacePanel::ValidateOutputFolder(CString& strOutputFolder) const {
	strOutputFolder = m_panelWorkflowInput.GetOutputFolder();
	strOutputFolder.Trim();
	if (!strOutputFolder.IsEmpty())
		return TRUE;
	AfxMessageBox(TAECHANG_UI_OUTPUT_REQUIRED, MB_ICONWARNING);
	return FALSE;
}

BOOL SageWorkspacePanel::BuildSelectedRowNums(int nTaskType, CString& strRowNums, BOOL& bOnePage) {
	SageResultTablePanel* pPanel = FindResultTable();
	bOnePage = (pPanel != NULL) ? pPanel->IsOnePageChecked() : FALSE;
	strRowNums.Empty();
	if (pPanel == NULL || !m_pHandler->UsesInputTable() || nTaskType != TAECHANG_TASK_GENERATE)
		return TRUE;

	int nSelectedCount = 0;
	int nRowCount = pPanel->GetRowCount();
	for (int i = 0; i < nRowCount; ++i) {
		if (!pPanel->IsRowChecked(i))
			continue;
		++nSelectedCount;
		DWORD_PTR nSourceRowIndex = pPanel->GetRowData(i);
		if (nSourceRowIndex == 0)
			continue;
		CString strNum;
		strNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		if (!strRowNums.IsEmpty())
			strRowNums += TAECHANG_UI_ROW_NUM_SEPARATOR;
		strRowNums += strNum;
	}

	CString strSelectionError;
	if (m_pHandler->ValidateSelectedRows(nSelectedCount, strRowNums.IsEmpty() ? FALSE : TRUE, bOnePage, strSelectionError))
		return TRUE;
	AfxMessageBox(strSelectionError, MB_ICONWARNING);
	return FALSE;
}

void SageWorkspacePanel::RequestRun(int nTaskType) {
	if (m_controller.IsRunning() || m_pHandler == NULL)
		return;

	CString strInputPath;
	CString strOutputFolder;
	if (!ValidateInputPath(strInputPath))
		return;
	if (nTaskType == TAECHANG_TASK_GENERATE && !ValidateOutputFolder(strOutputFolder))
		return;

	CString strSelectedRowNums;
	BOOL bOnePage = FALSE;
	if (!BuildSelectedRowNums(nTaskType, strSelectedRowNums, bOnePage))
		return;

	SageWorkflowRunRequest request;
	request.hNotifyWnd = GetSafeHwnd();
	request.nWorkflowType = m_nCurrentWorkflow;
	request.nTaskType = nTaskType;
	request.strInputPath = strInputPath;
	request.strOutputFolder = strOutputFolder;
	request.strSelectedRowNums = strSelectedRowNums;
	request.bEstimateOnePage = bOnePage;

	CString strError;
	if (!m_controller.Start(request, strError)) {
		AfxMessageBox(strError, MB_ICONWARNING);
		return;
	}
	SetRunningState(TRUE);
}

void SageWorkspacePanel::ResetInput() {
	if (m_pHandler == NULL || !m_pHandler->UsesInputTable())
		return;

	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel == NULL)
		return;

	m_panelWorkflowInput.SetInputPath(CString());
	ClearStatusCard();
	pPanel->RestoreFilter(CString(), pPanel->GetFilterCriteria());
	pPanel->SetOnePageChecked(FALSE);
	pPanel->ClearRows();
	m_controller.ClearResult();
	ApplyResultTableSchema();
	RefreshVisibility();
	LayoutActivePanel();
	NotifyStatus(TAECHANG_UI_READY);
	ForwardToParent(WM_TAECHANG_WORKSPACE_STATE_CHANGED, 0, 0);
}

void SageWorkspacePanel::ApplyDroppedInputPaths(const CString& strPaths) {
	if (strPaths.IsEmpty() || m_controller.IsRunning() || m_pHandler == NULL)
		return;

	int nIndex = 0;
	CString strInputPaths = strPaths.Tokenize(TAECHANG_UI_DROP_PATH_SEPARATOR, nIndex);
	strInputPaths.Trim();
	if (strInputPaths.IsEmpty())
		return;

	m_panelWorkflowInput.SetInputPath(strInputPaths);
	if (!IsInputTabSelected()) {
		SelectTab(TAECHANG_TAB_INDEX_INPUT);
		RefreshVisibility();
		LayoutActivePanel();
	}
	NotifyStatus(TAECHANG_UI_DROP_RECEIVED);
	if (m_pHandler->UsesInputTable())
		RequestRun(TAECHANG_TASK_LOAD);
}

void SageWorkspacePanel::DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson) {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
	BOOL bKeepInputTable =
		(nTaskType == TAECHANG_TASK_GENERATE && pHandler != NULL && pHandler->UsesInputTable())
		? TRUE : FALSE;

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows);

	m_controller.Finish(nWorkflowType, nTaskType, strResponseJson, bSuccess, bKeepInputTable);
	if (!bKeepInputTable)
		ApplyResultTableSchema();

	m_panelWorkflowHistory.AppendEntry(m_controller.GetRunningInputPath(), strResponseJson, bSuccess);
	if (!bKeepInputTable)
		SetResultTableRows(arrRows);

	if (pHandler != NULL && (nTaskType == TAECHANG_TASK_LOAD || nTaskType == TAECHANG_TASK_GENERATE)) {
		SelectTab(pHandler->UsesInputTable()
			? TAECHANG_TAB_INDEX_INPUT
			: TAECHANG_TAB_INDEX_DOCUMENT_RESULT);
		RefreshVisibility();
		LayoutActivePanel();
		if (nTaskType == TAECHANG_TASK_GENERATE && bSuccess) {
			LPCWSTR pszCompleted = pHandler->FindGenerateCompletedMessage();
			if (pszCompleted != NULL)
				AfxMessageBox(pszCompleted, MB_ICONINFORMATION);
		}
	}

	int nResultCount = bKeepInputTable
		? m_panelWorkflowInput.GetInputTable().GetCheckedRowCount()
		: static_cast<int>(arrRows.size());
	ApplyStatusCardResult(pHandler, nTaskType, strResponseJson, bSuccess, nResultCount);
	NotifyStatus(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
	ForwardToParent(WM_TAECHANG_WORKSPACE_STATE_CHANGED, 0, 0);
}

void SageWorkspacePanel::ApplyStatusCardResult(
	ISageWorkflowHandler* pHandler,
	int nTaskType,
	const CString& strResponseJson,
	BOOL bSuccess,
	int nResultCount) {
	if (pHandler == NULL)
		return;

	m_strLastOutputPath.Empty();
	CString strMessage;
	CString strDetail;
	if (!bSuccess) {
		if (nTaskType == TAECHANG_TASK_LOAD)
			strMessage = TAECHANG_UI_STATUS_CARD_LOAD_FAILED;
		else
			strMessage.Format(TAECHANG_UI_STATUS_CARD_FAILED_FORMAT, pHandler->GetActionButtonLabel());
		strDetail = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_MESSAGE);
		if (strDetail.IsEmpty())
			strDetail = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_CODE);
		m_panelWorkflowInput.SetStatusResult(FALSE, strMessage, strDetail, FALSE);
		return;
	}

	if (nTaskType == TAECHANG_TASK_LOAD)
		strMessage.Format(TAECHANG_UI_STATUS_CARD_LOAD_COMPLETED_FORMAT, nResultCount);
	else
		strMessage.Format(TAECHANG_UI_STATUS_CARD_COMPLETED_FORMAT, pHandler->GetActionButtonLabel(), nResultCount);

	strDetail = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_FILE_PATH);
	if (strDetail.IsEmpty())
		strDetail = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_OUTPUT_FOLDER);
	m_strLastOutputPath = strDetail;
	m_panelWorkflowInput.SetStatusResult(TRUE, strMessage, strDetail, HasResultTab());
}

BOOL SageWorkspacePanel::HasResultTab() const {
	if (m_pHandler == NULL)
		return FALSE;
	int nTabCount = m_pHandler->GetTabCount();
	for (int nVisualTabIndex = 0; nVisualTabIndex < nTabCount; ++nVisualTabIndex) {
		if (m_pHandler->GetTab(nVisualTabIndex).nSemanticIndex == TAECHANG_TAB_INDEX_DOCUMENT_RESULT)
			return TRUE;
	}
	return FALSE;
}

void SageWorkspacePanel::ClearStatusCard() {
	m_strLastOutputPath.Empty();
	m_panelWorkflowInput.ResetStatusCard();
}

LRESULT SageWorkspacePanel::OnOpenOutputFolder(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (m_strLastOutputPath.IsEmpty())
		return 0;

	DWORD dwAttributes = ::GetFileAttributesW(m_strLastOutputPath);
	if (dwAttributes == INVALID_FILE_ATTRIBUTES) {
		AfxMessageBox(TAECHANG_UI_OUTPUT_PATH_MISSING, MB_ICONWARNING);
		return 0;
	}

	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		::ShellExecuteW(GetSafeHwnd(), TAECHANG_UI_EXPLORER_VERB_OPEN,
			m_strLastOutputPath, NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}

	CString strArguments;
	strArguments.Format(TAECHANG_UI_EXPLORER_SELECT_FORMAT, static_cast<LPCWSTR>(m_strLastOutputPath));
	::ShellExecuteW(GetSafeHwnd(), NULL, TAECHANG_UI_EXPLORER_COMMAND,
		strArguments, NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT SageWorkspacePanel::OnViewResultTab(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	if (!HasResultTab())
		return 0;

	SelectTab(TAECHANG_TAB_INDEX_DOCUMENT_RESULT);
	RefreshVisibility();
	LayoutActivePanel();
	ForwardToParent(WM_TAECHANG_WORKSPACE_TAB_CHANGED, 0, 0);
	return 0;
}

void SageWorkspacePanel::NotifyStatus(LPCWSTR pszStatus) {
	CWnd* pParent = GetParent();
	if (pParent == NULL || !::IsWindow(pParent->GetSafeHwnd()))
		return;
	pParent->SendMessage(WM_TAECHANG_WORKSPACE_STATUS, 0, reinterpret_cast<LPARAM>(pszStatus));
}

SageWorkflowUiState& SageWorkspacePanel::GetWorkflowState(int nWorkflowType) {
	if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
		return m_stateDelivery;
	if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
		return m_stateEstimate;
	return m_stateReceivables;
}

void SageWorkspacePanel::SaveWorkflowState(int nWorkflowType) {
	if (SageWorkflowRegistry::FindHandler(nWorkflowType) == NULL)
		return;

	SageWorkflowUiState& state = GetWorkflowState(nWorkflowType);
	state.nSelectedTaskTab = m_nSelectedTaskTab;
	state.result = m_controller.CaptureResult();
	state.strInputPath = m_panelWorkflowInput.GetInputPath();
	state.strOutputFolder = m_panelWorkflowInput.GetOutputFolder();

	SageResultTablePanel* pPanel = FindResultTable();
	state.strCheckedRowNums.Empty();
	if (pPanel == NULL)
		return;
	state.strResultFilterKeyword = pPanel->GetFilterKeyword();
	state.nResultFilterCriteria = pPanel->GetFilterCriteria();
	state.bEstimateOnePage = pPanel->IsOnePageChecked();
	if (IsInputTableVisible())
		state.strCheckedRowNums = pPanel->GetCheckedRowNums();
}

void SageWorkspacePanel::RestoreWorkflowState(int nWorkflowType) {
	if (SageWorkflowRegistry::FindHandler(nWorkflowType) == NULL) {
		SelectTab(TAECHANG_TAB_INDEX_INPUT);
		m_controller.ClearResult();
		return;
	}

	SageWorkflowUiState& state = GetWorkflowState(nWorkflowType);
	SelectTab(state.nSelectedTaskTab);
	m_controller.RestoreResult(state.result);
	m_panelWorkflowInput.SetInputPath(state.strInputPath);
	m_panelWorkflowInput.SetOutputFolder(state.strOutputFolder);

	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel == NULL)
		return;
	pPanel->RestoreFilter(state.strResultFilterKeyword, state.nResultFilterCriteria);
	pPanel->SetOnePageChecked(state.bEstimateOnePage);
}

void SageWorkspacePanel::RebuildResultTable() {
	SageResultTablePanel* pPanel = FindResultTable();
	if (pPanel != NULL)
		pPanel->BeginBatchUpdate();

	ApplyResultTableSchema();
	if (IsResultFilterVisible()) {
		TaechangWorkflowResultPresenter presenter;
		std::vector<TaechangResultRow> arrRows;
		presenter.BuildRows(m_controller.GetLastWorkflowType(), m_controller.GetLastTaskType(),
			m_controller.GetLastResponseJson(), arrRows);
		SetResultTableRows(arrRows);
		if (IsInputTableVisible() && pPanel != NULL)
			pPanel->RestoreCheckedRowNums(GetWorkflowState(m_nCurrentWorkflow).strCheckedRowNums);
	}
	UpdateActionButtonState();

	if (pPanel != NULL)
		pPanel->EndBatchUpdate();
}
