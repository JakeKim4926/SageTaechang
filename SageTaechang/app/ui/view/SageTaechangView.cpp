
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
#include "app/common/TaechangJson.h"
#include "app/core/workflow/ISageWorkflowHandler.h"
#include "app/core/workflow/SageWorkflowRegistry.h"
#include "app/core/workflow/TaechangWorkflowResponse.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "app/core/price/TaechangPriceDto.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "app/ui/dialogs/TaechangPasswordChangeDlg.h"
#include "app/ui/dialogs/TaechangCompanyDlg.h"
#include "app/ui/dialogs/TaechangPriceRangeDlg.h"
#include "app/ui/dialogs/TaechangPriceSimpleDlg.h"
#include "app/ui/dialogs/TaechangCalcCompanyPickerDlg.h"
#include "app/ui/dialogs/TaechangCalcEstimateDlg.h"
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
	ON_CBN_SELCHANGE(ID_PRICE_COMPANY_EDIT, &CSageTaechangView::OnPriceCompanySelChanged)
	ON_CBN_EDITCHANGE(ID_PRICE_COMPANY_EDIT, &CSageTaechangView::OnPriceCompanyEditChanged)
	ON_BN_CLICKED(ID_PRICE_ADD_COMPANY_BTN, &CSageTaechangView::OnPriceAddCompany)
	ON_BN_CLICKED(ID_PRICE_RENAME_COMPANY_BTN, &CSageTaechangView::OnPriceRenameCompany)
	ON_BN_CLICKED(ID_PRICE_DELETE_COMPANY_BTN, &CSageTaechangView::OnPriceDeleteCompany)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_PRICE_COPIES_LIST, &CSageTaechangView::OnPriceCopiesSelChanged)
	ON_BN_CLICKED(ID_PRICE_NO_MAX_CHECK, &CSageTaechangView::OnPriceNoMaxCheck)
	ON_BN_CLICKED(ID_PRICE_SINGLE_CHECK, &CSageTaechangView::OnPriceSingleCheck)
	ON_EN_CHANGE(ID_PRICE_PRINT_EDIT, &CSageTaechangView::OnPricePrintChanged)
	ON_EN_CHANGE(ID_PRICE_COVER_EDIT, &CSageTaechangView::OnPriceCoverChanged)
	ON_BN_CLICKED(ID_PRICE_ADD_BTN, &CSageTaechangView::OnPriceAdd)
	ON_BN_CLICKED(ID_PRICE_MODIFY_BTN, &CSageTaechangView::OnPriceModify)
	ON_BN_CLICKED(ID_PRICE_DELETE_BTN, &CSageTaechangView::OnPriceDelete)
	ON_BN_CLICKED(ID_PRICE_CANCEL_BTN, &CSageTaechangView::OnPriceCancel)
	ON_CBN_SELCHANGE(ID_CALC_COMPANY_COMBO, &CSageTaechangView::OnCalcCompanyChanged)
	ON_CBN_EDITCHANGE(ID_CALC_COMPANY_COMBO, &CSageTaechangView::OnCalcCompanyChanged)
	ON_BN_CLICKED(ID_CALC_BTN, &CSageTaechangView::OnCalc)
	ON_BN_CLICKED(ID_CALC_RESET_BTN, &CSageTaechangView::OnCalcReset)
	ON_EN_CHANGE(ID_CALC_COPIES_EDIT, &CSageTaechangView::OnCalcInputChanged)
	ON_EN_CHANGE(ID_CALC_PAGES_EDIT, &CSageTaechangView::OnCalcInputChanged)
	ON_EN_CHANGE(ID_CALC_FREIGHT_EDIT, &CSageTaechangView::OnCalcFreightChanged)
	ON_BN_CLICKED(ID_CALC_COMPANY_PICK_BTN, &CSageTaechangView::OnCalcCompanyPick)
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
	, m_nCalcPrintPrice(0)
	, m_nCalcCoverPrice(0)
	, m_nCalcUnitPrice(0)
	, m_nPricePanelState(TAECHANG_PRICE_PANEL_SUMMARY)
	, m_bFormattingCalcFreight(FALSE)
	, m_bFormattingPricePrint(FALSE)
	, m_bFormattingPriceCover(FALSE)
	, m_rectPriceSummaryCard(0, 0, 0, 0)
	, m_nAuthDividerX(0)
	, m_nCoPanelState(TAECHANG_CO_PANEL_IDLE)
	, m_nCoSelectedOrderId(0)
	, m_rectCoCard(0, 0, 0, 0)
	, m_rectResultFilterBox(0, 0, 0, 0) {
	m_brushListHeader.CreateSolidBrush(TAECHANG_COLOR_LIST_HEADER);
}

CSageTaechangView::~CSageTaechangView() {}

BOOL CSageTaechangView::PreCreateWindow(CREATESTRUCT& cs) {
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
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB &&
		GetSelectedWorkflow() == TAECHANG_WORKFLOW_PRICE_CALC && GetKeyState(VK_SHIFT) >= 0) {
		COMBOBOXINFO cbiCalcCompany = {};
		cbiCalcCompany.cbSize = sizeof(COMBOBOXINFO);
		m_wndCalcCompanyCombo.GetComboBoxInfo(&cbiCalcCompany);
		if (pMsg->hwnd == m_wndCalcCompanyCombo.GetSafeHwnd() || pMsg->hwnd == cbiCalcCompany.hwndItem) {
			m_wndCalcCopiesEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndCalcCopiesEdit.GetSafeHwnd()) {
			m_wndCalcPagesEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndCalcPagesEdit.GetSafeHwnd()) {
			m_wndCalcFreightEdit.SetFocus();
			return TRUE;
		}
	}
	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB &&
		GetSelectedWorkflow() == TAECHANG_WORKFLOW_PRICE_MANAGE && GetKeyState(VK_SHIFT) >= 0) {
		if (pMsg->hwnd == m_wndPriceMinCopiesEdit.GetSafeHwnd()) {
			if (m_wndPriceMaxCopiesEdit.IsWindowEnabled())
				m_wndPriceMaxCopiesEdit.SetFocus();
			else
				m_wndPricePrintEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndPriceMaxCopiesEdit.GetSafeHwnd()) {
			m_wndPricePrintEdit.SetFocus();
			return TRUE;
		}
		if (pMsg->hwnd == m_wndPricePrintEdit.GetSafeHwnd()) {
			m_wndPriceCoverEdit.SetFocus();
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
	m_wndLoad.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndGenerate.Create(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_GENERATE_WORKFLOW);
	m_wndGenerate.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndSelectAll.Create(TAECHANG_UI_SELECT_ALL_BUTTON, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_SELECT_ALL);
	m_wndEstimateOnePage.Create(TAECHANG_UI_ESTIMATE_ONE_PAGE_CHECK, WS_CHILD | BS_AUTOCHECKBOX, rectEmpty, this, ID_TAECHANG_ESTIMATE_ONE_PAGE);
	m_wndInputReset.Create(TAECHANG_UI_INPUT_RESET_BTN, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_INPUT_RESET_BTN);
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
	m_wndResultSearchBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndResultSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndResultResetBtn.Create(TAECHANG_UI_RESULT_RESET_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RESULT_RESET_BTN);
	m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);
	m_wndEmptyStateHint.Create(TAECHANG_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
	m_wndActionStatus.Create(L"", WS_CHILD | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

	m_wndLoginBtn.Create(TAECHANG_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGIN_BTN);
	m_wndLoginBtn.SetVariant(SAGE_BUTTON_PRIMARY);
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

	CreatePriceManagePanel();
	CreatePriceCalcPanel();
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
	m_wndResultList.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	if (::IsWindow(m_wndResultHeader.GetSafeHwnd()))
		m_wndResultHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
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

	m_wndPriceCompanyCombo.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceAddCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceRenameCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceDeleteCompanyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceCopiesList.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	if (::IsWindow(m_wndPriceCopiesHeader.GetSafeHwnd()))
		m_wndPriceCopiesHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceMinCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceSingleCheck.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceMaxCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceNoMaxCheck.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPricePrintEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceCoverEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceAddBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceModifyBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceDeleteBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndPriceCancelBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));

	m_wndCalcCompanyCombo.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcCompanyPickBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcCopiesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcPagesEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcResetBtn.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcFreightEdit.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcHistorySection.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	m_wndCalcHistoryList.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	if (::IsWindow(m_wndCalcHistoryHeader.GetSafeHwnd()))
		m_wndCalcHistoryHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
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
	m_wndCoList.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
	if (::IsWindow(m_wndCoListHeader.GetSafeHwnd()))
		m_wndCoListHeader.SetFont(SageUiResources::GetFont(SAGE_FONT_CONTENT));
}

void CSageTaechangView::ApplyLabelRoles() {
	m_wndTitle.SetTextColorRole(SAGE_TEXT_SIDEBAR);
	m_wndTitle.SetBackgroundRole(SAGE_BG_SIDEBAR);
	m_wndTitle.SetFontRole(SAGE_FONT_TITLE);

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

	m_wndPriceSummaryTitle.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceSummaryTitle.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceSummaryTitle.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceSummaryCount.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceSummaryCount.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceSummaryCount.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceSummaryRange.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceSummaryRange.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceSummaryRange.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceMinCopiesLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceMinCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceMinCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceMaxCopiesLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceMaxCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceMaxCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPricePrintLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPricePrintLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPricePrintLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceCoverLabel.SetTextColorRole(SAGE_TEXT_SECONDARY);
	m_wndPriceCoverLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceCoverLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndPriceDetailHeader.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceDetailHeader.SetFontRole(SAGE_FONT_HEADER);

	m_wndCalcTotalLabel.SetTextColorRole(SAGE_TEXT_PRIMARY);
	m_wndCalcTotalLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcTotalLabel.SetFontRole(SAGE_FONT_HEADER);

	m_wndCalcTotalValue.SetTextColorRole(SAGE_TEXT_PRIMARY);
	m_wndCalcTotalValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcTotalValue.SetFontRole(SAGE_FONT_HEADER);

	m_wndCalcCompanyLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcCompanyLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcCopiesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcCopiesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcPagesLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcPagesLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcPrintLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcPrintLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcPrintValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcPrintValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcCoverLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcCoverLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcCoverValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcCoverValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcSubtotalLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcSubtotalLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcSubtotalValue.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcSubtotalValue.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcFreightLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcFreightLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcFreightUnitLabel.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcFreightUnitLabel.SetFontRole(SAGE_FONT_CONTENT);

	m_wndCalcDivider.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndCalcTotalDivider.SetBackgroundRole(SAGE_BG_PANEL);
	m_wndPriceDetailDivider.SetBackgroundRole(SAGE_BG_PANEL);

	m_wndWorkflowLabel.SetFontRole(SAGE_FONT_CONTENT);
	m_wndInputLabel.SetFontRole(SAGE_FONT_CONTENT);
	m_wndOutputLabel.SetFontRole(SAGE_FONT_CONTENT);
	m_wndProgressText.SetFontRole(SAGE_FONT_CONTENT);
	m_wndPriceCompanyLabel.SetFontRole(SAGE_FONT_CONTENT);
}

