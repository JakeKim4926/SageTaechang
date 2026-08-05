
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
#include "app/infra/db/SageDBMgr.h"
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
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TVN_SELCHANGED, ID_TAECHANG_SIDEBAR_TREE, &CSageTaechangView::OnSidebarSelectionChanged)
	ON_NOTIFY(TCN_SELCHANGE, ID_TAECHANG_TASK_TABS, &CSageTaechangView::OnTaskTabChanged)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_INPUT, &CSageTaechangView::OnSelectInput)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_OUTPUT, &CSageTaechangView::OnSelectOutput)
	ON_BN_CLICKED(ID_TAECHANG_LOAD_WORKFLOW, &CSageTaechangView::OnLoadWorkflow)
	ON_BN_CLICKED(ID_TAECHANG_GENERATE_WORKFLOW, &CSageTaechangView::OnGenerateWorkflow)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_ALL, &CSageTaechangView::OnSelectAll)
	ON_BN_CLICKED(ID_TAECHANG_ESTIMATE_ONE_PAGE, &CSageTaechangView::OnEstimateOnePage)
	ON_BN_CLICKED(ID_TAECHANG_INPUT_RESET_BTN, &CSageTaechangView::OnInputReset)
	ON_BN_CLICKED(ID_TAECHANG_LOGIN_BTN, &CSageTaechangView::OnLogin)
	ON_BN_CLICKED(ID_TAECHANG_LOGOUT_BTN, &CSageTaechangView::OnLogout)
	ON_BN_CLICKED(ID_TAECHANG_RESULT_SEARCH_BTN, &CSageTaechangView::OnResultSearch)
	ON_BN_CLICKED(ID_TAECHANG_RESULT_RESET_BTN, &CSageTaechangView::OnResultFilterReset)
	ON_CBN_SELCHANGE(ID_TAECHANG_RESULT_FILTER_CRITERIA, &CSageTaechangView::OnResultFilterCriteriaChanged)
	ON_BN_CLICKED(ID_COORDER_ADD_BTN, &CSageTaechangView::OnCoAdd)
	ON_BN_CLICKED(ID_COORDER_MODIFY_BTN, &CSageTaechangView::OnCoModify)
	ON_BN_CLICKED(ID_COORDER_DELETE_BTN, &CSageTaechangView::OnCoDelete)
	ON_BN_CLICKED(ID_COORDER_CANCEL_BTN, &CSageTaechangView::OnCoCancel)
	ON_BN_CLICKED(ID_COORDER_SEARCH_BTN, &CSageTaechangView::OnCoSearch)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_COORDER_LIST, &CSageTaechangView::OnCoListSelChanged)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
	ON_WM_DROPFILES()
	ON_NOTIFY(LVN_ITEMCHANGED, ID_TAECHANG_RESULT_LIST, &CSageTaechangView::OnResultListItemChanged)
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
	: m_bRunning(FALSE)
	, m_nProgressPercent(0)
	, m_nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT)
	, m_nLastWorkflowType(0)
	, m_nLastTaskType(0)
	, m_nResultFilterCriteria(TAECHANG_FILTER_CRITERIA_NONE)
	, m_nCurrentWorkflow(TAECHANG_WORKFLOW_DELIVERY)
	, m_hLastWorkflowItem(NULL)
	, m_colorHeaderStatus(TAECHANG_COLOR_SECONDARY_TEXT)
	, m_nHeaderStatusBgRole(SAGE_BG_APP)
	, m_bLastTaskSuccess(FALSE)
	, m_nAuthDividerX(0)
	, m_nCoPanelState(TAECHANG_CO_PANEL_IDLE)
	, m_nCoSelectedOrderId(0)
	, m_rectCoCard(0, 0, 0, 0)
	, m_rectResultFilterBox(0, 0, 0, 0) {
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
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
		pMsg->hwnd == m_wndResultFilter.GetSafeHwnd() && IsDocumentResultFilterVisible()) {
		OnResultSearch();
		return TRUE;
	}
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
		pMsg->hwnd == m_wndCoSearchEdit.GetSafeHwnd() && IsDataManageTab()) {
		OnCoSearch();
		return TRUE;
	}
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && IsDataManageTab()) {
		if (pMsg->hwnd == m_wndCoOrderEdit.GetSafeHwnd()) {
			m_wndCoCompanyEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndCoCompanyEdit.GetSafeHwnd()) {
			m_wndCoOrderEdit.SetFocus();
			return TRUE;
		}
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
	m_wndHeaderTitle.Create(TAECHANG_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE, rectEmpty, this);
	m_wndHeaderStatus.Create(TAECHANG_UI_READY, WS_CHILD | SS_RIGHT, rectEmpty, this);
	m_wndTaskTabs.Create(WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH, rectEmpty, this, ID_TAECHANG_TASK_TABS);
	m_wndInputSection.Create(TAECHANG_UI_SECTION_INPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_INPUT_SECTION);
	m_wndOutputSection.Create(TAECHANG_UI_SECTION_OUTPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_OUTPUT_SECTION);
	m_wndResultSection.Create(TAECHANG_UI_SECTION_RESULT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RESULT_SECTION);
	m_wndDetailSection.Create(TAECHANG_UI_SECTION_DETAIL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_DETAIL_SECTION);
	m_wndTitle.Create(TAECHANG_UI_APP_TITLE, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
	m_wndWorkflowLabel.Create(TAECHANG_UI_WORKFLOW_LABEL, WS_CHILD, rectEmpty, this);
	m_wndInputLabel.Create(TAECHANG_UI_INPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
	m_wndOutputLabel.Create(TAECHANG_UI_OUTPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
	m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, rectEmpty, this, ID_TAECHANG_INPUT_EDIT);
	m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, rectEmpty, this, ID_TAECHANG_OUTPUT_EDIT);
	m_wndSelectInput.Create(TAECHANG_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_SELECT_INPUT);
	m_wndSelectOutput.Create(TAECHANG_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_SELECT_OUTPUT);
	m_wndLoad.Create(TAECHANG_UI_LOAD_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOAD_WORKFLOW);
	m_wndGenerate.Create(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_GENERATE_WORKFLOW);
	m_wndGenerate.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndSelectAll.Create(TAECHANG_UI_SELECT_ALL_BUTTON, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_SELECT_ALL);
	m_wndEstimateOnePage.Create(TAECHANG_UI_ESTIMATE_ONE_PAGE_CHECK, WS_CHILD | BS_AUTOCHECKBOX, rectEmpty, this, ID_TAECHANG_ESTIMATE_ONE_PAGE);
	m_wndInputReset.Create(TAECHANG_UI_INPUT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_INPUT_RESET_BTN);
	m_wndInputReset.SetVariant(SAGE_BUTTON_GHOST);
	m_wndInputReset.SetSurfaceColor(TAECHANG_COLOR_APP_BACKGROUND);
	m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, rectEmpty, this, ID_TAECHANG_PROGRESS);
	m_wndProgressText.Create(L"", WS_CHILD | WS_VISIBLE | SS_RIGHT, rectEmpty, this);
	m_wndResultList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, rectEmpty, this, ID_TAECHANG_RESULT_LIST);
	m_wndResultList.SetAlternateRowColor(TRUE);
	{
		CHeaderCtrl* pHeader = m_wndResultList.GetHeaderCtrl();
		if (pHeader && pHeader->GetSafeHwnd()) {
			m_wndResultHeader.SubclassWindow(pHeader->GetSafeHwnd());
			SetWindowTheme(m_wndResultHeader.GetSafeHwnd(), L"", L"");
		}
	}
	m_wndResultFilterCriteria.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_RESULT_FILTER_CRITERIA);
	m_wndResultFilter.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, rectEmpty, this, ID_TAECHANG_RESULT_FILTER_EDIT);
	m_wndResultSearchBtn.Create(TAECHANG_UI_RESULT_SEARCH_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RESULT_SEARCH_BTN);
	m_wndResultSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndResultResetBtn.Create(TAECHANG_UI_RESULT_RESET_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RESULT_RESET_BTN);
	m_wndResultResetBtn.SetVariant(SAGE_BUTTON_GHOST);
	m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);
	m_wndEmptyStateHint.Create(TAECHANG_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
	m_wndActionStatus.Create(L"", WS_CHILD | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

	m_wndLoginBtn.Create(TAECHANG_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGIN_BTN);
	m_wndLogoutBtn.Create(TAECHANG_UI_LOGOUT_BTN, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGOUT_BTN);
	m_wndUserLabel.Create(L"", WS_CHILD | SS_CENTERIMAGE | SS_NOPREFIX, rectEmpty, this, ID_TAECHANG_USER_LABEL);

	m_wndResultList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndResultFilter.LimitText(20);
	EnableFileDropForWindow(m_wndInputSection);
	EnableFileDropForWindow(m_wndInputPath);
	EnableFileDropForWindow(m_wndSelectInput);
	EnableFileDropForWindow(m_wndResultSection);
	EnableFileDropForWindow(m_wndResultList);
	EnableFileDropForWindow(m_wndEmptyStateHint);
	m_wndProgress.SetMarquee(FALSE, 0);
	m_wndProgress.SetRange(0, TAECHANG_PROGRESS_COMPLETE);
	UpdateProgressPercent(0);

	m_panelPriceManage.Create(this, ID_PRICE_MANAGE_PANEL);
	m_panelPriceCalc.Create(this, ID_CALC_PANEL);
	CreateCompanyOrderPanel();

	ApplyControlFonts();
	ApplyLabelRoles();
	ApplyWorkflowTabs();
	ApplyResultColumns();
	UpdateWorkflowLabels();
	UpdateResultColumns();
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
	m_wndTaskTabs.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOutputSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResultSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndDetailSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputPath.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndOutputFolder.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectInput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectOutput.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLoad.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndGenerate.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndSelectAll.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndEstimateOnePage.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndInputReset.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndResultList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndResultHeader.GetSafeHwnd()))
		m_wndResultHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	m_wndResultFilterCriteria.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResultFilterCriteria.SetItemHeight(-1, TAECHANG_RESULT_CRITERIA_ITEM_HEIGHT);
	m_wndResultFilterCriteria.SetItemHeight(0, TAECHANG_RESULT_CRITERIA_ITEM_HEIGHT);
	m_wndResultFilter.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResultSearchBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndResultResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_HEADER));
	m_wndDetail.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndActionStatus.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLoginBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndLogoutBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoCrudSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoListSection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoAddBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoModifyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoDeleteBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoCancelBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoSearchEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoSearchBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoOrderEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoCompanyEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCoList.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
	if (::IsWindow(m_wndCoListHeader.GetSafeHwnd()))
		m_wndCoListHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_LIST));
}

