
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "SageTaechangDoc.h"
#include "SageTaechangView.h"
#include "app/application/services/TaechangAppSettingsService.h"
#include "app/application/services/TaechangCompareCsvExportService.h"
#include "app/application/services/TaechangDeliveryExcelService.h"
#include "app/application/services/TaechangEstimateExcelService.h"
#include "app/application/services/TaechangHwpCompareService.h"
#include "app/application/services/TaechangPdfCompareService.h"
#include "app/application/services/TaechangReceivablesExcelService.h"
#include "app/common/TaechangJson.h"
#include "app/common/TaechangDialogHelper.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include "app/presentation/TaechangWorkflowResultPresenter.h"
#include "TaechangAuthSession.h"
#include "TaechangLoginDlg.h"
#include "TaechangCompanyDlg.h"
#include "TaechangPriceRangeDlg.h"
#include "TaechangPriceSimpleDlg.h"
#include "SageDBMgr.h"
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
	CString m_strPdfFilePaths;
	CString m_strHwpFilePaths;
	CString m_strSelectedRowNums;
};

struct TaechangWorkflowResult {
	int m_nWorkflowType;
	int m_nTaskType;
	CString m_strResponseJson;
};

static CString BuildWorkflowPayload(const CString& strInputPath, const CString& strOutputFolder, const CString& strRowNums) {
	CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
	if (!strOutputFolder.IsEmpty())
		strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
	if (!strRowNums.IsEmpty())
		strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ROW_NUMS) + L"\":\"" + JsonEscapeString(strRowNums) + L"\"";
	strPayload += L"}";
	return strPayload;
}

static CString BuildComparePayload(const CString& strJsonKey, const CString& strFilePaths) {
	CString strPayload = L"{\"" + strJsonKey + L"\":[";
	CString strRemaining = strFilePaths;
	int nIndex = 0;
	BOOL bFirst = TRUE;
	while (TRUE) {
		CString strPath = strRemaining.Tokenize(L"\r\n", nIndex);
		if (strPath.IsEmpty())
			break;
		strPath.Trim();
		if (strPath.IsEmpty())
			continue;
		if (!bFirst)
			strPayload += L",";
		strPayload += L"\"" + JsonEscapeString(strPath) + L"\"";
		bFirst = FALSE;
	}
	strPayload += L"]}";
	return strPayload;
}

static CString GetTaskRequestId(const TaechangWorkflowTask* pTask) {
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
		return TAECHANG_REQUEST_PDF_COMPARE;
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
		return TAECHANG_REQUEST_HWP_COMPARE;
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
		CString strPayload;
		if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
			strPayload = BuildComparePayload(L"pdfFilePaths", pTask->m_strPdfFilePaths);
		else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
			strPayload = BuildComparePayload(L"hwpFilePaths", pTask->m_strHwpFilePaths);
		else
			strPayload = BuildWorkflowPayload(pTask->m_strInputPath, pTask->m_strOutputFolder, pTask->m_strSelectedRowNums);
		if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE) {
			TaechangPdfCompareService service;
			pResult->m_strResponseJson = service.BuildRunCompareResponse(TAECHANG_REQUEST_PDF_COMPARE, strPayload);
		} else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) {
			TaechangHwpCompareService service;
			pResult->m_strResponseJson = service.BuildRunCompareResponse(TAECHANG_REQUEST_HWP_COMPARE, strPayload);
		} else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
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