void CSageTaechangView::ApplyWorkflowTabs() {
	m_wndTaskTabs.DeleteAllItems();
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
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

	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
	if (pHandler == NULL)
		return;

	SageWorkflowResultStyle resultStyle = pHandler->GetResultStyle(m_nLastTaskType);
	DWORD dwExtStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	if (resultStyle.bCheckbox)
		dwExtStyle |= LVS_EX_CHECKBOXES;
	if (resultStyle.bGridLines)
		dwExtStyle |= LVS_EX_GRIDLINES;
	m_wndResultList.SetExtendedStyle(dwExtStyle);
	m_wndResultList.SetHighlightColumns(resultStyle.nHighlightStart, resultStyle.nHighlightCount);

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
	BOOL bShowOutput = (bShowInput || IsResultTab()) ? TRUE : FALSE;
	BOOL bShowAction = IsActionTabVisible();
	BOOL bShowResult = IsResultTab() || (IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable()));
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
	BOOL bShowSelectAll = (bShowAction && (IsDeliveryInputTable() || IsEstimateInputTable())) ? TRUE : FALSE;
	m_wndSelectAll.ShowWindow(bShowSelectAll ? SW_SHOW : SW_HIDE);
	BOOL bShowEstimateOnePage = (bShowAction && IsEstimateInputTable()) ? TRUE : FALSE;
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
		m_rectResultFilterBox.SetRectEmpty();
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

	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
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

	// 가격 워크플로우: 기존 탭/패널을 숨기고 전용 패널 표시
	ShowPriceManagePanel(FALSE);
	ShowPriceCalcPanel(FALSE);

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
		m_rectResultFilterBox.SetRectEmpty();
		m_wndDetailSection.ShowWindow(SW_HIDE);
		m_wndDetail.ShowWindow(SW_HIDE);
		m_wndEmptyStateHint.ShowWindow(SW_HIDE);

		int nPanelHeight = nContentHeight - (nContentTop - TAECHANG_MARGIN);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE) {
			LayoutPriceManagePanel(nContentLeft, nContentTop, nContentWidth, nPanelHeight);
			ShowPriceManagePanel(TRUE);
		} else {
			LayoutPriceCalcPanel(nContentLeft, nContentTop, nContentWidth, nPanelHeight);
			ShowPriceCalcPanel(TRUE);
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
	if (IsResultTab() || (IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable()))) {
		BOOL bShowSelectAll = IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable());
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
			if (IsEstimateInputTable()) {
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
			m_rectResultFilterBox.SetRect(
				nFilterLeft - TAECHANG_RESULT_FILTER_BOX_PAD,
				nFilterTop - TAECHANG_RESULT_FILTER_BOX_PAD,
				nResetLeft + TAECHANG_RESULT_RESET_WIDTH + TAECHANG_RESULT_FILTER_BOX_PAD,
				nFilterTop + TAECHANG_EDIT_HEIGHT + TAECHANG_RESULT_FILTER_BOX_PAD);
		} else {
			m_rectResultFilterBox.SetRectEmpty();
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
	DrawEditBorder(pDC, m_wndPriceCompanyCombo);
	DrawEditBorder(pDC, m_wndPriceMinCopiesEdit);
	DrawEditBorder(pDC, m_wndPriceMaxCopiesEdit);
	DrawEditBorder(pDC, m_wndPricePrintEdit);
	DrawEditBorder(pDC, m_wndPriceCoverEdit);
	DrawEditBorder(pDC, m_wndCalcCompanyCombo);
	DrawEditBorder(pDC, m_wndCalcCopiesEdit);
	DrawEditBorder(pDC, m_wndCalcPagesEdit);
	DrawEditBorder(pDC, m_wndCalcFreightEdit);
	if (!m_rectPriceSummaryCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectPriceSummaryCard, TAECHANG_COLOR_PANEL);
		CBrush brCardBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectPriceSummaryCard, &brCardBorder);
	}
	if (!m_rectCalcInputPanel.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCalcInputPanel, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCalcInputPanel, &brBorder);
	}
	if (!m_rectCalcResultPanel.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCalcResultPanel, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCalcResultPanel, &brBorder);
		CRect rectTotalDiv;
		m_wndCalcTotalDivider.GetWindowRect(&rectTotalDiv);
		ScreenToClient(&rectTotalDiv);
		pDC->FillSolidRect(rectTotalDiv.left, rectTotalDiv.top - 2, rectTotalDiv.Width(), 2, TAECHANG_COLOR_BORDER);
	}
	DrawEditBorder(pDC, m_wndCalcCompanyCombo);
	DrawEditBorder(pDC, m_wndCalcCopiesEdit);
	DrawEditBorder(pDC, m_wndCalcPagesEdit);
	DrawEditBorder(pDC, m_wndCalcFreightEdit);
	if (taechangAuth.IsLoggedIn() && m_nAuthDividerX > 0) {
		int nDivTop = (TAECHANG_TOP_BAR_HEIGHT - TAECHANG_BUTTON_HEIGHT) / 2;
		pDC->FillSolidRect(m_nAuthDividerX, nDivTop, 1, TAECHANG_BUTTON_HEIGHT, TAECHANG_COLOR_BORDER);
	}
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

void CSageTaechangView::UpdateWorkflowLabels() {
	int nWorkflowType = GetSelectedWorkflow();
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
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
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
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
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
	if (pHandler == NULL)
		return TAECHANG_TAB_INDEX_INPUT;
	if (nVisualTabIndex < 0 || nVisualTabIndex >= pHandler->GetTabCount())
		return TAECHANG_TAB_INDEX_INPUT;
	return pHandler->GetTab(nVisualTabIndex).nSemanticIndex;
}

BOOL CSageTaechangView::IsReceivablesResultTable() const {
	if (m_nLastWorkflowType != TAECHANG_WORKFLOW_RECEIVABLES)
		return FALSE;
	if (m_nLastTaskType == TAECHANG_TASK_LOAD)
		return TRUE;
	return (m_nLastTaskType == TAECHANG_TASK_GENERATE) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDeliveryInputTable() const {
	if (m_nLastWorkflowType != TAECHANG_WORKFLOW_DELIVERY)
		return FALSE;
	return (m_nLastTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsEstimateInputTable() const {
	if (m_nLastWorkflowType != TAECHANG_WORKFLOW_ESTIMATE)
		return FALSE;
	return (m_nLastTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsInputResetVisible() const {
	if (m_bRunning || !IsInputTabSelected())
		return FALSE;
	return (IsDeliveryInputTable() || IsEstimateInputTable()) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDataManageTab() const {
	return (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES &&
		m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_DATA_MANAGE) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDocumentResultFilterVisible() const {
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES && IsReceivablesResultTable())
		return TRUE;
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_DELIVERY && IsDeliveryInputTable())
		return TRUE;
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_ESTIMATE && IsEstimateInputTable())
		return TRUE;
	return FALSE;
}

BOOL CSageTaechangView::IsDocumentWorkflowStateTarget(int nWorkflowType) const {
	return (nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES ||
		nWorkflowType == TAECHANG_WORKFLOW_DELIVERY ||
		nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) ? TRUE : FALSE;
}

TaechangWorkflowUiState& CSageTaechangView::GetWorkflowUiState(int nWorkflowType) {
	if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
		return m_stateDelivery;
	if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
		return m_stateEstimate;
	return m_stateReceivables;
}

void CSageTaechangView::SaveWorkflowUiState(int nWorkflowType) {
	if (!IsDocumentWorkflowStateTarget(nWorkflowType))
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
	if (!IsDocumentWorkflowStateTarget(nWorkflowType)) {
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
	if (!IsDeliveryInputTable() && !IsEstimateInputTable())
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
	if (!IsDeliveryInputTable() && !IsEstimateInputTable())
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
		if (IsDocumentWorkflowStateTarget(GetSelectedWorkflow()))
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

			RefreshPriceCompanyList();
		} else {
			RefreshCalcCompanyCombo();
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

	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
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
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(GetSelectedWorkflow());
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
	if (bCheck && IsEstimateInputTable() && m_wndEstimateOnePage.GetCheck() == BST_CHECKED && nCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
		for (int i = 0; i < nCount; ++i)
			m_wndResultList.SetCheck(i, i < TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS ? TRUE : FALSE);
		AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
		return;
	}
	for (int i = 0; i < nCount; ++i)
		m_wndResultList.SetCheck(i, bCheck);
}

void CSageTaechangView::OnEstimateOnePage() {
	if (!IsEstimateInputTable() || m_wndEstimateOnePage.GetCheck() != BST_CHECKED)
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
	ISageWorkflowHandler* pHandler = SageWorkflowRegistry::FindHandler(nWorkflowType);
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

	CString strInputPath;
	CString strOutputFolder;
	if (!ValidateInputPath(strInputPath))
		return;

	int nWorkflowType = GetSelectedWorkflow();
	if (nTaskType == TAECHANG_TASK_GENERATE && !ValidateOutputFolder(strOutputFolder))
		return;

	CString strSelectedRowNums;
	if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) && nTaskType == TAECHANG_TASK_GENERATE) {
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
		if (strSelectedRowNums.IsEmpty()) {
			LPCWSTR pszMsg = (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
				? TAECHANG_UI_ESTIMATE_SELECT_ROW_REQUIRED
				: TAECHANG_UI_DELIVERY_SELECT_ROW_REQUIRED;
			AfxMessageBox(pszMsg, MB_ICONWARNING);
			return;
		}
		if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE &&
			m_wndEstimateOnePage.GetCheck() == BST_CHECKED &&
			nSelectedCount > TAECHANG_ESTIMATE_ONE_PAGE_MAX_ROWS) {
			AfxMessageBox(TAECHANG_UI_ESTIMATE_ONE_PAGE_LIMIT, MB_ICONWARNING);
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
	DrawEditBorder(pDC, m_wndPriceCompanyCombo);
	DrawEditBorder(pDC, m_wndPriceMinCopiesEdit);
	DrawEditBorder(pDC, m_wndPriceMaxCopiesEdit);
	DrawEditBorder(pDC, m_wndPricePrintEdit);
	DrawEditBorder(pDC, m_wndPriceCoverEdit);
	DrawEditBorder(pDC, m_wndCalcCompanyCombo);
	DrawEditBorder(pDC, m_wndCalcCopiesEdit);
	DrawEditBorder(pDC, m_wndCalcPagesEdit);
	DrawEditBorder(pDC, m_wndCalcFreightEdit);
	if (!m_rectPriceSummaryCard.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectPriceSummaryCard, TAECHANG_COLOR_PANEL);
		CBrush brCardBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectPriceSummaryCard, &brCardBorder);
	}
	if (!m_rectCalcInputPanel.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCalcInputPanel, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCalcInputPanel, &brBorder);
	}
	if (!m_rectCalcResultPanel.IsRectEmpty()) {
		pDC->FillSolidRect(m_rectCalcResultPanel, TAECHANG_COLOR_PANEL);
		CBrush brBorder(TAECHANG_COLOR_BORDER);
		pDC->FrameRect(m_rectCalcResultPanel, &brBorder);
	}
	DrawEditBorder(pDC, m_wndCalcCompanyCombo);
	DrawEditBorder(pDC, m_wndCalcCopiesEdit);
	DrawEditBorder(pDC, m_wndCalcPagesEdit);
	DrawEditBorder(pDC, m_wndCalcFreightEdit);
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
	if (pWnd->GetSafeHwnd() == m_wndPriceNoMaxCheck.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndPriceSingleCheck.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return SageUiResources::GetBrush(SAGE_BG_PANEL);
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
	BOOL bDocumentGenerateNoResult =
		(nTaskType == TAECHANG_TASK_GENERATE &&
		 (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE))
		? TRUE : FALSE;
	if (!bDocumentGenerateNoResult)
		m_wndResultList.DeleteAllItems();
	m_nLastWorkflowType = nWorkflowType;
	if (!bDocumentGenerateNoResult)
		m_nLastTaskType = nTaskType;
	if (!bDocumentGenerateNoResult)
		m_strLastResponseJson = strResponseJson;
	if (!bDocumentGenerateNoResult) {
		ApplyResultColumns();
		UpdateResultColumns();
	}

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows);
	AppendExecutionHistory(nWorkflowType, nTaskType, strResponseJson, bSuccess);
	m_wndDetail.SetWindowTextW(m_strExecutionHistory);

	if (!bDocumentGenerateNoResult) {
		if (nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES ||
			nWorkflowType == TAECHANG_WORKFLOW_DELIVERY ||
			nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
			RefreshDocumentResultFilter();
		else {
			m_wndResultList.SetRedraw(FALSE);
			for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
				InsertResultRow(arrRows[i]);
			m_wndResultList.SetRedraw(TRUE);
			m_wndResultList.Invalidate();
		}
	}

	if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) && nTaskType == TAECHANG_TASK_LOAD) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		UpdateTaskTabVisibility();
		LayoutChildControls();
	} else if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) &&
			   nTaskType == TAECHANG_TASK_GENERATE) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		UpdateTaskTabVisibility();
		LayoutChildControls();
		if (bSuccess) {
			AfxMessageBox(
				nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE
				? TAECHANG_UI_ESTIMATE_GENERATE_COMPLETED
				: TAECHANG_UI_DELIVERY_GENERATE_COMPLETED,
				MB_ICONINFORMATION);
		}
	} else if ((nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES && nTaskType == TAECHANG_TASK_GENERATE) ||
			   nTaskType == TAECHANG_TASK_LOAD) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_DOCUMENT_RESULT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		UpdateTaskTabVisibility();
		LayoutChildControls();
	}

	m_bLastTaskSuccess = bSuccess;
	m_wndActionStatus.SetWindowTextW(bSuccess ? TAECHANG_UI_ACTION_STATUS_COMPLETED : TAECHANG_UI_ACTION_STATUS_FAILED);
	m_wndActionStatus.Invalidate();
	SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
	SaveWorkflowUiState(nWorkflowType);
}