void CSageTaechangView::ApplyLabelRoles() {
	m_wndTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR);
	m_wndTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndTitle.SetFontRole(SAGE_FONT_LOGO);

	m_wndSidebarTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR_CATEGORY);
	m_wndSidebarTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndSidebarTitle.SetFontRole(SAGE_FONT_CONTROL);

	m_wndHeaderTitle.SetTextColorRole(SAGE_TEXT_PRIMARY);
	m_wndHeaderTitle.SetFontRole(SAGE_FONT_HEADER);

	m_wndUserLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndUserLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndEmptyStateHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndEmptyStateHint.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoSearchLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCoSearchLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoOrderLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCoOrderLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCoOrderLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCoNameLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndCoNameLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCoNameLabel.SetFontRole(SAGE_FONT_CONTENT);
}

void CSageTaechangView::ApplyWorkflowTabs() {
	m_wndTaskTabs.DeleteAllItems();
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;
	int nTabCount = pHandler->GetTabCount();
	for (int nVisualTabIndex = 0; nVisualTabIndex < nTabCount; ++nVisualTabIndex)
		m_wndTaskTabs.InsertItem(nVisualTabIndex, pHandler->GetTab(nVisualTabIndex).pszLabel);
	m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
	UpdateTaskTabVisibility();
}

void CSageTaechangView::ApplyResultColumns() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;

	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	SageWorkflowResultStyle resultStyle = pHandler->GetResultStyle(m_nLastTaskType);
	DWORD dwExtStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	if (resultStyle.bCheckbox)
		dwExtStyle |= LVS_EX_CHECKBOXES;
	m_wndResultList.SetExtendedStyle(dwExtStyle);
	m_wndResultList.SetRowSeparator(resultStyle.bGridLines);
	m_wndResultList.SetHighlightColumns(resultStyle.nHighlightStart, resultStyle.nHighlightCount);
	m_wndResultList.SetGroupColumn(resultStyle.nGroupColumn);

	m_wndResultList.DeleteAllItems();
	CHeaderCtrl* pHeader = m_wndResultList.GetHeaderCtrl();
	int nOldColumnCount = (pHeader != NULL) ? pHeader->GetItemCount() : 0;
	for (int i = nOldColumnCount - 1; i >= 0; --i)
		m_wndResultList.DeleteColumn(i);

	int nColumnCount = pHandler->GetResultColumnCount(m_nLastTaskType);
	for (int i = 0; i < nColumnCount; ++i) {
		const SageWorkflowColumn& column = pHandler->GetResultColumn(m_nLastTaskType, i);
		int nFormat = (column.nAlign == SAGE_COLUMN_ALIGN_RIGHT) ? LVCFMT_RIGHT : LVCFMT_LEFT;
		m_wndResultList.InsertColumn(i, column.pszLabel, nFormat, column.nWidth);
	}
}

void CSageTaechangView::UpdateTaskTabVisibility() {
	BOOL bShowInput = IsInputTabSelected();
	BOOL bShowOutput = bShowInput;
	BOOL bShowAction = IsActionTabVisible();
	BOOL bShowResult = IsResultTab() || (IsInputTabSelected() && IsInputTableVisible());
	BOOL bShowDetail = IsDetailTab();

	m_wndInputSection.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndInputLabel.ShowWindow(SW_HIDE);
	m_wndInputPath.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndSelectInput.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndOutputSection.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
	m_wndOutputLabel.ShowWindow(SW_HIDE);
	m_wndOutputFolder.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
	m_wndSelectOutput.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);

	BOOL bShowDataManage = IsDataManageTab();
	BOOL bShowHint = (!bShowResult && !bShowDetail && !m_bRunning && !bShowDataManage) ? TRUE : FALSE;

	m_wndLoad.ShowWindow(SW_HIDE);
	m_wndGenerate.ShowWindow(bShowAction ? SW_SHOW : SW_HIDE);
	BOOL bShowInputReset = (bShowAction && IsInputResetVisible()) ? TRUE : FALSE;
	m_wndInputReset.ShowWindow(bShowInputReset ? SW_SHOW : SW_HIDE);
	BOOL bShowSelectAll = (bShowAction && IsInputTableVisible()) ? TRUE : FALSE;
	m_wndSelectAll.ShowWindow(bShowSelectAll ? SW_SHOW : SW_HIDE);
	BOOL bShowEstimateOnePage = (bShowAction && IsOnePageOptionVisible()) ? TRUE : FALSE;
	m_wndEstimateOnePage.ShowWindow(bShowEstimateOnePage ? SW_SHOW : SW_HIDE);
	BOOL bShowActionStatus = (bShowAction && !m_bRunning && m_nLastTaskType != 0 && !bShowInputReset) ? TRUE : FALSE;
	m_wndProgress.ShowWindow((bShowAction && m_bRunning) ? SW_SHOW : SW_HIDE);
	m_wndProgressText.ShowWindow((bShowAction && m_bRunning) ? SW_SHOW : SW_HIDE);
	m_wndActionStatus.ShowWindow(bShowActionStatus ? SW_SHOW : SW_HIDE);

	m_wndResultSection.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
	m_wndResultList.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
	BOOL bShowResultFilter = (bShowResult && IsDocumentResultFilterVisible()) ? TRUE : FALSE;
	m_wndResultFilterCriteria.ShowWindow(bShowResultFilter ? SW_SHOW : SW_HIDE);
	if (bShowResultFilter)
		PopulateResultFilterCriteria();
	else
		SetCardRect(m_rectResultFilterBox, CRect(0, 0, 0, 0));
	m_wndResultFilter.ShowWindow(bShowResultFilter ? SW_SHOW : SW_HIDE);
	m_wndResultSearchBtn.ShowWindow(bShowResultFilter ? SW_SHOW : SW_HIDE);
	m_wndResultResetBtn.ShowWindow(bShowResultFilter ? SW_SHOW : SW_HIDE);
	m_wndDetailSection.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
	m_wndDetail.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
	m_wndEmptyStateHint.ShowWindow(bShowHint ? SW_SHOW : SW_HIDE);
	ShowCompanyOrderPanel(bShowDataManage);
}