BEGIN_MESSAGE_MAP(CTaechangHeaderCtrl, CHeaderCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangHeaderCtrl::OnPaint() {
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	dc.FillSolidRect(rectClient, TAECHANG_COLOR_LIST_HEADER);

	CFont* pFont = GetFont();
	CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(TAECHANG_COLOR_BUTTON_TEXT);

	int nCount = GetItemCount();
	for (int i = 0; i < nCount; ++i) {
		CRect rcItem;
		GetItemRect(i, &rcItem);

		HDITEM hdItem = {};
		wchar_t szText[256] = {};
		hdItem.mask = HDI_TEXT | HDI_FORMAT;
		hdItem.pszText = szText;
		hdItem.cchTextMax = 255;
		GetItem(i, &hdItem);

		if (i < nCount - 1)
			dc.FillSolidRect(rcItem.right - 1, rcItem.top + 4, 1, rcItem.Height() - 8, TAECHANG_COLOR_LIST_HEADER_DIVIDER);

		UINT uFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
		if (hdItem.fmt & HDF_RIGHT) {
			rcItem.right -= 8;
			uFormat |= DT_RIGHT;
		} else if (hdItem.fmt & HDF_CENTER) {
			uFormat |= DT_CENTER;
		} else {
			rcItem.left += 8;
			uFormat |= DT_LEFT;
		}
		dc.DrawText(szText, rcItem, uFormat);
	}

	if (pOldFont)
		dc.SelectObject(pOldFont);
}

BEGIN_MESSAGE_MAP(CTaechangTabCtrl, CTabCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CTaechangComboBox, CComboBox)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangComboBox::OnPaint() {
	CPaintDC dc(this);
	COMBOBOXINFO cbi = {};
	cbi.cbSize = sizeof(COMBOBOXINFO);
	GetComboBoxInfo(&cbi);
	CRect rcButton = cbi.rcButton;
	if (rcButton.IsRectEmpty())
		return;
	dc.FillSolidRect(rcButton, TAECHANG_COLOR_APP_BACKGROUND);
	int cx = (rcButton.left + rcButton.right) / 2;
	int cy = (rcButton.top + rcButton.bottom) / 2;
	POINT pts[3] = {
		{ cx - 4, cy - 2 },
		{ cx + 4, cy - 2 },
		{ cx,     cy + 3 }
	};
	CBrush br(TAECHANG_COLOR_PRIMARY);
	CPen pen(PS_NULL, 0, RGB(0, 0, 0));
	CBrush* pOldBr = dc.SelectObject(&br);
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.Polygon(pts, 3);
	dc.SelectObject(pOldBr);
	dc.SelectObject(pOldPen);
}

void CTaechangTabCtrl::OnPaint() {
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(rect, TAECHANG_COLOR_APP_BACKGROUND);

	CFont* pFont = GetFont();
	CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
	dc.SetBkMode(TRANSPARENT);

	int nCount = GetItemCount();
	int nCurSel = GetCurSel();
	for (int i = 0; i < nCount; ++i) {
		CRect rcItem;
		GetItemRect(i, &rcItem);
		BOOL bSelected = (i == nCurSel);

		dc.FillSolidRect(rcItem, bSelected ? TAECHANG_COLOR_PANEL : TAECHANG_COLOR_APP_BACKGROUND);

		if (bSelected) {
			CRect rcLine = rcItem;
			rcLine.top = rcLine.bottom - TAECHANG_TAB_INDICATOR_HEIGHT;
			dc.FillSolidRect(rcLine, TAECHANG_COLOR_PRIMARY);
		}

		TCITEM tcItem;
		wchar_t szText[64] = {};
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = szText;
		tcItem.cchTextMax = 63;
		GetItem(i, &tcItem);

		dc.SetTextColor(bSelected ? TAECHANG_COLOR_TEXT : TAECHANG_COLOR_SECONDARY_TEXT);
		dc.DrawText(szText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	if (pOldFont)
		dc.SelectObject(pOldFont);
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
	ON_BN_CLICKED(ID_TAECHANG_EXPORT_CSV, &CSageTaechangView::OnExportCsv)
	ON_BN_CLICKED(ID_TAECHANG_SELECT_ALL, &CSageTaechangView::OnSelectAll)
	ON_BN_CLICKED(ID_TAECHANG_LOGIN_BTN, &CSageTaechangView::OnLogin)
	ON_BN_CLICKED(ID_TAECHANG_LOGOUT_BTN, &CSageTaechangView::OnLogout)
	ON_CBN_SELCHANGE(ID_PRICE_COMPANY_EDIT, &CSageTaechangView::OnPriceCompanySelChanged)
	ON_CBN_EDITCHANGE(ID_PRICE_COMPANY_EDIT, &CSageTaechangView::OnPriceCompanyEditChanged)
	ON_BN_CLICKED(ID_PRICE_ADD_COMPANY_BTN, &CSageTaechangView::OnPriceAddCompany)
	ON_BN_CLICKED(ID_PRICE_RENAME_COMPANY_BTN, &CSageTaechangView::OnPriceRenameCompany)
	ON_BN_CLICKED(ID_PRICE_CHANGE_COVER_BTN, &CSageTaechangView::OnPriceChangeCover)
	ON_NOTIFY(LVN_ITEMCHANGED, ID_PRICE_COPIES_LIST, &CSageTaechangView::OnPriceCopiesSelChanged)
	ON_BN_CLICKED(ID_PRICE_NO_MAX_CHECK, &CSageTaechangView::OnPriceNoMaxCheck)
	ON_BN_CLICKED(ID_PRICE_ADD_BTN, &CSageTaechangView::OnPriceAdd)
	ON_BN_CLICKED(ID_PRICE_MODIFY_BTN, &CSageTaechangView::OnPriceModify)
	ON_BN_CLICKED(ID_PRICE_DELETE_BTN, &CSageTaechangView::OnPriceDelete)
	ON_BN_CLICKED(ID_PRICE_CANCEL_BTN, &CSageTaechangView::OnPriceCancel)
	ON_BN_CLICKED(ID_CALC_BTN, &CSageTaechangView::OnCalc)
	ON_EN_CHANGE(ID_CALC_FREIGHT_EDIT, &CSageTaechangView::OnCalcFreightChanged)
	ON_BN_CLICKED(ID_TAECHANG_RECEIVABLES_SEARCH_BTN, &CSageTaechangView::OnReceivablesSearch)
	ON_BN_CLICKED(ID_TAECHANG_RECEIVABLES_RESET_BTN, &CSageTaechangView::OnReceivablesFilterReset)
	ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
	ON_WM_DROPFILES()
	ON_WM_DRAWITEM()
	ON_NOTIFY(NM_CUSTOMDRAW, ID_TAECHANG_SIDEBAR_TREE, &CSageTaechangView::OnSidebarTreeCustomDraw)
	ON_NOTIFY(NM_CUSTOMDRAW, ID_TAECHANG_RESULT_LIST, &CSageTaechangView::OnListCustomDraw)
	ON_NOTIFY(NM_CUSTOMDRAW, ID_PRICE_COPIES_LIST, &CSageTaechangView::OnListCustomDraw)
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
	: m_bRunning(FALSE)
	, m_nProgressPercent(0)
	, m_nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT)
	, m_nLastWorkflowType(0)
	, m_nLastTaskType(0)
	, m_nCurrentWorkflow(TAECHANG_WORKFLOW_RECEIVABLES)
	, m_hLastWorkflowItem(NULL)
	, m_colorHeaderStatus(TAECHANG_COLOR_SECONDARY_TEXT)
	, m_colorHeaderStatusBg(TAECHANG_COLOR_APP_BACKGROUND)
	, m_bLastTaskSuccess(FALSE)
	, m_nCalcPrintPrice(0)
	, m_nCalcCoverPrice(0)
	, m_nPricePanelState(TAECHANG_PRICE_PANEL_SUMMARY)
	, m_rectPriceSummaryCard(0, 0, 0, 0) {
	m_brushAppBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
	m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
	m_brushSidebar.CreateSolidBrush(TAECHANG_COLOR_SIDEBAR);
	m_brushListHeader.CreateSolidBrush(TAECHANG_COLOR_LIST_HEADER);
	m_brushHeaderStatus.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
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
	m_wndExportCsv.Create(TAECHANG_UI_EXPORT_CSV_BUTTON, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_EXPORT_CSV);
	m_wndSelectAll.Create(TAECHANG_UI_SELECT_ALL_BUTTON, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_SELECT_ALL);
	m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, rectEmpty, this, ID_TAECHANG_PROGRESS);
	m_wndProgressText.Create(L"", WS_CHILD | WS_VISIBLE | SS_RIGHT, rectEmpty, this);
	m_wndResultList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, rectEmpty, this, ID_TAECHANG_RESULT_LIST);
	{
		CHeaderCtrl* pHeader = m_wndResultList.GetHeaderCtrl();
		if (pHeader && pHeader->GetSafeHwnd()) {
			m_wndResultHeader.SubclassWindow(pHeader->GetSafeHwnd());
			SetWindowTheme(m_wndResultHeader.GetSafeHwnd(), L"", L"");
		}
	}
	m_wndReceivablesFilter.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL, rectEmpty, this, ID_TAECHANG_RECEIVABLES_FILTER_EDIT);
	m_wndReceivablesSearchBtn.Create(TAECHANG_UI_RECEIVABLES_SEARCH_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RECEIVABLES_SEARCH_BTN);
	m_wndReceivablesResetBtn.Create(TAECHANG_UI_RECEIVABLES_RESET_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RECEIVABLES_RESET_BTN);
	m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);
	m_wndEmptyStateHint.Create(TAECHANG_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
	m_wndActionStatus.Create(L"", WS_CHILD | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

	m_wndLoginBtn.Create(TAECHANG_UI_LOGIN_BTN, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGIN_BTN);
	m_wndLogoutBtn.Create(TAECHANG_UI_LOGOUT_BTN, WS_CHILD | BS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_LOGOUT_BTN);
	m_wndUserLabel.Create(L"", WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, rectEmpty, this, ID_TAECHANG_USER_LABEL);

	m_wndResultList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_wndReceivablesFilter.LimitText(20);
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

	ApplyControlFonts();
	ApplyWorkflowTabs();
	ApplyResultColumns();
	UpdateWorkflowLabels();
	UpdateResultColumns();
	UpdateExportButtonState();
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

	HTREEITEM hInspection = m_wndSidebarTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_INSPECTION, TVI_ROOT, TVI_LAST);
	m_wndSidebarTree.SetItemData(hInspection, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hPdf = m_wndSidebarTree.InsertItem(CString(TAECHANG_UI_PDF_COMPARE_NAME) + TAECHANG_UI_PREPARING_SUFFIX, hInspection, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPdf, TAECHANG_WORKFLOW_PDF_COMPARE);
	HTREEITEM hHwp = m_wndSidebarTree.InsertItem(CString(TAECHANG_UI_HWP_COMPARE_NAME) + TAECHANG_UI_PREPARING_SUFFIX, hInspection, TVI_LAST);
	m_wndSidebarTree.SetItemData(hHwp, TAECHANG_WORKFLOW_HWP_COMPARE);

	HTREEITEM hPrice = m_wndSidebarTree.InsertItem(TAECHANG_UI_SIDEBAR_GROUP_PRICE, TVI_ROOT, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPrice, TAECHANG_SIDEBAR_ACTION_NONE);
	HTREEITEM hPriceManage = m_wndSidebarTree.InsertItem(TAECHANG_UI_PRICE_MANAGE_NAME, hPrice, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPriceManage, TAECHANG_WORKFLOW_PRICE_MANAGE);
	HTREEITEM hPriceCalc = m_wndSidebarTree.InsertItem(TAECHANG_UI_PRICE_CALC_NAME, hPrice, TVI_LAST);
	m_wndSidebarTree.SetItemData(hPriceCalc, TAECHANG_WORKFLOW_PRICE_CALC);

	m_wndSidebarTree.Expand(hDocument, TVE_EXPAND);
	m_wndSidebarTree.Expand(hInspection, TVE_EXPAND);
	m_wndSidebarTree.Expand(hPrice, TVE_EXPAND);

	m_hLastWorkflowItem = hReceivables;
	m_wndSidebarTree.SelectItem(hReceivables);
}

void CSageTaechangView::ApplyControlFonts() {
	if (m_fontTitle.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE)) {
		m_wndTitle.SetFont(&m_fontTitle);
	}

	if (m_fontHeader.CreatePointFont(TAECHANG_HEADER_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE)) {
		m_wndHeaderTitle.SetFont(&m_fontHeader);
	}

	if (!m_fontControl.CreatePointFont(TAECHANG_CONTROL_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE))
		return;

	m_wndSidebarTree.SetFont(&m_fontControl);
	m_wndSidebarTitle.SetFont(&m_fontControl);

	if (!m_fontContent.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE))
		return;

	m_wndHeaderStatus.SetFont(&m_fontContent);
	m_wndTaskTabs.SetFont(&m_fontContent);
	m_wndInputSection.SetFont(&m_fontContent);
	m_wndOutputSection.SetFont(&m_fontContent);
	m_wndResultSection.SetFont(&m_fontContent);
	m_wndDetailSection.SetFont(&m_fontContent);
	m_wndWorkflowLabel.SetFont(&m_fontContent);
	m_wndInputLabel.SetFont(&m_fontContent);
	m_wndOutputLabel.SetFont(&m_fontContent);
	m_wndInputPath.SetFont(&m_fontContent);
	m_wndOutputFolder.SetFont(&m_fontContent);
	m_wndSelectInput.SetFont(&m_fontContent);
	m_wndSelectOutput.SetFont(&m_fontContent);
	m_wndLoad.SetFont(&m_fontContent);
	m_wndGenerate.SetFont(&m_fontContent);
	m_wndExportCsv.SetFont(&m_fontContent);
	m_wndSelectAll.SetFont(&m_fontContent);
	m_wndProgressText.SetFont(&m_fontContent);
	m_wndResultList.SetFont(&m_fontContent);
	if (::IsWindow(m_wndResultHeader.GetSafeHwnd()))
		m_wndResultHeader.SetFont(&m_fontContent);
	m_wndReceivablesFilter.SetFont(&m_fontContent);
	m_wndReceivablesSearchBtn.SetFont(&m_fontContent);
	m_wndReceivablesResetBtn.SetFont(&m_fontContent);
	m_wndDetail.SetFont(&m_fontContent);
	m_wndEmptyStateHint.SetFont(&m_fontContent);
	m_wndActionStatus.SetFont(&m_fontContent);
	m_wndLoginBtn.SetFont(&m_fontContent);
	m_wndLogoutBtn.SetFont(&m_fontContent);
	m_wndUserLabel.SetFont(&m_fontContent);

	// 가격 데이터 관리 패널
	m_wndPriceCompanyLabel.SetFont(&m_fontContent);
	m_wndPriceCompanyCombo.SetFont(&m_fontContent);
	m_wndPriceAddCompanyBtn.SetFont(&m_fontContent);
	m_wndPriceRenameCompanyBtn.SetFont(&m_fontContent);
	m_wndPriceChangeCoverBtn.SetFont(&m_fontContent);
	m_wndPriceCopiesList.SetFont(&m_fontContent);
	if (::IsWindow(m_wndPriceCopiesHeader.GetSafeHwnd()))
		m_wndPriceCopiesHeader.SetFont(&m_fontContent);
	m_wndPriceMinCopiesLabel.SetFont(&m_fontContent);
	m_wndPriceMinCopiesEdit.SetFont(&m_fontContent);
	m_wndPriceMaxCopiesLabel.SetFont(&m_fontContent);
	m_wndPriceMaxCopiesEdit.SetFont(&m_fontContent);
	m_wndPriceNoMaxCheck.SetFont(&m_fontContent);
	m_wndPricePrintLabel.SetFont(&m_fontContent);
	m_wndPricePrintEdit.SetFont(&m_fontContent);
	m_wndPriceCoverLabel.SetFont(&m_fontContent);
	m_wndPriceCoverEdit.SetFont(&m_fontContent);
	m_wndPriceAddBtn.SetFont(&m_fontContent);
	m_wndPriceModifyBtn.SetFont(&m_fontContent);
	m_wndPriceDeleteBtn.SetFont(&m_fontContent);
	m_wndPriceCancelBtn.SetFont(&m_fontContent);
	m_wndPriceSummaryTitle.SetFont(&m_fontContent);
	m_wndPriceSummaryCount.SetFont(&m_fontContent);
	m_wndPriceSummaryRange.SetFont(&m_fontContent);

	// 부수 계산 패널
	m_wndCalcCompanyLabel.SetFont(&m_fontContent);
	m_wndCalcCompanyCombo.SetFont(&m_fontContent);
	m_wndCalcCopiesLabel.SetFont(&m_fontContent);
	m_wndCalcCopiesEdit.SetFont(&m_fontContent);
	m_wndCalcBtn.SetFont(&m_fontContent);
	m_wndCalcPrintLabel.SetFont(&m_fontContent);
	m_wndCalcPrintValue.SetFont(&m_fontContent);
	m_wndCalcCoverLabel.SetFont(&m_fontContent);
	m_wndCalcCoverValue.SetFont(&m_fontContent);
	m_wndCalcSubtotalLabel.SetFont(&m_fontContent);
	m_wndCalcSubtotalValue.SetFont(&m_fontContent);
	m_wndCalcFreightLabel.SetFont(&m_fontContent);
	m_wndCalcFreightEdit.SetFont(&m_fontContent);
	m_wndCalcFreightUnitLabel.SetFont(&m_fontContent);
	m_wndCalcTotalLabel.SetFont(&m_fontHeader);
	m_wndCalcTotalValue.SetFont(&m_fontHeader);
	m_wndCalcHistorySection.SetFont(&m_fontContent);
	m_wndCalcHistoryList.SetFont(&m_fontContent);
	if (::IsWindow(m_wndCalcHistoryHeader.GetSafeHwnd()))
		m_wndCalcHistoryHeader.SetFont(&m_fontContent);
}

void CSageTaechangView::ApplyWorkflowTabs() {
	m_wndTaskTabs.DeleteAllItems();
	if (IsCompareWorkflow(GetSelectedWorkflow())) {
		m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_FILES);
		m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_PREVIEW, TAECHANG_UI_TAB_INSPECTION);
		m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_RESULT, TAECHANG_UI_TAB_DETAIL);
		m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DETAIL, TAECHANG_UI_TAB_EXPORT);
	} else {
		m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT);
		if (HasDocumentResultTab()) {
			m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DOCUMENT_RESULT, TAECHANG_UI_TAB_RESULT);
			m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY);
		} else {
			m_wndTaskTabs.InsertItem(1, TAECHANG_UI_TAB_HISTORY);
		}
	}
	m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
	m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
	UpdateTaskTabVisibility();
}

