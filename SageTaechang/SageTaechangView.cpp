
// SageTaechangView.cpp: CSageTaechangView 클래스의 구현
//

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
#include "app/presentation/TaechangWorkflowResultPresenter.h"

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
};

struct TaechangWorkflowResult
{
    int m_nWorkflowType;
    int m_nTaskType;
    CString m_strResponseJson;
};

static CString BuildWorkflowPayload(const CString& strInputPath, const CString& strOutputFolder)
{
    CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
    if (!strOutputFolder.IsEmpty())
        strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
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

static UINT RunWorkflowWorker(LPVOID pParam)
{
    TaechangWorkflowTask* pTask = reinterpret_cast<TaechangWorkflowTask*>(pParam);
    TaechangWorkflowResult* pResult = new TaechangWorkflowResult();
    pResult->m_nWorkflowType = pTask->m_nWorkflowType;
    pResult->m_nTaskType = pTask->m_nTaskType;

    CString strPayload;
    if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
        strPayload = BuildComparePayload(L"pdfFilePaths", pTask->m_strPdfFilePaths);
    else if (pTask->m_nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
        strPayload = BuildComparePayload(L"hwpFilePaths", pTask->m_strHwpFilePaths);
    else
        strPayload = BuildWorkflowPayload(pTask->m_strInputPath, pTask->m_strOutputFolder);
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

    HWND hWnd = pTask->m_hWnd;
    delete pTask;

    if (::IsWindow(hWnd))
        ::PostMessageW(hWnd, WM_TAECHANG_WORKFLOW_COMPLETE, 0, reinterpret_cast<LPARAM>(pResult));
    else
        delete pResult;

    return 0;
}

IMPLEMENT_DYNCREATE(CSageTaechangView, CView)

BEGIN_MESSAGE_MAP(CSageTaechangView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
    ON_LBN_SELCHANGE(ID_TAECHANG_WORKFLOW_MENU, &CSageTaechangView::OnWorkflowChanged)
    ON_NOTIFY(TCN_SELCHANGE, ID_TAECHANG_TASK_TABS, &CSageTaechangView::OnTaskTabChanged)
    ON_BN_CLICKED(ID_TAECHANG_SELECT_INPUT, &CSageTaechangView::OnSelectInput)
    ON_BN_CLICKED(ID_TAECHANG_SELECT_OUTPUT, &CSageTaechangView::OnSelectOutput)
    ON_BN_CLICKED(ID_TAECHANG_LOAD_WORKFLOW, &CSageTaechangView::OnLoadWorkflow)
    ON_BN_CLICKED(ID_TAECHANG_GENERATE_WORKFLOW, &CSageTaechangView::OnGenerateWorkflow)
    ON_BN_CLICKED(ID_TAECHANG_EXPORT_CSV, &CSageTaechangView::OnExportCsv)
    ON_BN_CLICKED(ID_TAECHANG_SETTINGS, &CSageTaechangView::OnSettings)
    ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
    : m_bRunning(FALSE)
    , m_nSelectedTaskTab(TAECHANG_TAB_INDEX_INPUT)
    , m_nLastWorkflowType(0)
    , m_nLastTaskType(0)
    , m_colorHeaderStatus(TAECHANG_COLOR_SECONDARY_TEXT)
{
    m_brushAppBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    m_brushSidebar.CreateSolidBrush(TAECHANG_COLOR_SIDEBAR);
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

    CreateChildControls();
    SetStatusText(TAECHANG_UI_READY);
    return 0;
}

void CSageTaechangView::CreateChildControls()
{
    CRect rectEmpty(0, 0, 0, 0);
    m_wndSidebarTitle.Create(TAECHANG_UI_SIDEBAR_TITLE, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndWorkflowMenu.Create(WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_BORDER, rectEmpty, this, ID_TAECHANG_WORKFLOW_MENU);
    m_wndWorkflowMenu.AddString(TAECHANG_UI_RECEIVABLES_NAME);
    m_wndWorkflowMenu.AddString(TAECHANG_UI_DELIVERY_NAME);
    m_wndWorkflowMenu.AddString(TAECHANG_UI_ESTIMATE_NAME);
    m_wndWorkflowMenu.AddString(TAECHANG_UI_PDF_COMPARE_NAME);
    m_wndWorkflowMenu.AddString(TAECHANG_UI_HWP_COMPARE_NAME);
    m_wndWorkflowMenu.SetCurSel(0);
    m_wndHeaderTitle.Create(TAECHANG_UI_RECEIVABLES_NAME, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndHeaderStatus.Create(TAECHANG_UI_READY, WS_CHILD | WS_VISIBLE | SS_RIGHT, rectEmpty, this);
    m_wndTaskTabs.Create(WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH, rectEmpty, this, ID_TAECHANG_TASK_TABS);
    m_wndInputSection.Create(TAECHANG_UI_SECTION_INPUT, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndOutputSection.Create(TAECHANG_UI_SECTION_OUTPUT, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndResultSection.Create(TAECHANG_UI_SECTION_RESULT, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndDetailSection.Create(TAECHANG_UI_SECTION_DETAIL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndTitle.Create(TAECHANG_UI_APP_TITLE, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndWorkflowLabel.Create(TAECHANG_UI_WORKFLOW_LABEL, WS_CHILD, rectEmpty, this);
    m_wndInputLabel.Create(TAECHANG_UI_INPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndOutputLabel.Create(TAECHANG_UI_OUTPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_INPUT_EDIT);
    m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_OUTPUT_EDIT);
    m_wndSelectInput.Create(TAECHANG_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_INPUT);
    m_wndSelectOutput.Create(TAECHANG_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_OUTPUT);
    m_wndLoad.Create(TAECHANG_UI_LOAD_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_LOAD_WORKFLOW);
    m_wndGenerate.Create(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_GENERATE_WORKFLOW);
    m_wndExportCsv.Create(TAECHANG_UI_EXPORT_CSV_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_EXPORT_CSV);
    m_wndSettings.Create(TAECHANG_UI_SETTINGS_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SETTINGS);
    m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, rectEmpty, this, ID_TAECHANG_PROGRESS);
    m_wndResultList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, rectEmpty, this, ID_TAECHANG_RESULT_LIST);
    m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);

    m_wndProgress.SetMarquee(FALSE, 0);
    ApplyControlFonts();
    ApplyWorkflowTabs();
    ApplyResultColumns();
    UpdateWorkflowLabels();
    UpdateResultColumns();
    UpdateExportButtonState();
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

    m_wndWorkflowMenu.SetFont(&m_fontControl);
    m_wndHeaderStatus.SetFont(&m_fontControl);
    m_wndTaskTabs.SetFont(&m_fontControl);
    m_wndInputSection.SetFont(&m_fontControl);
    m_wndOutputSection.SetFont(&m_fontControl);
    m_wndResultSection.SetFont(&m_fontControl);
    m_wndDetailSection.SetFont(&m_fontControl);
    m_wndWorkflowLabel.SetFont(&m_fontControl);
    m_wndInputLabel.SetFont(&m_fontControl);
    m_wndOutputLabel.SetFont(&m_fontControl);
    m_wndInputPath.SetFont(&m_fontControl);
    m_wndOutputFolder.SetFont(&m_fontControl);
    m_wndSelectInput.SetFont(&m_fontControl);
    m_wndSelectOutput.SetFont(&m_fontControl);
    m_wndLoad.SetFont(&m_fontControl);
    m_wndGenerate.SetFont(&m_fontControl);
    m_wndExportCsv.SetFont(&m_fontControl);
    m_wndSettings.SetFont(&m_fontControl);
    m_wndResultList.SetFont(&m_fontControl);
    m_wndDetail.SetFont(&m_fontControl);
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
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_PREVIEW, TAECHANG_UI_TAB_PREVIEW);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_RESULT, TAECHANG_UI_TAB_RESULT);
        m_wndTaskTabs.InsertItem(TAECHANG_TAB_INDEX_DETAIL, TAECHANG_UI_TAB_HISTORY);
    }
    m_nSelectedTaskTab = TAECHANG_TAB_INDEX_INPUT;
    m_wndTaskTabs.SetCurSel(m_nSelectedTaskTab);
    UpdateTaskTabVisibility();
}

void CSageTaechangView::ApplyResultColumns()
{
    if (!::IsWindow(m_wndResultList.GetSafeHwnd()))
        return;

    m_wndResultList.DeleteAllItems();
    CHeaderCtrl* pHeader = m_wndResultList.GetHeaderCtrl();
    int nColumnCount = (pHeader != NULL) ? pHeader->GetItemCount() : 0;
    for (int i = nColumnCount - 1; i >= 0; --i)
        m_wndResultList.DeleteColumn(i);

    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    int nIndex = 0;
    if (bIsCompare)
        m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_FILENAME, LVCFMT_LEFT, TAECHANG_RESULT_FILE_WIDTH);
    m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_FIELD, LVCFMT_LEFT, TAECHANG_RESULT_FIELD_WIDTH);
    m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_VALUE, LVCFMT_LEFT, TAECHANG_RESULT_MIN_VALUE_WIDTH);
    m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_STATUS, LVCFMT_LEFT, TAECHANG_RESULT_STATUS_WIDTH);
    m_wndResultList.InsertColumn(nIndex++, TAECHANG_UI_RESULT_REASON, LVCFMT_LEFT, TAECHANG_RESULT_REASON_WIDTH);
}

