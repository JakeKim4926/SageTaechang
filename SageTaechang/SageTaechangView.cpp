
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
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct TaechangWorkflowTask
{
    HWND m_hWnd;
    int m_nWorkflowType;
    int m_nTaskType;
    CString m_strInputPath;
    CString m_strOutputFolder;
    CString m_strPdfFilePaths;
    CString m_strHwpFilePaths;
    CString m_strSelectedRowNums;
};

struct TaechangWorkflowResult
{
    int m_nWorkflowType;
    int m_nTaskType;
    CString m_strResponseJson;
};

static CString BuildWorkflowPayload(const CString& strInputPath, const CString& strOutputFolder, const CString& strRowNums)
{
    CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
    if (!strOutputFolder.IsEmpty())
        strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
    if (!strRowNums.IsEmpty())
        strPayload += L",\"" + CString(TAECHANG_JSON_KEY_ROW_NUMS) + L"\":\"" + JsonEscapeString(strRowNums) + L"\"";
    strPayload += L"}";
    return strPayload;
}

static CString BuildComparePayload(const CString& strJsonKey, const CString& strFilePaths)
{
    CString strPayload = L"{\"" + strJsonKey + L"\":[";
    CString strRemaining = strFilePaths;
    int nIndex = 0;
    BOOL bFirst = TRUE;
    while (TRUE)
    {
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

static CString GetTaskRequestId(const TaechangWorkflowTask* pTask)
{
    if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
        return TAECHANG_REQUEST_PDF_COMPARE;
    if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
        return TAECHANG_REQUEST_HWP_COMPARE;
    if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
    {
        if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
            return TAECHANG_REQUEST_ESTIMATE_LOAD;
        return TAECHANG_REQUEST_ESTIMATE_GENERATE;
    }
    if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
    {
        if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
            return TAECHANG_REQUEST_DELIVERY_LOAD;
        return TAECHANG_REQUEST_DELIVERY_GENERATE;
    }
    if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
        return TAECHANG_REQUEST_RECEIVABLES_LOAD;
    return TAECHANG_REQUEST_RECEIVABLES_GENERATE;
}

static UINT RunWorkflowWorker(LPVOID pParam)
{
    TaechangWorkflowTask* pTask = reinterpret_cast<TaechangWorkflowTask*>(pParam);
    TaechangWorkflowResult* pResult = new TaechangWorkflowResult();
    pResult->m_nWorkflowType = pTask->m_nWorkflowType;
    pResult->m_nTaskType = pTask->m_nTaskType;

    try
    {
        CString strPayload;
        if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
            strPayload = BuildComparePayload(L"pdfFilePaths", pTask->m_strPdfFilePaths);
        else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
            strPayload = BuildComparePayload(L"hwpFilePaths", pTask->m_strHwpFilePaths);
        else
            strPayload = BuildWorkflowPayload(pTask->m_strInputPath, pTask->m_strOutputFolder, pTask->m_strSelectedRowNums);
        if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
        {
            TaechangPdfCompareService service;
            pResult->m_strResponseJson = service.BuildRunCompareResponse(TAECHANG_REQUEST_PDF_COMPARE, strPayload);
        }
        else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
        {
            TaechangHwpCompareService service;
            pResult->m_strResponseJson = service.BuildRunCompareResponse(TAECHANG_REQUEST_HWP_COMPARE, strPayload);
        }
        else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
        {
            TaechangEstimateExcelService service;
            if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
                pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_ESTIMATE_LOAD, strPayload);
            else
                pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_ESTIMATE_GENERATE, strPayload);
        }
        else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
        {
            TaechangDeliveryExcelService service;
            if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
                pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_DELIVERY_LOAD, strPayload);
            else
                pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_DELIVERY_GENERATE, strPayload);
        }
        else
        {
            TaechangReceivablesExcelService service;
            if (pTask->m_nTaskType == TAECHANG_TASK_LOAD)
                pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_RECEIVABLES_LOAD, strPayload);
            else
                pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_RECEIVABLES_GENERATE, strPayload);
        }
    }
    catch (...)
    {
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

BEGIN_MESSAGE_MAP(CTaechangHeaderCtrl, CHeaderCtrl)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CTaechangHeaderCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rectClient;
    GetClientRect(&rectClient);
    dc.FillSolidRect(rectClient, TAECHANG_COLOR_LIST_HEADER);

    CFont* pFont = GetFont();
    CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(TAECHANG_COLOR_BUTTON_TEXT);

    int nCount = GetItemCount();
    for (int i = 0; i < nCount; ++i)
    {
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
        if (hdItem.fmt & HDF_RIGHT)
        {
            rcItem.right -= 8;
            uFormat |= DT_RIGHT;
        }
        else
        {
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

void CTaechangTabCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);
    dc.FillSolidRect(rect, TAECHANG_COLOR_APP_BACKGROUND);

    CFont* pFont = GetFont();
    CFont* pOldFont = pFont ? dc.SelectObject(pFont) : NULL;
    dc.SetBkMode(TRANSPARENT);

    int nCount = GetItemCount();
    int nCurSel = GetCurSel();
    for (int i = 0; i < nCount; ++i)
    {
        CRect rcItem;
        GetItemRect(i, &rcItem);
        BOOL bSelected = (i == nCurSel);

        dc.FillSolidRect(rcItem, bSelected ? TAECHANG_COLOR_PANEL : TAECHANG_COLOR_APP_BACKGROUND);

        if (bSelected)
        {
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
    ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
    ON_WM_DROPFILES()
    ON_WM_DRAWITEM()
    ON_NOTIFY(NM_CUSTOMDRAW, ID_TAECHANG_SIDEBAR_TREE, &CSageTaechangView::OnSidebarTreeCustomDraw)
    ON_NOTIFY(NM_CUSTOMDRAW, ID_TAECHANG_RESULT_LIST, &CSageTaechangView::OnListCustomDraw)
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
{
    m_brushAppBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    m_brushSidebar.CreateSolidBrush(TAECHANG_COLOR_SIDEBAR);
    m_brushHeaderStatus.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
}

CSageTaechangView::~CSageTaechangView()
{
}

BOOL CSageTaechangView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

int CSageTaechangView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    DragAcceptFiles(TRUE);
    CreateChildControls();
    SetStatusText(TAECHANG_UI_READY);
    return 0;
}

void CSageTaechangView::CreateChildControls()
{
    CRect rectEmpty(0, 0, 0, 0);
    m_wndSidebarTitle.Create(TAECHANG_UI_SIDEBAR_TITLE, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndSidebarTree.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP | TVS_NOSCROLL, rectEmpty, this, ID_TAECHANG_SIDEBAR_TREE);
    SetWindowTheme(m_wndSidebarTree.GetSafeHwnd(), L"", L"");
    m_wndSidebarTree.SetBkColor(TAECHANG_COLOR_SIDEBAR);
    m_wndSidebarTree.SetTextColor(TAECHANG_COLOR_SIDEBAR_TEXT);
    m_wndSidebarTree.SetItemHeight(TAECHANG_SIDEBAR_ITEM_HEIGHT);
    m_wndHeaderTitle.Create(TAECHANG_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndHeaderStatus.Create(TAECHANG_UI_READY, WS_CHILD | WS_VISIBLE | SS_RIGHT, rectEmpty, this);
    m_wndTaskTabs.Create(WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH, rectEmpty, this, ID_TAECHANG_TASK_TABS);
    m_wndInputSection.Create(TAECHANG_UI_SECTION_INPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_INPUT_SECTION);
    m_wndOutputSection.Create(TAECHANG_UI_SECTION_OUTPUT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_OUTPUT_SECTION);
    m_wndResultSection.Create(TAECHANG_UI_SECTION_RESULT, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_RESULT_SECTION);
    m_wndDetailSection.Create(TAECHANG_UI_SECTION_DETAIL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this, ID_TAECHANG_DETAIL_SECTION);
    m_wndTitle.Create(TAECHANG_UI_APP_TITLE, WS_CHILD | WS_VISIBLE, rectEmpty, this);
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
        if (pHeader && pHeader->GetSafeHwnd())
        {
            m_wndResultHeader.SubclassWindow(pHeader->GetSafeHwnd());
            SetWindowTheme(m_wndResultHeader.GetSafeHwnd(), L"", L"");
        }
    }
    m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);
    m_wndEmptyStateHint.Create(TAECHANG_UI_EMPTY_STATE_HINT, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rectEmpty, this);
    m_wndActionStatus.Create(L"", WS_CHILD | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

    m_wndResultList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    m_wndProgress.SetMarquee(FALSE, 0);
    m_wndProgress.SetRange(0, TAECHANG_PROGRESS_COMPLETE);
    UpdateProgressPercent(0);
    ApplyControlFonts();
    ApplyWorkflowTabs();
    ApplyResultColumns();
    UpdateWorkflowLabels();
    UpdateResultColumns();
    UpdateExportButtonState();
    BuildSidebarTree();
}

void CSageTaechangView::BuildSidebarTree()
{
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

    m_wndSidebarTree.Expand(hDocument, TVE_EXPAND);
    m_wndSidebarTree.Expand(hInspection, TVE_EXPAND);

    m_hLastWorkflowItem = hReceivables;
    m_wndSidebarTree.SelectItem(hReceivables);
}

void CSageTaechangView::ApplyControlFonts()
{
    if (m_fontTitle.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE))
    {
        m_wndTitle.SetFont(&m_fontTitle);
        m_wndSidebarTitle.SetFont(&m_fontTitle);
    }

    if (m_fontHeader.CreatePointFont(TAECHANG_HEADER_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE))
    {
        m_wndHeaderTitle.SetFont(&m_fontHeader);
    }

    if (!m_fontControl.CreatePointFont(TAECHANG_CONTROL_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE))
        return;

    m_wndSidebarTree.SetFont(&m_fontControl);

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
    m_wndDetail.SetFont(&m_fontContent);
    m_wndEmptyStateHint.SetFont(&m_fontContent);
    m_wndActionStatus.SetFont(&m_fontContent);
}

void CSageTaechangView::ApplyWorkflowTabs()
{
    m_wndTaskTabs.DeleteAllItems();
    if (IsCompareWorkflow(GetSelectedWorkflow()))
    {
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_FILES);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_PREVIEW, TAECHANG_UI_TAB_INSPECTION);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_RESULT, TAECHANG_UI_TAB_DETAIL);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DETAIL, TAECHANG_UI_TAB_EXPORT);
    }
    else
    {
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_INPUT, TAECHANG_UI_TAB_INPUT);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DOCUMENT_RESULT, TAECHANG_UI_TAB_RESULT);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DOCUMENT_HISTORY, TAECHANG_UI_TAB_HISTORY);
    }
    m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
    m_wndTaskTabs.SetCurSel(m_nSelectedTaskTab);
    UpdateTaskTabVisibility();
}

