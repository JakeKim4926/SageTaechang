
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "app/ui/frame/SageTaechangDoc.h"
#include "app/ui/view/SageTaechangView.h"
#include "app/ui/drawing/SageUiResources.h"
#include "app/infra/file/TaechangAppSettingsService.h"
#include "app/infra/office/TaechangDeliveryExcelService.h"
#include "app/infra/office/TaechangEstimateExcelService.h"
#include "app/infra/office/TaechangReceivablesExcelService.h"
#include "app/common/SageNumberFormat.h"
#include "app/common/TaechangJson.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/core/workflow/TaechangWorkflowResponse.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "app/ui/dialogs/TaechangPasswordChangeDlg.h"
#include "app/ui/dialogs/TaechangCompanyDlg.h"
#include "app/infra/file/TaechangFileUtils.h"
#include <climits>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct TaechangWorkflowTask {
	HWND m_hWnd;
	int m_nWorkflowType;
	int m_nTaskType;
	CString m_strInputPath;
	CString m_strOutputFolder;
	CString m_strSelectedRowNums;
	BOOL m_bEstimateOnePage;
};

struct TaechangWorkflowResult {
	int m_nWorkflowType;
	int m_nTaskType;
	CString m_strResponseJson;
};

static CString BuildWorkflowPayload(const CString& strInputPath, const CString& strOutputFolder, const CString& strRowNums, BOOL bEstimateOnePage) {
	CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
	if (!strOutputFolder.IsEmpty())
		strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
	if (!strRowNums.IsEmpty())
		strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ROW_NUMS) + L"\":\"" + JsonEscapeString(strRowNums) + L"\"";
	if (bEstimateOnePage)
		strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ESTIMATE_ONE_PAGE) + L"\":true";
	strPayload += L"}";
	return strPayload;
}

static CString GetTaskRequestId(const TaechangWorkflowTask* pTask) {
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
		if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
			return TAECHANG_REQUEST_ESTIMATE_LOAD;
		return TAECHANG_REQUEST_ESTIMATE_GENERATE;
	}
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY) {
		if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
			return TAECHANG_REQUEST_DELIVERY_LOAD;
		return TAECHANG_REQUEST_DELIVERY_GENERATE;
	}
	if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
		return TAECHANG_REQUEST_RECEIVABLES_LOAD;
	return TAECHANG_REQUEST_RECEIVABLES_GENERATE;
}

static UINT RunWorkflowWorker(LPVOID pParam) {
	TaechangWorkflowTask* pTask = reinterpret_cast<TaechangWorkflowTask*>(pParam);
	TaechangWorkflowResult* pResult = new TaechangWorkflowResult();
	pResult->m_nWorkflowType = pTask->m_nWorkflowType;
	pResult->m_nTaskType = pTask->m_nTaskType;

	try {
		CString strPayload = BuildWorkflowPayload(pTask->m_strInputPath, pTask->m_strOutputFolder, pTask->m_strSelectedRowNums, pTask->m_bEstimateOnePage);
		if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
			TaechangEstimateExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_ESTIMATE_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_ESTIMATE_GENERATE, strPayload);
		} else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY) {
			TaechangDeliveryExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_DELIVERY_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_DELIVERY_GENERATE, strPayload);
		} else {
			TaechangReceivablesExcelService service;
			if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
				pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_RECEIVABLES_LOAD, strPayload);
			else
				pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_RECEIVABLES_GENERATE, strPayload);
		}
	} catch (...) {
		pResult->m_strResponseJson = BuildErrorResponse(
			GetTaskRequestId(pTask),
			L"SNX_TAECHANG_WORKFLOW_001",
			TAECHANG_UI_WORKFLOW_EXCEPTION);
	}

	HWND hWnd = pTask->m_hWnd;
	delete pTask;

	if (!::PostMessageW(hWnd, WM_TAECHANG_WORKFLOW_COMPLETE, 0, reinterpret_cast<LPARAM>(pResult)))
		delete pResult;

	return 0;
}

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
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_RUN_REQUESTED, &CSageTaechangView::OnWorkflowRunRequested)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_INPUT_RESET, &CSageTaechangView::OnWorkflowInputReset)
	ON_MESSAGE(WM_TAECHANG_RESULT_TABLE_CHANGED, &CSageTaechangView::OnResultTableChanged)
	ON_MESSAGE(WM_TAECHANG_RESULT_SELECTION_CHANGED, &CSageTaechangView::OnResultSelectionChanged)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
	: m_bRunning(FALSE)
	, m_nLastWorkflowType(0)
	, m_nLastTaskType(0)
	, m_nCurrentWorkflow(TAECHANG_WORKFLOW_DELIVERY)
	, m_hLastWorkflowItem(NULL)
	, m_colorHeaderStatus(TAECHANG_COLOR_SECONDARY_TEXT)
	, m_nHeaderStatusBgRole(SAGE_BG_APP)
	, m_bLastTaskSuccess(FALSE)
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
	ApplyResultTableSchema();
	UpdateWorkflowLabels();
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