void CSageTaechangView::UpdateResultColumns() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;

	CRect rectList;
	m_wndResultList.GetClientRect(&rectList);
	int nWidth = rectList.Width();
	if (nWidth <= 0)
		return;

	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	int nColumnCount = pHandler->GetResultColumnCount(m_nLastTaskType);
	int nFixedWidth = 0;
	int nDefinedWidth = 0;
	BOOL bHasStretchColumn = FALSE;
	for (int i = 0; i < nColumnCount; ++i) {
		const SageWorkflowColumn& column = pHandler->GetResultColumn(m_nLastTaskType, i);
		nDefinedWidth += column.nWidth;
		if (column.bStretch)
			bHasStretchColumn = TRUE;
		else
			nFixedWidth += column.nWidth;
	}

	if (bHasStretchColumn) {
		for (int i = 0; i < nColumnCount; ++i) {
			const SageWorkflowColumn& column = pHandler->GetResultColumn(m_nLastTaskType, i);
			int nColumnWidth = column.nWidth;
			if (column.bStretch && nWidth - nFixedWidth > nColumnWidth)
				nColumnWidth = nWidth - nFixedWidth;
			m_wndResultList.SetColumnWidth(i, nColumnWidth);
		}
		return;
	}

	int nAssignedWidth = 0;
	for (int i = 0; i < nColumnCount; ++i) {
		const SageWorkflowColumn& column = pHandler->GetResultColumn(m_nLastTaskType, i);
		int nColumnWidth = column.nWidth;
		if (nWidth > nDefinedWidth) {
			nColumnWidth = (i == nColumnCount - 1)
				? nWidth - nAssignedWidth
				: ::MulDiv(column.nWidth, nWidth, nDefinedWidth);
		}
		nAssignedWidth += nColumnWidth;
		m_wndResultList.SetColumnWidth(i, nColumnWidth);
	}
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
	int nSidebarTop = 0;
	int nSidebarHeight = rectClient.Height();
	int nContentLeft = TAECHANG_SIDEBAR_WIDTH + TAECHANG_MARGIN;
	int nContentTop = TAECHANG_MARGIN;
	int nContentWidth = rectClient.Width() - nContentLeft - TAECHANG_MARGIN;
	int nContentHeight = rectClient.Height() - (TAECHANG_MARGIN * 2);

	m_wndTitle.MoveWindow(TAECHANG_MARGIN, nSidebarTop + TAECHANG_MARGIN, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), TAECHANG_TOP_BAR_HEIGHT - (TAECHANG_MARGIN * 2));
	m_wndSidebarTitle.MoveWindow(TAECHANG_MARGIN, TAECHANG_TOP_BAR_HEIGHT, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), TAECHANG_SIDEBAR_TITLE_HEIGHT);
	m_wndSidebarTree.MoveWindow(TAECHANG_MARGIN, TAECHANG_TOP_BAR_HEIGHT + TAECHANG_SIDEBAR_TITLE_HEIGHT, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), nSidebarHeight - TAECHANG_TOP_BAR_HEIGHT - TAECHANG_SIDEBAR_TITLE_HEIGHT - TAECHANG_MARGIN);

	{
		int nLoginBtnTop = (TAECHANG_TOP_BAR_HEIGHT - TAECHANG_BUTTON_HEIGHT) / 2;
		int nLoginBtnRight = rectClient.Width() - TAECHANG_MARGIN;
		int nLoginBtnLeft = nLoginBtnRight - TAECHANG_LOGIN_BTN_WIDTH;
		int nUserLabelLeft = nLoginBtnLeft - TAECHANG_USER_LABEL_WIDTH - TAECHANG_ROW_GAP;
		m_nAuthDividerX = nUserLabelLeft - TAECHANG_ROW_GAP;

		m_wndLoginBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndLogoutBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndUserLabel.MoveWindow(nUserLabelLeft, nLoginBtnTop + TAECHANG_BUTTON_TEXT_TOP_OFFSET, TAECHANG_USER_LABEL_WIDTH, TAECHANG_BUTTON_HEIGHT);
	}

	int nHeaderTitleWidth = nContentWidth - TAECHANG_LOGIN_BTN_WIDTH - TAECHANG_USER_LABEL_WIDTH - TAECHANG_ROW_GAP * 2 - TAECHANG_MARGIN;
	m_wndHeaderTitle.MoveWindow(nContentLeft, nContentTop, nHeaderTitleWidth, TAECHANG_SECTION_TITLE_HEIGHT);
	m_wndHeaderStatus.MoveWindow(0, 0, 0, 0);
	nContentTop += TAECHANG_HEADER_HEIGHT;

	InvalidateContentArea();

	// 가격 워크플로우: 기존 탭/패널을 숨기고 전용 패널 표시
	m_panelPriceManage.ShowWindow(SW_HIDE);
	m_panelPriceCalc.ShowWindow(SW_HIDE);

	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_wndTaskTabs.ShowWindow(SW_HIDE);
		m_wndInputSection.ShowWindow(SW_HIDE);
		m_wndInputPath.ShowWindow(SW_HIDE);
		m_wndSelectInput.ShowWindow(SW_HIDE);
		m_wndOutputSection.ShowWindow(SW_HIDE);
		m_wndOutputFolder.ShowWindow(SW_HIDE);
		m_wndSelectOutput.ShowWindow(SW_HIDE);
		m_wndLoad.ShowWindow(SW_HIDE);
		m_wndGenerate.ShowWindow(SW_HIDE);
		m_wndSelectAll.ShowWindow(SW_HIDE);
		m_wndEstimateOnePage.ShowWindow(SW_HIDE);
		m_wndInputReset.ShowWindow(SW_HIDE);
		m_wndProgress.ShowWindow(SW_HIDE);
		m_wndProgressText.ShowWindow(SW_HIDE);
		m_wndActionStatus.ShowWindow(SW_HIDE);
		m_wndResultSection.ShowWindow(SW_HIDE);
		m_wndResultList.ShowWindow(SW_HIDE);
		m_wndResultFilterCriteria.ShowWindow(SW_HIDE);
		m_wndResultFilter.ShowWindow(SW_HIDE);
		m_wndResultSearchBtn.ShowWindow(SW_HIDE);
		m_wndResultResetBtn.ShowWindow(SW_HIDE);
		SetCardRect(m_rectResultFilterBox, CRect(0, 0, 0, 0));
		m_wndDetailSection.ShowWindow(SW_HIDE);
		m_wndDetail.ShowWindow(SW_HIDE);
		m_wndEmptyStateHint.ShowWindow(SW_HIDE);
		ShowCompanyOrderPanel(FALSE);

		int nPanelHeight = nContentHeight - (nContentTop - TAECHANG_MARGIN);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) {
			m_panelPriceManage.Layout(CRect(nContentLeft, nContentTop, nContentLeft + nContentWidth, nContentTop + nPanelHeight));
			m_panelPriceManage.ShowWindow(SW_SHOW);
		} else {
			m_panelPriceCalc.Layout(CRect(nContentLeft, nContentTop, nContentLeft + nContentWidth, nContentTop + nPanelHeight));
			m_panelPriceCalc.ShowWindow(SW_SHOW);
		}
		UNREFERENCED_PARAMETER(nSidebarLeft);
		return;
	}

	if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_RECEIVABLES ||
		m_nCurrentWorkflow == TAECHANG_WORKFLOW_DELIVERY ||
		m_nCurrentWorkflow == TAECHANG_WORKFLOW_ESTIMATE)
		nContentTop += TAECHANG_PANEL_GAP;

	m_wndTaskTabs.ShowWindow(SW_SHOW);
	m_wndTaskTabs.MoveWindow(nContentLeft, nContentTop, nContentWidth, TAECHANG_TAB_HEIGHT);
	nContentTop += TAECHANG_TAB_HEIGHT + TAECHANG_PANEL_GAP;

	if (IsDataManageTab()) {
		LayoutCompanyOrderPanel(nContentLeft, nContentTop, nContentWidth, nContentHeight - nContentTop + TAECHANG_MARGIN);
		UpdateTaskTabVisibility();
		UNREFERENCED_PARAMETER(nSidebarLeft);
		return;
	}

	if (IsInputTabSelected()) {
		LayoutInputSection(nContentLeft, nContentTop, nContentWidth, TRUE);
		nContentTop += TAECHANG_INPUT_PANEL_HEIGHT + TAECHANG_PANEL_GAP;
	}

	if (IsActionTabVisible()) {
		LayoutActionSection(nContentLeft, nContentTop, nContentWidth);
		nContentTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_PANEL_GAP;
	}

	LayoutResultSection(nContentLeft, nContentTop, nContentWidth, nContentHeight - nContentTop + TAECHANG_MARGIN);
	UpdateTaskTabVisibility();
	UNREFERENCED_PARAMETER(nSidebarLeft);
}