void CSageTaechangView::InsertResultRow(const TaechangResultRow& row) {
	int nCount = m_wndResultList.GetItemCount();
	int nCol = 0;
	int nIndex;
	if (IsReceivablesResultTable()) {
		nIndex = m_wndResultList.InsertItem(nCount, row.m_strCompanyName);
		m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
		m_wndResultList.SetItemText(nIndex, 1, row.m_strManager);
		m_wndResultList.SetItemText(nIndex, 2, row.m_strIssueDate);
		m_wndResultList.SetItemText(nIndex, 3, row.m_strItemName);
		m_wndResultList.SetItemText(nIndex, 4, row.m_strIssueType);
		m_wndResultList.SetItemText(nIndex, 5, row.m_strTotalAmount);
		m_wndResultList.SetItemText(nIndex, 6, row.m_strDepositAmount);
		m_wndResultList.SetItemText(nIndex, 7, row.m_strReceivableAmount);
		m_wndResultList.SetItemText(nIndex, 8, row.m_strBankName);
		m_wndResultList.SetItemText(nIndex, 9, row.m_strNote);
		return;
	}
	if (IsDeliveryInputTable()) {
		nIndex = m_wndResultList.InsertItem(nCount, row.m_strField);
		m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
		m_wndResultList.SetItemText(nIndex, 1, row.m_strCompanyName);
		m_wndResultList.SetItemText(nIndex, 2, row.m_strDepartment);
		m_wndResultList.SetItemText(nIndex, 3, row.m_strOrderDate);
		m_wndResultList.SetItemText(nIndex, 4, row.m_strDeliveryDate);
		m_wndResultList.SetItemText(nIndex, 5, row.m_strDeliveryTime);
		m_wndResultList.SetItemText(nIndex, 6, row.m_strItemName);
		m_wndResultList.SetItemText(nIndex, 7, row.m_strProductType);
		m_wndResultList.SetItemText(nIndex, 8, row.m_strCompanyCopies);
		m_wndResultList.SetItemText(nIndex, 9, row.m_strCorporationCopies);
		m_wndResultList.SetItemText(nIndex, 10, row.m_strTotalCopies);
		return;
	}
	if (IsEstimateInputTable()) {
		nIndex = m_wndResultList.InsertItem(nCount, row.m_strField);
		m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
		m_wndResultList.SetItemText(nIndex, 1, row.m_strCompanyName);
		m_wndResultList.SetItemText(nIndex, 2, row.m_strIssueDate);
		m_wndResultList.SetItemText(nIndex, 3, row.m_strItemName);
		m_wndResultList.SetItemText(nIndex, 4, row.m_strCompanyCopies);
		m_wndResultList.SetItemText(nIndex, 5, row.m_strCorporationCopies);
		m_wndResultList.SetItemText(nIndex, 6, row.m_strTotalCopies);
		m_wndResultList.SetItemText(nIndex, 7, row.m_strValue);
		m_wndResultList.SetItemText(nIndex, 8, row.m_strReason);
		return;
	}
	nIndex = m_wndResultList.InsertItem(nCount, row.m_strField);
	++nCol;
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strValue);
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strStatus);
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strReason);
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
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES)
		return TAECHANG_FILTER_CRITERIA_COMPANY;
	return TAECHANG_FILTER_CRITERIA_ITEM;
}

int CSageTaechangView::GetEffectiveFilterCriteria() const {
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES) {
		if (m_nResultFilterCriteria == TAECHANG_FILTER_CRITERIA_COMPANY ||
			m_nResultFilterCriteria == TAECHANG_FILTER_CRITERIA_MANAGER ||
			m_nResultFilterCriteria == TAECHANG_FILTER_CRITERIA_ITEM)
			return m_nResultFilterCriteria;
	} else {
		if (m_nResultFilterCriteria == TAECHANG_FILTER_CRITERIA_ITEM ||
			m_nResultFilterCriteria == TAECHANG_FILTER_CRITERIA_COMPANY)
			return m_nResultFilterCriteria;
	}
	return GetDefaultFilterCriteria();
}