SageResultTablePanel* CSageTaechangView::FindResultTablePanel(ISageWorkflowHandler* pHandler) {
	if (pHandler == NULL)
		return NULL;
	return pHandler->UsesInputTable()
		? &m_panelWorkspace.GetInputPanel().GetInputTable()
		: &m_panelWorkspace.GetResultPanel().GetResultTable();
}

void CSageTaechangView::ApplyResultTableSchema() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	if (pPanel == NULL)
		return;

	std::vector<SageWorkflowColumn> arrColumns;
	int nColumnCount = pHandler->GetResultColumnCount(m_nLastTaskType);
	for (int i = 0; i < nColumnCount; ++i)
		arrColumns.push_back(pHandler->GetResultColumn(m_nLastTaskType, i));

	std::vector<SageWorkflowFilterCriteria> arrCriteria;
	int nCriteriaCount = pHandler->GetFilterCriteriaCount();
	for (int i = 0; i < nCriteriaCount; ++i)
		arrCriteria.push_back(pHandler->GetFilterCriteria(i));

	pPanel->SetColumns(arrColumns, pHandler->GetResultStyle(m_nLastTaskType));
	pPanel->SetFilterCriteria(arrCriteria);
}

void CSageTaechangView::SetResultTableRows(const std::vector<TaechangResultRow>& arrRows) {
	SageResultTablePanel* pPanel = FindResultTablePanel(FindCurrentHandler());
	if (pPanel == NULL)
		return;
	pPanel->SetRows(arrRows);
	UpdateResultSummary();
	UpdateActionButtonState();
}

void CSageTaechangView::UpdateResultSummary() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	if (pPanel == NULL)
		return;

	std::vector<SageResultSummaryItem> arrItems;
	if (pHandler->GetWorkflowType() != m_nLastWorkflowType ||
		!pHandler->BuildResultSummary(m_nLastTaskType, pPanel->GetVisibleRows(), m_strLastResponseJson, arrItems)) {
		pPanel->ClearSummary();
		pPanel->ClearTotals();
		return;
	}
	pPanel->SetSummaryItems(arrItems);

	std::vector<SageResultTotalCell> arrTotalCells;
	if (!pHandler->BuildResultTotals(m_nLastTaskType, pPanel->GetVisibleRows(), arrTotalCells)) {
		pPanel->ClearTotals();
		return;
	}
	pPanel->SetTotalCells(arrTotalCells);
}

void CSageTaechangView::RefreshResultTableRows() {
	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	presenter.BuildRows(m_nLastWorkflowType, m_nLastTaskType, m_strLastResponseJson, arrRows);
	SetResultTableRows(arrRows);
}

void CSageTaechangView::UpdateTaskTabVisibility() {
	SageWorkspaceVisibility state;
	state.bInputResetVisible = IsInputResetVisible();
	state.bHasLastResult = (m_nLastTaskType != 0) ? TRUE : FALSE;
	state.bInputTableVisible = IsInputTableVisible();
	state.bOnePageVisible = IsOnePageOptionVisible();
	state.bFilterVisible = IsDocumentResultFilterVisible();
	m_panelWorkspace.UpdateVisibility(state);

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
	UpdateTaskTabVisibility();

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

void CSageTaechangView::UpdateWorkflowLabels() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;
	m_wndHeaderTitle.SetWindowTextW(pHandler->GetHeaderTitle());
	m_panelWorkspace.GetInputPanel().SetSectionLabel(pHandler->GetInputSectionLabel());
	m_panelWorkspace.GetInputPanel().SetActionButtonLabel(pHandler->GetActionButtonLabel());
	m_panelWorkspace.GetInputPanel().SetInputDialogTitle(pHandler->GetInputDialogTitle());
	m_panelWorkspace.GetInputPanel().SetAutoLoadOnInput(pHandler->UsesInputTable());
	m_panelWorkspace.GetHistoryPanel().SetSectionLabel(pHandler->GetDetailSectionLabel());
	m_panelWorkspace.SetWorkflow(m_nCurrentWorkflow, pHandler);
	ApplyResultTableSchema();
	UpdateActionButtonState();
	LayoutChildControls();
}