void CSageTaechangView::LayoutInputSection(int nLeft, int nTop, int nWidth, BOOL bShowOutput) {
	int nPathWidth = nWidth - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP;
	int nEditLeft = nLeft + TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndInputSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
	nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	m_wndSelectInput.MoveWindow(nLeft, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	m_wndInputPath.MoveWindow(nEditLeft, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
	{
		CRect rcFmt;
		m_wndInputPath.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndInputPath.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
	}
	if (!bShowOutput)
		return;
	nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;
	m_wndOutputSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
	nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	m_wndSelectOutput.MoveWindow(nLeft, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	m_wndOutputFolder.MoveWindow(nEditLeft, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
	{
		CRect rcFmt;
		m_wndOutputFolder.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndOutputFolder.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
	}
}

void CSageTaechangView::LayoutActionSection(int nLeft, int nTop, int nWidth) {
	BOOL bShowAction = IsActionTabVisible();
	BOOL bShowLoad = FALSE;
	BOOL bShowGenerate = bShowAction;

	int nX = nLeft;
	if (bShowLoad) {
		m_wndLoad.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
	}
	if (bShowGenerate) {
		m_wndGenerate.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
	}
	if (IsInputResetVisible()) {
		m_wndInputReset.MoveWindow(nX, nTop, TAECHANG_INPUT_RESET_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_INPUT_RESET_WIDTH + TAECHANG_ACTION_GAP;
	}
	if (bShowAction) {
		int nProgressLeft = nX;
		int nProgressWidth = nWidth - (nProgressLeft - nLeft) - TAECHANG_PROGRESS_TEXT_WIDTH - TAECHANG_ACTION_GAP;
		if (nProgressWidth < 0)
			nProgressWidth = 0;
		int nStatusWidth = nProgressWidth + TAECHANG_ACTION_GAP + TAECHANG_PROGRESS_TEXT_WIDTH;
		m_wndProgress.MoveWindow(nProgressLeft, nTop + TAECHANG_PROGRESS_VERT_OFFSET, nProgressWidth, TAECHANG_PROGRESS_HEIGHT);
		m_wndProgressText.MoveWindow(nProgressLeft + nProgressWidth + TAECHANG_ACTION_GAP, nTop + TAECHANG_PROGRESS_TEXT_VERT_OFFSET, TAECHANG_PROGRESS_TEXT_WIDTH, TAECHANG_EDIT_HEIGHT);
		m_wndActionStatus.MoveWindow(nProgressLeft, nTop + TAECHANG_LABEL_VERT_OFFSET, nStatusWidth, TAECHANG_EDIT_HEIGHT);
	}
}

void CSageTaechangView::LayoutResultSection(int nLeft, int nTop, int nWidth, int nHeight) {
	int nBodyHeight = max(TAECHANG_RESULT_MIN_HEIGHT, nHeight - TAECHANG_RESULT_HEADER_HEIGHT);
	if (IsResultTab() || (IsInputTabSelected() && IsInputTableVisible())) {
		BOOL bShowSelectAll = IsInputTabSelected() && IsInputTableVisible();
		BOOL bShowResultFilter = IsDocumentResultFilterVisible();
		int nFilterTotalW = TAECHANG_RESULT_CRITERIA_WIDTH + TAECHANG_ACTION_GAP
			+ TAECHANG_RESULT_FILTER_WIDTH + TAECHANG_ACTION_GAP
			+ TAECHANG_RESULT_SEARCH_WIDTH + TAECHANG_ACTION_GAP + TAECHANG_RESULT_RESET_WIDTH;
		int nRight = nLeft + nWidth;
		int nFilterLeft = nRight - nFilterTotalW;
		if (bShowResultFilter)
			nRight = nFilterLeft - TAECHANG_ROW_GAP;
		int nSectionWidth = nRight - nLeft;
		if (nSectionWidth < 0)
			nSectionWidth = 0;
		if (bShowSelectAll) {
			m_wndResultSection.MoveWindow(0, 0, 0, 0);
			m_wndSelectAll.MoveWindow(nLeft, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
			if (IsOnePageOptionVisible()) {
				int nOnePageLeft = nLeft + TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
				m_wndEstimateOnePage.MoveWindow(nOnePageLeft, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_ESTIMATE_ONE_PAGE_WIDTH, TAECHANG_BUTTON_HEIGHT);
			}
		} else {
			m_wndResultSection.MoveWindow(nLeft, nTop, nSectionWidth, TAECHANG_RESULT_HEADER_HEIGHT);
		}
		if (bShowResultFilter) {
			int nFilterTop = nTop - 8;
			m_wndResultFilterCriteria.MoveWindow(nFilterLeft, nFilterTop, TAECHANG_RESULT_CRITERIA_WIDTH, TAECHANG_EDIT_HEIGHT * 8);
			int nFilterEditLeft = nFilterLeft + TAECHANG_RESULT_CRITERIA_WIDTH + TAECHANG_ACTION_GAP;
			m_wndResultFilter.MoveWindow(nFilterEditLeft, nFilterTop, TAECHANG_RESULT_FILTER_WIDTH, TAECHANG_EDIT_HEIGHT);
			CRect rcFmt;
			m_wndResultFilter.GetClientRect(&rcFmt);
			rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
			rcFmt.left += 6;
			rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
			m_wndResultFilter.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
			int nSearchLeft = nFilterEditLeft + TAECHANG_RESULT_FILTER_WIDTH + TAECHANG_ACTION_GAP;
			m_wndResultSearchBtn.MoveWindow(nSearchLeft, nFilterTop, TAECHANG_RESULT_SEARCH_WIDTH, TAECHANG_BUTTON_HEIGHT);
			int nResetLeft = nSearchLeft + TAECHANG_RESULT_SEARCH_WIDTH + TAECHANG_ACTION_GAP;
			m_wndResultResetBtn.MoveWindow(nResetLeft, nFilterTop, TAECHANG_RESULT_RESET_WIDTH, TAECHANG_BUTTON_HEIGHT);
			SetCardRect(m_rectResultFilterBox, CRect(
				nFilterLeft - TAECHANG_RESULT_FILTER_BOX_PAD,
				nFilterTop - TAECHANG_RESULT_FILTER_BOX_PAD,
				nResetLeft + TAECHANG_RESULT_RESET_WIDTH + TAECHANG_RESULT_FILTER_BOX_PAD,
				nFilterTop + TAECHANG_EDIT_HEIGHT + TAECHANG_RESULT_FILTER_BOX_PAD));
		} else {
			SetCardRect(m_rectResultFilterBox, CRect(0, 0, 0, 0));
		}
		m_wndResultList.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
		UpdateResultColumns();
	}
	if (IsDetailTab()) {
		m_wndDetailSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_RESULT_HEADER_HEIGHT);
		m_wndDetail.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
	}
	m_wndEmptyStateHint.MoveWindow(nLeft, nTop, nWidth, nBodyHeight);
}

void CSageTaechangView::OnDraw(CDC* pDC) {
	CSageTaechangDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
	pDC->FillSolidRect(0, TAECHANG_TOP_BAR_HEIGHT, TAECHANG_SIDEBAR_WIDTH, 1, RGB(55, 47, 38));
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH + 1, TAECHANG_MARGIN + TAECHANG_HEADER_HEIGHT, rectClient.Width() - TAECHANG_SIDEBAR_WIDTH - 1, 1, TAECHANG_COLOR_BORDER);
	DrawEditBorder(pDC, m_wndInputPath);
	DrawEditBorder(pDC, m_wndOutputFolder);
	if (!m_rectResultFilterBox.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectResultFilterBox, TAECHANG_COLOR_PANEL);
		CBrush brFilterBox(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectResultFilterBox, &brFilterBox);
	}
	DrawEditBorder(pDC, m_wndResultFilterCriteria);
	DrawEditBorder(pDC, m_wndResultFilter);
	if (!m_rectCoCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCoCard, TAECHANG_COLOR_PANEL);
		CBrush brCoCard(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCoCard, &brCoCard);
	}
	DrawEditBorder(pDC, m_wndCoSearchEdit);
	DrawEditBorder(pDC, m_wndCoOrderEdit);
	DrawEditBorder(pDC, m_wndCoCompanyEdit);
	if (taechangAuth.IsLoggedIn() && m_nAuthDividerX > 0) {
		int nDivTop = (TAECHANG_TOP_BAR_HEIGHT - TAECHANG_BUTTON_HEIGHT) / 2;
		pDC->FillSolidRect(m_nAuthDividerX, nDivTop, 1, TAECHANG_BUTTON_HEIGHT, TAECHANG_COLOR_BORDER);
	}
}

void CSageTaechangView::InvalidateContentArea() {
	CRect rectContent;
	GetClientRect(&rectContent);
	rectContent.left += TAECHANG_SIDEBAR_WIDTH;
	InvalidateRect(rectContent, TRUE);
}

void CSageTaechangView::SetCardRect(CRect& rectCard, const CRect& rectNew) {
	if (rectCard == rectNew)
		return;

	CRect rectStale;
	rectStale.UnionRect(rectCard, rectNew);
	rectCard = rectNew;
	if (rectStale.IsRectEmpty())
		return;

	rectStale.InflateRect(TAECHANG_CARD_REPAINT_MARGIN, TAECHANG_CARD_REPAINT_MARGIN);
	InvalidateRect(rectStale, TRUE);
}

void CSageTaechangView::DrawEditBorder(CDC* pDC, CWnd& wnd) {
	if (!::IsWindow(wnd.GetSafeHwnd()) || !wnd.IsWindowVisible())
		return;
	CRect rect;
	wnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.InflateRect(1, 1);
	pDC->FillSolidRect(rect.left, rect.top, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.left, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(rect.right - 1, rect.top, 1, rect.Height(), TAECHANG_COLOR_BORDER);
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
	m_wndInputSection.SetWindowTextW(pHandler->GetInputSectionLabel());
	m_wndGenerate.SetWindowTextW(pHandler->GetActionButtonLabel());
	m_wndDetailSection.SetWindowTextW(pHandler->GetDetailSectionLabel());
	m_wndDetail.SetWindowTextW(m_strExecutionHistory);
	ApplyWorkflowTabs();
	ApplyResultColumns();
	LayoutChildControls();
}

BOOL CSageTaechangView::IsInputTabSelected() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_INPUT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsResultTab() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_RESULT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDetailTab() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_HISTORY) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsActionTabVisible() const {
	return IsInputTabSelected() ? TRUE : FALSE;
}

int CSageTaechangView::GetTaskTabVisualIndex(int nSemanticTabIndex) const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return TAECHANG_TAB_INDEX_INPUT;
	int nTabCount = pHandler->GetTabCount();
	for (int nVisualTabIndex = 0; nVisualTabIndex < nTabCount; ++nVisualTabIndex) {
		if (pHandler->GetTab(nVisualTabIndex).nSemanticIndex == nSemanticTabIndex)
			return nVisualTabIndex;
	}
	return TAECHANG_TAB_INDEX_INPUT;
}

int CSageTaechangView::GetTaskTabSemanticIndex(int nVisualTabIndex) const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return TAECHANG_TAB_INDEX_INPUT;
	if (nVisualTabIndex < 0 || nVisualTabIndex >= pHandler->GetTabCount())
		return TAECHANG_TAB_INDEX_INPUT;
	return pHandler->GetTab(nVisualTabIndex).nSemanticIndex;
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
	if (m_bRunning || !IsInputTabSelected())
		return FALSE;
	return IsInputTableVisible();
}

BOOL CSageTaechangView::IsDataManageTab() const {
	return (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES &&
		m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_DATA_MANAGE) ? TRUE : FALSE;
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
	if (SageWorkflowRegistry::FindHandler(nWorkflowType) == NULL)
		return;

	TaechangWorkflowUiState& state = GetWorkflowUiState(nWorkflowType);
	state.nSelectedTaskTab = m_nSelectedTaskTab;
	state.nLastWorkflowType = m_nLastWorkflowType;
	state.nLastTaskType = m_nLastTaskType;
	state.bLastTaskSuccess = m_bLastTaskSuccess;
	state.strLastResponseJson = m_strLastResponseJson;
	state.strRunningInputPath = m_strRunningInputPath;
	state.strResultFilterKeyword = m_strResultFilterKeyword;
	state.nResultFilterCriteria = m_nResultFilterCriteria;
	if (::IsWindow(m_wndInputPath.GetSafeHwnd()))
		m_wndInputPath.GetWindowTextW(state.strInputPath);
	if (::IsWindow(m_wndOutputFolder.GetSafeHwnd()))
		m_wndOutputFolder.GetWindowTextW(state.strOutputFolder);
	if (::IsWindow(m_wndEstimateOnePage.GetSafeHwnd()))
		state.bEstimateOnePage = m_wndEstimateOnePage.GetCheck() == BST_CHECKED ? TRUE : FALSE;
	SaveCheckedRowNums(state);
}

void CSageTaechangView::RestoreWorkflowUiState(int nWorkflowType) {
	if (SageWorkflowRegistry::FindHandler(nWorkflowType) == NULL) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
		m_nLastWorkflowType = 0;
		m_nLastTaskType = 0;
		m_bLastTaskSuccess = FALSE;
		m_strLastResponseJson.Empty();
		m_strRunningInputPath.Empty();
		m_strResultFilterKeyword.Empty();
		m_nResultFilterCriteria = TAECHANG_FILTER_CRITERIA_NONE;
		return;
	}

	TaechangWorkflowUiState& state = GetWorkflowUiState(nWorkflowType);
	m_nSelectedTaskTab = state.nSelectedTaskTab;
	m_nLastWorkflowType = state.nLastWorkflowType;
	m_nLastTaskType = state.nLastTaskType;
	m_bLastTaskSuccess = state.bLastTaskSuccess;
	m_strLastResponseJson = state.strLastResponseJson;
	m_strRunningInputPath = state.strRunningInputPath;
	m_strResultFilterKeyword = state.strResultFilterKeyword;
	m_nResultFilterCriteria = state.nResultFilterCriteria;
	if (::IsWindow(m_wndInputPath.GetSafeHwnd()))
		m_wndInputPath.SetWindowTextW(state.strInputPath);
	if (::IsWindow(m_wndOutputFolder.GetSafeHwnd()))
		m_wndOutputFolder.SetWindowTextW(state.strOutputFolder);
	if (::IsWindow(m_wndResultFilter.GetSafeHwnd()))
		m_wndResultFilter.SetWindowTextW(state.strResultFilterKeyword);
	if (::IsWindow(m_wndEstimateOnePage.GetSafeHwnd()))
		m_wndEstimateOnePage.SetCheck(state.bEstimateOnePage ? BST_CHECKED : BST_UNCHECKED);
}

void CSageTaechangView::SaveCheckedRowNums(TaechangWorkflowUiState& state) {
	state.strCheckedRowNums.Empty();
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;
	if (!IsInputTableVisible())
		return;

	int nListCount = m_wndResultList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndResultList.GetCheck(i))
			continue;
		DWORD_PTR nSourceRowIndex = m_wndResultList.GetItemData(i);
		if (nSourceRowIndex == 0)
			continue;
		CString strNum;
		strNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		if (!state.strCheckedRowNums.IsEmpty())
			state.strCheckedRowNums += TAECHANG_UI_ROW_NUM_SEPARATOR;
		state.strCheckedRowNums += strNum;
	}
}