void CSageTaechangView::UpdateTaskTabVisibility()
{
    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    BOOL bShowInput = IsInputTabSelected();
    BOOL bShowOutput = (bShowInput && !bIsCompare) ? TRUE : FALSE;
    BOOL bShowAction = IsActionTabVisible();
    BOOL bShowResult = IsResultTab();
    BOOL bShowDetail = IsDetailTab();
    BOOL bShowExport = IsExportTab();
    BOOL bShowSettings = IsSettingsButtonVisible();

    m_wndInputSection.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
    m_wndInputLabel.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
    m_wndInputPath.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
    m_wndSelectInput.ShowWindow(bShowInput ? SW_SHOW : SW_HIDE);
    m_wndOutputSection.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
    m_wndOutputLabel.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
    m_wndOutputFolder.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);
    m_wndSelectOutput.ShowWindow(bShowOutput ? SW_SHOW : SW_HIDE);

    m_wndLoad.ShowWindow((bShowAction && !bIsCompare) ? SW_SHOW : SW_HIDE);
    m_wndGenerate.ShowWindow(bShowAction ? SW_SHOW : SW_HIDE);
    m_wndExportCsv.ShowWindow(bShowExport ? SW_SHOW : SW_HIDE);
    m_wndSettings.ShowWindow(bShowSettings ? SW_SHOW : SW_HIDE);
    m_wndProgress.ShowWindow(bShowAction ? SW_SHOW : SW_HIDE);

    m_wndResultSection.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
    m_wndResultList.ShowWindow(bShowResult ? SW_SHOW : SW_HIDE);
    m_wndDetailSection.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
    m_wndDetail.ShowWindow(bShowDetail ? SW_SHOW : SW_HIDE);
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
    if (!::IsWindow(m_wndWorkflowMenu.GetSafeHwnd()))
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
    m_wndWorkflowMenu.MoveWindow(TAECHANG_MARGIN, TAECHANG_TOP_BAR_HEIGHT + TAECHANG_SIDEBAR_TITLE_HEIGHT, TAECHANG_SIDEBAR_WIDTH - (TAECHANG_MARGIN * 2), nSidebarHeight - TAECHANG_TOP_BAR_HEIGHT - TAECHANG_SIDEBAR_TITLE_HEIGHT - TAECHANG_MARGIN);

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
    int nPathWidth = nWidth - TAECHANG_LABEL_WIDTH - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP;
    m_wndInputSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
    nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
    m_wndInputLabel.MoveWindow(nLeft, nTop + 4, TAECHANG_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
    m_wndInputPath.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
    m_wndSelectInput.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH + nPathWidth + TAECHANG_ROW_GAP, nTop - 2, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
    if (!bShowOutput)
        return;
    nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;
    m_wndOutputSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_SECTION_TITLE_HEIGHT);
    nTop += TAECHANG_SECTION_TITLE_HEIGHT + TAECHANG_ROW_GAP;
    m_wndOutputLabel.MoveWindow(nLeft, nTop + 4, TAECHANG_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
    m_wndOutputFolder.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
    m_wndSelectOutput.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH + nPathWidth + TAECHANG_ROW_GAP, nTop - 2, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
}