void CSageTaechangView::UpdateActionButtonState() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;
	ApplyActionButtonState(pHandler->UsesInputTable() ? m_panelWorkspace.GetInputPanel().GetInputTable().GetCheckedRowCount() : 0);
}

void CSageTaechangView::ApplyActionButtonState(int nSelectedCount) {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	BOOL bEnable = m_bRunning ? FALSE : TRUE;
	if (bEnable && pHandler->UsesInputTable())
		bEnable = (nSelectedCount > 0) ? TRUE : FALSE;
	m_panelWorkspace.GetInputPanel().EnableGenerateButton(bEnable);
}

BOOL CSageTaechangView::IsInputTableVisible() const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL || !pHandler->UsesInputTable())
		return FALSE;
	if (m_nLastWorkflowType != pHandler->GetWorkflowType())
		return FALSE;
	return pHandler->UsesCustomResultTable(m_nLastTaskType);
}

BOOL CSageTaechangView::IsOnePageOptionVisible() const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL || !pHandler->UsesOnePageOption())
		return FALSE;
	return IsInputTableVisible();
}

BOOL CSageTaechangView::IsInputResetVisible() const {
	if (m_bRunning || !m_panelWorkspace.IsInputTabSelected())
		return FALSE;
	return IsInputTableVisible();
}

BOOL CSageTaechangView::IsDocumentResultFilterVisible() const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return FALSE;
	if (m_nLastWorkflowType != pHandler->GetWorkflowType())
		return FALSE;
	return pHandler->UsesCustomResultTable(m_nLastTaskType);
}

TaechangWorkflowUiState& CSageTaechangView::GetWorkflowUiState(int nWorkflowType) {
	if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
		return m_stateDelivery;
	if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
		return m_stateEstimate;
	return m_stateReceivables;
}

void CSageTaechangView::SaveWorkflowUiState(int nWorkflowType) {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
	if (pHandler == NULL)
		return;

	TaechangWorkflowUiState& state = GetWorkflowUiState(nWorkflowType);
	state.nSelectedTaskTab = m_panelWorkspace.GetSelectedTab();
	state.nLastWorkflowType = m_nLastWorkflowType;
	state.nLastTaskType = m_nLastTaskType;
	state.bLastTaskSuccess = m_bLastTaskSuccess;
	state.strLastResponseJson = m_strLastResponseJson;
	state.strRunningInputPath = m_strRunningInputPath;
	state.strInputPath = m_panelWorkspace.GetInputPanel().GetInputPath();
	state.strOutputFolder = m_panelWorkspace.GetInputPanel().GetOutputFolder();

	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	state.strCheckedRowNums.Empty();
	if (pPanel == NULL)
		return;
	state.strResultFilterKeyword = pPanel->GetFilterKeyword();
	state.nResultFilterCriteria = pPanel->GetFilterCriteria();
	state.bEstimateOnePage = pPanel->IsOnePageChecked();
	if (IsInputTableVisible())
		state.strCheckedRowNums = pPanel->GetCheckedRowNums();
}

void CSageTaechangView::RestoreWorkflowUiState(int nWorkflowType) {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
	if (pHandler == NULL) {
		m_panelWorkspace.SelectTab(TAECHANG_TAB_INDEX_INPUT);
		m_nLastWorkflowType = 0;
		m_nLastTaskType = 0;
		m_bLastTaskSuccess = FALSE;
		m_strLastResponseJson.Empty();
		m_strRunningInputPath.Empty();
		return;
	}

	TaechangWorkflowUiState& state = GetWorkflowUiState(nWorkflowType);
	m_panelWorkspace.SelectTab(state.nSelectedTaskTab);
	m_nLastWorkflowType = state.nLastWorkflowType;
	m_nLastTaskType = state.nLastTaskType;
	m_bLastTaskSuccess = state.bLastTaskSuccess;
	m_strLastResponseJson = state.strLastResponseJson;
	m_strRunningInputPath = state.strRunningInputPath;
	m_panelWorkspace.GetInputPanel().SetInputPath(state.strInputPath);
	m_panelWorkspace.GetInputPanel().SetOutputFolder(state.strOutputFolder);

	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	if (pPanel == NULL)
		return;
	pPanel->RestoreFilter(state.strResultFilterKeyword, state.nResultFilterCriteria);
	pPanel->SetOnePageChecked(state.bEstimateOnePage);
}

void CSageTaechangView::RebuildCurrentWorkflowResultList() {
	ApplyResultTableSchema();
	if (!IsDocumentResultFilterVisible())
		return;

	RefreshResultTableRows();
	if (!IsInputTableVisible())
		return;

	SageResultTablePanel* pPanel = FindResultTablePanel(FindCurrentHandler());
	if (pPanel != NULL)
		pPanel->RestoreCheckedRowNums(GetWorkflowUiState(GetSelectedWorkflow()).strCheckedRowNums);
}