void CSageTaechangView::RestoreCheckedRowNums(const TaechangWorkflowUiState& state) {
	if (state.strCheckedRowNums.IsEmpty() || !::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;
	if (!IsInputTableVisible())
		return;

	int nListCount = m_wndResultList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		DWORD_PTR nSourceRowIndex = m_wndResultList.GetItemData(i);
		if (nSourceRowIndex == 0)
			continue;
		CString strCurrentNum;
		strCurrentNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
		CString strRemaining = state.strCheckedRowNums;
		int nTokenIndex = 0;
		CString strToken = strRemaining.Tokenize(TAECHANG_UI_ROW_NUM_SEPARATOR, nTokenIndex);
		while (!strToken.IsEmpty()) {
			strToken.Trim();
			if (strToken == strCurrentNum) {
				m_wndResultList.SetCheck(i, TRUE);
				break;
			}
			strToken = strRemaining.Tokenize(TAECHANG_UI_ROW_NUM_SEPARATOR, nTokenIndex);
		}
	}
}

void CSageTaechangView::RebuildCurrentWorkflowResultList() {
	ApplyResultColumns();
	UpdateResultColumns();
	if (IsDocumentResultFilterVisible()) {
		RefreshDocumentResultFilter();
		RestoreCheckedRowNums(GetWorkflowUiState(GetSelectedWorkflow()));
	}
}

void CSageTaechangView::OnWorkflowChanged() {
	RestoreWorkflowUiState(m_nCurrentWorkflow);

	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_wndHeaderTitle.SetWindowTextW(
			m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE
			? TAECHANG_UI_PRICE_MANAGE_NAME
			: TAECHANG_UI_PRICE_CALC_NAME
		);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) {

			m_panelPriceManage.RefreshCompanyList();
		} else {
			m_panelPriceCalc.RefreshCompanyCombo();
		}
		LayoutChildControls();
		Invalidate(FALSE);
		SetStatusText(TAECHANG_UI_READY);
		return;
	}

	UpdateWorkflowLabels();
	RestoreWorkflowUiState(m_nCurrentWorkflow);
	RebuildCurrentWorkflowResultList();
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

void CSageTaechangView::OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	m_nSelectedTaskTab = GetTaskTabSemanticIndex(m_wndTaskTabs.GetCurSel());
	if (IsDataManageTab())
		RefreshCompanyOrderList();
	LayoutChildControls();
	Invalidate();
	*pResult = 0;
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

	m_wndInputPath.SetWindowTextW(strInputPaths);
	if (!IsInputTabSelected()) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		LayoutChildControls();
	}
	SetStatusText(L"파일 드롭 수신");
	if (pHandler->UsesInputTable())
		RunWorkflowTask(TAECHANG_TASK_LOAD);
}

void CSageTaechangView::OnSelectInput() {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	CFileDialog dlg(TRUE, L"xls", NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, TAECHANG_UI_EXCEL_FILTER, this);
	dlg.m_ofn.lpstrTitle = pHandler->GetInputDialogTitle();
	if (dlg.DoModal() == IDOK) {
		m_wndInputPath.SetWindowTextW(dlg.GetPathName());
		if (pHandler->UsesInputTable())
			RunWorkflowTask(TAECHANG_TASK_LOAD);
	}
}

void CSageTaechangView::OnSelectOutput() {
	CFolderPickerDialog dlg(NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, this, 0);
	dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_OUTPUT_TITLE;
	if (dlg.DoModal() == IDOK)
		m_wndOutputFolder.SetWindowTextW(dlg.GetPathName());
}

BOOL CSageTaechangView::ValidateInputPath(CString& strInputPath) {
	m_wndInputPath.GetWindowTextW(strInputPath);
	strInputPath.Trim();
	if (strInputPath.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_INPUT_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

BOOL CSageTaechangView::ValidateOutputFolder(CString& strOutputFolder) {
	m_wndOutputFolder.GetWindowTextW(strOutputFolder);
	strOutputFolder.Trim();
	if (strOutputFolder.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_OUTPUT_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

void CSageTaechangView::OnLoadWorkflow() {
	RunWorkflowTask(TAECHANG_TASK_LOAD);
}

void CSageTaechangView::OnGenerateWorkflow() {
	RunWorkflowTask(TAECHANG_TASK_GENERATE);
}

void CSageTaechangView::OnSelectAll() {
	int nCount = m_wndResultList.GetItemCount();
	BOOL bAllChecked = TRUE;
	for (int i = 0; i < nCount; ++i) {
		if (!m_wndResultList.GetCheck(i)) {
			bAllChecked = FALSE;
			break;
		}
	}
	BOOL bCheck = bAllChecked ? FALSE : TRUE;
	if (bCheck && IsOnePageOptionVisible() && m_wndEstimateOnePage.GetCheck() == BST_CHECKED && nCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		for (int i = 0; i < nCount; ++i)
			m_wndResultList.SetCheck(i, i < TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS ? TRUE : FALSE);
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
		return;
	}
	for (int i = 0; i < nCount; ++i)
		m_wndResultList.SetCheck(i, bCheck);
}

void CSageTaechangView::OnEstimateOnePage() {
	if (!IsOnePageOptionVisible() || m_wndEstimateOnePage.GetCheck() != BST_CHECKED)
		return;

	int nCheckedCount = 0;
	int nListCount = m_wndResultList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (!m_wndResultList.GetCheck(i))
			continue;
		++nCheckedCount;
		if (nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
			m_wndResultList.SetCheck(i, FALSE);
	}
	if (nCheckedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
}

void CSageTaechangView::OnInputReset() {
	int nWorkflowType = GetSelectedWorkflow();
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL || !pHandler->UsesInputTable())
		return;

	m_wndInputPath.SetWindowTextW(CString());
	m_wndResultFilter.SetWindowTextW(CString());
	m_strResultFilterKeyword.Empty();
	m_strLastResponseJson.Empty();
	m_strRunningInputPath.Empty();
	m_nLastWorkflowType = 0;
	m_nLastTaskType = 0;
	m_bLastTaskSuccess = FALSE;
	m_wndEstimateOnePage.SetCheck(BST_UNCHECKED);
	m_wndResultList.DeleteAllItems();
	ApplyResultColumns();
	UpdateResultColumns();
	UpdateTaskTabVisibility();
	LayoutChildControls();
	SetStatusText(TAECHANG_UI_READY);
	SaveWorkflowUiState(nWorkflowType);
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

	CString strSelectedRowNums;
	if (pHandler->UsesInputTable() && nTaskType == TAECHANG_TASK_GENERATE) {
		int nSelectedCount = 0;
		int nListCount = m_wndResultList.GetItemCount();
		for (int i = 0; i < nListCount; ++i) {
			if (!m_wndResultList.GetCheck(i))
				continue;
			++nSelectedCount;
			DWORD_PTR nSourceRowIndex = m_wndResultList.GetItemData(i);
			if (nSourceRowIndex == 0)
				continue;
			CString strNum;
			strNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
			if (!strSelectedRowNums.IsEmpty())
				strSelectedRowNums += TAECHANG_UI_ROW_NUM_SEPARATOR;
			strSelectedRowNums += strNum;
		}
		BOOL bHasSelectedRowNums = strSelectedRowNums.IsEmpty() ? FALSE : TRUE;
		BOOL bOnePage = (m_wndEstimateOnePage.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
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
	pTask->m_bEstimateOnePage = (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE && m_wndEstimateOnePage.GetCheck() == BST_CHECKED) ? TRUE : FALSE;
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
	m_wndSelectInput.EnableWindow(!bRunning);
	m_wndSelectOutput.EnableWindow(!bRunning);
	m_wndLoad.EnableWindow(!bRunning);
	m_wndGenerate.EnableWindow(!bRunning);
	m_wndSelectAll.EnableWindow(!bRunning);
	m_wndEstimateOnePage.EnableWindow(!bRunning);
	m_wndInputReset.EnableWindow(!bRunning);
	if (bRunning) {
		UpdateProgressPercent(0);
		SetTimer(ID_TAECHANG_PROGRESS_TIMER, TAECHANG_PROGRESS_TIMER_MS, NULL);
	} else {
		KillTimer(ID_TAECHANG_PROGRESS_TIMER);
		UpdateProgressPercent(TAECHANG_PROGRESS_COMPLETE);
	}
	UpdateTaskTabVisibility();
	LayoutChildControls();
	if (bRunning)
		SetStatusText(TAECHANG_UI_RUNNING);
}

void CSageTaechangView::UpdateProgressPercent(int nPercent) {
	m_nProgressPercent = nPercent;
	m_wndProgress.SetPos(m_nProgressPercent);
	CString strProgress;
	strProgress.Format(TAECHANG_UI_PROGRESS_FORMAT, m_nProgressPercent);
	m_wndProgressText.SetWindowTextW(strProgress);
}

void CSageTaechangView::OnTimer(UINT_PTR nIDEvent) {
	if (nIDEvent == ID_TAECHANG_PROGRESS_TIMER) {
		if (m_bRunning && m_nProgressPercent < TAECHANG_PROGRESS_RUNNING_MAX) {
			int nNextPercent = m_nProgressPercent + TAECHANG_PROGRESS_STEP;
			if (nNextPercent > TAECHANG_PROGRESS_RUNNING_MAX)
				nNextPercent = TAECHANG_PROGRESS_RUNNING_MAX;
			UpdateProgressPercent(nNextPercent);
		}
		return;
	}
	CView::OnTimer(nIDEvent);
}

BOOL CSageTaechangView::OnEraseBkgnd(CDC* pDC) {
	CRect rectClient;
	GetClientRect(&rectClient);
	pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
	pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
	pDC->FillSolidRect(0, TAECHANG_TOP_BAR_HEIGHT, TAECHANG_SIDEBAR_WIDTH, 1, RGB(55, 47, 38));
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
	pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH + 1, TAECHANG_MARGIN + TAECHANG_HEADER_HEIGHT, rectClient.Width() - TAECHANG_SIDEBAR_WIDTH - 1, 1, TAECHANG_COLOR_BORDER);
	DrawEditBorder(pDC, m_wndInputPath);
	DrawEditBorder(pDC, m_wndOutputFolder);
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
	if (pWnd->GetSafeHwnd() == m_wndActionStatus.GetSafeHwnd()) {
		pDC->SetTextColor(m_bLastTaskSuccess ? TAECHANG_COLOR_SUCCESS : TAECHANG_COLOR_ERROR);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return SageUiResources::GetBrush(SAGE_BG_APP);
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
	if (!bKeepInputTable)
		m_wndResultList.DeleteAllItems();
	m_nLastWorkflowType = nWorkflowType;
	if (!bKeepInputTable) {
		m_nLastTaskType = nTaskType;
		m_strLastResponseJson = strResponseJson;
		ApplyResultColumns();
		UpdateResultColumns();
	}

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows);
	AppendExecutionHistory(nWorkflowType, nTaskType, strResponseJson, bSuccess);
	m_wndDetail.SetWindowTextW(m_strExecutionHistory);

	if (!bKeepInputTable) {
		if (pHandler != NULL)
			RefreshDocumentResultFilter();
		else {
			m_wndResultList.SetRedraw(FALSE);
			for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
				InsertResultRow(arrRows[i]);
			m_wndResultList.SetRedraw(TRUE);
			m_wndResultList.Invalidate();
		}
	}

	if (pHandler != NULL && (nTaskType == TAECHANG_TASK_LOAD || nTaskType == TAECHANG_TASK_GENERATE)) {
		m_nSelectedTaskTab = pHandler->UsesInputTable()
			? TAECHANG_TAB_INDEX_INPUT
			: TAECHANG_TAB_INDEX_DOCUMENT_RESULT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		UpdateTaskTabVisibility();
		LayoutChildControls();
		if (nTaskType == TAECHANG_TASK_GENERATE && bSuccess) {
			LPCWSTR pszCompleted = pHandler->FindGenerateCompletedMessage();
			if (pszCompleted != NULL)
				AfxMessageBox(pszCompleted, MB_ICONINFORMATION);
		}
	}

	m_bLastTaskSuccess = bSuccess;
	m_wndActionStatus.SetWindowTextW(bSuccess ? TAECHANG_UI_ACTION_STATUS_COMPLETED : TAECHANG_UI_ACTION_STATUS_FAILED);
	m_wndActionStatus.Invalidate();
	SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
	SaveWorkflowUiState(nWorkflowType);
}

const SageWorkflowColumn& CSageTaechangView::GetLastResultColumn(int nColumnIndex) const {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(m_nLastWorkflowType);
	if (pHandler == NULL)
		return SageWorkflowResultTable::GetGenericColumn(nColumnIndex);
	return pHandler->GetResultColumn(m_nLastTaskType, nColumnIndex);
}

int CSageTaechangView::GetLastResultColumnCount() const {
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(m_nLastWorkflowType);
	if (pHandler == NULL)
		return SageWorkflowResultTable::GetGenericColumnCount();
	return pHandler->GetResultColumnCount(m_nLastTaskType);
}

void CSageTaechangView::InsertResultRow(const TaechangResultRow& row) {
	int nCount = m_wndResultList.GetItemCount();
	int nColumnCount = GetLastResultColumnCount();
	if (nColumnCount < 1)
		return;

	int nIndex = m_wndResultList.InsertItem(nCount, SageWorkflowResultTable::GetRowText(row, GetLastResultColumn(0).nField));
	for (int nCol = 1; nCol < nColumnCount; ++nCol)
		m_wndResultList.SetItemText(nIndex, nCol, SageWorkflowResultTable::GetRowText(row, GetLastResultColumn(nCol).nField));
	m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
}

void CSageTaechangView::RefreshDocumentResultFilter() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()) || !IsDocumentResultFilterVisible())
		return;

	PopulateResultFilterCriteria();

	CString strFilter = m_strResultFilterKeyword;
	strFilter.Trim();
	CString strFilterLower = strFilter;
	strFilterLower.MakeLower();

	int nCriteria = GetEffectiveFilterCriteria();

	m_wndResultList.SetRedraw(FALSE);
	m_wndResultList.DeleteAllItems();

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	presenter.BuildRows(m_nLastWorkflowType, m_nLastTaskType, m_strLastResponseJson, arrRows);
	for (int i = 0; i < static_cast<int>(arrRows.size()); ++i) {
		if (!strFilterLower.IsEmpty()) {
			CString strTargetLower;
			if (nCriteria == TAECHANG_FILTER_CRITERIA_MANAGER)
				strTargetLower = arrRows[i].m_strManager;
			else if (nCriteria == TAECHANG_FILTER_CRITERIA_ITEM)
				strTargetLower = arrRows[i].m_strItemName;
			else
				strTargetLower = arrRows[i].m_strCompanyName;
			strTargetLower.MakeLower();
			if (strTargetLower.Find(strFilterLower) < 0)
				continue;
		}
		InsertResultRow(arrRows[i]);
	}

	m_wndResultList.SetRedraw(TRUE);
	m_wndResultList.Invalidate();
}

int CSageTaechangView::GetDefaultFilterCriteria() const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL || pHandler->GetFilterCriteriaCount() < 1)
		return TAECHANG_FILTER_CRITERIA_NONE;
	return pHandler->GetFilterCriteria(0).nCriteria;
}