void CSageTaechangView::ApplyResultColumns() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;

	BOOL bNeedCheckbox = (IsDeliveryInputTable() || IsEstimateInputTable()) ? TRUE : FALSE;
	BOOL bNeedGridLines = (IsReceivablesResultTable() || IsDeliveryInputTable() || IsEstimateInputTable()) ? TRUE : FALSE;
	DWORD dwExtStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
	if (bNeedCheckbox)
		dwExtStyle |= LVS_EX_CHECKBOXES;
	if (bNeedGridLines)
		dwExtStyle |= LVS_EX_GRIDLINES;
	m_wndResultList.SetExtendedStyle(dwExtStyle);

	m_wndResultList.DeleteAllItems();
	CHeaderCtrl* pHeader = m_wndResultList.GetHeaderCtrl();
	int nColumnCount = (pHeader != NULL) ? pHeader->GetItemCount() : 0;
	for (int i = nColumnCount - 1; i >= 0; --i)
		m_wndResultList.DeleteColumn(i);

	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
	int nIndex = 0;
	if (bIsCompare)
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_FILENAME, LVCFMT_LEFT, TAECHANG_RESULT_FILE_WIDTH);
	if (IsReceivablesResultTable()) {
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_COMPANY, LVCFMT_LEFT, TAECHANG_RECEIVABLES_COMPANY_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_MANAGER, LVCFMT_LEFT, TAECHANG_RECEIVABLES_MANAGER_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_ISSUE_DATE, LVCFMT_LEFT, TAECHANG_RECEIVABLES_DATE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_ITEM, LVCFMT_LEFT, TAECHANG_RECEIVABLES_ITEM_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_ISSUE_TYPE, LVCFMT_LEFT, TAECHANG_RECEIVABLES_TYPE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_TOTAL_AMOUNT, LVCFMT_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_DEPOSIT_AMOUNT, LVCFMT_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_RECEIVABLE_AMOUNT, LVCFMT_RIGHT, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_BANK, LVCFMT_LEFT, TAECHANG_RECEIVABLES_BANK_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RECEIVABLES_COL_NOTE, LVCFMT_LEFT, TAECHANG_RECEIVABLES_NOTE_WIDTH);
		return;
	}
	if (IsDeliveryInputTable()) {
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_ROW, LVCFMT_LEFT, TAECHANG_DELIVERY_ROW_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_COMPANY, LVCFMT_LEFT, TAECHANG_DELIVERY_COMPANY_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_DEPARTMENT, LVCFMT_LEFT, TAECHANG_DELIVERY_DEPARTMENT_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_ORDER_DATE, LVCFMT_LEFT, TAECHANG_DELIVERY_DATE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_DELIVERY_DATE, LVCFMT_LEFT, TAECHANG_DELIVERY_DATE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_DELIVERY_TIME, LVCFMT_LEFT, TAECHANG_DELIVERY_TIME_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_ITEM, LVCFMT_LEFT, TAECHANG_DELIVERY_ITEM_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_PRODUCT_TYPE, LVCFMT_LEFT, TAECHANG_DELIVERY_TYPE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_COMPANY_COPIES, LVCFMT_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_CORPORATION_COPIES, LVCFMT_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_DELIVERY_COL_TOTAL_COPIES, LVCFMT_RIGHT, TAECHANG_DELIVERY_COPIES_WIDTH);
		return;
	}
	if (IsEstimateInputTable()) {
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_ROW, LVCFMT_LEFT, TAECHANG_ESTIMATE_ROW_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_COMPANY, LVCFMT_LEFT, TAECHANG_ESTIMATE_COMPANY_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_DATE, LVCFMT_LEFT, TAECHANG_ESTIMATE_DATE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_ITEM, LVCFMT_LEFT, TAECHANG_ESTIMATE_ITEM_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_COPIES, LVCFMT_RIGHT, TAECHANG_ESTIMATE_COPIES_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_PAGES, LVCFMT_RIGHT, TAECHANG_ESTIMATE_PAGES_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_UNIT_PRICE, LVCFMT_RIGHT, TAECHANG_ESTIMATE_UNIT_PRICE_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_COVER, LVCFMT_RIGHT, TAECHANG_ESTIMATE_COVER_WIDTH);
		m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_ESTIMATE_COL_FREIGHT, LVCFMT_RIGHT, TAECHANG_ESTIMATE_FREIGHT_WIDTH);
		return;
	}
	m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_FIELD, LVCFMT_LEFT, TAECHANG_RESULT_FIELD_WIDTH);
	m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_VALUE, LVCFMT_LEFT, TAECHANG_RESULT_MIN_VALUE_WIDTH);
	m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_STATUS, LVCFMT_LEFT, TAECHANG_RESULT_STATUS_WIDTH);
	m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_REASON, LVCFMT_LEFT, TAECHANG_RESULT_REASON_WIDTH);
}

void CSageTaechangView::UpdateTaskTabVisibility() {
	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
	BOOL bShowInput = IsInputTabSelected();
	BOOL bShowOutput = ((bShowInput || IsResultTab()) && !bIsCompare) ? TRUE : FALSE;
	BOOL bShowAction = IsActionTabVisible();
	BOOL bShowResult = IsResultTab() || (IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable()));
	BOOL bShowDetail = IsDetailTab();
	BOOL bShowExport = IsExportTab();

	m_wndInputSection.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndInputLabel.ShowWindow(SW_HIDE);
	m_wndInputPath.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndSelectInput.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
	m_wndOutputSection.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
	m_wndOutputLabel.ShowWindow(SW_HIDE);
	m_wndOutputFolder.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
	m_wndSelectOutput.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);

	BOOL bShowHint = (!bShowResult && !bShowDetail && !bShowExport && !m_bRunning) ? TRUE : FALSE;

	m_wndLoad.ShowWindow(SW_HIDE);
	m_wndGenerate.ShowWindow(bShowAction ? SW_SHOW : SW_HIDE);
	m_wndExportCsv.ShowWindow(bShowExport ? SW_SHOW : SW_HIDE);
	BOOL bShowSelectAll = (bShowAction && (IsDeliveryInputTable() || IsEstimateInputTable())) ? TRUE : FALSE;
	m_wndSelectAll.ShowWindow(bShowSelectAll ? SW_SHOW : SW_HIDE);
	BOOL bShowActionStatus = (bShowAction && !m_bRunning && m_nLastTaskType != 0) ? TRUE : FALSE;
	m_wndProgress.ShowWindow((bShowAction && m_bRunning) ? SW_SHOW : SW_HIDE);
	m_wndProgressText.ShowWindow((bShowAction && m_bRunning) ? SW_SHOW : SW_HIDE);
	m_wndActionStatus.ShowWindow(bShowActionStatus ? SW_SHOW : SW_HIDE);

	m_wndResultSection.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
	m_wndResultList.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
	BOOL bShowReceivablesFilter =
		(bShowResult && GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES && IsReceivablesResultTable()) ? TRUE : FALSE;
	m_wndReceivablesFilter.ShowWindow(bShowReceivablesFilter ? SW_SHOW : SW_HIDE);
	m_wndReceivablesSearchBtn.ShowWindow(bShowReceivablesFilter ? SW_SHOW : SW_HIDE);
	m_wndReceivablesResetBtn.ShowWindow(bShowReceivablesFilter ? SW_SHOW : SW_HIDE);
	m_wndDetailSection.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
	m_wndDetail.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
	m_wndEmptyStateHint.ShowWindow(bShowHint ? SW_SHOW : SW_HIDE);
}

void CSageTaechangView::UpdateResultColumns() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
		return;

	CRect rectList;
	m_wndResultList.GetClientRect(&rectList);
	int nWidth = rectList.Width();
	if (nWidth <= 0)
		return;

	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
	if (IsReceivablesResultTable()) {
		m_wndResultList.SetColumnWidth(0, TAECHANG_RECEIVABLES_COMPANY_WIDTH);
		m_wndResultList.SetColumnWidth(1, TAECHANG_RECEIVABLES_MANAGER_WIDTH);
		m_wndResultList.SetColumnWidth(2, TAECHANG_RECEIVABLES_DATE_WIDTH);
		m_wndResultList.SetColumnWidth(3, TAECHANG_RECEIVABLES_ITEM_WIDTH);
		m_wndResultList.SetColumnWidth(4, TAECHANG_RECEIVABLES_TYPE_WIDTH);
		m_wndResultList.SetColumnWidth(5, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.SetColumnWidth(6, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.SetColumnWidth(7, TAECHANG_RECEIVABLES_AMOUNT_WIDTH);
		m_wndResultList.SetColumnWidth(8, TAECHANG_RECEIVABLES_BANK_WIDTH);
		m_wndResultList.SetColumnWidth(9, TAECHANG_RECEIVABLES_NOTE_WIDTH);
		return;
	}
	if (IsDeliveryInputTable()) {
		m_wndResultList.SetColumnWidth(0, TAECHANG_DELIVERY_ROW_WIDTH);
		m_wndResultList.SetColumnWidth(1, TAECHANG_DELIVERY_COMPANY_WIDTH);
		m_wndResultList.SetColumnWidth(2, TAECHANG_DELIVERY_DEPARTMENT_WIDTH);
		m_wndResultList.SetColumnWidth(3, TAECHANG_DELIVERY_DATE_WIDTH);
		m_wndResultList.SetColumnWidth(4, TAECHANG_DELIVERY_DATE_WIDTH);
		m_wndResultList.SetColumnWidth(5, TAECHANG_DELIVERY_TIME_WIDTH);
		m_wndResultList.SetColumnWidth(6, TAECHANG_DELIVERY_ITEM_WIDTH);
		m_wndResultList.SetColumnWidth(7, TAECHANG_DELIVERY_TYPE_WIDTH);
		m_wndResultList.SetColumnWidth(8, TAECHANG_DELIVERY_COPIES_WIDTH);
		m_wndResultList.SetColumnWidth(9, TAECHANG_DELIVERY_COPIES_WIDTH);
		m_wndResultList.SetColumnWidth(10, TAECHANG_DELIVERY_COPIES_WIDTH);
		return;
	}
	if (IsEstimateInputTable()) {
		m_wndResultList.SetColumnWidth(0, TAECHANG_ESTIMATE_ROW_WIDTH);
		m_wndResultList.SetColumnWidth(1, TAECHANG_ESTIMATE_COMPANY_WIDTH);
		m_wndResultList.SetColumnWidth(2, TAECHANG_ESTIMATE_DATE_WIDTH);
		m_wndResultList.SetColumnWidth(3, TAECHANG_ESTIMATE_ITEM_WIDTH);
		m_wndResultList.SetColumnWidth(4, TAECHANG_ESTIMATE_COPIES_WIDTH);
		m_wndResultList.SetColumnWidth(5, TAECHANG_ESTIMATE_PAGES_WIDTH);
		m_wndResultList.SetColumnWidth(6, TAECHANG_ESTIMATE_UNIT_PRICE_WIDTH);
		m_wndResultList.SetColumnWidth(7, TAECHANG_ESTIMATE_COVER_WIDTH);
		m_wndResultList.SetColumnWidth(8, TAECHANG_ESTIMATE_FREIGHT_WIDTH);
		return;
	}
	int nFixedWidth = TAECHANG_RESULT_FIELD_WIDTH + TAECHANG_RESULT_STATUS_WIDTH + TAECHANG_RESULT_REASON_WIDTH;
	if (bIsCompare)
		nFixedWidth += TAECHANG_RESULT_FILE_WIDTH;
	int nValueWidth = nWidth - nFixedWidth;
	if (nValueWidth < TAECHANG_RESULT_MIN_VALUE_WIDTH)
		nValueWidth = TAECHANG_RESULT_MIN_VALUE_WIDTH;

	int nCol = 0;
	if (bIsCompare)
		m_wndResultList.SetColumnWidth(nCol++, TAECHANG_RESULT_FILE_WIDTH);
	m_wndResultList.SetColumnWidth(nCol++, TAECHANG_RESULT_FIELD_WIDTH);
	m_wndResultList.SetColumnWidth(nCol++, nValueWidth);
	m_wndResultList.SetColumnWidth(nCol++, TAECHANG_RESULT_STATUS_WIDTH);
	m_wndResultList.SetColumnWidth(nCol++, TAECHANG_RESULT_REASON_WIDTH);
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

		m_wndLoginBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndLogoutBtn.MoveWindow(nLoginBtnLeft, nLoginBtnTop, TAECHANG_LOGIN_BTN_WIDTH, TAECHANG_BUTTON_HEIGHT);
		m_wndUserLabel.MoveWindow(nUserLabelLeft, nLoginBtnTop, TAECHANG_USER_LABEL_WIDTH, TAECHANG_BUTTON_HEIGHT);
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
		m_wndExportCsv.ShowWindow(SW_HIDE);
		m_wndSelectAll.ShowWindow(SW_HIDE);
		m_wndProgress.ShowWindow(SW_HIDE);
		m_wndProgressText.ShowWindow(SW_HIDE);
		m_wndActionStatus.ShowWindow(SW_HIDE);
		m_wndResultSection.ShowWindow(SW_HIDE);
		m_wndResultList.ShowWindow(SW_HIDE);
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

	m_wndTaskTabs.ShowWindow(SW_SHOW);
	m_wndTaskTabs.MoveWindow(nContentLeft, nContentTop, nContentWidth, TAECHANG_TAB_HEIGHT);
	nContentTop += TAECHANG_TAB_HEIGHT + TAECHANG_PANEL_GAP;

	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
	if (IsInputTabSelected()) {
		LayoutInputSection(nContentLeft, nContentTop, nContentWidth, !bIsCompare);
		nContentTop += (bIsCompare ? TAECHANG_INPUT_PANEL_HEIGHT / 2 : TAECHANG_INPUT_PANEL_HEIGHT) + TAECHANG_PANEL_GAP;
	}

	if (IsActionTabVisible() || IsExportTab()) {
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
	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
	BOOL bShowAction = IsActionTabVisible();
	BOOL bShowLoad = FALSE;
	BOOL bShowGenerate = bShowAction;
	BOOL bShowExport = IsExportTab();

	int nX = nLeft;
	if (bShowLoad) {
		m_wndLoad.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
	}
	if (bShowGenerate) {
		m_wndGenerate.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
	}
	if (bShowExport) {
		m_wndExportCsv.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
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
		int nSectionWidth = bShowSelectAll ? nWidth - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP : nWidth;
		BOOL bShowReceivablesFilter =
			(GetSelectedWorkflow() == TAECHANG_WORKFLOW_RECEIVABLES && IsReceivablesResultTable()) ? TRUE : FALSE;
		int nFilterTotalW = TAECHANG_RECEIVABLES_FILTER_WIDTH + TAECHANG_ACTION_GAP
			+ TAECHANG_RECEIVABLES_SEARCH_WIDTH + TAECHANG_ACTION_GAP + TAECHANG_RECEIVABLES_RESET_WIDTH;
		if (bShowReceivablesFilter)
			nSectionWidth -= nFilterTotalW + TAECHANG_ROW_GAP;
		if (nSectionWidth < 0)
			nSectionWidth = 0;
		m_wndResultSection.MoveWindow(nLeft, nTop, nSectionWidth, TAECHANG_RESULT_HEADER_HEIGHT);
		if (bShowSelectAll)
			m_wndSelectAll.MoveWindow(nLeft + nWidth - TAECHANG_BUTTON_WIDTH, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
		if (bShowReceivablesFilter) {
			int nFilterTop = nTop - 8;
			int nFilterLeft = nLeft + nWidth - nFilterTotalW;
			m_wndReceivablesFilter.MoveWindow(nFilterLeft, nFilterTop, TAECHANG_RECEIVABLES_FILTER_WIDTH, TAECHANG_EDIT_HEIGHT);
			CRect rcFmt;
			m_wndReceivablesFilter.GetClientRect(&rcFmt);
			rcFmt.top += TAECHANG_EDIT_TEXT_TOP_PAD;
			rcFmt.left += 6;
			rcFmt.right = TAECHANG_EDIT_FORMAT_MAX_WIDTH;
			m_wndReceivablesFilter.SendMessage(EM_SETRECT, 0, reinterpret_cast<LPARAM>(&rcFmt));
			int nSearchLeft = nFilterLeft + TAECHANG_RECEIVABLES_FILTER_WIDTH + TAECHANG_ACTION_GAP;
			m_wndReceivablesSearchBtn.MoveWindow(nSearchLeft, nFilterTop, TAECHANG_RECEIVABLES_SEARCH_WIDTH, TAECHANG_BUTTON_HEIGHT);
			int nResetLeft = nSearchLeft + TAECHANG_RECEIVABLES_SEARCH_WIDTH + TAECHANG_ACTION_GAP;
			m_wndReceivablesResetBtn.MoveWindow(nResetLeft, nFilterTop, TAECHANG_RECEIVABLES_RESET_WIDTH, TAECHANG_BUTTON_HEIGHT);
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
	DrawEditBorder(pDC, m_wndReceivablesFilter);
	DrawEditBorder(pDC, m_wndPriceCompanyCombo);
	DrawEditBorder(pDC, m_wndPriceMinCopiesEdit);
	DrawEditBorder(pDC, m_wndPriceMaxCopiesEdit);
	DrawEditBorder(pDC, m_wndPricePrintEdit);
	DrawEditBorder(pDC, m_wndPriceCoverEdit);
	DrawEditBorder(pDC, m_wndCalcCompanyCombo);
	DrawEditBorder(pDC, m_wndCalcCopiesEdit);
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
	if (nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) {
		m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_HWP_COMPARE_NAME);
		m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INSPECTION_INPUT);
		m_wndGenerate.SetWindowTextW(TAECHANG_UI_HWP_COMPARE_BUTTON);
	} else if (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE) {
		m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_PDF_COMPARE_NAME);
		m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INSPECTION_INPUT);
		m_wndGenerate.SetWindowTextW(TAECHANG_UI_PDF_COMPARE_BUTTON);
	} else if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) {
		m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_ESTIMATE_NAME);
		m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INPUT);
		m_wndGenerate.SetWindowTextW(TAECHANG_UI_ESTIMATE_GENERATE_BUTTON);
	} else if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY) {
		m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_DELIVERY_NAME);
		m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INPUT);
		m_wndGenerate.SetWindowTextW(TAECHANG_UI_DELIVERY_GENERATE_BUTTON);
	} else {
		m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_RECEIVABLES_NAME);
		m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INPUT);
		m_wndGenerate.SetWindowTextW(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON);
	}
	m_wndDetailSection.SetWindowTextW(IsCompareWorkflow(nWorkflowType) ? TAECHANG_UI_SECTION_DETAIL : TAECHANG_UI_SECTION_HISTORY);
	m_wndDetail.SetWindowTextW(IsCompareWorkflow(nWorkflowType) ? CString() : m_strExecutionHistory);
	ApplyWorkflowTabs();
	ApplyResultColumns();
	LayoutChildControls();
	UpdateExportButtonState();
}