void CSageTaechangView::OnWorkflowChanged() {
	m_panelWorkspace.SetWorkflow(m_nCurrentWorkflow, FindCurrentHandler());
	RestoreWorkflowUiState(m_nCurrentWorkflow);

	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_wndHeaderTitle.SetWindowTextW(
			m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE
			? TAECHANG_UI_PRICE_MANAGE_NAME
			: TAECHANG_UI_PRICE_CALC_NAME
		);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) {
			m_panelWorkspace.GetPriceManagePanel().RefreshCompanyList();
		} else {
			m_panelWorkspace.GetPriceCalcPanel().RefreshCompanyCombo();
		}
		LayoutChildControls();
		Invalidate(FALSE);
		SetStatusText(TAECHANG_UI_READY);
		return;
	}

	SageResultTablePanel* pPanel = FindResultTablePanel(FindCurrentHandler());
	if (pPanel != NULL)
		pPanel->BeginBatchUpdate();
	UpdateWorkflowLabels();
	RestoreWorkflowUiState(m_nCurrentWorkflow);
	RebuildCurrentWorkflowResultList();
	if (pPanel != NULL)
		pPanel->EndBatchUpdate();
	if (!m_bRunning)
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
	SaveWorkflowUiState(m_nCurrentWorkflow);
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

void CSageTaechangView::OnDropFiles(HDROP hDropInfo) {
	CString strPaths = BuildDroppedPathList(hDropInfo);
	DragFinish(hDropInfo);
	ApplyDroppedInputPaths(strPaths);
}

void CSageTaechangView::ApplyDroppedInputPaths(const CString& strPaths) {
	if (strPaths.IsEmpty() || m_bRunning)
		return;

	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	int nIndex = 0;
	CString strInputPaths = strPaths.Tokenize(L"\r\n", nIndex);
	strInputPaths.Trim();
	if (strInputPaths.IsEmpty())
		return;

	m_panelWorkspace.GetInputPanel().SetInputPath(strInputPaths);
	if (!m_panelWorkspace.IsInputTabSelected()) {
		m_panelWorkspace.SelectTab(TAECHANG_TAB_INDEX_INPUT);
		LayoutChildControls();
	}
	SetStatusText(TAECHANG_UI_DROP_RECEIVED);
	if (pHandler->UsesInputTable())
		RunWorkflowTask(TAECHANG_TASK_LOAD);
}