int CSageTaechangView::GetEffectiveFilterCriteria() const {
	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return TAECHANG_FILTER_CRITERIA_NONE;

	int nCount = pHandler->GetFilterCriteriaCount();
	for (int i = 0; i < nCount; ++i) {
		if (pHandler->GetFilterCriteria(i).nCriteria == m_nResultFilterCriteria)
			return m_nResultFilterCriteria;
	}
	return GetDefaultFilterCriteria();
}

void CSageTaechangView::PopulateResultFilterCriteria() {
	if (!::IsWindow(m_wndResultFilterCriteria.GetSafeHwnd()))
		return;

	ISageWorkflowHandler* pHandler = FindCurrentHandler();
	if (pHandler == NULL)
		return;

	int nEffective = GetEffectiveFilterCriteria();
	m_wndResultFilterCriteria.ResetContent();
	int nCriteriaCount = pHandler->GetFilterCriteriaCount();
	for (int i = 0; i < nCriteriaCount; ++i) {
		const SageWorkflowFilterCriteria& criteria = pHandler->GetFilterCriteria(i);
		int nIndex = m_wndResultFilterCriteria.AddString(criteria.pszLabel);
		m_wndResultFilterCriteria.SetItemData(nIndex, criteria.nCriteria);
	}

	int nCount = m_wndResultFilterCriteria.GetCount();
	for (int i = 0; i < nCount; ++i) {
		if (static_cast<int>(m_wndResultFilterCriteria.GetItemData(i)) == nEffective) {
			m_wndResultFilterCriteria.SetCurSel(i);
			return;
		}
	}
	if (nCount > 0)
		m_wndResultFilterCriteria.SetCurSel(0);
}

void CSageTaechangView::OnResultSearch() {
	if (!IsDocumentResultFilterVisible())
		return;

	m_wndResultFilter.GetWindowTextW(m_strResultFilterKeyword);
	m_strResultFilterKeyword.Trim();
	RefreshDocumentResultFilter();
	SaveWorkflowUiState(GetSelectedWorkflow());
}

void CSageTaechangView::OnResultFilterReset() {
	if (!IsDocumentResultFilterVisible())
		return;

	m_strResultFilterKeyword.Empty();
	m_wndResultFilter.SetWindowTextW(L"");
	RefreshDocumentResultFilter();
	SaveWorkflowUiState(GetSelectedWorkflow());
}

void CSageTaechangView::OnResultFilterCriteriaChanged() {
	if (!IsDocumentResultFilterVisible())
		return;

	int nSel = m_wndResultFilterCriteria.GetCurSel();
	if (nSel == CB_ERR)
		return;

	m_nResultFilterCriteria = static_cast<int>(m_wndResultFilterCriteria.GetItemData(nSel));
	RefreshDocumentResultFilter();
	SaveWorkflowUiState(GetSelectedWorkflow());
}

void CSageTaechangView::AppendExecutionHistory(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess) {
	CString strLine = BuildExecutionHistoryLine(nWorkflowType, nTaskType, strResponseJson, bSuccess);
	if (strLine.IsEmpty())
		return;

	if (!m_strExecutionHistory.IsEmpty())
		m_strExecutionHistory += TAECHANG_UI_HISTORY_ENTRY_BREAK;
	m_strExecutionHistory += strLine;
}

CString CSageTaechangView::BuildExecutionHistoryLine(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess) const {
	UNREFERENCED_PARAMETER(nWorkflowType);
	UNREFERENCED_PARAMETER(nTaskType);

	CTime now = CTime::GetCurrentTime();
	CString strLine = TAECHANG_UI_HISTORY_ENTRY_PREFIX + now.Format(TAECHANG_UI_HISTORY_TIME_FORMAT) +
		TAECHANG_UI_HISTORY_ENTRY_SUFFIX + (bSuccess ? TAECHANG_UI_HISTORY_SUCCESS : TAECHANG_UI_HISTORY_FAILED);
	CString strInputPath = m_strRunningInputPath;
	if (strInputPath.IsEmpty())
		strInputPath = TAECHANG_UI_HISTORY_EMPTY_VALUE;
	strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
	strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
	strLine += TAECHANG_UI_HISTORY_INPUT_PREFIX;
	strLine += strInputPath;

	if (bSuccess) {
		CString strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_FILE_PATH);
		if (strOutputPath.IsEmpty())
			strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_OUTPUT_FOLDER);
		if (strOutputPath.IsEmpty())
			strOutputPath = TAECHANG_UI_HISTORY_EMPTY_VALUE;
		strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
		strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
		strLine += TAECHANG_UI_HISTORY_OUTPUT_PREFIX;
		strLine += strOutputPath;
	} else {
		CString strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_MESSAGE);
		if (strReason.IsEmpty())
			strReason = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_CODE);
		if (strReason.IsEmpty())
			strReason = TAECHANG_UI_HISTORY_EMPTY_VALUE;
		strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
		strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
		strLine += TAECHANG_UI_HISTORY_REASON_PREFIX;
		strLine += strReason;
	}

	return strLine;
}