void CSageTaechangView::ApplyResultColumns()
{
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
    if (IsReceivablesResultTable())
    {
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
    if (IsDeliveryInputTable())
    {
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
    if (IsEstimateInputTable())
    {
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

void CSageTaechangView::UpdateTaskTabVisibility()
{
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
    m_wndDetailSection.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
    m_wndDetail.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
    m_wndEmptyStateHint.ShowWindow(bShowHint ? SW_SHOW : SW_HIDE);
}

void CSageTaechangView::UpdateResultColumns()
{
    if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
        return;

    CRect rectList;
    m_wndResultList.GetClientRect(&rectList);
    int nWidth = rectList.Width();
    if (nWidth <= 0)
        return;

    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    if (IsReceivablesResultTable())
    {
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
    if (IsDeliveryInputTable())
    {
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
    if (IsEstimateInputTable())
    {
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

void CSageTaechangView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    LayoutChildControls();
}

void CSageTaechangView::LayoutChildControls()
{
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

    m_wndTitle.MoveWindow(TAECHANG_MARGIN, nSidebarTop + TAECHANG_MARGIN, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), TAECHANG_SECTION_TITLE_HEIGHT);
    m_wndSidebarTitle.MoveWindow(TAECHANG_MARGIN, TAECHANG_TOP_BAR_HEIGHT, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), TAECHANG_SIDEBAR_TITLE_HEIGHT);
    m_wndSidebarTree.MoveWindow(TAECHANG_MARGIN, TAECHANG_TOP_BAR_HEIGHT + TAECHANG_SIDEBAR_TITLE_HEIGHT, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), nSidebarHeight - TAECHANG_TOP_BAR_HEIGHT - TAECHANG_SIDEBAR_TITLE_HEIGHT - TAECHANG_MARGIN);

    m_wndHeaderTitle.MoveWindow(nContentLeft, nContentTop, nContentWidth - TAECHANG_HEADER_STATUS_WIDTH, TAECHANG_SECTION_TITLE_HEIGHT);
    m_wndHeaderStatus.MoveWindow(nContentLeft + nContentWidth - TAECHANG_HEADER_STATUS_WIDTH, nContentTop, TAECHANG_HEADER_STATUS_WIDTH, TAECHANG_SECTION_TITLE_HEIGHT);
    nContentTop += TAECHANG_HEADER_HEIGHT;

    m_wndTaskTabs.MoveWindow(nContentLeft, nContentTop, nContentWidth, TAECHANG_TAB_HEIGHT);
    nContentTop += TAECHANG_TAB_HEIGHT + TAECHANG_PANEL_GAP;

    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    if (IsInputTabSelected())
    {
        LayoutInputSection(nContentLeft, nContentTop, nContentWidth, !bIsCompare);
        nContentTop += (bIsCompare ? TAECHANG_INPUT_PANEL_HEIGHT / 2 : TAECHANG_INPUT_PANEL_HEIGHT) + TAECHANG_PANEL_GAP;
    }

    if (IsActionTabVisible() || IsExportTab())
    {
        LayoutActionSection(nContentLeft, nContentTop, nContentWidth);
        nContentTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_PANEL_GAP;
    }

    LayoutResultSection(nContentLeft, nContentTop, nContentWidth, nContentHeight - nContentTop + TAECHANG_MARGIN);
    UpdateTaskTabVisibility();
    UNREFERENCED_PARAMETER(nSidebarLeft);
}

void CSageTaechangView::LayoutInputSection(int nLeft, int nTop, int nWidth, BOOL bShowOutput)
{
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

void CSageTaechangView::LayoutActionSection(int nLeft, int nTop, int nWidth)
{
    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    BOOL bShowAction = IsActionTabVisible();
    BOOL bShowLoad = FALSE;
    BOOL bShowGenerate = bShowAction;
    BOOL bShowExport = IsExportTab();

    int nX = nLeft;
    if (bShowLoad)
    {
        m_wndLoad.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
        nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
    }
    if (bShowGenerate)
    {
        m_wndGenerate.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
        nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
    }
    if (bShowExport)
    {
        m_wndExportCsv.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
        nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
    }
    if (bShowAction)
    {
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

void CSageTaechangView::LayoutResultSection(int nLeft, int nTop, int nWidth, int nHeight)
{
    int nBodyHeight = max(TAECHANG_RESULT_MIN_HEIGHT, nHeight - TAECHANG_RESULT_HEADER_HEIGHT);
    if (IsResultTab() || (IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable())))
    {
        BOOL bShowSelectAll = IsInputTabSelected() && (IsDeliveryInputTable() || IsEstimateInputTable());
        int nSectionWidth = bShowSelectAll ? nWidth - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP : nWidth;
        m_wndResultSection.MoveWindow(nLeft, nTop, nSectionWidth, TAECHANG_RESULT_HEADER_HEIGHT);
        if (bShowSelectAll)
            m_wndSelectAll.MoveWindow(nLeft + nWidth - TAECHANG_BUTTON_WIDTH, nTop - TAECHANG_BUTTON_VERT_ADJUST, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
        m_wndResultList.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
        UpdateResultColumns();
    }
    if (IsDetailTab())
    {
        m_wndDetailSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_RESULT_HEADER_HEIGHT);
        m_wndDetail.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
    }
    m_wndEmptyStateHint.MoveWindow(nLeft, nTop, nWidth, nBodyHeight);
}

void CSageTaechangView::OnDraw(CDC* pDC)
{
    CSageTaechangDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CRect rectClient;
    GetClientRect(&rectClient);
    pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
    pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
    pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
    pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH + 1, TAECHANG_MARGIN + TAECHANG_HEADER_HEIGHT, rectClient.Width() - TAECHANG_SIDEBAR_WIDTH - 1, 1, TAECHANG_COLOR_BORDER);
    DrawEditBorder(pDC, m_wndInputPath);
    DrawEditBorder(pDC, m_wndOutputFolder);
}

void CSageTaechangView::DrawEditBorder(CDC* pDC, CWnd& wnd)
{
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

int CSageTaechangView::GetSelectedWorkflow() const
{
    return m_nCurrentWorkflow;
}

void CSageTaechangView::UpdateWorkflowLabels()
{
    int nWorkflowType = GetSelectedWorkflow();
    if (nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
    {
        m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_HWP_COMPARE_NAME);
        m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INSPECTION_INPUT);
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_HWP_COMPARE_BUTTON);
    }
    else if (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
    {
        m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_PDF_COMPARE_NAME);
        m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INSPECTION_INPUT);
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_PDF_COMPARE_BUTTON);
    }
    else if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
    {
        m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_ESTIMATE_NAME);
        m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INPUT);
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_ESTIMATE_GENERATE_BUTTON);
    }
    else if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
    {
        m_wndHeaderTitle.SetWindowTextW(TAECHANG_UI_DELIVERY_NAME);
        m_wndInputSection.SetWindowTextW(TAECHANG_UI_SECTION_INPUT);
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_DELIVERY_GENERATE_BUTTON);
    }
    else
    {
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

BOOL CSageTaechangView::IsCompareWorkflow(int nWorkflowType) const
{
    return (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE || nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsInputTabSelected() const
{
    return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_INPUT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsResultTab() const
{
    if (IsCompareWorkflow(GetSelectedWorkflow()))
        return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_PREVIEW) ? TRUE : FALSE;
    return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_RESULT) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDetailTab() const
{
    if (IsCompareWorkflow(GetSelectedWorkflow()))
        return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_RESULT) ? TRUE : FALSE;
    return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DOCUMENT_HISTORY) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsExportTab() const
{
    return (IsCompareWorkflow(GetSelectedWorkflow()) && m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DETAIL) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsActionTabVisible() const
{
    return IsInputTabSelected() ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsReceivablesResultTable() const
{
    if (m_nLastWorkflowType != TAECHANG_WORKFLOW_RECEIVABLES)
        return FALSE;
    if (m_nLastTaskType == TAECHANG_TASK_LOAD)
        return TRUE;
    return (m_nLastTaskType == TAECHANG_TASK_GENERATE) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsDeliveryInputTable() const
{
    if (m_nLastWorkflowType != TAECHANG_WORKFLOW_DELIVERY)
        return FALSE;
    return (m_nLastTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsEstimateInputTable() const
{
    if (m_nLastWorkflowType != TAECHANG_WORKFLOW_ESTIMATE)
        return FALSE;
    return (m_nLastTaskType == TAECHANG_TASK_LOAD) ? TRUE : FALSE;
}

void CSageTaechangView::UpdateExportButtonState()
{
    BOOL bEnabled = (!m_bRunning && IsCompareWorkflow(GetSelectedWorkflow()) && !m_strLastResponseJson.IsEmpty()) ? TRUE : FALSE;
    if (::IsWindow(m_wndExportCsv.GetSafeHwnd()))
        m_wndExportCsv.EnableWindow(bEnabled);
}

void CSageTaechangView::OnWorkflowChanged()
{
    m_strLastResponseJson.Empty();
    m_nLastWorkflowType = 0;
    m_nLastTaskType = 0;
    m_strRunningInputPath.Empty();
    m_wndInputPath.SetWindowTextW(L"");
    m_wndOutputFolder.SetWindowTextW(L"");
    UpdateWorkflowLabels();
    UpdateExportButtonState();
    UpdateResultColumns();
    if (!m_bRunning)
        SetStatusText(TAECHANG_UI_READY);
}

void CSageTaechangView::OnSidebarSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    *pResult = 0;
    HTREEITEM hItem = m_wndSidebarTree.GetSelectedItem();
    if (hItem == NULL)
        return;
    DWORD_PTR nItemData = m_wndSidebarTree.GetItemData(hItem);
    if (nItemData == TAECHANG_SIDEBAR_ACTION_NONE)
        return;
    if (nItemData == TAECHANG_WORKFLOW_PDF_COMPARE || nItemData == TAECHANG_WORKFLOW_HWP_COMPARE)
    {
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

void CSageTaechangView::OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    m_nSelectedTaskTab = m_wndTaskTabs.GetCurSel();
    LayoutChildControls();
    Invalidate();
    *pResult = 0;
}

void CSageTaechangView::OnDropFiles(HDROP hDropInfo)
{
    UINT nFileCount = DragQueryFileW(hDropInfo, 0xFFFFFFFF, NULL, 0);
    if (nFileCount == 0 || m_bRunning)
    {
        DragFinish(hDropInfo);
        return;
    }

    int nWorkflowType = GetSelectedWorkflow();
    BOOL bIsCompare = IsCompareWorkflow(nWorkflowType);
    CString strPaths;
    for (UINT i = 0; i < nFileCount; ++i)
    {
        wchar_t szPath[MAX_PATH] = {};
        DragQueryFileW(hDropInfo, i, szPath, MAX_PATH);
        if (!strPaths.IsEmpty())
            strPaths += L"\r\n";
        strPaths += szPath;
        if (!bIsCompare)
            break;
    }
    DragFinish(hDropInfo);

    m_wndInputPath.SetWindowTextW(strPaths);

    if (!IsInputTabSelected())
    {
        m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
        m_wndTaskTabs.SetCurSel(m_nSelectedTaskTab);
        LayoutChildControls();
    }
    if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
        RunWorkflowTask(TAECHANG_TASK_LOAD);
}

void CSageTaechangView::OnSelectInput()
{
    int nWorkflowType = GetSelectedWorkflow();
    if (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE || nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
    {
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
        if (dlg.DoModal() == IDOK)
        {
            POSITION pos = dlg.GetStartPosition();
            CString strPaths;
            while (pos != NULL)
            {
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
    if (dlg.DoModal() == IDOK)
    {
        m_wndInputPath.SetWindowTextW(dlg.GetPathName());
        if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
            RunWorkflowTask(TAECHANG_TASK_LOAD);
    }
}

void CSageTaechangView::OnSelectOutput()
{
    CFolderPickerDialog dlg(NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, this, 0);
    dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_OUTPUT_TITLE;
    if (dlg.DoModal() == IDOK)
        m_wndOutputFolder.SetWindowTextW(dlg.GetPathName());
}

BOOL CSageTaechangView::ValidateInputPath(CString& strInputPath)
{
    m_wndInputPath.GetWindowTextW(strInputPath);
    strInputPath.Trim();
    if (strInputPath.IsEmpty())
    {
        AfxMessageBox(TAECHANG_UI_INPUT_REQUIRED, MB_ICONWARNING);
        return FALSE;
    }
    return TRUE;
}

BOOL CSageTaechangView::ValidateOutputFolder(CString& strOutputFolder)
{
    m_wndOutputFolder.GetWindowTextW(strOutputFolder);
    strOutputFolder.Trim();
    if (strOutputFolder.IsEmpty())
    {
        AfxMessageBox(TAECHANG_UI_OUTPUT_REQUIRED, MB_ICONWARNING);
        return FALSE;
    }
    return TRUE;
}

void CSageTaechangView::OnLoadWorkflow()
{
    RunWorkflowTask(TAECHANG_TASK_LOAD);
}

void CSageTaechangView::OnGenerateWorkflow()
{
    RunWorkflowTask(TAECHANG_TASK_GENERATE);
}

void CSageTaechangView::OnExportCsv()
{
    if (m_strLastResponseJson.IsEmpty() || !IsCompareWorkflow(m_nLastWorkflowType))
    {
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
    if (!service.ExportCompareResult(m_strLastResponseJson, strPath, strError))
    {
        AfxMessageBox(strError, MB_ICONERROR);
        return;
    }

    SetStatusText(TAECHANG_UI_EXPORT_COMPLETED);
}

void CSageTaechangView::OnSelectAll()
{
    int nCount = m_wndResultList.GetItemCount();
    BOOL bAllChecked = TRUE;
    for (int i = 0; i < nCount; ++i)
    {
        if (!m_wndResultList.GetCheck(i))
        {
            bAllChecked = FALSE;
            break;
        }
    }
    BOOL bCheck = bAllChecked ? FALSE : TRUE;
    for (int i = 0; i < nCount; ++i)
        m_wndResultList.SetCheck(i, bCheck);
}

void CSageTaechangView::RunWorkflowTask(int nTaskType)
{
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
    if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) && nTaskType == TAECHANG_TASK_GENERATE)
    {
        int nListCount = m_wndResultList.GetItemCount();
        for (int i = 0; i < nListCount; ++i)
        {
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
        if (strSelectedRowNums.IsEmpty())
        {
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

void CSageTaechangView::SetRunningState(BOOL bRunning)
{
    m_bRunning = bRunning;
    m_wndSidebarTree.EnableWindow(!bRunning);
    m_wndSelectInput.EnableWindow(!bRunning);
    m_wndSelectOutput.EnableWindow(!bRunning);
    m_wndLoad.EnableWindow(!bRunning);
    m_wndGenerate.EnableWindow(!bRunning);
    m_wndSelectAll.EnableWindow(!bRunning);
    if (bRunning)
    {
        UpdateProgressPercent(0);
        SetTimer(ID_TAECHANG_PROGRESS_TIMER, TAECHANG_PROGRESS_TIMER_MS, NULL);
    }
    else
    {
        KillTimer(ID_TAECHANG_PROGRESS_TIMER);
        UpdateProgressPercent(TAECHANG_PROGRESS_COMPLETE);
    }
    UpdateExportButtonState();
    UpdateTaskTabVisibility();
    if (bRunning)
        SetStatusText(TAECHANG_UI_RUNNING);
}

void CSageTaechangView::UpdateProgressPercent(int nPercent)
{
    m_nProgressPercent = nPercent;
    m_wndProgress.SetPos(m_nProgressPercent);
    CString strProgress;
    strProgress.Format(TAECHANG_UI_PROGRESS_FORMAT, m_nProgressPercent);
    m_wndProgressText.SetWindowTextW(strProgress);
}

void CSageTaechangView::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == ID_TAECHANG_PROGRESS_TIMER)
    {
        if (m_bRunning && m_nProgressPercent < TAECHANG_PROGRESS_RUNNING_MAX)
        {
            int nNextPercent = m_nProgressPercent + TAECHANG_PROGRESS_STEP;
            if (nNextPercent > TAECHANG_PROGRESS_RUNNING_MAX)
                nNextPercent = TAECHANG_PROGRESS_RUNNING_MAX;
            UpdateProgressPercent(nNextPercent);
        }
        return;
    }
    CView::OnTimer(nIDEvent);
}

BOOL CSageTaechangView::OnEraseBkgnd(CDC* pDC)
{
    CRect rectClient;
    GetClientRect(&rectClient);
    pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
    pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
    pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH, 0, 1, rectClient.Height(), TAECHANG_COLOR_BORDER);
    pDC->FillSolidRect(TAECHANG_SIDEBAR_WIDTH + 1, TAECHANG_MARGIN + TAECHANG_HEADER_HEIGHT, rectClient.Width() - TAECHANG_SIDEBAR_WIDTH - 1, 1, TAECHANG_COLOR_BORDER);
    DrawEditBorder(pDC, m_wndInputPath);
    DrawEditBorder(pDC, m_wndOutputFolder);
    return TRUE;
}

HBRUSH CSageTaechangView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hBrush = CView::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetTextColor(TAECHANG_COLOR_TEXT);
    if (pWnd->GetSafeHwnd() == m_wndSidebarTitle.GetSafeHwnd() ||
        pWnd->GetSafeHwnd() == m_wndTitle.GetSafeHwnd())
    {
        pDC->SetTextColor(TAECHANG_COLOR_SIDEBAR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_SIDEBAR);
        return m_brushSidebar;
    }
    if (pWnd->GetSafeHwnd() == m_wndHeaderTitle.GetSafeHwnd())
    {
        pDC->SetTextColor(TAECHANG_COLOR_PRIMARY);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushAppBackground;
    }
    if (pWnd->GetSafeHwnd() == m_wndHeaderStatus.GetSafeHwnd())
    {
        pDC->SetTextColor(m_colorHeaderStatus);
        pDC->SetBkColor(m_colorHeaderStatusBg);
        return m_brushHeaderStatus;
    }
    if (pWnd->GetSafeHwnd() == m_wndEmptyStateHint.GetSafeHwnd())
    {
        pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushAppBackground;
    }
    if (pWnd->GetSafeHwnd() == m_wndActionStatus.GetSafeHwnd())
    {
        pDC->SetTextColor(m_bLastTaskSuccess ? TAECHANG_COLOR_SUCCESS : TAECHANG_COLOR_ERROR);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushAppBackground;
    }
    if (nCtlColor == CTLCOLOR_STATIC)
    {
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushAppBackground;
    }
    if (nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX)
    {
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }
    return hBrush;
}

void CSageTaechangView::SetStatusText(const CString& strStatus)
{
    CFrameWnd* pFrame = GetParentFrame();
    if (pFrame != NULL)
        pFrame->SetMessageText(strStatus);

    if (::IsWindow(m_wndHeaderStatus.GetSafeHwnd()))
    {
        m_colorHeaderStatus = ResolveStatusColor(strStatus);
        m_colorHeaderStatusBg = ResolveStatusBgColor(strStatus);
        m_brushHeaderStatus.DeleteObject();
        m_brushHeaderStatus.CreateSolidBrush(m_colorHeaderStatusBg);
        m_wndHeaderStatus.SetWindowTextW(strStatus);
        m_wndHeaderStatus.Invalidate();
    }
}

COLORREF CSageTaechangView::ResolveStatusColor(const CString& strStatus) const
{
    if (strStatus == TAECHANG_UI_RUNNING)
        return TAECHANG_COLOR_PRIMARY;
    if (strStatus == TAECHANG_UI_COMPLETED ||
        strStatus == TAECHANG_UI_EXPORT_COMPLETED)
        return TAECHANG_COLOR_SUCCESS;
    if (strStatus == TAECHANG_UI_FAILED)
        return TAECHANG_COLOR_ERROR;
    return TAECHANG_COLOR_SECONDARY_TEXT;
}

COLORREF CSageTaechangView::ResolveStatusBgColor(const CString& strStatus) const
{
    if (strStatus == TAECHANG_UI_COMPLETED || strStatus == TAECHANG_UI_EXPORT_COMPLETED)
        return TAECHANG_COLOR_STATUS_BG_SUCCESS;
    if (strStatus == TAECHANG_UI_RUNNING)
        return TAECHANG_COLOR_STATUS_BG_WARNING;
    if (strStatus == TAECHANG_UI_FAILED)
        return TAECHANG_COLOR_STATUS_BG_ERROR;
    return TAECHANG_COLOR_APP_BACKGROUND;
}

LRESULT CSageTaechangView::OnWorkflowComplete(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    TaechangWorkflowResult* pResult = reinterpret_cast<TaechangWorkflowResult*>(lParam);
    if (pResult != NULL)
    {
        DisplayResponse(pResult->m_nWorkflowType, pResult->m_nTaskType, pResult->m_strResponseJson);
        delete pResult;
    }
    SetRunningState(FALSE);
    return 0;
}

void CSageTaechangView::DisplayResponse(int nWorkflowType, int nTaskType, const CString& strResponseJson)
{
    m_wndResultList.DeleteAllItems();
    m_nLastWorkflowType = nWorkflowType;
    m_nLastTaskType = nTaskType;
    m_strLastResponseJson = strResponseJson;
    ApplyResultColumns();
    UpdateResultColumns();

    TaechangWorkflowResultPresenter presenter;
    std::vector<TaechangResultRow> arrRows;
    CString strDetailText;
    BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows, strDetailText);
    AppendExecutionHistory(nWorkflowType, nTaskType, strResponseJson, bSuccess);
    if (IsCompareWorkflow(nWorkflowType))
        m_wndDetail.SetWindowTextW(strDetailText);
    else
        m_wndDetail.SetWindowTextW(m_strExecutionHistory);

    for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
        InsertResultRow(arrRows[i]);

    if ((nWorkflowType == TAECHANG_WORKFLOW_DELIVERY || nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE) && nTaskType == TAECHANG_TASK_LOAD)
    {
        m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
        m_wndTaskTabs.SetCurSel(m_nSelectedTaskTab);
        UpdateTaskTabVisibility();
        LayoutChildControls();
    }
    else if ((nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES && nTaskType == TAECHANG_TASK_GENERATE) ||
        (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY && nTaskType == TAECHANG_TASK_GENERATE) ||
        (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE && nTaskType == TAECHANG_TASK_GENERATE) ||
        nTaskType == TAECHANG_TASK_LOAD)
    {
        m_nSelectedTaskTab = TAECHANG_TAB_INDEX_DOCUMENT_RESULT;
        m_wndTaskTabs.SetCurSel(m_nSelectedTaskTab);
        UpdateTaskTabVisibility();
        LayoutChildControls();
    }

    m_bLastTaskSuccess = bSuccess;
    m_wndActionStatus.SetWindowTextW(bSuccess ? TAECHANG_UI_ACTION_STATUS_COMPLETED : TAECHANG_UI_ACTION_STATUS_FAILED);
    m_wndActionStatus.Invalidate();
    SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
    UpdateExportButtonState();
}

void CSageTaechangView::InsertResultRow(const TaechangResultRow& row)
{
    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    int nCount = m_wndResultList.GetItemCount();
    int nCol = 0;
    int nIndex;
    if (IsReceivablesResultTable())
    {
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
    if (IsDeliveryInputTable())
    {
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
    if (IsEstimateInputTable())
    {
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
    if (bIsCompare)
    {
        nIndex = m_wndResultList.InsertItem(nCount, row.m_strFile);
        ++nCol;
        m_wndResultList.SetItemText(nIndex, nCol++, row.m_strField);
    }
    else
    {
        nIndex = m_wndResultList.InsertItem(nCount, row.m_strField);
        ++nCol;
    }
    m_wndResultList.SetItemText(nIndex, nCol++, row.m_strValue);
    m_wndResultList.SetItemText(nIndex, nCol++, row.m_strStatus);
    m_wndResultList.SetItemText(nIndex, nCol++, row.m_strReason);
    m_wndResultList.SetItemData(nIndex, static_cast<DWORD_PTR>(row.m_nSourceRowIndex));
}

void CSageTaechangView::AppendExecutionHistory(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess)
{
    CString strLine = BuildExecutionHistoryLine(nWorkflowType, nTaskType, strResponseJson, bSuccess);
    if (strLine.IsEmpty())
        return;

    if (!m_strExecutionHistory.IsEmpty())
        m_strExecutionHistory += TAECHANG_UI_HISTORY_ENTRY_BREAK;
    m_strExecutionHistory += strLine;
}

CString CSageTaechangView::BuildExecutionHistoryLine(int nWorkflowType, int nTaskType, const CString& strResponseJson, BOOL bSuccess) const
{
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

    if (bSuccess)
    {
        CString strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_FILE_PATH);
        if (strOutputPath.IsEmpty())
            strOutputPath = JsonExtractString(strResponseJson, TAECHANG_JSON_KEY_OUTPUT_FOLDER);
        if (strOutputPath.IsEmpty())
            strOutputPath = TAECHANG_UI_HISTORY_EMPTY_VALUE;
        strLine += TAECHANG_UI_HISTORY_LINE_BREAK;
        strLine += TAECHANG_UI_HISTORY_FIELD_INDENT;
        strLine += TAECHANG_UI_HISTORY_OUTPUT_PREFIX;
        strLine += strOutputPath;
    }
    else
    {
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

void CSageTaechangView::DrawSectionLabel(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
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

void CSageTaechangView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (lpDrawItemStruct->CtlType == ODT_STATIC &&
        (nIDCtl == ID_TAECHANG_INPUT_SECTION || nIDCtl == ID_TAECHANG_OUTPUT_SECTION ||
         nIDCtl == ID_TAECHANG_RESULT_SECTION || nIDCtl == ID_TAECHANG_DETAIL_SECTION))
    {
        DrawSectionLabel(lpDrawItemStruct);
        return;
    }
    if (lpDrawItemStruct->CtlType != ODT_BUTTON)
    {
        CView::OnDrawItem(nIDCtl, lpDrawItemStruct);
        return;
    }

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rect = lpDrawItemStruct->rcItem;
    BOOL bPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
    BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;

    BOOL bPrimary = (nIDCtl == ID_TAECHANG_GENERATE_WORKFLOW || nIDCtl == ID_TAECHANG_LOAD_WORKFLOW);

    if (bPrimary)
    {
        COLORREF clrBg = bDisabled ? TAECHANG_COLOR_BORDER
            : bPressed ? TAECHANG_COLOR_PRIMARY_PRESS : TAECHANG_COLOR_PRIMARY;
        pDC->FillSolidRect(rect, clrBg);
        pDC->SetTextColor(bDisabled ? TAECHANG_COLOR_SECONDARY_TEXT : TAECHANG_COLOR_BUTTON_TEXT);
    }
    else
    {
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
    CFont* pOldFont = pDC->SelectObject(&m_fontContent);
    rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
    pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (pOldFont)
        pDC->SelectObject(pOldFont);
}

void CSageTaechangView::OnSidebarTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMTVCUSTOMDRAW* pCD = reinterpret_cast<NMTVCUSTOMDRAW*>(pNMHDR);
    *pResult = CDRF_DODEFAULT;
    switch (pCD->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
        pCD->clrText = TAECHANG_COLOR_SIDEBAR_TEXT;
        pCD->clrTextBk = (pCD->nmcd.uItemState & CDIS_SELECTED)
            ? TAECHANG_COLOR_SIDEBAR_SELECTED
            : TAECHANG_COLOR_SIDEBAR;
        *pResult = CDRF_NEWFONT;
        break;
    }
}

void CSageTaechangView::OnListCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVCUSTOMDRAW* pCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    switch (pCD->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
    {
        int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
        UINT uState = ListView_GetItemState(pCD->nmcd.hdr.hwndFrom, nItem, LVIS_SELECTED);
        if (!(uState & LVIS_SELECTED))
        {
            pCD->clrTextBk = (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_ROW_ALT : TAECHANG_COLOR_PANEL;
            pCD->clrText = TAECHANG_COLOR_TEXT;
            *pResult = CDRF_NEWFONT;
            if (IsReceivablesResultTable())
                *pResult |= CDRF_NOTIFYSUBITEMDRAW;
        }
        break;
    }
    case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
    {
        int nItem = static_cast<int>(pCD->nmcd.dwItemSpec);
        int nSubItem = pCD->iSubItem;
        UINT uState = ListView_GetItemState(pCD->nmcd.hdr.hwndFrom, nItem, LVIS_SELECTED);
        if (!(uState & LVIS_SELECTED) &&
            (nSubItem == TAECHANG_RECEIVABLES_COL_IDX_TOTAL_AMOUNT ||
             nSubItem == TAECHANG_RECEIVABLES_COL_IDX_DEPOSIT_AMOUNT ||
             nSubItem == TAECHANG_RECEIVABLES_COL_IDX_RECEIVABLE_AMOUNT))
        {
            pCD->clrTextBk = (nItem % 2 == 1) ? TAECHANG_COLOR_LIST_AMOUNT_COL_ALT : TAECHANG_COLOR_LIST_AMOUNT_COL;
            pCD->clrText = TAECHANG_COLOR_TEXT;
            *pResult = CDRF_NEWFONT;
        }
        break;
    }
    }
}


#ifdef _DEBUG
void CSageTaechangView::AssertValid() const
{
    CView::AssertValid();
}

void CSageTaechangView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CSageTaechangDoc* CSageTaechangView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSageTaechangDoc)));
    return (CSageTaechangDoc*)m_pDocument;
}
#endif