BOOL CSageTaechangView::ValidateInputPath(CString& strInputPath) {
	strInputPath = m_panelWorkspace.GetInputPanel().GetInputPath();
	strInputPath.Trim();
	if (strInputPath.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_INPUT_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

BOOL CSageTaechangView::ValidateOutputFolder(CString& strOutputFolder) {
	strOutputFolder = m_panelWorkspace.GetInputPanel().GetOutputFolder();
	strOutputFolder.Trim();
	if (strOutputFolder.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_OUTPUT_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

LRESULT CSageTaechangView::OnWorkflowRunRequested(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	RunWorkflowTask(static_cast<int>(wParam));
	return 0;
}

LRESULT CSageTaechangView::OnWorkflowInputReset(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	int nWorkflowType = GetSelectedWorkflow();
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL || !pHandler->UsesInputTable())
		return 0;

	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	if (pPanel == NULL)
		return 0;

	m_panelWorkspace.GetInputPanel().SetInputPath(CString());
	pPanel->RestoreFilter(CString(), pPanel->GetFilterCriteria());
	pPanel->SetOnePageChecked(FALSE);
	pPanel->ClearRows();
	m_strLastResponseJson.Empty();
	m_strRunningInputPath.Empty();
	m_nLastWorkflowType = 0;
	m_nLastTaskType = 0;
	m_bLastTaskSuccess = FALSE;
	ApplyResultTableSchema();
	UpdateTaskTabVisibility();
	LayoutChildControls();
	SetStatusText(TAECHANG_UI_READY);
	SaveWorkflowUiState(nWorkflowType);
	return 0;
}

void CSageTaechangView::RunWorkflowTask(int nTaskType) {
	if (m_bRunning)
		return;

	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	CString strInputPath;
	CString strOutputFolder;
	if (!ValidateInputPath(strInputPath))
		return;

	int nWorkflowType = GetSelectedWorkflow();
	if (nTaskType == TAECHANG_TASK_GENERATE && !ValidateOutputFolder(strOutputFolder))
		return;

	SageResultTablePanel* pPanel = FindResultTablePanel(pHandler);
	CString strSelectedRowNums;
	BOOL bOnePage = (pPanel != NULL) ? pPanel->IsOnePageChecked() : FALSE;
	if (pPanel != NULL && pHandler->UsesInputTable() && nTaskType == TAECHANG_TASK_GENERATE) {
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
			if (!strSelectedRowNums.IsEmpty())
				strSelectedRowNums += TAECHANG_UI_ROW_NUM_SEPARATOR;
			strSelectedRowNums += strNum;
		}
		BOOL bHasSelectedRowNums = strSelectedRowNums.IsEmpty() ? FALSE : TRUE;
		CString strSelectionError;
		if (!pHandler->ValidateSelectedRows(nSelectedCount, bHasSelectedRowNums, bOnePage, strSelectionError)) {
			AfxMessageBox(strSelectionError, MB_ICONWARNING);
			return;
		}
	}

	TaechangWorkflowTask* pTask = new TaechangWorkflowTask();
	pTask->m_hWnd = GetSafeHwnd();
	pTask->m_nWorkflowType = nWorkflowType;
	pTask->m_nTaskType = nTaskType;
	pTask->m_strInputPath = strInputPath;
	pTask->m_strOutputFolder = strOutputFolder;
	pTask->m_strSelectedRowNums = strSelectedRowNums;
	pTask->m_bEstimateOnePage = bOnePage;
	m_strRunningInputPath = strInputPath;

	SetRunningState(TRUE);
	AfxBeginThread(RunWorkflowWorker, pTask, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
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

void CSageTaechangView::SetRunningState(BOOL bRunning) {
	m_bRunning = bRunning;
	m_wndSidebarTree.EnableWindow(!bRunning);
	m_panelWorkspace.GetInputPanel().SetRunningState(bRunning);
	UpdateActionButtonState();
	UpdateTaskTabVisibility();
	LayoutChildControls();
	if (bRunning)
		SetStatusText(TAECHANG_UI_RUNNING);
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

LRESULT CSageTaechangView::OnWorkflowComplete(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	TaechangWorkflowResult* pResult = reinterpret_cast<TaechangWorkflowResult*>(lParam);
	if (pResult != NULL) {
		DisplayResponse(pResult->m_nWorkflowType, pResult->m_nTaskType, pResult->m_strResponseJson);
		delete pResult;
	}
	SetRunningState(FALSE);
	return 0;
}

void CSageTaechangView::DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson) {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
	BOOL bKeepInputTable =
		(nTaskType == TAECHANG_TASK_GENERATE && pHandler != NULL && pHandler->UsesInputTable())
		? TRUE : FALSE;
	m_nLastWorkflowType = nWorkflowType;
	if (!bKeepInputTable) {
		m_nLastTaskType = nTaskType;
		m_strLastResponseJson = strResponseJson;
		ApplyResultTableSchema();
	}

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows);
	m_panelWorkspace.GetHistoryPanel().AppendEntry(m_strRunningInputPath, strResponseJson, bSuccess);

	if (!bKeepInputTable)
		SetResultTableRows(arrRows);

	if (pHandler != NULL && (nTaskType == TAECHANG_TASK_LOAD || nTaskType == TAECHANG_TASK_GENERATE)) {
		m_panelWorkspace.SelectTab(pHandler->UsesInputTable()
			? TAECHANG_TAB_INDEX_INPUT
			: TAECHANG_TAB_INDEX_DOCUMENT_RESULT);
		UpdateTaskTabVisibility();
		LayoutChildControls();
		if (nTaskType == TAECHANG_TASK_GENERATE && bSuccess) {
			LPCWSTR pszCompleted = pHandler->FindGenerateCompletedMessage();
			if (pszCompleted != NULL)
				AfxMessageBox(pszCompleted, MB_ICONINFORMATION);
		}
	}

	m_bLastTaskSuccess = bSuccess;
	m_panelWorkspace.GetInputPanel().SetActionStatusText(
		bSuccess ? TAECHANG_UI_ACTION_STATUS_COMPLETED : TAECHANG_UI_ACTION_STATUS_FAILED, bSuccess);
	SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
	SaveWorkflowUiState(nWorkflowType);
}

LRESULT CSageTaechangView::OnResultTableChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	SaveWorkflowUiState(GetSelectedWorkflow());
	UpdateResultSummary();
	UpdateActionButtonState();
	return 0;
}

LRESULT CSageTaechangView::OnResultSelectionChanged(WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	ApplyActionButtonState(static_cast<int>(wParam));
	return 0;
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


