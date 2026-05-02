
// SageTaechangView.cpp: CSageTaechangView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "SageTaechangDoc.h"
#include "SageTaechangView.h"
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
    ON_CBN_SELCHANGE(ID_TAECHANG_WORKFLOW_COMBO, &CSageTaechangView::OnWorkflowChanged)
    ON_BN_CLICKED(ID_TAECHANG_SELECT_INPUT, &CSageTaechangView::OnSelectInput)
    ON_BN_CLICKED(ID_TAECHANG_SELECT_OUTPUT, &CSageTaechangView::OnSelectOutput)
    ON_BN_CLICKED(ID_TAECHANG_LOAD_WORKFLOW, &CSageTaechangView::OnLoadWorkflow)
    ON_BN_CLICKED(ID_TAECHANG_GENERATE_WORKFLOW, &CSageTaechangView::OnGenerateWorkflow)
    ON_MESSAGE(WM_TAECHANG_WORKFLOW_COMPLETE, &CSageTaechangView::OnWorkflowComplete)
END_MESSAGE_MAP()

CSageTaechangView::CSageTaechangView() noexcept
    : m_bRunning(FALSE)
{
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
    m_wndTitle.Create(TAECHANG_UI_TITLE, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndWorkflowLabel.Create(TAECHANG_UI_WORKFLOW_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndWorkflow.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, rectEmpty, this, ID_TAECHANG_WORKFLOW_COMBO);
    m_wndWorkflow.AddString(TAECHANG_UI_RECEIVABLES_NAME);
    m_wndWorkflow.AddString(TAECHANG_UI_DELIVERY_NAME);
    m_wndWorkflow.AddString(TAECHANG_UI_ESTIMATE_NAME);
    m_wndWorkflow.AddString(TAECHANG_UI_PDF_COMPARE_NAME);
    m_wndWorkflow.AddString(TAECHANG_UI_HWP_COMPARE_NAME);
    m_wndWorkflow.SetCurSel(0);
    m_wndInputLabel.Create(TAECHANG_UI_INPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndOutputLabel.Create(TAECHANG_UI_OUTPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_INPUT_EDIT);
    m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_OUTPUT_EDIT);
    m_wndSelectInput.Create(TAECHANG_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_INPUT);
    m_wndSelectOutput.Create(TAECHANG_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_OUTPUT);
    m_wndLoad.Create(TAECHANG_UI_LOAD_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_LOAD_WORKFLOW);
    m_wndGenerate.Create(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_GENERATE_WORKFLOW);
    m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, rectEmpty, this, ID_TAECHANG_PROGRESS);
    m_wndResultList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, rectEmpty, this, ID_TAECHANG_RESULT_LIST);
    m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);

    m_wndResultList.InsertColumn(0, TAECHANG_UI_RESULT_FIELD, LVCFMT_LEFT, 120);
    m_wndResultList.InsertColumn(1, TAECHANG_UI_RESULT_VALUE, LVCFMT_LEFT, 260);
    m_wndResultList.InsertColumn(2, TAECHANG_UI_RESULT_STATUS, LVCFMT_LEFT, 110);
    m_wndResultList.InsertColumn(3, TAECHANG_UI_RESULT_REASON, LVCFMT_LEFT, 430);
    m_wndProgress.SetMarquee(FALSE, 0);
    UpdateWorkflowLabels();
}

void CSageTaechangView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    LayoutChildControls();
}