BOOL CSageTaechangView::IsCompareWorkflow(int nWorkflowType) const {
	return (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE || nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsInputTabSelected() const {
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_INPUT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsResultTab() const {
	if (IsCompareWorkflow(GetSelectedWorkflow()))
		return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_PREVIEW) ? TRUE : FALSE;
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_RESULT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDetailTab() const {
	if (IsCompareWorkflow(GetSelectedWorkflow()))
		return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_RESULT) ? TRUE : FALSE;
	return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_HISTORY) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsExportTab() const {
	return (IsCompareWorkflow(GetSelectedWorkflow()) && m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DETAIL) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsActionTabVisible() const {
	return IsInputTabSelected() ? TRUE : FALSE;
}

BOOL CSageTaechangView::HasDocumentResultTab() const {
	int nWorkflowType = GetSelectedWorkflow();
	return (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) ? FALSE : TRUE;
}

int CSageTaechangView::GetTaskTabVisualIndex(int nSemanticTabIndex) const {
	if (IsCompareWorkflow(GetSelectedWorkflow()))
		return nSemanticTabIndex;
	if (HasDocumentResultTab())
		return nSemanticTabIndex;
	if (nSemanticTabIndex == TAECHANG_TAB_INDEX_DOCUMENT_HISTORY)
		return 1;
	return TAECHANG_TAB_INDEX_INPUT;
}

int CSageTaechangView::GetTaskTabSemanticIndex(int nVisualTabIndex) const {
	if (IsCompareWorkflow(GetSelectedWorkflow()))
		return nVisualTabIndex;
	if (HasDocumentResultTab())
		return nVisualTabIndex;
	if (nVisualTabIndex == 1)
		return TAECHANG_TAB_INDEX_DOCUMENT_HISTORY;
	return TAECHANG_TAB_INDEX_INPUT;
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

void CSageTaechangView::UpdateExportButtonState() {
	BOOL bEnabled = (!m_bRunning && IsCompareWorkflow(GetSelectedWorkflow()) && !m_strLastResponseJson.IsEmpty()) ? TRUE : FALSE;
	if (::IsWindow(m_wndExportCsv.GetSafeHwnd()))
		m_wndExportCsv.EnableWindow(bEnabled);
}

void CSageTaechangView::OnWorkflowChanged() {
	m_strLastResponseJson.Empty();
	m_nLastWorkflowType = 0;
	m_nLastTaskType = 0;
	m_strRunningInputPath.Empty();
	m_wndInputPath.SetWindowTextW(L"");
	m_wndOutputFolder.SetWindowTextW(L"");
	m_wndReceivablesFilter.SetWindowTextW(L"");
	m_strReceivablesFilterKeyword.Empty();

	if (IsPriceWorkflowType(m_nCurrentWorkflow)) {
		m_wndHeaderTitle.SetWindowTextW(
			m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE
				? TAECHANG_UI_PRICE_MANAGE_NAME
				: TAECHANG_UI_PRICE_CALC_NAME
		);
		if (m_nCurrentWorkflow == TAECHANG_WORKFLOW_PRICE_MANAGE)
			RefreshPriceCompanyList();
		else
			RefreshCalcCompanyCombo();
		LayoutChildControls();
		Invalidate(FALSE);
		SetStatusText(TAECHANG_UI_READY);
		return;
	}

	UpdateWorkflowLabels();
	UpdateExportButtonState();
	UpdateResultColumns();
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
	if (nItemData == TAECHANG_WORKFLOW_PDF_COMPARE || nItemData == TAECHANG_WORKFLOW_HWP_COMPARE) {
		SetStatusText(TAECHANG_UI_FEATURE_PREPARING);
		if (m_hLastWorkflowItem != NULL)
			m_wndSidebarTree.SelectItem(m_hLastWorkflowItem);
		return;
	}
	int nNewWorkflow = static_cast<int>(nItemData);
	m_hLastWorkflowItem = hItem;
	if (nNewWorkflow == m_nCurrentWorkflow)
		return;
	m_nCurrentWorkflow = nNewWorkflow;
	OnWorkflowChanged();
}

void CSageTaechangView::OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult) {
	UNREFERENCED_PARAMETER(pNMHDR);
	m_nSelectedTaskTab = GetTaskTabSemanticIndex(m_wndTaskTabs.GetCurSel());
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

	int nWorkflowType = GetSelectedWorkflow();
	BOOL bIsCompare = IsCompareWorkflow(nWorkflowType);
	CString strInputPaths;
	if (bIsCompare) {
		strInputPaths = strPaths;
	} else {
		int nIndex = 0;
		strInputPaths = strPaths.Tokenize(L"\r\n", nIndex);
		strInputPaths.Trim();
	}
	if (strInputPaths.IsEmpty())
		return;

	m_wndInputPath.SetWindowTextW(strInputPaths);
	if (!IsInputTabSelected()) {
		m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
		m_wndTaskTabs.SetCurSel(GetTaskTabVisualIndex(m_nSelectedTaskTab));
		LayoutChildControls();
	}
	SetStatusText(L"파일 드롭 수신");
	if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
		RunWorkflowTask(TAECHANG_TASK_LOAD);
}

void CSageTaechangView::OnSelectInput() {
	int nWorkflowType = GetSelectedWorkflow();
	if (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE || nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) {
		LPCWSTR pszExt = nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE ? L"hwp" : L"pdf";
		LPCWSTR pszFilter = nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE ? TAECHANG_UI_HWP_FILTER : TAECHANG_UI_PDF_FILTER;
		LPCWSTR pszTitle = nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE ? TAECHANG_UI_SELECT_HWP_INPUT_TITLE : TAECHANG_UI_SELECT_PDF_INPUT_TITLE;
		CFileDialog dlg(TRUE, pszExt, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, pszFilter, this);
		CString strBuffer;
		LPTSTR pszBuffer = strBuffer.GetBuffer(32768);
		ZeroMemory(pszBuffer, sizeof(TCHAR) * 32768);
		dlg.m_ofn.lpstrFile = pszBuffer;
		dlg.m_ofn.nMaxFile = 32768;
		dlg.m_ofn.lpstrTitle = pszTitle;
		if (dlg.DoModal() == IDOK) {
			POSITION pos = dlg.GetStartPosition();
			CString strPaths;
			while (pos != NULL) {
				if (!strPaths.IsEmpty())
					strPaths += L"\r\n";
				strPaths += dlg.GetNextPathName(pos);
			}
			m_wndInputPath.SetWindowTextW(strPaths);
		}
		strBuffer.ReleaseBuffer();
		return;
	}

	CFileDialog dlg(TRUE, L"xls", NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, TAECHANG_UI_EXCEL_FILTER, this);
	if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
		dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_ESTIMATE_INPUT_TITLE;
	else if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
		dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_DELIVERY_INPUT_TITLE;
	else
		dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_RECEIVABLES_INPUT_TITLE;
	if (dlg.DoModal() == IDOK) {
		m_wndInputPath.SetWindowTextW(dlg.GetPathName());
		if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
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

void CSageTaechangView::OnExportCsv() {
	if (m_strLastResponseJson.IsEmpty() || !IsCompareWorkflow(m_nLastWorkflowType)) {
		AfxMessageBox(TAECHANG_UI_EXPORT_RESULT_REQUIRED, MB_ICONWARNING);
		return;
	}

	COMDLG_FILTERSPEC arrTypes[] =
	{
		{ L"CSV Files", L"*.csv" },
		{ L"All Files", L"*.*" }
	};
	CString strPath = ShowIFileSaveDialog(
		GetSafeHwnd(),
		TAECHANG_UI_SELECT_CSV_OUTPUT_TITLE,
		L"csv",
		arrTypes,
		2,
		L"taechang-compare-result.csv");
	if (strPath.IsEmpty())
		return;

	CString strError;
	TaechangCompareCsvExportService service;
	if (!service.ExportCompareResult(m_strLastResponseJson, strPath, strError)) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	SetStatusText(TAECHANG_UI_EXPORT_COMPLETED);
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
	for (int i = 0; i < nCount; ++i)
		m_wndResultList.SetCheck(i, bCheck);
}

void CSageTaechangView::RunWorkflowTask(int nTaskType) {
	if (m_bRunning)
		return;

	CString strInputPath;
	CString strOutputFolder;
	if (!ValidateInputPath(strInputPath))
		return;

	int nWorkflowType = GetSelectedWorkflow();
	if (nTaskType == TAECHANG_TASK_GENERATE && nWorkflowType != TAECHANG_WORKFLOW_PDF_COMPARE && nWorkflowType != TAECHANG_WORKFLOW_HWP_COMPARE && !ValidateOutputFolder(strOutputFolder))
		return;

	CString strSelectedRowNums;
	if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) && nTaskType == TAECHANG_TASK_GENERATE) {
		int nListCount = m_wndResultList.GetItemCount();
		for (int i = 0; i < nListCount; ++i) {
			if (!m_wndResultList.GetCheck(i))
				continue;
			DWORD_PTR nSourceRowIndex = m_wndResultList.GetItemData(i);
			if (nSourceRowIndex == 0)
				continue;
			CString strNum;
			strNum.Format(TAECHANG_UI_ROW_NUM_FORMAT, static_cast<unsigned long>(nSourceRowIndex));
			if (!strSelectedRowNums.IsEmpty())
				strSelectedRowNums += L",";
			strSelectedRowNums += strNum;
		}
		if (strSelectedRowNums.IsEmpty()) {
			LPCWSTR pszMsg = (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
				? TAECHANG_UI_ESTIMATE_SELECT_ROW_REQUIRED
				: TAECHANG_UI_DELIVERY_SELECT_ROW_REQUIRED;
			AfxMessageBox(pszMsg, MB_ICONWARNING);
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
	m_strRunningInputPath = strInputPath;
	if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
		pTask->m_strPdfFilePaths = strInputPath;
	else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
		pTask->m_strHwpFilePaths = strInputPath;

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
	if (bRunning) {
		UpdateProgressPercent(0);
		SetTimer(ID_TAECHANG_PROGRESS_TIMER, TAECHANG_PROGRESS_TIMER_MS, NULL);
	} else {
		KillTimer(ID_TAECHANG_PROGRESS_TIMER);
		UpdateProgressPercent(TAECHANG_PROGRESS_COMPLETE);
	}
	UpdateExportButtonState();
	UpdateTaskTabVisibility();
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
	return TRUE;
}

HBRUSH CSageTaechangView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hBrush = CView::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	if (pWnd->GetSafeHwnd() == m_wndTitle.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_SIDEBAR_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_SIDEBAR);
		return m_brushSidebar;
	}
	if (pWnd->GetSafeHwnd() == m_wndSidebarTitle.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_SIDEBAR_CATEGORY);
		pDC->SetBkColor(TAECHANG_COLOR_SIDEBAR);
		return m_brushSidebar;
	}
	if (pWnd->GetSafeHwnd() == m_wndHeaderTitle.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_PRIMARY);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return m_brushAppBackground;
	}
	if (pWnd->GetSafeHwnd() == m_wndUserLabel.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return m_brushAppBackground;
	}
	if (pWnd->GetSafeHwnd() == m_wndHeaderStatus.GetSafeHwnd()) {
		pDC->SetTextColor(m_colorHeaderStatus);
		pDC->SetBkColor(m_colorHeaderStatusBg);
		return m_brushHeaderStatus;
	}
	if (pWnd->GetSafeHwnd() == m_wndEmptyStateHint.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return m_brushAppBackground;
	}
	if (pWnd->GetSafeHwnd() == m_wndActionStatus.GetSafeHwnd()) {
		pDC->SetTextColor(m_bLastTaskSuccess ? TAECHANG_COLOR_SUCCESS : TAECHANG_COLOR_ERROR);
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return m_brushAppBackground;
	}
	if (pWnd->GetSafeHwnd() == m_wndPriceSummaryTitle.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_PRIMARY);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	if (pWnd->GetSafeHwnd() == m_wndPriceSummaryCount.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndPriceSummaryRange.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	if (pWnd->GetSafeHwnd() == m_wndPriceMinCopiesLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndPriceMaxCopiesLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndPricePrintLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndPriceCoverLabel.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_BUTTON_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_LIST_HEADER);
		return m_brushListHeader;
	}
	if (pWnd->GetSafeHwnd() == m_wndCalcTotalLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcTotalValue.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_PRIMARY);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	if (pWnd->GetSafeHwnd() == m_wndCalcCompanyLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcCopiesLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcPrintLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcPrintValue.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcCoverLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcCoverValue.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcSubtotalLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcSubtotalValue.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcFreightLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcFreightUnitLabel.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcDivider.GetSafeHwnd() ||
		pWnd->GetSafeHwnd() == m_wndCalcTotalDivider.GetSafeHwnd()) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	if (pWnd->GetSafeHwnd() == m_wndPriceCoverEdit.GetSafeHwnd()) {
		pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
		return m_brushAppBackground;
	}
	if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX) {
		pDC->SetBkColor(TAECHANG_COLOR_PANEL);
		return m_brushPanel;
	}
	return hBrush;
}

void CSageTaechangView::SetStatusText(const CString& strStatus) {
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		pFrame->SetMessageText(strStatus);

	if (::IsWindow(m_wndHeaderStatus.GetSafeHwnd())) {
		m_colorHeaderStatus = ResolveStatusColor(strStatus);
		m_colorHeaderStatusBg = ResolveStatusBgColor(strStatus);
		m_brushHeaderStatus.DeleteObject();
		m_brushHeaderStatus.CreateSolidBrush(m_colorHeaderStatusBg);
		m_wndHeaderStatus.SetWindowTextW(strStatus);
		m_wndHeaderStatus.Invalidate();
	}
}

COLORREF CSageTaechangView::ResolveStatusColor(const CString& strStatus) const {
	if (strStatus == TAECHANG_UI_RUNNING)
		return TAECHANG_COLOR_PRIMARY;
	if (strStatus == TAECHANG_UI_COMPLETED ||
		strStatus == TAECHANG_UI_EXPORT_COMPLETED)
		return TAECHANG_COLOR_SUCCESS;
	if (strStatus == TAECHANG_UI_FAILED)
		return TAECHANG_COLOR_ERROR;
	return TAECHANG_COLOR_SECONDARY_TEXT;
}

COLORREF CSageTaechangView::ResolveStatusBgColor(const CString& strStatus) const {
	if (strStatus == TAECHANG_UI_COMPLETED || strStatus == TAECHANG_UI_EXPORT_COMPLETED)
		return TAECHANG_COLOR_STATUS_BG_SUCCESS;
	if (strStatus == TAECHANG_UI_RUNNING)
		return TAECHANG_COLOR_STATUS_BG_WARNING;
	if (strStatus == TAECHANG_UI_FAILED)
		return TAECHANG_COLOR_STATUS_BG_ERROR;
	return TAECHANG_COLOR_APP_BACKGROUND;
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
	m_strLastResponseJson = strResponseJson;
	if (!bDocumentGenerateNoResult) {
		ApplyResultColumns();
		UpdateResultColumns();
	}

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	CString strDetailText;
	BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows, strDetailText);
	AppendExecutionHistory(nWorkflowType, nTaskType, strResponseJson, bSuccess);
	if (IsCompareWorkflow(nWorkflowType))
		m_wndDetail.SetWindowTextW(strDetailText);
	else
		m_wndDetail.SetWindowTextW(m_strExecutionHistory);

	if (!bDocumentGenerateNoResult) {
		if (nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES)
			RefreshReceivablesResultFilter();
		else {
			for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
				InsertResultRow(arrRows[i]);
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
	UpdateExportButtonState();
}

void CSageTaechangView::InsertResultRow(const TaechangResultRow& row) {
	BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
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
	if (bIsCompare) {
		nIndex = m_wndResultList.InsertItem(nCount, row.m_strFile);
		++nCol;
		m_wndResultList.SetItemText(nIndex, nCol++, row.m_strField);
	} else {
		nIndex = m_wndResultList.InsertItem(nCount, row.m_strField);
		++nCol;
	}
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strValue);
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strStatus);
	m_wndResultList.SetItemText(nIndex, nCol++, row.m_strReason);
	m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
}