void CSageTaechangView::LayoutActionSection(int nLeft, int nTop, int nWidth)
{
    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    BOOL bShowAction = IsActionTabVisible();
    BOOL bShowLoad = (bShowAction && !bIsCompare) ? TRUE : FALSE;
    BOOL bShowGenerate = bShowAction;
    BOOL bShowExport = IsExportTab();
    BOOL bShowSettings = IsSettingsButtonVisible();

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
    if (bShowSettings)
    {
        m_wndSettings.MoveWindow(nX, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
        nX += TAECHANG_BUTTON_WIDTH + TAECHANG_ACTION_GAP;
    }
    if (bShowAction)
    {
        int nProgressLeft = nX;
        int nProgressWidth = nWidth - (nProgressLeft - nLeft);
        if (nProgressWidth < 0)
            nProgressWidth = 0;
        m_wndProgress.MoveWindow(nProgressLeft, nTop + 5, nProgressWidth, TAECHANG_PROGRESS_HEIGHT);
    }
}

void CSageTaechangView::LayoutResultSection(int nLeft, int nTop, int nWidth, int nHeight)
{
    int nBodyHeight = max(TAECHANG_RESULT_MIN_HEIGHT, nHeight - TAECHANG_RESULT_HEADER_HEIGHT);
    if (IsResultTab())
    {
        m_wndResultSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_RESULT_HEADER_HEIGHT);
        m_wndResultList.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
        UpdateResultColumns();
    }
    if (IsDetailTab())
    {
        m_wndDetailSection.MoveWindow(nLeft, nTop, nWidth, TAECHANG_RESULT_HEADER_HEIGHT);
        m_wndDetail.MoveWindow(nLeft, nTop + TAECHANG_RESULT_HEADER_HEIGHT, nWidth, nBodyHeight);
    }
}