void CSageTaechangView::OnResultListItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	*pResult = 0;
	if (!IsOnePageOptionVisible() || m_wndEstimateOnePage.GetCheck() != BST_CHECKED)
		return;

	NM_LISTVIEW* pList = reinterpret_cast<NM_LISTVIEW*>(pNMHDR);
	if ((pList->uChanged & LVIF_STATE) == 0 || pList->iItem < 0)
		return;

	UINT uOldCheck = pList->uOldState & LVIS_STATEIMAGEMASK;
	UINT uNewCheck = pList->uNewState & LVIS_STATEIMAGEMASK;
	if (uOldCheck == uNewCheck || uNewCheck != INDEXTOSTATEIMAGEMASK(2))
		return;

	int nCheckedCount = 0;
	int nListCount = m_wndResultList.GetItemCount();
	for (int i = 0; i < nListCount; ++i) {
		if (m_wndResultList.GetCheck(i))
			++nCheckedCount;
	}
	if (nCheckedCount <= TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS)
		return;

	m_wndResultList.SetCheck(pList->iItem, FALSE);
	AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
}

void CSageTaechangView::CreateCompanyOrderPanel() {
	CRect r(0, 0, 0, 0);
	m_wndCoCrudSection.Create(TAECHANG_UI_CO_CRUD_SECTION, WS_CHILD | SS_OWNERDRAW, r, this, ID_COORDER_CRUD_SECTION);
	m_wndCoListSection.Create(TAECHANG_UI_CO_LIST_SECTION, WS_CHILD | SS_OWNERDRAW, r, this, ID_COORDER_LIST_SECTION);
	m_wndCoAddBtn.Create(TAECHANG_UI_CO_ADD_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_ADD_BTN);
	m_wndCoAddBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCoModifyBtn.Create(TAECHANG_UI_CO_MODIFY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_MODIFY_BTN);
	m_wndCoDeleteBtn.Create(TAECHANG_UI_CO_DELETE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_DELETE_BTN);
	m_wndCoDeleteBtn.SetVariant(SAGE_BUTTON_DANGER);
	m_wndCoCancelBtn.Create(TAECHANG_UI_CO_CANCEL_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_CANCEL_BTN);
	m_wndCoSearchLabel.Create(TAECHANG_UI_CO_SEARCH_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoSearchEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_SEARCH_EDIT);
	m_wndCoSearchBtn.Create(L"", WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_SEARCH_BTN);
	m_wndCoSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndCoOrderLabel.Create(TAECHANG_UI_CO_ORDER_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoOrderEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER, r, this, ID_COORDER_ORDER_EDIT);
	m_wndCoOrderEdit.LimitText(6);
	m_wndCoNameLabel.Create(TAECHANG_UI_CO_NAME_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoCompanyEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_COMPANY_EDIT);
	m_wndCoCompanyEdit.LimitText(TAECHANG_CO_COMPANY_NAME_MAX);
	m_wndCoList.Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, r, this, ID_COORDER_LIST);
	m_wndCoList.SetAlternateRowColor(TRUE);
	m_wndCoList.SetFirstColumnAlign(SAGE_LIST_FIRST_COLUMN_CENTER);
	m_wndCoList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndCoList.SetRowSeparator(TRUE);
	m_wndCoList.InsertColumn(0, TAECHANG_UI_CO_COL_ORDER, LVCFMT_CENTER, TAECHANG_CO_ORDER_COL_WIDTH);
	m_wndCoList.InsertColumn(1, TAECHANG_UI_CO_COL_COMPANY, LVCFMT_LEFT, TAECHANG_CO_COMPANY_NAME_WIDTH);
	if (CHeaderCtrl* pHeader = m_wndCoList.GetHeaderCtrl()) {
		m_wndCoListHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndCoListHeader.GetSafeHwnd(), L"", L"");
		HDITEM hdi = {};
		hdi.mask = HDI_FORMAT;
		m_wndCoListHeader.GetItem(1, &hdi);
		hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
		m_wndCoListHeader.SetItem(1, &hdi);
	}
}

void CSageTaechangView::LayoutCompanyOrderPanel(int nLeft, int nTop, int nWidth, int nHeight) {
	if (!::IsWindow(m_wndCoList.GetSafeHwnd()))
		return;

	int nPad = TAECHANG_MARGIN;
	int nSectionTop = nTop;
	m_wndCoCrudSection.MoveWindow(nLeft, nSectionTop, TAECHANG_CO_LIST_WIDTH, TAECHANG_SECTION_TITLE_HEIGHT);

	int nCardTop = nSectionTop + TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
	int nCardContentLeft  = nLeft + nPad;
	int nCardContentRight = nLeft + TAECHANG_CO_LIST_WIDTH - nPad;

	int nInputTop = nCardTop + nPad;
	int nX = nCardContentLeft;
	m_wndCoOrderLabel.MoveWindow(nX, nInputTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_ORDER_LABEL_W, TAECHANG_EDIT_HEIGHT);
	nX += TAECHANG_CO_ORDER_LABEL_W + TAECHANG_LABEL_EDIT_GAP;
	m_wndCoOrderEdit.MoveWindow(nX, nInputTop, TAECHANG_CO_ORDER_EDIT_WIDTH, TAECHANG_EDIT_HEIGHT);
	{
		CRect rcFmt;
		m_wndCoOrderEdit.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.left += 4;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndCoOrderEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
	}
	nX += TAECHANG_CO_ORDER_EDIT_WIDTH + TAECHANG_ACTION_GAP;
	m_wndCoNameLabel.MoveWindow(nX, nInputTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_NAME_LABEL_W, TAECHANG_EDIT_HEIGHT);
	nX += TAECHANG_CO_NAME_LABEL_W + TAECHANG_LABEL_EDIT_GAP;
	int nCompanyEditWidth = nCardContentRight - nX;
	if (nCompanyEditWidth < 80)
		nCompanyEditWidth = 80;
	m_wndCoCompanyEdit.MoveWindow(nX, nInputTop, nCompanyEditWidth, TAECHANG_EDIT_HEIGHT);
	{
		CRect rcFmt;
		m_wndCoCompanyEdit.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.left += 6;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndCoCompanyEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
	}

	int nBtnTop = nInputTop + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;
	nX = nCardContentLeft + (nCardContentRight - nCardContentLeft - TAECHANG_CO_BTN_GROUP_WIDTH) / 2;
	m_wndCoAddBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndCoModifyBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndCoCancelBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nX += TAECHANG_CO_SMALL_BTN_WIDTH + TAECHANG_ACTION_GAP;
	m_wndCoDeleteBtn.MoveWindow(nX, nBtnTop, TAECHANG_CO_SMALL_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);

	int nCardHeight = nPad + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP + TAECHANG_BUTTON_HEIGHT + nPad;
	SetCardRect(m_rectCoCard, CRect(nLeft, nCardTop, nLeft + TAECHANG_CO_LIST_WIDTH, nCardTop + nCardHeight));

	int nListSectionTop = nCardTop + nCardHeight + TAECHANG_PANEL_GAP;
	int nListWidth = TAECHANG_CO_LIST_WIDTH - TAECHANG_MARGIN;
	int nListRight = nLeft + nListWidth;
	int nSearchTop = nListSectionTop + TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_ROW_GAP;

	int nSearchBtnLeft   = nListRight - TAECHANG_RESULT_SEARCH_WIDTH;
	int nSearchEditLeft  = nSearchBtnLeft - TAECHANG_ACTION_GAP - TAECHANG_RESULT_FILTER_WIDTH;
	int nSearchLabelLeft = nSearchEditLeft - TAECHANG_LABEL_EDIT_GAP - TAECHANG_CO_SEARCH_LABEL_W;
	int nSectionLabelWidth = nSearchLabelLeft - TAECHANG_ACTION_GAP - nLeft;
	if (nSectionLabelWidth < 0) nSectionLabelWidth = 0;

	m_wndCoListSection.MoveWindow(nLeft, nListSectionTop, nSectionLabelWidth, TAECHANG_RESULT_HEADER_HEIGHT);
	m_wndCoSearchLabel.MoveWindow(nSearchLabelLeft, nSearchTop + TAECHANG_LABEL_VERT_OFFSET, TAECHANG_CO_SEARCH_LABEL_W, TAECHANG_EDIT_HEIGHT);
	m_wndCoSearchEdit.MoveWindow(nSearchEditLeft, nSearchTop, TAECHANG_RESULT_FILTER_WIDTH, TAECHANG_EDIT_HEIGHT);
	{
		CRect rcFmt;
		m_wndCoSearchEdit.GetClientRect(&rcFmt);
		rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
		rcFmt.left += 6;
		rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
		m_wndCoSearchEdit.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
	}
	m_wndCoSearchBtn.MoveWindow(nSearchBtnLeft, nSearchTop, TAECHANG_RESULT_SEARCH_WIDTH, TAECHANG_BUTTON_HEIGHT);

	int nListTop = nSearchTop + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;
	int nListHeight = nHeight - (nListTop - nTop) - TAECHANG_MARGIN;
	if (nListHeight < TAECHANG_RESULT_MIN_HEIGHT)
		nListHeight = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndCoList.MoveWindow(nLeft, nListTop, nListWidth, nListHeight);
	UpdateCoListColumns();
}

void CSageTaechangView::ShowCompanyOrderPanel(BOOL bShow) {
	int nCmdShow = bShow ? SW_SHOW : SW_HIDE;
	m_wndCoCrudSection.ShowWindow(nCmdShow);
	m_wndCoListSection.ShowWindow(nCmdShow);
	m_wndCoAddBtn.ShowWindow(nCmdShow);
	m_wndCoModifyBtn.ShowWindow(nCmdShow);
	m_wndCoCancelBtn.ShowWindow(nCmdShow);
	m_wndCoDeleteBtn.ShowWindow(nCmdShow);
	m_wndCoSearchLabel.ShowWindow(nCmdShow);
	m_wndCoSearchEdit.ShowWindow(nCmdShow);
	m_wndCoSearchBtn.ShowWindow(nCmdShow);
	m_wndCoOrderLabel.ShowWindow(nCmdShow);
	m_wndCoOrderEdit.ShowWindow(nCmdShow);
	m_wndCoNameLabel.ShowWindow(nCmdShow);
	m_wndCoCompanyEdit.ShowWindow(nCmdShow);
	m_wndCoList.ShowWindow(nCmdShow);
	if (!bShow) {
		m_nCoPanelState = TAECHANG_CO_PANEL_IDLE;
		SetCardRect(m_rectCoCard, CRect(0, 0, 0, 0));
	}
	else {
		UpdateCoPanelState();
		UpdateCoListColumns();
	}
}