void CSageTaechangView::RefreshReceivablesResultFilter() {
	if (!::IsWindow(m_wndResultList.GetSafeHwnd()) || !IsReceivablesResultTable())
		return;

	CString strFilter = m_strReceivablesFilterKeyword;
	strFilter.Trim();
	CString strFilterLower = strFilter;
	strFilterLower.MakeLower();

	m_wndResultList.DeleteAllItems();

	TaechangWorkflowResultPresenter presenter;
	std::vector<TaechangResultRow> arrRows;
	CString strDetailText;
	presenter.BuildRows(m_nLastWorkflowType, m_nLastTaskType, m_strLastResponseJson, arrRows, strDetailText);
	for (int i = 0; i < static_cast<int>(arrRows.size()); ++i) {
		if (!strFilterLower.IsEmpty()) {
			CString strCompanyLower = arrRows[i].m_strCompanyName;
			strCompanyLower.MakeLower();
			if (strCompanyLower.Find(strFilterLower) < 0)
				continue;
		}
		InsertResultRow(arrRows[i]);
	}
}

void CSageTaechangView::OnReceivablesSearch() {
	if (GetSelectedWorkflow() != TAECHANG_WORKFLOW_RECEIVABLES || !IsReceivablesResultTable())
		return;

	m_wndReceivablesFilter.GetWindowTextW(m_strReceivablesFilterKeyword);
	m_strReceivablesFilterKeyword.Trim();
	RefreshReceivablesResultFilter();
}