void CSageTaechangView::PopulateResultFilterCriteria() {
	if (!::IsWindow(m_wndResultFilterCriteria.GetSafeHwnd()))
		return;

	int nEffective = GetEffectiveFilterCriteria();
	m_wndResultFilterCriteria.ResetContent();
	if (GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES) {
		int nIndexCompany = m_wndResultFilterCriteria.AddString(TAECHANG_UI_FILTER_CRITERIA_COMPANY);
		m_wndResultFilterCriteria.SetItemData(nIndexCompany, TAECHANG_FILTER_CRITERIA_COMPANY);
		int nIndexManager = m_wndResultFilterCriteria.AddString(TAECHANG_UI_FILTER_CRITERIA_MANAGER);
		m_wndResultFilterCriteria.SetItemData(nIndexManager, TAECHANG_FILTER_CRITERIA_MANAGER);
		int nIndexItem = m_wndResultFilterCriteria.AddString(TAECHANG_UI_FILTER_CRITERIA_ITEM);
		m_wndResultFilterCriteria.SetItemData(nIndexItem, TAECHANG_FILTER_CRITERIA_ITEM);
	} else {
		int nIndexItem = m_wndResultFilterCriteria.AddString(TAECHANG_UI_FILTER_CRITERIA_ITEM);
		m_wndResultFilterCriteria.SetItemData(nIndexItem, TAECHANG_FILTER_CRITERIA_ITEM);
		int nIndexCompany = m_wndResultFilterCriteria.AddString(TAECHANG_UI_FILTER_CRITERIA_COMPANY);
		m_wndResultFilterCriteria.SetItemData(nIndexCompany, TAECHANG_FILTER_CRITERIA_COMPANY);
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
	if (!IsEstimateInputTable() || m_wndEstimateOnePage.GetCheck() != BST_CHECKED)
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


// ── 가격 관련 유틸 ────────────────────────────────────────────────────────────

static CString FormatPrice(LONGLONG nPrice) {
	CString str;
	str.Format(L"%I64d", nPrice);
	int nLen = str.GetLength();
	for (int i = nLen - 3; i > 0; i -= 3)
		str.Insert(i, L',');
	return str;
}

static CString RemovePriceSeparators(const CString& strText) {
	CString strResult = strText;
	strResult.Remove(L',');
	strResult.Trim();
	return strResult;
}

static int PriceTextToInt(const CString& strText) {
	CString strValue = RemovePriceSeparators(strText);
	return strValue.IsEmpty() ? 0 : _wtoi(strValue);
}

// ── 가격 데이터 관리 패널 생성 ────────────────────────────────────────────────

void CSageTaechangView::CreatePriceManagePanel() {
	CRect r(0, 0, 0, 0);
	m_wndPriceCompanyLabel.Create(TAECHANG_UI_PRICE_COMPANY_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndPriceCompanyCombo.Create(WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_PRICE_COMPANY_EDIT);
	m_wndPriceAddCompanyBtn.Create(TAECHANG_UI_PRICE_ADD_COMPANY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_ADD_COMPANY_BTN);
	m_wndPriceAddCompanyBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndPriceRenameCompanyBtn.Create(TAECHANG_UI_PRICE_RENAME_COMPANY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_RENAME_COMPANY_BTN);
	m_wndPriceRenameCompanyBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndPriceDeleteCompanyBtn.Create(TAECHANG_UI_PRICE_DELETE_COMPANY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_DELETE_COMPANY_BTN);
	m_wndPriceDeleteCompanyBtn.SetVariant(SAGE_BUTTON_PRIMARY);

	m_wndPriceCopiesList.Create(WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, r, this, ID_PRICE_COPIES_LIST);
	m_wndPriceCopiesList.SetAlternateRowColor(TRUE);
	m_wndPriceCopiesList.SetCenterFirstColumn(TRUE);
	m_wndPriceCopiesList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
	m_wndPriceCopiesList.InsertColumn(0, TAECHANG_UI_PRICE_COL_MIN_COPIES, LVCFMT_CENTER, TAECHANG_PRICE_COL_MIN_WIDTH);
	m_wndPriceCopiesList.InsertColumn(1, TAECHANG_UI_PRICE_COL_MAX_COPIES, LVCFMT_CENTER, TAECHANG_PRICE_COL_MAX_WIDTH);
	m_wndPriceCopiesList.InsertColumn(2, TAECHANG_UI_PRICE_COL_PRINT_PRICE, LVCFMT_CENTER, TAECHANG_PRICE_COL_PRINT_WIDTH);
	m_wndPriceCopiesList.InsertColumn(3, TAECHANG_UI_PRICE_COL_COVER_PRICE, LVCFMT_CENTER, TAECHANG_PRICE_COL_COVER_WIDTH);
	if (CHeaderCtrl* pHeader = m_wndPriceCopiesList.GetHeaderCtrl()) {
		m_wndPriceCopiesHeader.SubclassWindow(pHeader->GetSafeHwnd());
		SetWindowTheme(m_wndPriceCopiesHeader.GetSafeHwnd(), L"", L"");
		HDITEM hdi = {};
		hdi.mask = HDI_FORMAT;
		m_wndPriceCopiesHeader.GetItem(0, &hdi);
		hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
		m_wndPriceCopiesHeader.SetItem(0, &hdi);
	}

	m_wndPriceMinCopiesLabel.Create(TAECHANG_UI_PRICE_MIN_COPIES_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceMinCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_MIN_COPIES_EDIT);
	m_wndPriceSingleCheck.Create(TAECHANG_UI_PRICE_SINGLE_LABEL, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_PRICE_SINGLE_CHECK);
	m_wndPriceMaxCopiesLabel.Create(TAECHANG_UI_PRICE_MAX_COPIES_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceMaxCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_MAX_COPIES_EDIT);
	m_wndPriceNoMaxCheck.Create(TAECHANG_UI_PRICE_NO_MAX_LABEL, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_PRICE_NO_MAX_CHECK);

	m_wndPricePrintLabel.Create(TAECHANG_UI_PRICE_PRINT_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPricePrintEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_PRINT_EDIT);
	m_wndPriceCoverLabel.Create(TAECHANG_UI_PRICE_COVER_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceCoverEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_EDIT);

	m_wndPriceAddBtn.Create(TAECHANG_UI_PRICE_ADD_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_ADD_BTN);
	m_wndPriceAddBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndPriceModifyBtn.Create(TAECHANG_UI_PRICE_SAVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_MODIFY_BTN);
	m_wndPriceModifyBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndPriceDeleteBtn.Create(TAECHANG_UI_PRICE_REMOVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_DELETE_BTN);
	m_wndPriceCancelBtn.Create(TAECHANG_UI_PRICE_CANCEL_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_CANCEL_BTN);

	m_wndPriceDetailHeader.Create(TAECHANG_UI_PRICE_DETAIL_HEADER, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceDetailDivider.Create(L"", WS_CHILD | SS_ETCHEDHORZ, r, this);
	SetWindowTheme(m_wndPriceSingleCheck.GetSafeHwnd(), L"", L"");
	SetWindowTheme(m_wndPriceNoMaxCheck.GetSafeHwnd(), L"", L"");
	m_wndPriceSummaryTitle.Create(TAECHANG_UI_PRICE_SUMMARY_GUIDE, WS_CHILD | SS_LEFT, r, this);
	m_wndPriceSummaryCount.Create(L"", WS_CHILD | SS_LEFT, r, this);
	m_wndPriceSummaryRange.Create(L"", WS_CHILD | SS_LEFT, r, this);

	m_wndPriceCompanyCombo.LimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
	m_wndPriceMinCopiesEdit.SetLimitText(7);
	m_wndPriceMaxCopiesEdit.SetLimitText(7);
	m_wndPricePrintEdit.SetLimitText(10);
	m_wndPriceCoverEdit.SetLimitText(10);
}

// ── 부수 계산 패널 생성 ───────────────────────────────────────────────────────

void CSageTaechangView::CreatePriceCalcPanel() {
	CRect r(0, 0, 0, 0);
	m_wndCalcCompanyLabel.Create(TAECHANG_UI_CALC_COMPANY_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCompanyCombo.Create(WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_CALC_COMPANY_COMBO);
	m_wndCalcCompanyPickBtn.Create(L"…", WS_CHILD | BS_OWNERDRAW, r, this, ID_CALC_COMPANY_PICK_BTN);
	m_wndCalcCopiesLabel.Create(TAECHANG_UI_CALC_COPIES_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_COPIES_EDIT);
	m_wndCalcPagesLabel.Create(TAECHANG_UI_CALC_PAGES_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcPagesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_PAGES_EDIT);
	m_wndCalcBtn.Create(L"", WS_CHILD | BS_OWNERDRAW, r, this, ID_CALC_BTN);
	m_wndCalcBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCalcBtn.SetIcon(SAGE_BUTTON_ICON_CALCULATE);
	m_wndCalcResetBtn.Create(L"", WS_CHILD | BS_OWNERDRAW, r, this, ID_CALC_RESET_BTN);
	m_wndCalcResetBtn.SetIcon(SAGE_BUTTON_ICON_RESET);

	m_wndCalcPrintLabel.Create(TAECHANG_UI_CALC_PRINT_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcPrintValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCoverLabel.Create(TAECHANG_UI_CALC_COVER_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCoverValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcSubtotalLabel.Create(TAECHANG_UI_CALC_SUBTOTAL_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcSubtotalValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcFreightLabel.Create(TAECHANG_UI_CALC_FREIGHT_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcFreightEdit.Create(WS_CHILD | ES_MULTILINE | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_FREIGHT_EDIT);
	m_wndCalcFreightUnitLabel.Create(L"원", WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcDivider.Create(L"", WS_CHILD | SS_ETCHEDHORZ, r, this);
	m_wndCalcTotalDivider.Create(L"", WS_CHILD | SS_ETCHEDHORZ, r, this);
	m_wndCalcTotalLabel.Create(TAECHANG_UI_CALC_TOTAL_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcTotalValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);

	m_wndCalcHistorySection.Create(TAECHANG_UI_CALC_SECTION_HISTORY, WS_CHILD | SS_OWNERDRAW, r, this, ID_CALC_HISTORY_SECTION);
	m_wndCalcHistoryList.Create(WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, r, this, ID_CALC_HISTORY_LIST);
	m_wndCalcHistoryList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	{
		CHeaderCtrl* pHeader = m_wndCalcHistoryList.GetHeaderCtrl();
		if (pHeader && pHeader->GetSafeHwnd()) {
			m_wndCalcHistoryHeader.SubclassWindow(pHeader->GetSafeHwnd());
			SetWindowTheme(m_wndCalcHistoryHeader.GetSafeHwnd(), L"", L"");
		}
	}
	m_wndCalcHistoryList.InsertColumn(0, TAECHANG_UI_CALC_HIST_COL_COMPANY, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COMPANY_W);
	m_wndCalcHistoryList.InsertColumn(1, TAECHANG_UI_CALC_HIST_COL_ITEM, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_ITEM_W);
	m_wndCalcHistoryList.InsertColumn(2, TAECHANG_UI_CALC_HIST_COL_DATE, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_DATE_W);
	m_wndCalcHistoryList.InsertColumn(3, TAECHANG_UI_CALC_HIST_COL_COPIES, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndCalcHistoryList.InsertColumn(4, TAECHANG_UI_CALC_HIST_COL_PAGES, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_PAGES_W);
	m_wndCalcHistoryList.InsertColumn(5, TAECHANG_UI_CALC_HIST_COL_PRINT, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndCalcHistoryList.InsertColumn(6, TAECHANG_UI_CALC_HIST_COL_COVER, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndCalcHistoryList.InsertColumn(7, TAECHANG_UI_CALC_HIST_COL_FREIGHT, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndCalcHistoryList.InsertColumn(8, TAECHANG_UI_CALC_HIST_COL_TOTAL, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndCalcHistoryList.InsertColumn(9, TAECHANG_UI_CALC_HIST_COL_TIME, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TIME_W);

	m_wndCalcCopiesEdit.SetLimitText(7);
	m_wndCalcPagesEdit.SetLimitText(7);
	m_wndCalcFreightEdit.SetLimitText(10);
	m_wndCalcCompanyCombo.LimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
	if (CHeaderCtrl* pHeader = m_wndCalcHistoryList.GetHeaderCtrl()) {
		for (int i = 0; i < pHeader->GetItemCount(); ++i) {
			HDITEM hdi = {};
			hdi.mask = HDI_FORMAT;
			pHeader->GetItem(i, &hdi);
			hdi.fmt = (hdi.fmt & ~HDF_JUSTIFYMASK) | HDF_CENTER;
			pHeader->SetItem(i, &hdi);
		}
	}
}

// ── 가격 데이터 관리 패널 레이아웃 ───────────────────────────────────────────

void CSageTaechangView::LayoutPriceManagePanel(int nLeft, int nTop, int nWidth, int nHeight) {
	int nLabelW = TAECHANG_PRICE_FORM_LABEL_WIDTH;
	int nCardW = TAECHANG_PRICE_SUMMARY_CARD_WIDTH;
	int nCardGap = TAECHANG_PRICE_SUMMARY_CARD_GAP;
	int nCardPad = TAECHANG_PRICE_SUMMARY_CARD_PADDING;

	// 상하좌우 내부 여백
	int nInnerLeft = nLeft + TAECHANG_MARGIN;
	int nInnerW = nWidth - TAECHANG_MARGIN * 2;

	int nLeftW = nInnerW - nCardW - nCardGap;
	int nRightX = nInnerLeft + nLeftW + nCardGap;

	int nY = nTop + TAECHANG_MARGIN;

	// 법인명 행 ([법인명] [콤보] [법인추가] [단가추가] [법인수정] [표지수정])
	int nActionButtonCount = 4;
	int nCompanyComboW = min(TAECHANG_PRICE_COMPANY_COMBO_WIDTH,
							 nLeftW - nLabelW - TAECHANG_LABEL_EDIT_GAP - TAECHANG_BUTTON_WIDTH * nActionButtonCount - TAECHANG_ROW_GAP * nActionButtonCount);
	if (nCompanyComboW < 180)
		nCompanyComboW = 180;
	m_wndPriceCompanyLabel.MoveWindow(nInnerLeft - 4, nY + TAECHANG_LABEL_VERT_OFFSET - 2, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndPriceCompanyCombo.MoveWindow(nInnerLeft + nLabelW + TAECHANG_LABEL_EDIT_GAP, nY, nCompanyComboW, TAECHANG_EDIT_HEIGHT * 8);
	int nBtnX = nInnerLeft + nLabelW + TAECHANG_LABEL_EDIT_GAP + nCompanyComboW + TAECHANG_ROW_GAP;
	m_wndPriceAddCompanyBtn.MoveWindow(nBtnX, nY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndPriceAddBtn.MoveWindow(nBtnX, nY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndPriceRenameCompanyBtn.MoveWindow(nBtnX, nY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nBtnX += TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP;
	m_wndPriceDeleteCompanyBtn.MoveWindow(nBtnX, nY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
	nY += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

	// 단가 테이블 (하단 여백 적용)
	int nListH = nHeight - (nY - nTop) - TAECHANG_MARGIN;
	if (nListH < TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_EDIT_HEIGHT * 4)
		nListH = TAECHANG_RESULT_HEADER_HEIGHT + TAECHANG_EDIT_HEIGHT * 4;

	m_wndPriceCopiesList.MoveWindow(nInnerLeft, nY, nLeftW, nListH);
	int nColMinMax = 80;
	int nColPrice = (nLeftW - nColMinMax * 2) / 2;
	if (nColPrice > 220)
		nColPrice = 220;
	m_wndPriceCopiesList.SetColumnWidth(0, nColMinMax);
	m_wndPriceCopiesList.SetColumnWidth(1, nColMinMax);
	m_wndPriceCopiesList.SetColumnWidth(2, nColPrice);
	m_wndPriceCopiesList.SetColumnWidth(3, nLeftW - nColMinMax * 2 - nColPrice);

	// 우측 패널 (상하 여백 적용)
	int nCardTop = nY;
	int nCardInnerX = nRightX + nCardPad;
	int nCardInnerW = nCardW - nCardPad * 2;

	int nPanelY = nCardTop + nCardPad;

	// 헤더 + 구분선 (요약/편집 공통)
	m_wndPriceDetailHeader.MoveWindow(nCardInnerX, nPanelY, nCardInnerW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	int nContentY = nPanelY + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndPriceDetailDivider.MoveWindow(nCardInnerX, nContentY, nCardInnerW, 2);
	nContentY += 2 + TAECHANG_ROW_GAP;

	// 요약 컨트롤
	int nSummaryY = nContentY;
	m_wndPriceSummaryTitle.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP * 2;
	m_wndPriceSummaryCount.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_ROW_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP;
	m_wndPriceSummaryRange.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);

	// 편집 폼
	int nFormY = nContentY;
	int nCheckW = 70;
	int nFormLabelW = TAECHANG_PRICE_FORM_LABEL_WIDTH;
	int nInlineEditW = nCardInnerW - nFormLabelW - TAECHANG_LABEL_EDIT_GAP;
	int nEditX = nCardInnerX + nFormLabelW + TAECHANG_LABEL_EDIT_GAP;
	auto ApplyPriceEditTextRect = [](CEdit& edit) {
		CRect rc;
		edit.GetClientRect(&rc);
		rc.left += 2;
		rc.top += 4;
		rc.right -= 2;
		rc.bottom -= 2;
		edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
	};

	m_wndPriceMinCopiesLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndPriceMinCopiesEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPriceMinCopiesEdit);
	m_wndPriceSingleCheck.MoveWindow(nEditX, nFormY + TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP, nInlineEditW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPriceMaxCopiesLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndPriceMaxCopiesEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPriceMaxCopiesEdit);
	m_wndPriceNoMaxCheck.MoveWindow(nEditX, nFormY + TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP, nInlineEditW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP + TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPricePrintLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndPricePrintEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPricePrintEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPriceCoverLabel.MoveWindow(nCardInnerX, nFormY, nFormLabelW, TAECHANG_PRICE_EDIT_HEIGHT);
	m_wndPriceCoverEdit.MoveWindow(nEditX, nFormY, nInlineEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPriceCoverEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT;
	m_rectPriceSummaryCard = CRect(nRightX, nCardTop, nRightX + nCardW, nFormY + nCardPad);

	// 편집 버튼 ([저장][취소] 나란히, [삭제] 아래)
	int nHalfBtnW = (nCardInnerW - TAECHANG_ACTION_GAP) / 2;
	int nButtonY = m_rectPriceSummaryCard.bottom + TAECHANG_ROW_GAP;
	m_wndPriceModifyBtn.MoveWindow(nCardInnerX, nButtonY, nHalfBtnW, TAECHANG_BUTTON_HEIGHT);
	m_wndPriceCancelBtn.MoveWindow(nCardInnerX + nHalfBtnW + TAECHANG_ACTION_GAP, nButtonY, nHalfBtnW, TAECHANG_BUTTON_HEIGHT);
	nButtonY += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;
	m_wndPriceDeleteBtn.MoveWindow(nCardInnerX, nButtonY, nCardInnerW, TAECHANG_BUTTON_HEIGHT);
}

// ── 부수 계산 패널 레이아웃 ───────────────────────────────────────────────────

void CSageTaechangView::LayoutPriceCalcPanel(int nLeft, int nTop, int nWidth, int nHeight) {
	int nPad = TAECHANG_CALC_PANEL_PADDING;
	int nLabelW = TAECHANG_CALC_RESULT_LABEL_WIDTH;
	int nValW = TAECHANG_CALC_RESULT_VALUE_WIDTH;
	int nInputLabelW = 46;
	int nInputEditW = TAECHANG_CALC_COPIES_EDIT_SHORT_W;
	int nX = nLeft + TAECHANG_MARGIN;
	int nY = nTop + TAECHANG_MARGIN;
	int nW = nWidth - TAECHANG_MARGIN * 2;
	int nInputContentW = nInputLabelW + TAECHANG_LABEL_EDIT_GAP + nInputEditW
		+ TAECHANG_ROW_GAP + nInputLabelW + TAECHANG_LABEL_EDIT_GAP + nInputEditW;
	int nInputPanelW = nInputContentW + nPad * 2;
	if (nInputPanelW > nW)
		nInputPanelW = nW;
	auto ApplyCalcEditTextRect = [](CEdit& edit) {
		CRect rc;
		edit.GetClientRect(&rc);
		rc.left += 2;
		rc.top += 4;
		rc.right -= 2;
		rc.bottom -= 2;
		edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
	};

	// ── 입력 패널 ────────────────────────────────────────────────────────────
	int nInputPanelH = nPad + TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP + TAECHANG_EDIT_HEIGHT + nPad;
	m_rectCalcInputPanel = CRect(nX, nY, nX + nInputPanelW, nY + nInputPanelH);

	int nCX = nX + nPad;
	int nCY = nY + nPad;
	int nPickBtnGap = TAECHANG_LABEL_EDIT_GAP;
	int nPickBtnW = TAECHANG_CALC_COMPANY_PICK_BTN_W;
	int nComboW = min(TAECHANG_CALC_COMBO_WIDTH,
		nInputContentW - nInputLabelW - TAECHANG_LABEL_EDIT_GAP - nPickBtnW - nPickBtnGap);

	m_wndCalcCompanyLabel.MoveWindow(nCX, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nComboX = nCX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCalcCompanyCombo.MoveWindow(nComboX, nCY, nComboW, TAECHANG_EDIT_HEIGHT * 8);
	m_wndCalcCompanyPickBtn.MoveWindow(nComboX + nComboW + nPickBtnGap, nCY - TAECHANG_BUTTON_VERT_ADJUST, nPickBtnW, TAECHANG_BUTTON_HEIGHT);
	COMBOBOXINFO cbiCalcCompany = {};
	cbiCalcCompany.cbSize = sizeof(COMBOBOXINFO);
	if (m_wndCalcCompanyCombo.GetComboBoxInfo(&cbiCalcCompany) && ::IsWindow(cbiCalcCompany.hwndItem)) {
		CRect rcComboEdit;
		::GetClientRect(cbiCalcCompany.hwndItem, &rcComboEdit);
		rcComboEdit.left += 2;
		rcComboEdit.top += 4;
		rcComboEdit.right -= 2;
		rcComboEdit.bottom -= 2;
		::SendMessage(cbiCalcCompany.hwndItem, EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rcComboEdit));
	}
	nCY += TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndCalcCopiesLabel.MoveWindow(nCX - 6, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nCopiesEditX = nCX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCalcCopiesEdit.MoveWindow(nCopiesEditX, nCY, nInputEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCalcCopiesEdit);
	int nPagesLabelX = nCopiesEditX + nInputEditW + TAECHANG_ROW_GAP;
	m_wndCalcPagesLabel.MoveWindow(nPagesLabelX - 6, nCY, nInputLabelW, TAECHANG_EDIT_HEIGHT);
	int nPagesEditX = nPagesLabelX + nInputLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCalcPagesEdit.MoveWindow(nPagesEditX, nCY, nInputEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCalcPagesEdit);
	int nIconBtnW = 30;
	int nIconBtnH = 38;
	int nIconBtnGap = 6;
	int nIconBtnTopPad = (nInputPanelH - nIconBtnH * 2 - nIconBtnGap) / 2;
	int nBtnX = m_rectCalcInputPanel.right + TAECHANG_ROW_GAP;
	if (nBtnX + nIconBtnW > nX + nW)
		nBtnX = nX + nW - nIconBtnW;
	m_wndCalcBtn.MoveWindow(nBtnX, nY + nIconBtnTopPad, nIconBtnW, nIconBtnH);
	m_wndCalcResetBtn.MoveWindow(nBtnX, nY + nIconBtnTopPad + nIconBtnH + nIconBtnGap, nIconBtnW, nIconBtnH);

	nY += nInputPanelH + TAECHANG_CALC_SECTION_GAP;

	// ── 결과 패널 ────────────────────────────────────────────────────────────
	int nRowH = TAECHANG_EDIT_HEIGHT + TAECHANG_CALC_RESULT_ROW_GAP;
	int nDivH = 2;
	int nResultPanelH = nPad + nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
		+ nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
		+ TAECHANG_EDIT_HEIGHT + nPad;

	int nRX = nX + nPad;
	int nRY = nY + nPad;
	int nValX = nRX + nLabelW + TAECHANG_LABEL_EDIT_GAP;
	int nResultPanelW = nInputPanelW;
	if (nResultPanelW > nW)
		nResultPanelW = nW;
	m_rectCalcResultPanel = CRect(nX, nY, nX + nResultPanelW, nY + nResultPanelH);
	int nContentW = nResultPanelW - nPad * 2;
	if (nContentW < nLabelW + TAECHANG_LABEL_EDIT_GAP + nValW)
		nContentW = nLabelW + TAECHANG_LABEL_EDIT_GAP + nValW;
	int nFreightUnitW = 14;
	int nFreightEditW = nValW - nFreightUnitW - TAECHANG_LABEL_EDIT_GAP;
	if (nFreightEditW < 100)
		nFreightEditW = 100;
	int nUnitRightX = nValX + nValW;

	m_wndCalcPrintLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcPrintValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndCalcCoverLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcCoverValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndCalcDivider.MoveWindow(nRX, nRY, nContentW, nDivH);
	nRY += nDivH + TAECHANG_CALC_RESULT_ROW_GAP;

	m_wndCalcSubtotalLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcSubtotalValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndCalcFreightLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcFreightUnitLabel.MoveWindow(nUnitRightX - nFreightUnitW, nRY, nFreightUnitW, TAECHANG_EDIT_HEIGHT);
	int nFreightEditGap = 2;
	m_wndCalcFreightEdit.MoveWindow(nUnitRightX - nFreightUnitW - nFreightEditGap - nFreightEditW,
									nRY, nFreightEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCalcFreightEdit);
	nRY += nRowH;

	m_wndCalcTotalDivider.MoveWindow(nRX, nRY, nContentW, nDivH);
	nRY += nDivH + TAECHANG_CALC_RESULT_ROW_GAP;

	m_wndCalcTotalLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcTotalValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);

	nY += nResultPanelH + TAECHANG_CALC_SECTION_GAP;

	// ── 이력 섹션 ────────────────────────────────────────────────────────────
	int nHistoryW = TAECHANG_CALC_HIST_COL_COMPANY_W + TAECHANG_CALC_HIST_COL_ITEM_W
		+ TAECHANG_CALC_HIST_COL_DATE_W
		+ TAECHANG_CALC_HIST_COL_COPIES_W + TAECHANG_CALC_HIST_COL_PAGES_W
		+ TAECHANG_CALC_HIST_COL_PRINT_W + TAECHANG_CALC_HIST_COL_COVER_W
		+ TAECHANG_CALC_HIST_COL_FREIGHT_W + TAECHANG_CALC_HIST_COL_TOTAL_W
		+ TAECHANG_CALC_HIST_COL_TIME_W + ::GetSystemMetrics(SM_CXVSCROLL) + 2;
	if (nHistoryW > nW)
		nHistoryW = nW;
	m_wndCalcHistorySection.MoveWindow(nX, nY, nHistoryW, TAECHANG_SECTION_TITLE_HEIGHT);
	nY += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_PANEL_GAP;

	int nListH = nTop + nHeight - TAECHANG_MARGIN - nY;
	if (nListH < TAECHANG_RESULT_MIN_HEIGHT)
		nListH = TAECHANG_RESULT_MIN_HEIGHT;
	m_wndCalcHistoryList.MoveWindow(nX, nY, nHistoryW, nListH);
	m_wndCalcHistoryList.SetColumnWidth(0, TAECHANG_CALC_HIST_COL_COMPANY_W);
	m_wndCalcHistoryList.SetColumnWidth(1, TAECHANG_CALC_HIST_COL_ITEM_W);
	m_wndCalcHistoryList.SetColumnWidth(2, TAECHANG_CALC_HIST_COL_DATE_W);
	m_wndCalcHistoryList.SetColumnWidth(3, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndCalcHistoryList.SetColumnWidth(4, TAECHANG_CALC_HIST_COL_PAGES_W);
	m_wndCalcHistoryList.SetColumnWidth(5, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndCalcHistoryList.SetColumnWidth(6, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndCalcHistoryList.SetColumnWidth(7, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndCalcHistoryList.SetColumnWidth(8, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndCalcHistoryList.SetColumnWidth(9, TAECHANG_CALC_HIST_COL_TIME_W);
	int nHistoryCount = static_cast<int>(m_arrCalcHistory.GetSize());
	TrimCalcHistoryToVisibleCapacity();
	if (m_arrCalcHistory.GetSize() != nHistoryCount)
		RefreshCalcHistoryList();
}

// ── show/hide 헬퍼 ────────────────────────────────────────────────────────────

void CSageTaechangView::ShowPriceManagePanel(BOOL bShow) {
	int nCmd = bShow ? SW_SHOW : SW_HIDE;
	m_wndPriceCompanyLabel.ShowWindow(nCmd);
	m_wndPriceCompanyCombo.ShowWindow(nCmd);
	m_wndPriceAddCompanyBtn.ShowWindow(nCmd);
	m_wndPriceRenameCompanyBtn.ShowWindow(nCmd);
	m_wndPriceDeleteCompanyBtn.ShowWindow(nCmd);
	m_wndPriceCopiesList.ShowWindow(nCmd);
	m_wndPriceMinCopiesLabel.ShowWindow(nCmd);
	m_wndPriceMinCopiesEdit.ShowWindow(nCmd);
	m_wndPriceSingleCheck.ShowWindow(nCmd);
	m_wndPriceMaxCopiesLabel.ShowWindow(nCmd);
	m_wndPriceMaxCopiesEdit.ShowWindow(nCmd);
	m_wndPriceNoMaxCheck.ShowWindow(nCmd);
	m_wndPricePrintLabel.ShowWindow(nCmd);
	m_wndPricePrintEdit.ShowWindow(nCmd);
	m_wndPriceCoverLabel.ShowWindow(nCmd);
	m_wndPriceCoverEdit.ShowWindow(nCmd);
	m_wndPriceAddBtn.ShowWindow(nCmd);
	m_wndPriceModifyBtn.ShowWindow(nCmd);
	m_wndPriceDeleteBtn.ShowWindow(nCmd);
	m_wndPriceCancelBtn.ShowWindow(nCmd);
	m_wndPriceDetailHeader.ShowWindow(nCmd);
	m_wndPriceDetailDivider.ShowWindow(nCmd);
	m_wndPriceSummaryTitle.ShowWindow(nCmd);
	m_wndPriceSummaryCount.ShowWindow(nCmd);
	m_wndPriceSummaryRange.ShowWindow(nCmd);
	if (!bShow) {
		m_rectPriceSummaryCard.SetRectEmpty();
		m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
		Invalidate(TRUE);
	}
	if (bShow)
		ApplyPriceRightPanel();
}

void CSageTaechangView::ApplyPriceRightPanel() {
	BOOL bSummary = (m_nPricePanelState == TAECHANG_PRICE_PANEL_SUMMARY);
	BOOL bEditModify = (m_nPricePanelState == TAECHANG_PRICE_PANEL_EDIT_MODIFY);
	int nSummaryCmd = bSummary ? SW_SHOW : SW_HIDE;
	int nEditCmd = bSummary ? SW_HIDE : SW_SHOW;

	m_wndPriceSummaryTitle.ShowWindow(nSummaryCmd);
	m_wndPriceSummaryCount.ShowWindow(nSummaryCmd);
	m_wndPriceSummaryRange.ShowWindow(nSummaryCmd);

	m_wndPriceMinCopiesLabel.ShowWindow(nEditCmd);
	m_wndPriceMinCopiesEdit.ShowWindow(nEditCmd);
	m_wndPriceSingleCheck.ShowWindow(nEditCmd);
	m_wndPriceMaxCopiesLabel.ShowWindow(nEditCmd);
	m_wndPriceMaxCopiesEdit.ShowWindow(nEditCmd);
	m_wndPriceNoMaxCheck.ShowWindow(nEditCmd);
	m_wndPricePrintLabel.ShowWindow(nEditCmd);
	m_wndPricePrintEdit.ShowWindow(nEditCmd);
	m_wndPriceCoverLabel.ShowWindow(nEditCmd);
	m_wndPriceCoverEdit.ShowWindow(nEditCmd);
	m_wndPriceModifyBtn.ShowWindow(nEditCmd);
	m_wndPriceCancelBtn.ShowWindow(nEditCmd);
	m_wndPriceDeleteBtn.ShowWindow(bEditModify ? SW_SHOW : SW_HIDE);

	Invalidate(FALSE);
}

void CSageTaechangView::ShowPriceCalcPanel(BOOL bShow) {
	int nCmd = bShow ? SW_SHOW : SW_HIDE;
	m_wndCalcCompanyLabel.ShowWindow(nCmd);
	m_wndCalcCompanyCombo.ShowWindow(nCmd);
	m_wndCalcCompanyPickBtn.ShowWindow(nCmd);
	m_wndCalcCopiesLabel.ShowWindow(nCmd);
	m_wndCalcCopiesEdit.ShowWindow(nCmd);
	m_wndCalcPagesLabel.ShowWindow(nCmd);
	m_wndCalcPagesEdit.ShowWindow(nCmd);
	m_wndCalcBtn.ShowWindow(nCmd);
	m_wndCalcResetBtn.ShowWindow(nCmd);
	m_wndCalcPrintLabel.ShowWindow(nCmd);
	m_wndCalcPrintValue.ShowWindow(nCmd);
	m_wndCalcCoverLabel.ShowWindow(nCmd);
	m_wndCalcCoverValue.ShowWindow(nCmd);
	m_wndCalcSubtotalLabel.ShowWindow(nCmd);
	m_wndCalcSubtotalValue.ShowWindow(nCmd);
	m_wndCalcFreightLabel.ShowWindow(nCmd);
	m_wndCalcFreightEdit.ShowWindow(nCmd);
	m_wndCalcFreightUnitLabel.ShowWindow(nCmd);
	m_wndCalcDivider.ShowWindow(nCmd);
	m_wndCalcTotalDivider.ShowWindow(nCmd);
	m_wndCalcTotalLabel.ShowWindow(nCmd);
	m_wndCalcTotalValue.ShowWindow(nCmd);
	m_wndCalcHistorySection.ShowWindow(nCmd);
	m_wndCalcHistoryList.ShowWindow(nCmd);
	if (!bShow) {
		m_rectCalcInputPanel.SetRectEmpty();
		m_rectCalcResultPanel.SetRectEmpty();
		Invalidate(TRUE);
	}
}

// ── 가격 관리 데이터 헬퍼 ────────────────────────────────────────────────────

void CSageTaechangView::RefreshPriceCompanyList(const CString& strFilter) {
	m_wndPriceCompanyCombo.ResetContent();
	m_wndPriceCopiesList.DeleteAllItems();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE)
		return;
	CString strTarget = strFilter;
	strTarget.Trim();
	CString strNeedle = strTarget;
	strNeedle.MakeLower();
	int nExactIndex = TAECHANG_LIST_NO_ITEM;
	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strName = arrNames[i];
		CString strHaystack = strName;
		strHaystack.MakeLower();
		int nIndex = m_wndPriceCompanyCombo.AddString(strName);
		if (!strNeedle.IsEmpty() && strHaystack == strNeedle)
			nExactIndex = nIndex;
	}
	if (nExactIndex != TAECHANG_LIST_NO_ITEM) {
		m_wndPriceCompanyCombo.SetCurSel(nExactIndex);
		RefreshPriceCopiesList(strTarget);
		return;
	}
	if (!strTarget.IsEmpty()) {
		m_wndPriceCompanyCombo.SetWindowTextW(strTarget);
		UpdatePriceSummaryCard();
		return;
	}
	UpdatePriceSummaryCard();
}

void CSageTaechangView::RefreshPriceCopiesList(const CString& strCompanyName) {
	m_wndPriceCopiesList.DeleteAllItems();
	if (strCompanyName.IsEmpty()) {
		UpdatePriceSummaryCard();
		return;
	}

	CArray<TaechangPriceDto, TaechangPriceDto&> arrPrice;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadByCompany(strCompanyName, arrPrice, strError) == FALSE) {
		UpdatePriceSummaryCard();
		return;
	}

	m_wndPriceCopiesList.SetRedraw(FALSE);
	for (int i = 0; i < arrPrice.GetSize(); ++i) {
		const TaechangPriceDto& dto = arrPrice[i];
		CString strMin;
		strMin.Format(L"%d", dto.nMinCopies);
		int nIndex = m_wndPriceCopiesList.InsertItem(i, strMin);
		m_wndPriceCopiesList.SetItemData(nIndex, static_cast<DWORD_PTR>(dto.nPriceId));

		CString strMax;
		if (dto.bHasMaxCopies)
			strMax.Format(L"%d", dto.nMaxCopies);
		else
			strMax = TAECHANG_UI_PRICE_MAX_COPIES_NONE;
		m_wndPriceCopiesList.SetItemText(nIndex, 1, strMax);

		CString strPrint, strCover;
		strPrint = FormatPrice(dto.nPrintPrice);
		strCover = FormatPrice(dto.nCoverPrice);
		m_wndPriceCopiesList.SetItemText(nIndex, 2, strPrint);
		m_wndPriceCopiesList.SetItemText(nIndex, 3, strCover);
	}
	UpdatePriceSummaryCard();
	m_wndPriceCopiesList.SetRedraw(TRUE);
	m_wndPriceCopiesList.Invalidate();
}

void CSageTaechangView::UpdatePriceSummaryCard() {
	if (!::IsWindow(m_wndPriceSummaryTitle.GetSafeHwnd()))
		return;
	m_wndPriceSummaryTitle.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_GUIDE);
	m_wndPriceSummaryCount.SetWindowTextW(L"");
	m_wndPriceSummaryRange.SetWindowTextW(L"");
}

CString CSageTaechangView::GetSelectedCompanyName() const {
	CString strCompany;
	m_wndPriceCompanyCombo.GetWindowTextW(strCompany);
	strCompany.Trim();
	return strCompany;
}

void CSageTaechangView::LoadSelectedCopiesRowToForm() {
	POSITION pos = m_wndPriceCopiesList.GetFirstSelectedItemPosition();
	if (pos == NULL)
		return;
	int nItem = m_wndPriceCopiesList.GetNextSelectedItem(pos);

	CString strMin = m_wndPriceCopiesList.GetItemText(nItem, 0);
	CString strMax = m_wndPriceCopiesList.GetItemText(nItem, 1);
	CString strPrint = m_wndPriceCopiesList.GetItemText(nItem, 2);
	CString strCover = m_wndPriceCopiesList.GetItemText(nItem, 3);

	BOOL bNoMax = (strMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE);
	BOOL bSingle = (!bNoMax && strMin == strMax);
	m_wndPriceMinCopiesEdit.SetWindowTextW(strMin);
	m_wndPriceSingleCheck.SetCheck(bSingle ? BST_CHECKED : BST_UNCHECKED);
	m_wndPriceNoMaxCheck.SetCheck(bNoMax ? BST_CHECKED : BST_UNCHECKED);
	m_wndPriceMaxCopiesEdit.SetWindowTextW((bNoMax || bSingle) ? CString() : strMax);
	m_wndPriceMaxCopiesEdit.EnableWindow(!bNoMax && !bSingle);
	m_wndPricePrintEdit.SetWindowTextW(strPrint);
	m_wndPriceCoverEdit.SetWindowTextW(strCover);
}

void CSageTaechangView::ClearPriceForm() {
	m_wndPriceMinCopiesEdit.SetWindowTextW(L"");
	m_wndPriceSingleCheck.SetCheck(BST_UNCHECKED);
	m_wndPriceMaxCopiesEdit.SetWindowTextW(L"");
	m_wndPriceMaxCopiesEdit.EnableWindow(TRUE);
	m_wndPriceNoMaxCheck.SetCheck(BST_UNCHECKED);
	m_wndPricePrintEdit.SetWindowTextW(L"");
	m_wndPriceCoverEdit.SetWindowTextW(L"");
}

void CSageTaechangView::FormatPriceEditText(CEdit& edit, BOOL& bFormatting) {
	if (bFormatting)
		return;

	CString strText;
	edit.GetWindowTextW(strText);
	CString strDigits = RemovePriceSeparators(strText);
	if (strDigits.IsEmpty())
		return;

	for (int i = 0; i < strDigits.GetLength(); ++i) {
		wchar_t ch = strDigits[i];
		if (ch < L'0' || ch > L'9')
			return;
	}

	CString strFormatted = FormatPrice(_wtoi(strDigits));
	if (strFormatted == strText)
		return;

	bFormatting = TRUE;
	edit.SetWindowTextW(strFormatted);
	edit.SetSel(strFormatted.GetLength(), strFormatted.GetLength());
	bFormatting = FALSE;
}

BOOL CSageTaechangView::ReadPriceFormToDto(TaechangPriceDto& dto, CString& strError) {
	CString strMin, strMax, strPrint, strCover;
	m_wndPriceMinCopiesEdit.GetWindowTextW(strMin);
	m_wndPriceMaxCopiesEdit.GetWindowTextW(strMax);
	m_wndPricePrintEdit.GetWindowTextW(strPrint);
	m_wndPriceCoverEdit.GetWindowTextW(strCover);
	strMin.Trim(); strMax.Trim(); strPrint.Trim(); strCover.Trim();

	dto.nReportType = REPORT_TYPE_AUDIT_REPORT;
	dto.nMinCopies = strMin.IsEmpty() ? 0 : _wtoi(strMin);
	if (dto.nMinCopies < 1 || dto.nMinCopies > TAECHANG_PRICE_COPIES_MAX) {
		strError = TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE;
		return FALSE;
	}

	BOOL bSingle = (m_wndPriceSingleCheck.GetCheck() == BST_CHECKED);
	dto.bHasMaxCopies = (m_wndPriceNoMaxCheck.GetCheck() == BST_CHECKED) ? FALSE : TRUE;
	if (bSingle) {
		dto.nMaxCopies = dto.nMinCopies;
	} else {
		dto.nMaxCopies = (dto.bHasMaxCopies && !strMax.IsEmpty()) ? _wtoi(strMax) : 0;
		if (dto.bHasMaxCopies) {
			if (dto.nMaxCopies < 1 || dto.nMaxCopies > TAECHANG_PRICE_COPIES_MAX) {
				strError = TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE;
				return FALSE;
			}
			if (dto.nMaxCopies < dto.nMinCopies) {
				strError = TAECHANG_UI_PRICE_MAX_LESS_THAN_MIN;
				return FALSE;
			}
		}
	}

	dto.nPrintPrice = PriceTextToInt(strPrint);
	if (dto.nPrintPrice < 0 || dto.nPrintPrice > TAECHANG_PRICE_AMOUNT_MAX) {
		strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
		return FALSE;
	}

	dto.nCoverPrice = PriceTextToInt(strCover);
	if (dto.nCoverPrice < 0 || dto.nCoverPrice > TAECHANG_PRICE_AMOUNT_MAX) {
		strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
		return FALSE;
	}

	return TRUE;
}

// ── 가격 데이터 관리 이벤트 ──────────────────────────────────────────────────

void CSageTaechangView::OnPriceAddCompany() {
	TaechangCompanyDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString strName = dlg.GetCompanyName();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strItem = arrNames[i];
		strItem.Trim();
		if (strItem.CompareNoCase(strName) != 0)
			continue;

		AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_EXISTS, MB_ICONINFORMATION);
		RefreshPriceCompanyList(strItem);
		m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
		ClearPriceForm();
		ApplyPriceRightPanel();
		return;
	}

	// 새 법인명을 목록에 추가 (DB에는 첫 가격 추가 시 반영됨)
	RefreshPriceCompanyList(strName);
	int nIndex = m_wndPriceCompanyCombo.AddString(strName);
	m_wndPriceCompanyCombo.SetCurSel(nIndex);
	m_wndPriceCompanyCombo.SetWindowTextW(strName);
	m_wndPriceCopiesList.DeleteAllItems();
	ClearPriceForm();
	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceRenameCompany() {
	CString strCompany = GetSelectedCompanyName();
	int nIndex = m_wndPriceCompanyCombo.FindStringExact(-1, strCompany);
	if (strCompany.IsEmpty() || nIndex == CB_ERR) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	TaechangCompanyRenameDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	CString strNewName = dlg.GetCompanyName();
	strNewName.Trim();
	if (strNewName.CompareNoCase(strCompany) == 0)
		return;

	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	for (int i = 0; i < arrNames.GetSize(); ++i) {
		CString strItem = arrNames[i];
		strItem.Trim();
		if (strItem.CompareNoCase(strCompany) != 0 && strItem.CompareNoCase(strNewName) == 0) {
			AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_EXISTS, MB_ICONINFORMATION);
			return;
		}
	}

	int nAffectedCount = 0;
	if (sageDBMgr.GetTaechangPriceService()->RenameCompany(strCompany, strNewName, nAffectedCount, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCompanyList(strNewName);
	RefreshCalcCompanyCombo();
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceDeleteCompany() {
	CString strCompany = GetSelectedCompanyName();
	int nIndex = m_wndPriceCompanyCombo.FindStringExact(-1, strCompany);
	if (strCompany.IsEmpty() || nIndex == CB_ERR) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	CString strConfirm;
	strConfirm.Format(TAECHANG_UI_PRICE_DELETE_COMPANY_CONFIRM_FORMAT, strCompany.GetString());
	if (AfxMessageBox(strConfirm, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
		return;
	}

	int nAffectedCount = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->RemoveCompany(
		strCompany,
		nAffectedCount,
		strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCompanyList();
	RefreshCalcCompanyCombo();
	m_wndPriceCopiesList.DeleteAllItems();
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceCompanySelChanged() {
	int nSel = m_wndPriceCompanyCombo.GetCurSel();
	if (nSel == CB_ERR)
		return;
	CString strCompany;
	m_wndPriceCompanyCombo.GetLBText(nSel, strCompany);
	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strCompany);
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceCompanyEditChanged() {
	CString strCompany = GetSelectedCompanyName();
	if (strCompany.IsEmpty())
		return;
	int nIndex = m_wndPriceCompanyCombo.FindString(-1, strCompany);
	if (nIndex == CB_ERR)
		return;
	CString strMatch;
	m_wndPriceCompanyCombo.GetLBText(nIndex, strMatch);
	m_wndPriceCompanyCombo.SetCurSel(nIndex);
	m_wndPriceCompanyCombo.SetEditSel(strCompany.GetLength(), -1);
	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strMatch);
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceCopiesSelChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLISTVIEW* pNM = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
	*pResult = 0;
	if (!(pNM->uChanged & LVIF_STATE) || !(pNM->uNewState & LVIS_SELECTED))
		return;
	m_nPricePanelState = TAECHANG_PRICE_PANEL_EDIT_MODIFY;
	LoadSelectedCopiesRowToForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceNoMaxCheck() {
	BOOL bNoMax = (m_wndPriceNoMaxCheck.GetCheck() == BST_CHECKED);
	if (bNoMax && m_wndPriceSingleCheck.GetCheck() == BST_CHECKED) {
		AfxMessageBox(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, MB_ICONWARNING);
		m_wndPriceNoMaxCheck.SetCheck(BST_UNCHECKED);
		return;
	}
	m_wndPriceMaxCopiesEdit.EnableWindow(!bNoMax);
	if (bNoMax)
		m_wndPriceMaxCopiesEdit.SetWindowTextW(L"");
}

void CSageTaechangView::OnPriceSingleCheck() {
	BOOL bSingle = (m_wndPriceSingleCheck.GetCheck() == BST_CHECKED);
	if (bSingle && m_wndPriceNoMaxCheck.GetCheck() == BST_CHECKED) {
		AfxMessageBox(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, MB_ICONWARNING);
		m_wndPriceSingleCheck.SetCheck(BST_UNCHECKED);
		return;
	}
	m_wndPriceMaxCopiesEdit.EnableWindow(!bSingle);
	if (bSingle)
		m_wndPriceMaxCopiesEdit.SetWindowTextW(L"");
}

void CSageTaechangView::OnPricePrintChanged() {
	FormatPriceEditText(m_wndPricePrintEdit, m_bFormattingPricePrint);
}

void CSageTaechangView::OnPriceCoverChanged() {
	FormatPriceEditText(m_wndPriceCoverEdit, m_bFormattingPriceCover);
}

void CSageTaechangView::OnPriceAdd() {
	int nSel = m_wndPriceCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	CString strCompany;
	m_wndPriceCompanyCombo.GetLBText(nSel, strCompany);
	strCompany.Trim();
	if (strCompany.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	TaechangPriceRangeDlg dlg(this);
	for (int i = 0; i < m_wndPriceCopiesList.GetItemCount(); ++i) {
		int nExistingMin = _wtoi(m_wndPriceCopiesList.GetItemText(i, 0));
		CString strExistingMax = m_wndPriceCopiesList.GetItemText(i, 1);
		BOOL bExistingHasMax = (strExistingMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE) ? FALSE : TRUE;
		int nExistingMax = bExistingHasMax ? _wtoi(strExistingMax) : 0;
		dlg.AddExistingRange(nExistingMin, bExistingHasMax, nExistingMax);
	}
	if (dlg.DoModal() != IDOK)
		return;

	TaechangPriceDto dto;
	dto.strCompanyName = strCompany;
	dto.nReportType = REPORT_TYPE_AUDIT_REPORT;
	dto.nMinCopies = dlg.GetMinCopies();
	dto.bHasMaxCopies = dlg.HasMaxCopies();
	dto.nMaxCopies = dlg.GetMaxCopies();
	dto.nPrintPrice = dlg.GetPrintPrice();
	dto.nCoverPrice = dlg.GetCoverPrice();

	int nNewId = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->AddPrice(dto, nNewId, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCompanyList(strCompany);
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceModify() {
	CString strCompany = GetSelectedCompanyName();
	TaechangPriceDto dto;
	CString strError;
	if (ReadPriceFormToDto(dto, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONWARNING);
		return;
	}
	dto.strCompanyName = strCompany;

	if (m_nPricePanelState == TAECHANG_PRICE_PANEL_EDIT_ADD) {
		int nNewId;
		if (sageDBMgr.GetTaechangPriceService()->AddPrice(dto, nNewId, strError) == FALSE) {
			AfxMessageBox(strError, MB_ICONERROR);
			return;
		}
	} else {
		POSITION pos = m_wndPriceCopiesList.GetFirstSelectedItemPosition();
		if (pos == NULL) {
			AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
			return;
		}
		int nItem = m_wndPriceCopiesList.GetNextSelectedItem(pos);
		dto.nPriceId = static_cast<int>(m_wndPriceCopiesList.GetItemData(nItem));
		if (sageDBMgr.GetTaechangPriceService()->ModifyPriceById(dto, strError) == FALSE) {
			AfxMessageBox(strError, MB_ICONERROR);
			return;
		}
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strCompany);
	ClearPriceForm();
	ApplyPriceRightPanel();
}

void CSageTaechangView::OnPriceDelete() {
	POSITION pos = m_wndPriceCopiesList.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
		return;
	}
	int nItem = m_wndPriceCopiesList.GetNextSelectedItem(pos);
	int nPriceId = static_cast<int>(m_wndPriceCopiesList.GetItemData(nItem));
	if (nPriceId <= 0) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COPIES_ROW, MB_ICONWARNING);
		return;
	}

	if (AfxMessageBox(TAECHANG_UI_PRICE_DELETE_CONFIRM, MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->RemovePrice(nPriceId, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	CString strCompany = GetSelectedCompanyName();
	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strCompany);
	ClearPriceForm();
	ApplyPriceRightPanel();
	if (m_wndPriceCopiesList.GetItemCount() == 0)
		RefreshPriceCompanyList();
}

void CSageTaechangView::OnPriceCancel() {
	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	ClearPriceForm();
	m_wndPriceCopiesList.SetItemState(-1, 0, LVIS_SELECTED);
	ApplyPriceRightPanel();
}

// ── 부수 계산 데이터 헬퍼 ────────────────────────────────────────────────────

void CSageTaechangView::RefreshCalcCompanyCombo() {
	m_wndCalcCompanyCombo.ResetContent();
	CStringArray arrNames;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadAllCompanyNames(arrNames, strError) == FALSE)
		return;
	for (int i = 0; i < arrNames.GetSize(); ++i)
		m_wndCalcCompanyCombo.AddString(arrNames[i]);
}

void CSageTaechangView::ClearCalcInputAndResult() {
	m_wndCalcCopiesEdit.SetWindowTextW(L"");
	m_wndCalcPagesEdit.SetWindowTextW(L"");
	m_wndCalcFreightEdit.SetWindowTextW(L"");
	ClearCalcResult();
}

void CSageTaechangView::ClearCalcResult() {
	m_nCalcPrintPrice = 0;
	m_nCalcCoverPrice = 0;
	m_nCalcUnitPrice = 0;
	m_wndCalcPrintValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndCalcCoverValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndCalcSubtotalValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
	m_wndCalcTotalValue.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
}

BOOL CSageTaechangView::UpdateCalcPreview(BOOL bShowMessage) {
	int nSel = m_wndCalcCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_SELECT_COMPANY, MB_ICONWARNING);
		return FALSE;
	}

	CString strCompany;
	m_wndCalcCompanyCombo.GetLBText(nSel, strCompany);

	CString strCopies;
	m_wndCalcCopiesEdit.GetWindowTextW(strCopies);
	strCopies.Trim();
	if (strCopies.IsEmpty()) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_COPIES_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}

	int nCopies = _wtoi(strCopies);
	if (nCopies < 1) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_COPIES_INVALID, MB_ICONWARNING);
		return FALSE;
	}
	if (nCopies > TAECHANG_PRICE_COPIES_MAX) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE, MB_ICONWARNING);
		return FALSE;
	}

	CString strPages;
	m_wndCalcPagesEdit.GetWindowTextW(strPages);
	strPages.Trim();
	if (strPages.IsEmpty()) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_PAGES_REQUIRED, MB_ICONWARNING);
		return FALSE;
	}

	int nPages = _wtoi(strPages);
	if (nPages < 1 || nPages > TAECHANG_PRICE_COPIES_MAX) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_PAGES_INVALID, MB_ICONWARNING);
		return FALSE;
	}

	TaechangPriceDto dto;
	BOOL bFound;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadByCompanyAndCopies(strCompany, nCopies, dto, bFound, strError) == FALSE) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(strError, MB_ICONERROR);
		return FALSE;
	}
	if (!bFound) {
		ClearCalcResult();
		if (bShowMessage)
			AfxMessageBox(TAECHANG_UI_CALC_NO_DATA, MB_ICONWARNING);
		return FALSE;
	}

	LONGLONG nPrintPrice = static_cast<LONGLONG>(dto.nPrintPrice) * nPages;
	m_nCalcPrintPrice = nPrintPrice;
	m_nCalcCoverPrice = dto.nCoverPrice;
	m_nCalcUnitPrice = dto.nPrintPrice;

	CString strPrint, strCover, strSub;
	strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(nPrintPrice).GetString());
	strCover.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(dto.nCoverPrice).GetString());
	strSub.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(nPrintPrice + dto.nCoverPrice).GetString());
	m_wndCalcPrintValue.SetWindowTextW(strPrint);
	m_wndCalcCoverValue.SetWindowTextW(strCover);
	m_wndCalcSubtotalValue.SetWindowTextW(strSub);
	UpdateCalcTotal();
	return TRUE;
}

void CSageTaechangView::UpdateCalcTotal() {
	CString strFreight;
	m_wndCalcFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();
	int nFreight = PriceTextToInt(strFreight);
	if (nFreight < 0) nFreight = 0;
	if (nFreight > TAECHANG_PRICE_AMOUNT_MAX) nFreight = TAECHANG_PRICE_AMOUNT_MAX;
	LONGLONG nTotal = m_nCalcPrintPrice + m_nCalcCoverPrice + nFreight;
	CString strTotal;
	strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(nTotal).GetString());
	m_wndCalcTotalValue.SetWindowTextW(strTotal);
}

// ── 부수 계산 이벤트 ─────────────────────────────────────────────────────────

void CSageTaechangView::OnCalc() {
	if (UpdateCalcPreview(TRUE) == FALSE)
		return;

	int nSel = m_wndCalcCompanyCombo.GetCurSel();
	CString strCompany;
	m_wndCalcCompanyCombo.GetLBText(nSel, strCompany);

	CString strCopies;
	m_wndCalcCopiesEdit.GetWindowTextW(strCopies);
	strCopies.Trim();
	int nCopies = _wtoi(strCopies);

	CString strPages;
	m_wndCalcPagesEdit.GetWindowTextW(strPages);
	strPages.Trim();
	int nPages = _wtoi(strPages);

	CString strFreight;
	m_wndCalcFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();
	int nFreight = PriceTextToInt(strFreight);
	if (nFreight < 0) nFreight = 0;

	CString strPluginDir;
	if (!GetExecutableDirectory(strPluginDir)) {
		AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}
	CString strTemplatePath = CombinePath(strPluginDir, TAECHANG_ESTIMATE_TEMPLATE_REL_PATH);
	CString strScriptPath   = CombinePath(strPluginDir, TAECHANG_CALC_ESTIMATE_SCRIPT_REL_PATH);

	if (!FileExists(strTemplatePath)) {
		AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_TEMPLATE_MISSING, MB_ICONERROR);
		return;
	}
	if (!FileExists(strScriptPath)) {
		AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_SCRIPT_MISSING, MB_ICONERROR);
		return;
	}

	TaechangCalcEstimateDlg dlg(strCompany, nCopies, nPages,
		m_nCalcUnitPrice, m_nCalcCoverPrice, nFreight,
		strTemplatePath, strScriptPath, this);
	if (dlg.DoModal() == IDOK) {
		AddCalcHistory(strCompany, nCopies, nPages, dlg.GetItemName(), dlg.GetDate(),
			m_nCalcPrintPrice, m_nCalcCoverPrice, nFreight,
			m_nCalcPrintPrice + m_nCalcCoverPrice + nFreight);
	}
}

void CSageTaechangView::OnCalcReset() {
	m_wndCalcCopiesEdit.SetWindowTextW(L"");
	m_wndCalcPagesEdit.SetWindowTextW(L"");
	m_wndCalcFreightEdit.SetWindowTextW(L"");
	ClearCalcResult();
	m_wndCalcCopiesEdit.SetFocus();
}

void CSageTaechangView::OnCalcCompanyChanged() {
	ClearCalcInputAndResult();
}

void CSageTaechangView::OnCalcInputChanged() {
	UpdateCalcPreview(FALSE);
}

void CSageTaechangView::OnCalcFreightChanged() {
	FormatPriceEditText(m_wndCalcFreightEdit, m_bFormattingCalcFreight);
	UpdateCalcTotal();
}

void CSageTaechangView::OnCalcCompanyPick() {
	int nCount = m_wndCalcCompanyCombo.GetCount();
	CStringArray arrNames;
	for (int i = 0; i < nCount; i++) {
		CString strName;
		m_wndCalcCompanyCombo.GetLBText(i, strName);
		arrNames.Add(strName);
	}

	CString strCurrent;
	int nCurSel = m_wndCalcCompanyCombo.GetCurSel();
	if (nCurSel != CB_ERR)
		m_wndCalcCompanyCombo.GetLBText(nCurSel, strCurrent);

	TaechangCalcCompanyPickerDlg dlg(arrNames, strCurrent, this);
	if (dlg.DoModal() == IDOK) {
		CString strSelected = dlg.GetSelectedName();
		int nIdx = m_wndCalcCompanyCombo.FindStringExact(-1, strSelected);
		if (nIdx != CB_ERR) {
			m_wndCalcCompanyCombo.SetCurSel(nIdx);
			ClearCalcInputAndResult();
		}
	}
}

void CSageTaechangView::AddCalcHistory(const CString& strCompany, int nCopies, int nPages, const CString& strItemName, const CString& strDate, LONGLONG nPrintPrice, int nCoverPrice, int nFreight, LONGLONG nTotal) {
	CalcHistoryEntry entry;
	entry.strCompanyName = strCompany;
	entry.strItemName = strItemName;
	entry.strDate = strDate;
	entry.nCopies = nCopies;
	entry.nPages = nPages;
	entry.nPrintPrice = nPrintPrice;
	entry.nCoverPrice = nCoverPrice;
	entry.nFreight = nFreight;
	entry.nTotal = nTotal;
	entry.timeCalc = CTime::GetCurrentTime();

	m_arrCalcHistory.InsertAt(0, entry);
	TrimCalcHistoryToVisibleCapacity();

	RefreshCalcHistoryList();
}

int CSageTaechangView::GetCalcHistoryVisibleCapacity() const {
	if (!::IsWindow(m_wndCalcHistoryList.GetSafeHwnd()))
		return TAECHANG_CALC_MAX_HISTORY;

	int nCapacity = m_wndCalcHistoryList.GetCountPerPage();
	if (nCapacity <= 0)
		return TAECHANG_CALC_MAX_HISTORY;
	return nCapacity;
}

void CSageTaechangView::TrimCalcHistoryToVisibleCapacity() {
	int nCapacity = GetCalcHistoryVisibleCapacity();
	if (nCapacity < 1)
		nCapacity = 1;
	while (m_arrCalcHistory.GetSize() > nCapacity)
		m_arrCalcHistory.RemoveAt(nCapacity);
}

void CSageTaechangView::RefreshCalcHistoryList() {
	m_wndCalcHistoryList.SetRedraw(FALSE);
	m_wndCalcHistoryList.DeleteAllItems();
	for (int i = 0; i < m_arrCalcHistory.GetSize(); ++i) {
		const CalcHistoryEntry& e = m_arrCalcHistory[i];
		m_wndCalcHistoryList.InsertItem(i, e.strCompanyName);
		m_wndCalcHistoryList.SetItemText(i, 1, e.strItemName);
		m_wndCalcHistoryList.SetItemText(i, 2, e.strDate);

		CString strCopies;
		strCopies.Format(TAECHANG_UI_CALC_HIST_COPIES_FMT, e.nCopies);
		m_wndCalcHistoryList.SetItemText(i, 3, strCopies);

		CString strPages;
		strPages.Format(TAECHANG_UI_CALC_HIST_PAGES_FMT, e.nPages);
		m_wndCalcHistoryList.SetItemText(i, 4, strPages);

		CString strPrint, strCover, strFreight, strTotal;
		strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nPrintPrice).GetString());
		strCover.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nCoverPrice).GetString());
		strFreight.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nFreight).GetString());
		strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nTotal).GetString());
		m_wndCalcHistoryList.SetItemText(i, 5, strPrint);
		m_wndCalcHistoryList.SetItemText(i, 6, strCover);
		m_wndCalcHistoryList.SetItemText(i, 7, strFreight);
		m_wndCalcHistoryList.SetItemText(i, 8, strTotal);

		CString strTime = e.timeCalc.Format(TAECHANG_UI_CALC_HIST_TIME_FMT);
		m_wndCalcHistoryList.SetItemText(i, 9, strTime);
	}
	m_wndCalcHistoryList.SetRedraw(TRUE);
	m_wndCalcHistoryList.Invalidate();
}