void CSageTaechangView::UpdateCoListColumns() {
	if (!::IsWindow(m_wndCoList.GetSafeHwnd()))
		return;
	HWND hHeader = (HWND)m_wndCoList.SendMessage(LVM_GETHEADER);
	if (::IsWindow(hHeader)) {
		int nColCount = (int)::SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0);
		for (int i = nColCount - 1; i >= 2; --i)
			m_wndCoList.DeleteColumn(i);
	}
	CRect rectList;
	m_wndCoList.GetClientRect(&rectList);
	m_wndCoList.SetColumnWidth(0, TAECHANG_CO_ORDER_COL_WIDTH);
	m_wndCoList.SetColumnWidth(1, max(0, rectList.Width() - TAECHANG_CO_ORDER_COL_WIDTH));
}

void CSageTaechangView::UpdateCoPanelState() {
	BOOL bEditing = (m_nCoPanelState == TAECHANG_CO_PANEL_MODIFY) ? TRUE : FALSE;
	BOOL bHasSelection = (m_nCoSelectedOrderId > 0) ? TRUE : FALSE;

	m_wndCoAddBtn.EnableWindow(!bEditing ? TRUE : FALSE);
	m_wndCoModifyBtn.EnableWindow((bEditing || bHasSelection) ? TRUE : FALSE);
	m_wndCoModifyBtn.SetWindowTextW(bEditing ? TAECHANG_UI_CO_SAVE_BTN : TAECHANG_UI_CO_MODIFY_BTN);
	m_wndCoDeleteBtn.EnableWindow((!bEditing && bHasSelection) ? TRUE : FALSE);
	m_wndCoCancelBtn.EnableWindow(bEditing ? TRUE : FALSE);
	m_wndCoSearchEdit.EnableWindow(!bEditing ? TRUE : FALSE);
	m_wndCoSearchBtn.EnableWindow(!bEditing ? TRUE : FALSE);
}

void CSageTaechangView::RefreshCompanyOrderList() {
	if (!::IsWindow(m_wndCoList.GetSafeHwnd()))
		return;
	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->LoadAllCompanyOrders(m_arrCoOrders, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	m_wndCoList.SetRedraw(FALSE);
	m_wndCoList.DeleteAllItems();
	CString strFilterLower = m_strCoSearchKeyword;
	strFilterLower.MakeLower();
	for (int i = 0; i < m_arrCoOrders.GetSize(); ++i) {
		const TaechangReceivableCompanyOrderDto& dto = m_arrCoOrders[i];
		if (!strFilterLower.IsEmpty()) {
			CString strNameLower = dto.strCompanyName;
			strNameLower.MakeLower();
			if (strNameLower.Find(strFilterLower) < 0)
				continue;
		}
		int nItem = m_wndCoList.InsertItem(m_wndCoList.GetItemCount(), L"");
		CString strOrder;
		strOrder.Format(L"%d", dto.nSortOrder);
		m_wndCoList.SetItemText(nItem, 0, strOrder);
		m_wndCoList.SetItemText(nItem, 1, dto.strCompanyName);
		m_wndCoList.SetItemData(nItem, static_cast<DWORD_PTR>(dto.nOrderId));
	}
	if (m_nCoSelectedOrderId > 0) {
		for (int i = 0; i < m_wndCoList.GetItemCount(); ++i) {
			if (static_cast<int>(m_wndCoList.GetItemData(i)) == m_nCoSelectedOrderId) {
				m_wndCoList.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				m_wndCoList.EnsureVisible(i, FALSE);
				break;
			}
		}
	}
	UpdateCoPanelState();
	Invalidate(FALSE);
	m_wndCoList.SetRedraw(TRUE);
	m_wndCoList.Invalidate();
}

void CSageTaechangView::OnCoAdd() {
	CString strCompanyName;
	CString strOrderStr;
	m_wndCoCompanyEdit.GetWindowTextW(strCompanyName);
	m_wndCoOrderEdit.GetWindowTextW(strOrderStr);
	strCompanyName.Trim();
	if (strCompanyName.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_CO_COMPANY_REQUIRED);
		m_wndCoCompanyEdit.SetFocus();
		return;
	}
	int nSortOrder;
	strOrderStr.Trim();
	if (strOrderStr.IsEmpty()) {
		nSortOrder = 1;
		for (int i = 0; i < m_arrCoOrders.GetSize(); ++i) {
			if (m_arrCoOrders[i].nSortOrder >= nSortOrder)
				nSortOrder = m_arrCoOrders[i].nSortOrder + 1;
		}
	} else {
		nSortOrder = _wtoi(strOrderStr);
	}
	TaechangReceivableCompanyOrderDto dto;
	dto.strCompanyName = strCompanyName;
	dto.nSortOrder = nSortOrder;
	int nNewOrderId = 0;
	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->AddCompanyOrder(dto, nNewOrderId, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	m_wndCoOrderEdit.SetWindowTextW(L"");
	m_wndCoCompanyEdit.SetWindowTextW(L"");
	m_nCoSelectedOrderId = nNewOrderId;
	m_wndCoList.SetItemState(-1, 0, LVIS_SELECTED);
	RefreshCompanyOrderList();
}

void CSageTaechangView::OnCoModify() {
	if (m_nCoPanelState == TAECHANG_CO_PANEL_IDLE) {
		if (m_nCoSelectedOrderId <= 0) {
			AfxMessageBox(TAECHANG_UI_CO_SELECT_REQUIRED);
			return;
		}
		m_nCoPanelState = TAECHANG_CO_PANEL_MODIFY;
		UpdateCoPanelState();
		m_wndCoCompanyEdit.SetFocus();
		Invalidate(FALSE);
	} else {
		CString strCompanyName;
		CString strOrderStr;
		m_wndCoCompanyEdit.GetWindowTextW(strCompanyName);
		m_wndCoOrderEdit.GetWindowTextW(strOrderStr);
		strCompanyName.Trim();
		if (strCompanyName.IsEmpty()) {
			AfxMessageBox(TAECHANG_UI_CO_COMPANY_REQUIRED);
			m_wndCoCompanyEdit.SetFocus();
			return;
		}
		TaechangReceivableCompanyOrderDto dto;
		BOOL bFound = FALSE;
		for (int i = 0; i < m_arrCoOrders.GetSize(); ++i) {
			if (m_arrCoOrders[i].nOrderId == m_nCoSelectedOrderId) {
				dto = m_arrCoOrders[i];
				bFound = TRUE;
				break;
			}
		}
		if (!bFound)
			return;
		dto.strCompanyName = strCompanyName;
		strOrderStr.Trim();
		if (!strOrderStr.IsEmpty())
			dto.nSortOrder = _wtoi(strOrderStr);
		CString strError;
		if (sageDBMgr.GetReceivableCompanyOrderService()->ChangeCompanyOrder(dto, strError) == FALSE) {
			AfxMessageBox(strError);
			return;
		}
		m_nCoPanelState = TAECHANG_CO_PANEL_IDLE;
		RefreshCompanyOrderList();
	}
}

void CSageTaechangView::OnCoDelete() {
	if (m_nCoSelectedOrderId <= 0) {
		AfxMessageBox(TAECHANG_UI_CO_SELECT_REQUIRED);
		return;
	}
	CString strCompanyName;
	for (int i = 0; i < m_arrCoOrders.GetSize(); ++i) {
		if (m_arrCoOrders[i].nOrderId == m_nCoSelectedOrderId) {
			strCompanyName = m_arrCoOrders[i].strCompanyName;
			break;
		}
	}
	CString strConfirm;
	strConfirm.Format(TAECHANG_UI_CO_DELETE_CONFIRM_FMT, (LPCWSTR)strCompanyName);
	if (AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;
	CString strError;
	if (sageDBMgr.GetReceivableCompanyOrderService()->RemoveCompanyOrder(m_nCoSelectedOrderId, strError) == FALSE) {
		AfxMessageBox(strError);
		return;
	}
	m_nCoSelectedOrderId = 0;
	m_wndCoOrderEdit.SetWindowTextW(L"");
	m_wndCoCompanyEdit.SetWindowTextW(L"");
	RefreshCompanyOrderList();
}

void CSageTaechangView::OnCoCancel() {
	m_nCoPanelState = TAECHANG_CO_PANEL_IDLE;
	m_nCoSelectedOrderId = 0;
	m_wndCoList.SetItemState(-1, 0, LVIS_SELECTED);
	m_wndCoOrderEdit.SetWindowTextW(L"");
	m_wndCoCompanyEdit.SetWindowTextW(L"");
	UpdateCoPanelState();
	Invalidate(FALSE);
}

void CSageTaechangView::OnCoSearch() {
	m_wndCoSearchEdit.GetWindowTextW(m_strCoSearchKeyword);
	m_strCoSearchKeyword.Trim();
	m_nCoSelectedOrderId = 0;
	m_wndCoOrderEdit.SetWindowTextW(L"");
	m_wndCoCompanyEdit.SetWindowTextW(L"");
	RefreshCompanyOrderList();
}

void CSageTaechangView::OnCoListSelChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	int nSel = m_wndCoList.GetNextItem(-1, LVNI_SELECTED);
	m_nCoSelectedOrderId = (nSel >= 0) ? static_cast<int>(m_wndCoList.GetItemData(nSel)) : 0;
	if (m_nCoPanelState == TAECHANG_CO_PANEL_IDLE) {
		if (m_nCoSelectedOrderId > 0) {
			for (int i = 0; i < m_arrCoOrders.GetSize(); ++i) {
				if (m_arrCoOrders[i].nOrderId == m_nCoSelectedOrderId) {
					CString strOrder;
					strOrder.Format(L"%d", m_arrCoOrders[i].nSortOrder);
					m_wndCoOrderEdit.SetWindowTextW(strOrder);
					m_wndCoCompanyEdit.SetWindowTextW(m_arrCoOrders[i].strCompanyName);
					Invalidate(FALSE);
					break;
				}
			}
		} else {
			m_wndCoOrderEdit.SetWindowTextW(L"");
			m_wndCoCompanyEdit.SetWindowTextW(L"");
			Invalidate(FALSE);
		}
	}
	UpdateCoPanelState();
	*pResult = 0;
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