void CSageTaechangView::OnReceivablesFilterReset() {
	if (GetSelectedWorkflow() != TAECHANG_WORKFLOW_RECEIVABLES || !IsReceivablesResultTable())
		return;

	m_strReceivablesFilterKeyword.Empty();
	m_wndReceivablesFilter.SetWindowTextW(L"");
	RefreshReceivablesResultFilter();
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

void CSageTaechangView::DrawSectionLabel(LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	pDC->FillSolidRect(rect, TAECHANG_COLOR_APP_BACKGROUND);
	constexpr int nAccentWidth = 3;
	pDC->FillSolidRect(rect.left, rect.top + 2, nAccentWidth, rect.Height() - 4, TAECHANG_COLOR_PRIMARY);
	CWnd* pWnd = CWnd::FromHandle(lpDrawItemStruct->hwndItem);
	CString strText;
	pWnd->GetWindowText(strText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(TAECHANG_COLOR_TEXT);
	CFont* pOldFont = pDC->SelectObject(&m_fontContent);
	CRect rcText = rect;
	rcText.left += nAccentWidth + 8;
	pDC->DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

void CSageTaechangView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	if (lpDrawItemStruct->CtlType == ODT_STATIC &&
		(nIDCtl == ID_TAECHANG_INPUT_SECTION || nIDCtl == ID_TAECHANG_OUTPUT_SECTION ||
		 nIDCtl == ID_TAECHANG_RESULT_SECTION || nIDCtl == ID_TAECHANG_DETAIL_SECTION ||
		 nIDCtl == ID_CALC_HISTORY_SECTION)) {
		DrawSectionLabel(lpDrawItemStruct);
		return;
	}
	if (lpDrawItemStruct->CtlType != ODT_BUTTON) {
		CView::OnDrawItem(nIDCtl, lpDrawItemStruct);
		return;
	}

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	BOOL bPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
	BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

	BOOL bPrimary = (nIDCtl == ID_TAECHANG_GENERATE_WORKFLOW || nIDCtl == ID_TAECHANG_LOAD_WORKFLOW
		|| nIDCtl == ID_TAECHANG_LOGIN_BTN
		|| nIDCtl == ID_TAECHANG_RECEIVABLES_SEARCH_BTN
		|| nIDCtl == ID_PRICE_ADD_COMPANY_BTN || nIDCtl == ID_PRICE_ADD_BTN || nIDCtl == ID_PRICE_RENAME_COMPANY_BTN
		|| nIDCtl == ID_PRICE_CHANGE_COVER_BTN || nIDCtl == ID_PRICE_MODIFY_BTN
		|| nIDCtl == ID_CALC_BTN);

	if (bPrimary) {
		COLORREF clrBg = bDisabled ? TAECHANG_COLOR_BORDER
			: bPressed ? TAECHANG_COLOR_PRIMARY_PRESS : TAECHANG_COLOR_PRIMARY;
		pDC->FillSolidRect(rect, clrBg);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_BUTTON_TEXT);
	} else {
		pDC->FillSolidRect(rect, bDisabled ? TAECHANG_COLOR_APP_BACKGROUND : TAECHANG_COLOR_PANEL);
		CBrush brBorder;
		brBorder.CreateSolidBrush(bDisabled ? TAECHANG_COLOR_BORDER : TAECHANG_COLOR_PRIMARY);
		pDC->FrameRect(rect, &brBorder);
		pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_PRIMARY);
	}

	CWnd* pWnd = CWnd::FromHandle(lpDrawItemStruct->hwndItem);
	CString strText;
	pWnd->GetWindowText(strText);

	pDC->SetBkMode(TRANSPARENT);
	if (nIDCtl == ID_TAECHANG_RECEIVABLES_SEARCH_BTN) {
		COLORREF clrIcon = bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_BUTTON_TEXT;
		CPen pen(PS_SOLID, 2, clrIcon);
		CPen* pOldPen = pDC->SelectObject(&pen);
		CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		int nCx = rect.CenterPoint().x - 2;
		int nCy = rect.CenterPoint().y - 2;
		pDC->Ellipse(nCx - 6, nCy - 6, nCx + 7, nCy + 7);
		pDC->MoveTo(nCx + 5, nCy + 5);
		pDC->LineTo(nCx + 11, nCy + 11);
		if (pOldBrush)
			pDC->SelectObject(pOldBrush);
		if (pOldPen)
			pDC->SelectObject(pOldPen);
		return;
	}

	CFont* pOldFont = pDC->SelectObject(nIDCtl == ID_TAECHANG_RECEIVABLES_RESET_BTN ? &m_fontHeader : &m_fontContent);
	rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
	pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	if (pOldFont)
		pDC->SelectObject(pOldFont);
}

void CSageTaechangView::OnSidebarTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
	NMTVCUSTOMDRAW* pCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	switch (pCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
		{
			HTREEITEM hItem = reinterpret_cast<HTREEITEM>(pCD->nmcd.dwItemSpec);
			BOOL bIsGroupHeader = (m_wndSidebarTree.GetParentItem(hItem) == NULL);
			BOOL bIsSelected = (pCD->nmcd.uItemState & CDIS_SELECTED) != 0;
			if (bIsGroupHeader) {
				pCD->clrText = TAECHANG_COLOR_SIDEBAR_CATEGORY;
				pCD->clrTextBk = TAECHANG_COLOR_SIDEBAR;
				*pResult = CDRF_NEWFONT;
			} else {
				pCD->clrText = TAECHANG_COLOR_SIDEBAR_TEXT;
				pCD->clrTextBk = bIsSelected ? TAECHANG_COLOR_SIDEBAR_SELECTED : TAECHANG_COLOR_SIDEBAR;
				*pResult = CDRF_NEWFONT;
				if (bIsSelected)
					*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
			break;
		}
		case CDDS_ITEMPOSTPAINT:
		{
			if (pCD->nmcd.uItemState & CDIS_SELECTED) {
				CDC* pItemDC = CDC::FromHandle(pCD->nmcd.hdc);
				CRect rcItem(pCD->nmcd.rc);
				pItemDC->FillSolidRect(rcItem.left, rcItem.top, 3, rcItem.Height(), TAECHANG_COLOR_PRIMARY);
			}
			break;
		}
	}
}