// ── 법인 순서 데이터 관리 패널 ────────────────────────────────────────────────

void CSageTaechangView::CreateCompanyOrderPanel() {
	CRect r(0, 0, 0, 0);
	m_wndCoCrudSection.Create(TAECHANG_UI_CO_CRUD_SECTION, WS_CHILD | SS_OWNERDRAW, r, this, ID_COORDER_CRUD_SECTION);
	m_wndCoListSection.Create(TAECHANG_UI_CO_LIST_SECTION, WS_CHILD | SS_OWNERDRAW, r, this, ID_COORDER_LIST_SECTION);
	m_wndCoAddBtn.Create(TAECHANG_UI_CO_ADD_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_ADD_BTN);
	m_wndCoAddBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCoModifyBtn.Create(TAECHANG_UI_CO_MODIFY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_MODIFY_BTN);
	m_wndCoDeleteBtn.Create(TAECHANG_UI_CO_DELETE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_DELETE_BTN);
	m_wndCoCancelBtn.Create(TAECHANG_UI_CO_CANCEL_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_CANCEL_BTN);
	m_wndCoSearchLabel.Create(TAECHANG_UI_CO_SEARCH_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoSearchEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_SEARCH_EDIT);
	m_wndCoSearchBtn.Create(L"", WS_CHILD | BS_OWNERDRAW, r, this, ID_COORDER_SEARCH_BTN);
	m_wndCoSearchBtn.SetVariant(SAGE_BUTTON_PRIMARY);
	m_wndCoSearchBtn.SetIcon(SAGE_BUTTON_ICON_SEARCH);
	m_wndCoOrderLabel.Create(TAECHANG_UI_CO_ORDER_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoOrderEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER, r, this, ID_COORDER_ORDER_EDIT);
	m_wndCoOrderEdit.LimitText(6);
	m_wndCoNameLabel.Create(TAECHANG_UI_CO_NAME_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndCoCompanyEdit.Create(WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_COORDER_COMPANY_EDIT);
	m_wndCoCompanyEdit.LimitText(TAECHANG_CO_COMPANY_NAME_MAX);
	m_wndCoList.Create(WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, r, this, ID_COORDER_LIST);
	m_wndCoList.SetAlternateRowColor(TRUE);
	m_wndCoList.SetCenterFirstColumn(TRUE);
	m_wndCoList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES);
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
	m_rectCoCard = CRect(nLeft, nCardTop, nLeft + TAECHANG_CO_LIST_WIDTH, nCardTop + nCardHeight);

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
		m_rectCoCard.SetRectEmpty();
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