void CSageTaechangView::LayoutChildControls()
{
    if (!::IsWindow(m_wndTitle.GetSafeHwnd()))
        return;

    CRect rectClient;
    GetClientRect(&rectClient);
    int nLeft = TAECHANG_MARGIN;
    int nTop = TAECHANG_MARGIN;
    int nWidth = rectClient.Width() - (TAECHANG_MARGIN * 2);
    int nHeight = rectClient.Height() - (TAECHANG_MARGIN * 2);

    m_wndTitle.MoveWindow(nLeft, nTop, nWidth, 26);
    nTop += 34;

    m_wndWorkflowLabel.MoveWindow(nLeft, nTop + 4, TAECHANG_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
    m_wndWorkflow.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH, nTop, TAECHANG_COMBO_WIDTH, 180);
    nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

    int nPathWidth = nWidth - TAECHANG_LABEL_WIDTH - TAECHANG_BUTTON_WIDTH - TAECHANG_ROW_GAP;
    m_wndInputLabel.MoveWindow(nLeft, nTop + 4, TAECHANG_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
    m_wndInputPath.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
    m_wndSelectInput.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH + nPathWidth + TAECHANG_ROW_GAP, nTop - 2, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
    nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

    m_wndOutputLabel.MoveWindow(nLeft, nTop + 4, TAECHANG_LABEL_WIDTH, TAECHANG_EDIT_HEIGHT);
    m_wndOutputFolder.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH, nTop, nPathWidth, TAECHANG_EDIT_HEIGHT);
    m_wndSelectOutput.MoveWindow(nLeft + TAECHANG_LABEL_WIDTH + nPathWidth + TAECHANG_ROW_GAP, nTop - 2, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
    nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

    m_wndLoad.MoveWindow(nLeft, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
    m_wndGenerate.MoveWindow(nLeft + TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP, nTop, TAECHANG_BUTTON_WIDTH, TAECHANG_BUTTON_HEIGHT);
    m_wndProgress.MoveWindow(nLeft + (TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP) * 2, nTop + 5, nWidth - ((TAECHANG_BUTTON_WIDTH + TAECHANG_ROW_GAP) * 2), TAECHANG_PROGRESS_HEIGHT);
    nTop += TAECHANG_BUTTON_HEIGHT + TAECHANG_ROW_GAP;

    int nResultHeight = max(TAECHANG_RESULT_MIN_HEIGHT, (nHeight - nTop) / 2);
    m_wndResultList.MoveWindow(nLeft, nTop, nWidth, nResultHeight);
    nTop += nResultHeight + TAECHANG_ROW_GAP;
    m_wndDetail.MoveWindow(nLeft, nTop, nWidth, max(80, rectClient.bottom - nTop - TAECHANG_MARGIN));
}

void CSageTaechangView::OnDraw(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    CSageTaechangDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
}

int CSageTaechangView::GetSelectedWorkflow() const
{
    if (m_wndWorkflow.GetCurSel() == 4)
        return TAECHANG_WORKFLOW_HWP_COMPARE;
    if (m_wndWorkflow.GetCurSel() == 3)
        return TAECHANG_WORKFLOW_PDF_COMPARE;
    if (m_wndWorkflow.GetCurSel() == 2)
        return TAECHANG_WORKFLOW_ESTIMATE;
    if (m_wndWorkflow.GetCurSel() == 1)
        return TAECHANG_WORKFLOW_DELIVERY;
    return TAECHANG_WORKFLOW_RECEIVABLES;
}

void CSageTaechangView::UpdateWorkflowLabels()
{
    int nWorkflowType = GetSelectedWorkflow();
    if (nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE)
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_HWP_COMPARE_BUTTON);
    else if (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE)
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_PDF_COMPARE_BUTTON);
    else if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE)
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_ESTIMATE_GENERATE_BUTTON);
    else if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY)
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_DELIVERY_GENERATE_BUTTON);
    else
        m_wndGenerate.SetWindowTextW(TAECHANG_UI_RECEIVABLES_GENERATE_BUTTON);
    m_wndResultList.DeleteAllItems();
    m_wndDetail.SetWindowTextW(L"");
}

void CSageTaechangView::OnWorkflowChanged()
{
    UpdateWorkflowLabels();
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
    m_wndWorkflow.EnableWindow(!bRunning);
    m_wndSelectInput.EnableWindow(!bRunning);
    m_wndSelectOutput.EnableWindow(!bRunning);
    m_wndLoad.EnableWindow(!bRunning);
    m_wndGenerate.EnableWindow(!bRunning);
    m_wndProgress.SetMarquee(bRunning, 30);
    SetStatusText(bRunning ? TAECHANG_UI_RUNNING : TAECHANG_UI_READY);
}

void CSageTaechangView::SetStatusText(const CString& strStatus)
{
    CFrameWnd* pFrame = GetParentFrame();
    if (pFrame != NULL)
        pFrame->SetMessageText(strStatus);
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

    TaechangWorkflowResultPresenter presenter;
    std::vector<TaechangResultRow> arrRows;
    CString strDetailText;
    BOOL bSuccess = presenter.BuildRows(nWorkflowType, nTaskType, strResponseJson, arrRows, strDetailText);
    m_wndDetail.SetWindowTextW(strDetailText);

    for (int i = 0; i < static_cast<int>(arrRows.size()); ++i)
    {
        InsertResultRow(
            arrRows[i].m_strField,
            arrRows[i].m_strValue,
            arrRows[i].m_strStatus,
            arrRows[i].m_strReason);
    }

    SetStatusText(bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);
}

void CSageTaechangView::InsertResultRow(
    const CString& strField,
    const CString& strValue,
    const CString& strStatus,
    const CString& strReason)
{
    int nIndex = m_wndResultList.InsertItem(m_wndResultList.GetItemCount(), strField);
    m_wndResultList.SetItemText(nIndex, 1, strValue);
    m_wndResultList.SetItemText(nIndex, 2, strStatus);
    m_wndResultList.SetItemText(nIndex, 3, strReason);
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