void CSageTaechangView::OnDraw(CDC* pDC)
{
    CSageTaechangDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CRect rectClient;
    GetClientRect(&rectClient);
    pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
    pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
}

int CSageTaechangView::GetSelectedWorkflow() const
{
    if (m_wndWorkflowMenu.GetCurSel() == 4)
        return TAECHANG_WORKFLOW_HWP_COMPARE;
    if (m_wndWorkflowMenu.GetCurSel() == 3)
        return TAECHANG_WORKFLOW_PDF_COMPARE;
    if (m_wndWorkflowMenu.GetCurSel() == 2)
        return TAECHANG_WORKFLOW_ESTIMATE;
    if (m_wndWorkflowMenu.GetCurSel() == 1)
        return TAECHANG_WORKFLOW_DELIVERY;
    return TAECHANG_WORKFLOW_RECEIVABLES;
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
    m_wndDetail.SetWindowTextW(L"");
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
    if (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_PREVIEW)
        return TRUE;
    if (!IsCompareWorkflow(GetSelectedWorkflow()) && m_nSelectedTaskTab == TAECHANG_TAB_INDEX_RESULT)
        return TRUE;
    return FALSE;
}

BOOL CSageTaechangView::IsDetailTab() const
{
    if (IsCompareWorkflow(GetSelectedWorkflow()))
        return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_RESULT) ? TRUE : FALSE;
    return (m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DETAIL) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsExportTab() const
{
    return (IsCompareWorkflow(GetSelectedWorkflow()) && m_nSelectedTaskTab == TAECHANG_TAB_INDEX_DETAIL) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsActionTabVisible() const
{
    return (IsInputTabSelected() || IsResultTab()) ? TRUE : FALSE;
}

BOOL CSageTaechangView::IsSettingsButtonVisible() const
{
    return (IsInputTabSelected() && GetSelectedWorkflow() == TAECHANG_WORKFLOW_PDF_COMPARE) ? TRUE : FALSE;
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
    UpdateWorkflowLabels();
    UpdateExportButtonState();
    UpdateResultColumns();
}

void CSageTaechangView::OnTaskTabChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    m_nSelectedTaskTab = m_wndTaskTabs.GetCurSel();
    LayoutChildControls();
    Invalidate();
    *pResult = 0;
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
        m_wndInputPath.SetWindowTextW(dlg.GetPathName());
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

void CSageTaechangView::OnSettings()
{
    TaechangAppSettingsService settingsService;
    TaechangAppSettings settings;
    settingsService.Load(settings);

    CFileDialog dlg(TRUE, L"exe", settings.m_strPdfToTextPath, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, TAECHANG_UI_EXE_FILTER, this);
    dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_PDFTOTEXT_TITLE;
    if (dlg.DoModal() != IDOK)
        return;

    settings.m_strPdfToTextPath = dlg.GetPathName();
    CString strError;
    if (!settingsService.Save(settings, strError))
    {
        AfxMessageBox(strError, MB_ICONERROR);
        return;
    }

    SetStatusText(TAECHANG_UI_SETTINGS_SAVED);
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

    TaechangWorkflowTask* pTask = new TaechangWorkflowTask();
    pTask->m_hWnd = GetSafeHwnd();
    pTask->m_nWorkflowType = nWorkflowType;
    pTask->m_nTaskType = nTaskType;
    pTask->m_strInputPath = strInputPath;
    pTask->m_strOutputFolder = strOutputFolder;
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
    m_wndWorkflowMenu.EnableWindow(!bRunning);
    m_wndSelectInput.EnableWindow(!bRunning);
    m_wndSelectOutput.EnableWindow(!bRunning);
    m_wndLoad.EnableWindow(!bRunning);
    m_wndGenerate.EnableWindow(!bRunning);
    m_wndSettings.EnableWindow(!bRunning);
    m_wndProgress.SetMarquee(bRunning, 30);
    UpdateExportButtonState();
    SetStatusText(bRunning ? TAECHANG_UI_RUNNING : TAECHANG_UI_READY);
}

BOOL CSageTaechangView::OnEraseBkgnd(CDC* pDC)
{
    CRect rectClient;
    GetClientRect(&rectClient);
    pDC->FillSolidRect(rectClient, TAECHANG_COLOR_APP_BACKGROUND);
    pDC->FillSolidRect(0, 0, TAECHANG_SIDEBAR_WIDTH, rectClient.Height(), TAECHANG_COLOR_SIDEBAR);
    return TRUE;
}

HBRUSH CSageTaechangView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hBrush = CView::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetTextColor(TAECHANG_COLOR_TEXT);
    if (pWnd->GetSafeHwnd() == m_wndSidebarTitle.GetSafeHwnd() ||
        pWnd->GetSafeHwnd() == m_wndTitle.GetSafeHwnd() ||
        pWnd->GetSafeHwnd() == m_wndWorkflowMenu.GetSafeHwnd())
    {
        pDC->SetBkColor(TAECHANG_COLOR_SIDEBAR);
        return m_brushSidebar;
    }
    if (pWnd->GetSafeHwnd() == m_wndHeaderStatus.GetSafeHwnd())
    {
        pDC->SetTextColor(m_colorHeaderStatus);
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
        m_wndHeaderStatus.SetWindowTextW(strStatus);
        m_wndHeaderStatus.Invalidate();
    }
}

COLORREF CSageTaechangView::ResolveStatusColor(const CString& strStatus) const
{
    if (strStatus == TAECHANG_UI_RUNNING)
        return TAECHANG_COLOR_PRIMARY;
    if (strStatus == TAECHANG_UI_COMPLETED ||
        strStatus == TAECHANG_UI_EXPORT_COMPLETED ||
        strStatus == TAECHANG_UI_SETTINGS_SAVED)
        return TAECHANG_COLOR_SUCCESS;
    if (strStatus == TAECHANG_UI_FAILED)
        return TAECHANG_COLOR_ERROR;
    return TAECHANG_COLOR_SECONDARY_TEXT;
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

    TaechangWorkflowResultPresenter presenter;
    std::vector<TaechangResultRow> arrRows;
    CString strDetailText;
    BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows, strDetailText);
    m_wndDetail.SetWindowTextW(strDetailText);

    for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
        InsertResultRow(arrRows[i]);

    SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
    UpdateExportButtonState();
}

void CSageTaechangView::InsertResultRow(const TaechangResultRow& row)
{
    BOOL bIsCompare = IsCompareWorkflow(GetSelectedWorkflow());
    int nCount = m_wndResultList.GetItemCount();
    int nCol = 0;
    int nIndex;
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