void CSageTaechangView::OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
	NMLVCUSTOMDRAW* pCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	switch (pCD->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			UINT uState = ListView_GetItemState(pCD->nmcd.hdr.hwndFrom, nItem, LVIS_SELECTED);
			if (!(uState & LVIS_SELECTED)) {
				pCD->clrTextBk = (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_ROW_ALT : TAECHANG_COLOR_PANEL;
				pCD->clrText = TAECHANG_COLOR_TEXT;
				*pResult = CDRF_NEWFONT;
				if (IsReceivablesResultTable() || pCD->nmcd.hdr.idFrom == ID_PRICE_COPIES_LIST ||
					pCD->nmcd.hdr.idFrom == ID_CALC_HISTORY_LIST)
					*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			}
			break;
		}
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
			int nSubItem = pCD->iSubItem;
			UINT uState = ListView_GetItemState(pCD->nmcd.hdr.hwndFrom, nItem, LVIS_SELECTED);
			if (!(uState & LVIS_SELECTED)) {
				if (IsReceivablesResultTable() &&
					(nSubItem == TAECHANG_RECEIVABLES_COL_IDX_TOTAL_AMOUNT ||
					 nSubItem == TAECHANG_RECEIVABLES_COL_IDX_DEPOSIT_AMOUNT ||
					 nSubItem == TAECHANG_RECEIVABLES_COL_IDX_RECEIVABLE_AMOUNT)) {
					pCD->clrTextBk = (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_AMOUNT_COL_ALT : TAECHANG_COLOR_LIST_AMOUNT_COL;
					pCD->clrText = TAECHANG_COLOR_TEXT;
					*pResult = CDRF_NEWFONT;
				}
				if ((pCD->nmcd.hdr.idFrom == ID_PRICE_COPIES_LIST ||
					 pCD->nmcd.hdr.idFrom == ID_CALC_HISTORY_LIST) && nSubItem == 0) {
					CDC* pDC = CDC::FromHandle(pCD->nmcd.hdc);
					COLORREF clrBk = (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_ROW_ALT : TAECHANG_COLOR_PANEL;
					RECT rcItem;
					ListView_GetSubItemRect(pCD->nmcd.hdr.hwndFrom, nItem, 0, LVIR_LABEL, &rcItem);
					pDC->FillSolidRect(&rcItem, clrBk);
					wchar_t szText[64] = {};
					LVITEM lvi = {};
					lvi.mask = LVIF_TEXT;
					lvi.iItem = nItem;
					lvi.iSubItem = 0;
					lvi.pszText = szText;
					lvi.cchTextMax = 63;
					ListView_GetItem(pCD->nmcd.hdr.hwndFrom, &lvi);
					pDC->SetTextColor(TAECHANG_COLOR_TEXT);
					pDC->SetBkMode(TRANSPARENT);
					pDC->DrawText(szText, -1, &rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
					*pResult = CDRF_SKIPDEFAULT;
				}
			}
			break;
		}
	}
}


// ── 가격 관련 유틸 ────────────────────────────────────────────────────────────

static CString FormatPrice(int nPrice) {
	CString str;
	str.Format(L"%d", nPrice);
	int nLen = str.GetLength();
	for (int i = nLen - 3; i > 0; i -= 3)
		str.Insert(i, L',');
	return str;
}

static BOOL IsCopiesRangeOverlap(int nMinA, BOOL bHasMaxA, int nMaxA, int nMinB, BOOL bHasMaxB, int nMaxB) {
	int nEndA = bHasMaxA ? nMaxA : INT_MAX;
	int nEndB = bHasMaxB ? nMaxB : INT_MAX;
	return (nMinA <= nEndB && nMinB <= nEndA) ? TRUE : FALSE;
}

// ── 가격 데이터 관리 패널 생성 ────────────────────────────────────────────────

void CSageTaechangView::CreatePriceManagePanel() {
	CRect r(0, 0, 0, 0);
	m_wndPriceCompanyLabel.Create(TAECHANG_UI_PRICE_COMPANY_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndPriceCompanyCombo.Create(WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_PRICE_COMPANY_EDIT);
	m_wndPriceAddCompanyBtn.Create(TAECHANG_UI_PRICE_ADD_COMPANY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_ADD_COMPANY_BTN);
	m_wndPriceRenameCompanyBtn.Create(TAECHANG_UI_PRICE_RENAME_COMPANY_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_RENAME_COMPANY_BTN);
	m_wndPriceChangeCoverBtn.Create(TAECHANG_UI_PRICE_CHANGE_COVER_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_CHANGE_COVER_BTN);

	m_wndPriceCopiesList.Create(WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, r, this, ID_PRICE_COPIES_LIST);
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
	m_wndPriceMaxCopiesLabel.Create(TAECHANG_UI_PRICE_MAX_COPIES_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceMaxCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_MAX_COPIES_EDIT);
	m_wndPriceNoMaxCheck.Create(TAECHANG_UI_PRICE_NO_MAX_LABEL, WS_CHILD | BS_AUTOCHECKBOX, r, this, ID_PRICE_NO_MAX_CHECK);

	m_wndPricePrintLabel.Create(TAECHANG_UI_PRICE_PRINT_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPricePrintEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_PRINT_EDIT);
	m_wndPriceCoverLabel.Create(TAECHANG_UI_PRICE_COVER_LABEL, WS_CHILD | SS_LEFT | SS_CENTERIMAGE, r, this);
	m_wndPriceCoverEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_EDIT);

	m_wndPriceAddBtn.Create(TAECHANG_UI_PRICE_ADD_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_ADD_BTN);
	m_wndPriceModifyBtn.Create(TAECHANG_UI_PRICE_SAVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_MODIFY_BTN);
	m_wndPriceDeleteBtn.Create(TAECHANG_UI_PRICE_REMOVE_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_DELETE_BTN);
	m_wndPriceCancelBtn.Create(TAECHANG_UI_PRICE_CANCEL_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_PRICE_CANCEL_BTN);

	m_wndPriceSummaryTitle.Create(TAECHANG_UI_PRICE_SUMMARY_NO_COMPANY, WS_CHILD | SS_LEFT | SS_ENDELLIPSIS, r, this);
	m_wndPriceSummaryCount.Create(L"", WS_CHILD | SS_LEFT, r, this);
	m_wndPriceSummaryRange.Create(L"", WS_CHILD | SS_LEFT, r, this);

	m_wndPriceCompanyCombo.LimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
	m_wndPriceMinCopiesEdit.SetLimitText(7);
	m_wndPriceMaxCopiesEdit.SetLimitText(7);
	m_wndPricePrintEdit.SetLimitText(8);
	m_wndPriceCoverEdit.SetLimitText(8);
	m_wndPriceCoverEdit.SetReadOnly(TRUE);
}

// ── 부수 계산 패널 생성 ───────────────────────────────────────────────────────

void CSageTaechangView::CreatePriceCalcPanel() {
	CRect r(0, 0, 0, 0);
	m_wndCalcCompanyLabel.Create(TAECHANG_UI_CALC_COMPANY_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCompanyCombo.Create(WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL, r, this, ID_CALC_COMPANY_COMBO);
	m_wndCalcCopiesLabel.Create(TAECHANG_UI_CALC_COPIES_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCopiesEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_CALC_COPIES_EDIT);
	m_wndCalcBtn.Create(TAECHANG_UI_CALC_BTN, WS_CHILD | BS_OWNERDRAW, r, this, ID_CALC_BTN);

	m_wndCalcPrintLabel.Create(TAECHANG_UI_CALC_PRINT_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcPrintValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCoverLabel.Create(TAECHANG_UI_CALC_COVER_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcCoverValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcSubtotalLabel.Create(TAECHANG_UI_CALC_SUBTOTAL_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcSubtotalValue.Create(TAECHANG_UI_PRICE_SUMMARY_EMPTY, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcFreightLabel.Create(TAECHANG_UI_CALC_FREIGHT_LABEL, WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, r, this);
	m_wndCalcFreightEdit.Create(WS_CHILD | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL, r, this, ID_CALC_FREIGHT_EDIT);
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
	m_wndCalcHistoryList.InsertColumn(1, TAECHANG_UI_CALC_HIST_COL_COPIES,  LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndCalcHistoryList.InsertColumn(2, TAECHANG_UI_CALC_HIST_COL_PRINT,   LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndCalcHistoryList.InsertColumn(3, TAECHANG_UI_CALC_HIST_COL_COVER,   LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndCalcHistoryList.InsertColumn(4, TAECHANG_UI_CALC_HIST_COL_FREIGHT, LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndCalcHistoryList.InsertColumn(5, TAECHANG_UI_CALC_HIST_COL_TOTAL,   LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndCalcHistoryList.InsertColumn(6, TAECHANG_UI_CALC_HIST_COL_TIME,    LVCFMT_CENTER, TAECHANG_CALC_HIST_COL_TIME_W);

	m_wndCalcCopiesEdit.SetLimitText(7);
	m_wndCalcFreightEdit.SetLimitText(8);
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
	m_wndPriceChangeCoverBtn.MoveWindow(nBtnX, nY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
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

	// 요약 컨트롤
	int nSummaryY = nPanelY;
	m_wndPriceSummaryTitle.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_TITLE_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP * 2;
	m_wndPriceSummaryCount.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);
	nSummaryY += TAECHANG_PRICE_SUMMARY_ROW_HEIGHT + TAECHANG_PRICE_SUMMARY_ROW_GAP;
	m_wndPriceSummaryRange.MoveWindow(nCardInnerX, nSummaryY, nCardInnerW, TAECHANG_PRICE_SUMMARY_ROW_HEIGHT);

	// 편집 폼
	int nFormY = nPanelY;
	int nCheckW = 70;
	int nEditW = nCardInnerW;
	auto ApplyPriceEditTextRect = [](CEdit& edit) {
		CRect rc;
		edit.GetClientRect(&rc);
		rc.left += 2;
		rc.top += 4;
		rc.right -= 2;
		rc.bottom -= 2;
		edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
	};

	m_wndPriceMinCopiesLabel.MoveWindow(nCardInnerX, nFormY, nCardInnerW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndPriceMinCopiesEdit.MoveWindow(nCardInnerX, nFormY, nEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPriceMinCopiesEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	int nMaxLabelW = nCardInnerW - nCheckW - TAECHANG_ACTION_GAP;
	m_wndPriceMaxCopiesLabel.MoveWindow(nCardInnerX, nFormY, nMaxLabelW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	m_wndPriceNoMaxCheck.MoveWindow(nCardInnerX + nMaxLabelW + TAECHANG_ACTION_GAP, nFormY, nCheckW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndPriceMaxCopiesEdit.MoveWindow(nCardInnerX, nFormY, nEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPriceMaxCopiesEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPricePrintLabel.MoveWindow(nCardInnerX, nFormY, nCardInnerW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndPricePrintEdit.MoveWindow(nCardInnerX, nFormY, nEditW, TAECHANG_PRICE_EDIT_HEIGHT);
	ApplyPriceEditTextRect(m_wndPricePrintEdit);
	nFormY += TAECHANG_PRICE_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndPriceCoverLabel.MoveWindow(nCardInnerX, nFormY, nCardInnerW, TAECHANG_PRICE_PANEL_LABEL_HEIGHT);
	nFormY += TAECHANG_PRICE_PANEL_LABEL_HEIGHT + TAECHANG_PRICE_PANEL_LABEL_FIELD_GAP;
	m_wndPriceCoverEdit.MoveWindow(nCardInnerX, nFormY, nEditW, TAECHANG_PRICE_EDIT_HEIGHT);
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
	int nPad   = TAECHANG_CALC_PANEL_PADDING;
	int nLabelW = TAECHANG_CALC_RESULT_LABEL_WIDTH;
	int nValW   = TAECHANG_CALC_RESULT_VALUE_WIDTH;
	int nX = nLeft + TAECHANG_MARGIN;
	int nY = nTop  + TAECHANG_MARGIN;
	int nW = nWidth - TAECHANG_MARGIN * 2;
	int nInputContentW = nLabelW + TAECHANG_LABEL_EDIT_GAP + TAECHANG_CALC_COPIES_EDIT_SHORT_W
		+ TAECHANG_ROW_GAP + TAECHANG_BUTTON_WIDTH;
	int nInputPanelW = nInputContentW + nPad * 2 + TAECHANG_ROW_GAP;
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
	int nComboW = min(TAECHANG_CALC_COMBO_WIDTH, nInputContentW - nLabelW - TAECHANG_LABEL_EDIT_GAP);

	m_wndCalcCompanyLabel.MoveWindow(nCX, nCY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcCompanyCombo.MoveWindow(nCX + nLabelW + TAECHANG_LABEL_EDIT_GAP, nCY, nComboW, TAECHANG_EDIT_HEIGHT * 8);
	nCY += TAECHANG_EDIT_HEIGHT + TAECHANG_ROW_GAP;

	m_wndCalcCopiesLabel.MoveWindow(nCX, nCY, nLabelW, TAECHANG_EDIT_HEIGHT);
	int nCopiesEditX = nCX + nLabelW + TAECHANG_LABEL_EDIT_GAP;
	m_wndCalcCopiesEdit.MoveWindow(nCopiesEditX, nCY, TAECHANG_CALC_COPIES_EDIT_SHORT_W, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCalcCopiesEdit);
	m_wndCalcBtn.MoveWindow(nCopiesEditX + TAECHANG_CALC_COPIES_EDIT_SHORT_W + TAECHANG_ROW_GAP,
		nCY - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);

	nY += nInputPanelH + TAECHANG_CALC_SECTION_GAP;

	// ── 결과 패널 ────────────────────────────────────────────────────────────
	int nRowH    = TAECHANG_EDIT_HEIGHT + TAECHANG_CALC_RESULT_ROW_GAP;
	int nDivH    = 2;
	int nResultPanelH = nPad + nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
	                  + nRowH + nRowH + nDivH + TAECHANG_CALC_RESULT_ROW_GAP
	                  + TAECHANG_EDIT_HEIGHT + nPad;

	int nRX = nX + nPad;
	int nRY = nY + nPad;
	int nValX = nRX + nLabelW + TAECHANG_LABEL_EDIT_GAP;
	int nContentW = nLabelW + TAECHANG_LABEL_EDIT_GAP + nValW;
	int nResultPanelW = nInputPanelW;
	if (nResultPanelW > nW)
		nResultPanelW = nW;
	m_rectCalcResultPanel = CRect(nX, nY, nX + nResultPanelW, nY + nResultPanelH);
	int nFreightUnitW = 28;
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
	m_wndCalcFreightEdit.MoveWindow(nValX, nRY, nFreightEditW, TAECHANG_EDIT_HEIGHT);
	ApplyCalcEditTextRect(m_wndCalcFreightEdit);
	m_wndCalcFreightUnitLabel.MoveWindow(nUnitRightX - nFreightUnitW, nRY, nFreightUnitW, TAECHANG_EDIT_HEIGHT);
	nRY += nRowH;

	m_wndCalcTotalDivider.MoveWindow(nRX, nRY, nContentW, nDivH);
	nRY += nDivH + TAECHANG_CALC_RESULT_ROW_GAP;

	m_wndCalcTotalLabel.MoveWindow(nRX, nRY, nLabelW, TAECHANG_EDIT_HEIGHT);
	m_wndCalcTotalValue.MoveWindow(nValX, nRY, nValW, TAECHANG_EDIT_HEIGHT);

	nY += nResultPanelH + TAECHANG_CALC_SECTION_GAP;

	// ── 이력 섹션 ────────────────────────────────────────────────────────────
	int nHistoryW = TAECHANG_CALC_HIST_COL_COMPANY_W + TAECHANG_CALC_HIST_COL_COPIES_W
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
	m_wndCalcHistoryList.SetColumnWidth(1, TAECHANG_CALC_HIST_COL_COPIES_W);
	m_wndCalcHistoryList.SetColumnWidth(2, TAECHANG_CALC_HIST_COL_PRINT_W);
	m_wndCalcHistoryList.SetColumnWidth(3, TAECHANG_CALC_HIST_COL_COVER_W);
	m_wndCalcHistoryList.SetColumnWidth(4, TAECHANG_CALC_HIST_COL_FREIGHT_W);
	m_wndCalcHistoryList.SetColumnWidth(5, TAECHANG_CALC_HIST_COL_TOTAL_W);
	m_wndCalcHistoryList.SetColumnWidth(6, TAECHANG_CALC_HIST_COL_TIME_W);
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
	m_wndPriceChangeCoverBtn.ShowWindow(nCmd);
	m_wndPriceCopiesList.ShowWindow(nCmd);
	m_wndPriceMinCopiesLabel.ShowWindow(nCmd);
	m_wndPriceMinCopiesEdit.ShowWindow(nCmd);
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
	m_wndCalcCopiesLabel.ShowWindow(nCmd);
	m_wndCalcCopiesEdit.ShowWindow(nCmd);
	m_wndCalcBtn.ShowWindow(nCmd);
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
	if (m_wndPriceCompanyCombo.GetCount() > 0) {
		m_wndPriceCompanyCombo.SetCurSel(0);
		CString strCompany;
		m_wndPriceCompanyCombo.GetLBText(0, strCompany);
		RefreshPriceCopiesList(strCompany);
	} else {
		UpdatePriceSummaryCard();
	}
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
		strPrint.Format(L"%d", dto.nPrintPrice);
		strCover.Format(L"%d", dto.nCoverPrice);
		m_wndPriceCopiesList.SetItemText(nIndex, 2, strPrint);
		m_wndPriceCopiesList.SetItemText(nIndex, 3, strCover);
	}
	UpdatePriceSummaryCard();
}

void CSageTaechangView::UpdatePriceSummaryCard() {
	if (!::IsWindow(m_wndPriceSummaryTitle.GetSafeHwnd()))
		return;
	CString strCompany = GetSelectedCompanyName();
	if (strCompany.IsEmpty()) {
		m_wndPriceSummaryTitle.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_NO_COMPANY);
		m_wndPriceSummaryCount.SetWindowTextW(L"");
		m_wndPriceSummaryRange.SetWindowTextW(L"");
		return;
	}
	m_wndPriceSummaryTitle.SetWindowTextW(strCompany);
	int nCount = m_wndPriceCopiesList.GetItemCount();
	if (nCount == 0) {
		m_wndPriceSummaryCount.SetWindowTextW(TAECHANG_UI_PRICE_SUMMARY_EMPTY);
		m_wndPriceSummaryRange.SetWindowTextW(L"");
		return;
	}
	CString strCount;
	strCount.Format(TAECHANG_UI_PRICE_SUMMARY_COUNT_FMT, nCount);
	m_wndPriceSummaryCount.SetWindowTextW(strCount);
	CString strMinCopies = m_wndPriceCopiesList.GetItemText(0, 0);
	CString strMaxCopies = m_wndPriceCopiesList.GetItemText(nCount - 1, 1);
	CString strRange;
	if (strMaxCopies == TAECHANG_UI_PRICE_MAX_COPIES_NONE)
		strRange.Format(TAECHANG_UI_PRICE_SUMMARY_RANGE_OPEN_FMT, (LPCWSTR)strMinCopies);
	else
		strRange.Format(TAECHANG_UI_PRICE_SUMMARY_RANGE_FMT, (LPCWSTR)strMinCopies, (LPCWSTR)strMaxCopies);
	m_wndPriceSummaryRange.SetWindowTextW(strRange);
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
	m_wndPriceMinCopiesEdit.SetWindowTextW(strMin);
	m_wndPriceNoMaxCheck.SetCheck(bNoMax ? BST_CHECKED : BST_UNCHECKED);
	m_wndPriceMaxCopiesEdit.SetWindowTextW(bNoMax ? CString() : strMax);
	m_wndPriceMaxCopiesEdit.EnableWindow(!bNoMax);
	m_wndPricePrintEdit.SetWindowTextW(strPrint);
	m_wndPriceCoverEdit.SetWindowTextW(strCover);
}

void CSageTaechangView::ClearPriceForm() {
	m_wndPriceMinCopiesEdit.SetWindowTextW(L"");
	m_wndPriceMaxCopiesEdit.SetWindowTextW(L"");
	m_wndPriceMaxCopiesEdit.EnableWindow(TRUE);
	m_wndPriceNoMaxCheck.SetCheck(BST_UNCHECKED);
	m_wndPricePrintEdit.SetWindowTextW(L"");
	m_wndPriceCoverEdit.SetWindowTextW(L"");
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

	dto.bHasMaxCopies = (m_wndPriceNoMaxCheck.GetCheck() == BST_CHECKED) ? FALSE : TRUE;
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

	dto.nPrintPrice = strPrint.IsEmpty() ? 0 : _wtoi(strPrint);
	if (dto.nPrintPrice < 0 || dto.nPrintPrice > TAECHANG_PRICE_AMOUNT_MAX) {
		strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
		return FALSE;
	}

	dto.nCoverPrice = strCover.IsEmpty() ? 0 : _wtoi(strCover);
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
	int nCoverPrice = dlg.GetCoverPrice();
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
	CString strCover;
	strCover.Format(L"%d", nCoverPrice);
	m_wndPriceCoverEdit.SetWindowTextW(strCover);
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

void CSageTaechangView::OnPriceChangeCover() {
	CString strCompany = GetSelectedCompanyName();
	int nIndex = m_wndPriceCompanyCombo.FindStringExact(-1, strCompany);
	if (strCompany.IsEmpty() || nIndex == CB_ERR) {
		AfxMessageBox(TAECHANG_UI_PRICE_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}

	TaechangCoverPriceDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	int nAffectedCount = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->ChangeCoverPriceByCompany(
		strCompany,
		dlg.GetCoverPrice(),
		nAffectedCount,
		strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strCompany);
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
	m_wndPriceMaxCopiesEdit.EnableWindow(!bNoMax);
	if (bNoMax)
		m_wndPriceMaxCopiesEdit.SetWindowTextW(L"");
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

	int nCoverPrice = 0;
	if (m_wndPriceCopiesList.GetItemCount() > 0) {
		nCoverPrice = _wtoi(m_wndPriceCopiesList.GetItemText(0, 3));
	} else {
		CString strCover;
		m_wndPriceCoverEdit.GetWindowTextW(strCover);
		strCover.Trim();
		nCoverPrice = strCover.IsEmpty() ? 0 : _wtoi(strCover);
	}

	TaechangPriceRangeDlg dlg(this);
	if (dlg.DoModal() != IDOK)
		return;

	int nMinCopies = dlg.GetMinCopies();
	BOOL bHasMaxCopies = dlg.HasMaxCopies();
	int nMaxCopies = dlg.GetMaxCopies();
	for (int i = 0; i < m_wndPriceCopiesList.GetItemCount(); ++i) {
		int nExistingMin = _wtoi(m_wndPriceCopiesList.GetItemText(i, 0));
		CString strExistingMax = m_wndPriceCopiesList.GetItemText(i, 1);
		BOOL bExistingHasMax = (strExistingMax == TAECHANG_UI_PRICE_MAX_COPIES_NONE) ? FALSE : TRUE;
		int nExistingMax = bExistingHasMax ? _wtoi(strExistingMax) : 0;
		if (IsCopiesRangeOverlap(nMinCopies, bHasMaxCopies, nMaxCopies, nExistingMin, bExistingHasMax, nExistingMax)) {
			AfxMessageBox(TAECHANG_UI_PRICE_RANGE_OVERLAP, MB_ICONWARNING);
			return;
		}
	}

	TaechangPriceDto dto;
	dto.strCompanyName = strCompany;
	dto.nReportType = REPORT_TYPE_AUDIT_REPORT;
	dto.nMinCopies = nMinCopies;
	dto.bHasMaxCopies = bHasMaxCopies;
	dto.nMaxCopies = nMaxCopies;
	dto.nPrintPrice = dlg.GetPrintPrice();
	dto.nCoverPrice = nCoverPrice;

	int nNewId = 0;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->AddPrice(dto, nNewId, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	m_nPricePanelState = TAECHANG_PRICE_PANEL_SUMMARY;
	RefreshPriceCopiesList(strCompany);
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
	if (m_wndCalcCompanyCombo.GetCount() > 0)
		m_wndCalcCompanyCombo.SetCurSel(0);
}

void CSageTaechangView::UpdateCalcTotal() {
	CString strFreight;
	m_wndCalcFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();
	int nFreight = strFreight.IsEmpty() ? 0 : _wtoi(strFreight);
	if (nFreight < 0) nFreight = 0;
	if (nFreight > TAECHANG_PRICE_AMOUNT_MAX) nFreight = TAECHANG_PRICE_AMOUNT_MAX;
	int nTotal = m_nCalcPrintPrice + m_nCalcCoverPrice + nFreight;
	CString strTotal;
	strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(nTotal).GetString());
	m_wndCalcTotalValue.SetWindowTextW(strTotal);
}

// ── 부수 계산 이벤트 ─────────────────────────────────────────────────────────

void CSageTaechangView::OnCalc() {
	int nSel = m_wndCalcCompanyCombo.GetCurSel();
	if (nSel == CB_ERR) {
		AfxMessageBox(TAECHANG_UI_CALC_SELECT_COMPANY, MB_ICONWARNING);
		return;
	}
	CString strCompany;
	m_wndCalcCompanyCombo.GetLBText(nSel, strCompany);

	CString strCopies;
	m_wndCalcCopiesEdit.GetWindowTextW(strCopies);
	strCopies.Trim();
	if (strCopies.IsEmpty()) {
		AfxMessageBox(TAECHANG_UI_CALC_COPIES_REQUIRED, MB_ICONWARNING);
		return;
	}
	int nCopies = _wtoi(strCopies);
	if (nCopies < 1) {
		AfxMessageBox(TAECHANG_UI_CALC_COPIES_INVALID, MB_ICONWARNING);
		return;
	}

	TaechangPriceDto dto;
	BOOL bFound;
	CString strError;
	if (sageDBMgr.GetTaechangPriceService()->LoadByCompanyAndCopies(strCompany, nCopies, dto, bFound, strError) == FALSE) {
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}
	if (!bFound) {
		AfxMessageBox(TAECHANG_UI_CALC_NO_DATA, MB_ICONWARNING);
		return;
	}

	m_nCalcPrintPrice = dto.nPrintPrice;
	m_nCalcCoverPrice = dto.nCoverPrice;

	CString strPrint, strCover, strSub;
	strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(dto.nPrintPrice).GetString());
	strCover.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(dto.nCoverPrice).GetString());
	strSub.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(dto.nPrintPrice + dto.nCoverPrice).GetString());
	m_wndCalcPrintValue.SetWindowTextW(strPrint);
	m_wndCalcCoverValue.SetWindowTextW(strCover);
	m_wndCalcSubtotalValue.SetWindowTextW(strSub);
	UpdateCalcTotal();

	CString strFreight;
	m_wndCalcFreightEdit.GetWindowTextW(strFreight);
	strFreight.Trim();
	int nFreight = strFreight.IsEmpty() ? 0 : _wtoi(strFreight);
	if (nFreight < 0) nFreight = 0;
	AddCalcHistory(strCompany, nCopies, dto.nPrintPrice, dto.nCoverPrice, nFreight,
		dto.nPrintPrice + dto.nCoverPrice + nFreight);
}

void CSageTaechangView::OnCalcFreightChanged() {
	UpdateCalcTotal();
}

void CSageTaechangView::AddCalcHistory(const CString& strCompany, int nCopies, int nPrintPrice, int nCoverPrice, int nFreight, int nTotal) {
	CalcHistoryEntry entry;
	entry.strCompanyName = strCompany;
	entry.nCopies        = nCopies;
	entry.nPrintPrice    = nPrintPrice;
	entry.nCoverPrice    = nCoverPrice;
	entry.nFreight       = nFreight;
	entry.nTotal         = nTotal;
	entry.timeCalc       = CTime::GetCurrentTime();

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
	m_wndCalcHistoryList.DeleteAllItems();
	for (int i = 0; i < m_arrCalcHistory.GetSize(); ++i) {
		const CalcHistoryEntry& e = m_arrCalcHistory[i];
		m_wndCalcHistoryList.InsertItem(i, e.strCompanyName);

		CString strCopies;
		strCopies.Format(TAECHANG_UI_CALC_HIST_COPIES_FMT, e.nCopies);
		m_wndCalcHistoryList.SetItemText(i, 1, strCopies);

		CString strPrint, strCover, strFreight, strTotal;
		strPrint.Format(TAECHANG_UI_CALC_WON_FORMAT,   FormatPrice(e.nPrintPrice).GetString());
		strCover.Format(TAECHANG_UI_CALC_WON_FORMAT,   FormatPrice(e.nCoverPrice).GetString());
		strFreight.Format(TAECHANG_UI_CALC_WON_FORMAT, FormatPrice(e.nFreight).GetString());
		strTotal.Format(TAECHANG_UI_CALC_WON_FORMAT,   FormatPrice(e.nTotal).GetString());
		m_wndCalcHistoryList.SetItemText(i, 2, strPrint);
		m_wndCalcHistoryList.SetItemText(i, 3, strCover);
		m_wndCalcHistoryList.SetItemText(i, 4, strFreight);
		m_wndCalcHistoryList.SetItemText(i, 5, strTotal);

		CString strTime = e.timeCalc.Format(TAECHANG_UI_CALC_HIST_TIME_FMT);
		m_wndCalcHistoryList.SetItemText(i, 6, strTime);
	}
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


