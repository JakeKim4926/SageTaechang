
// SageTaechangView.cpp: CSageTaechangView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "SageTaechangDoc.h"
#include "SageTaechangView.h"
#include "app/application/services/TaechangReceivablesExcelService.h"
#include "app/common/TaechangJson.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct TaechangReceivablesTask
{
    HWND m_hWnd;
    int m_nTaskType;
    CString m_strInputPath;
    CString m_strOutputFolder;
};

struct TaechangReceivablesResult
{
    int m_nTaskType;
    CString m_strResponseJson;
};

static CString BuildReceivablesPayload(const CString& strInputPath, const CString& strOutputFolder)
{
    CString strPayload = L"{\"inputPath\":\"" + JsonEscapeString(strInputPath) + L"\"";
    if (!strOutputFolder.IsEmpty())
        strPayload += L",\"outputFolder\":\"" + JsonEscapeString(strOutputFolder) + L"\"";
    strPayload += L"}";
    return strPayload;
}

static UINT RunReceivablesWorker(LPVOID pParam)
{
    TaechangReceivablesTask* pTask = reinterpret_cast<TaechangReceivablesTask*>(pParam);
    TaechangReceivablesResult* pResult = new TaechangReceivablesResult();
    pResult->m_nTaskType = pTask->m_nTaskType;

    TaechangReceivablesExcelService service;
    CString strPayload = BuildReceivablesPayload(pTask->m_strInputPath, pTask->m_strOutputFolder);
    if (pTask->m_nTaskType == TAECHANG_TASK_LOAD_RECEIVABLES)
    {
        pResult->m_strResponseJson = service.BuildLoadInputDataResponse(TAECHANG_REQUEST_LOAD, strPayload);
    }
    else
    {
        pResult->m_strResponseJson = service.BuildGenerateResponse(TAECHANG_REQUEST_GENERATE, strPayload);
    }

    HWND hWnd = pTask->m_hWnd;
    delete pTask;

    if (::IsWindow(hWnd))
        ::PostMessageW(hWnd, WM_TAECHANG_RECEIVABLES_COMPLETE, 0, reinterpret_cast<LPARAM>(pResult));
    else
        delete pResult;

    return 0;
}

IMPLEMENT_DYNCREATE(CSageTaechangView, CView)

BEGIN_MESSAGE_MAP(CSageTaechangView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_BN_CLICKED(ID_TAECHANG_SELECT_INPUT, &CSageTaechangView::OnSelectInput)
    ON_BN_CLICKED(ID_TAECHANG_SELECT_OUTPUT, &CSageTaechangView::OnSelectOutput)
    ON_BN_CLICKED(ID_TAECHANG_LOAD_RECEIVABLES, &CSageTaechangView::OnLoadReceivables)
    ON_BN_CLICKED(ID_TAECHANG_GENERATE_RECEIVABLES, &CSageTaechangView::OnGenerateReceivables)
    ON_MESSAGE(WM_TAECHANG_RECEIVABLES_COMPLETE, &CSageTaechangView::OnReceivablesComplete)
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
    m_wndInputLabel.Create(TAECHANG_UI_INPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndOutputLabel.Create(TAECHANG_UI_OUTPUT_LABEL, WS_CHILD | WS_VISIBLE, rectEmpty, this);
    m_wndInputPath.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_INPUT_EDIT);
    m_wndOutputFolder.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, rectEmpty, this, ID_TAECHANG_OUTPUT_EDIT);
    m_wndSelectInput.Create(TAECHANG_UI_INPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_INPUT);
    m_wndSelectOutput.Create(TAECHANG_UI_OUTPUT_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_SELECT_OUTPUT);
    m_wndLoad.Create(TAECHANG_UI_LOAD_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_LOAD_RECEIVABLES);
    m_wndGenerate.Create(TAECHANG_UI_GENERATE_BUTTON, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectEmpty, this, ID_TAECHANG_GENERATE_RECEIVABLES);
    m_wndProgress.Create(WS_CHILD | WS_VISIBLE | PBS_MARQUEE, rectEmpty, this, ID_TAECHANG_PROGRESS);
    m_wndResultList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL, rectEmpty, this, ID_TAECHANG_RESULT_LIST);
    m_wndDetail.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, rectEmpty, this, ID_TAECHANG_DETAIL_EDIT);

    m_wndResultList.InsertColumn(0, TAECHANG_UI_RESULT_FIELD, LVCFMT_LEFT, 160);
    m_wndResultList.InsertColumn(1, TAECHANG_UI_RESULT_VALUE, LVCFMT_LEFT, 520);
    m_wndProgress.SetMarquee(FALSE, 0);
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

    int nLeft = TAECHANG_MARGIN;
    int nTop = TAECHANG_MARGIN;
    int nWidth = 0;
    int nHeight = 0;
    CRect rectClient;
    GetClientRect(&rectClient);
    nWidth = rectClient.Width() - (TAECHANG_MARGIN * 2);
    nHeight = rectClient.Height() - (TAECHANG_MARGIN * 2);

    m_wndTitle.MoveWindow(nLeft, nTop, nWidth, 26);
    nTop += 34;

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

void CSageTaechangView::OnSelectInput()
{
    CFileDialog dlg(TRUE, L"xls", NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, TAECHANG_UI_EXCEL_FILTER, this);
    dlg.m_ofn.lpstrTitle = TAECHANG_UI_SELECT_INPUT_TITLE;
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

void CSageTaechangView::OnLoadReceivables()
{
    RunReceivablesTask(TAECHANG_TASK_LOAD_RECEIVABLES);
}

void CSageTaechangView::OnGenerateReceivables()
{
    RunReceivablesTask(TAECHANG_TASK_GENERATE_RECEIVABLES);
}

void CSageTaechangView::RunReceivablesTask(int nTaskType)
{
    if (m_bRunning)
        return;

    CString strInputPath;
    CString strOutputFolder;
    if (!ValidateInputPath(strInputPath))
        return;

    if (nTaskType == TAECHANG_TASK_GENERATE_RECEIVABLES && !ValidateOutputFolder(strOutputFolder))
        return;

    TaechangReceivablesTask* pTask = new TaechangReceivablesTask();
    pTask->m_hWnd = GetSafeHwnd();
    pTask->m_nTaskType = nTaskType;
    pTask->m_strInputPath = strInputPath;
    pTask->m_strOutputFolder = strOutputFolder;

    SetRunningState(TRUE);
    AfxBeginThread(RunReceivablesWorker, pTask, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void CSageTaechangView::SetRunningState(BOOL bRunning)
{
    m_bRunning = bRunning;
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

LRESULT CSageTaechangView::OnReceivablesComplete(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    TaechangReceivablesResult* pResult = reinterpret_cast<TaechangReceivablesResult*>(lParam);
    if (pResult != NULL)
    {
        DisplayResponse(pResult->m_nTaskType, pResult->m_strResponseJson);
        delete pResult;
    }
    SetRunningState(FALSE);
    return 0;
}

void CSageTaechangView::DisplayResponse(int nTaskType, const CString& strResponseJson)
{
    UNREFERENCED_PARAMETER(nTaskType);
    m_wndResultList.DeleteAllItems();
    m_wndDetail.SetWindowTextW(strResponseJson);

    BOOL bSuccess = JsonExtractBool(strResponseJson, L"success");
    InsertResultRow(TAECHANG_UI_RESULT_STATUS, bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED);

    if (bSuccess)
    {
        CString strFileName = JsonExtractString(strResponseJson, L"fileName");
        CString strFolder = JsonExtractString(strResponseJson, L"outputFolder");
        if (!strFileName.IsEmpty())
            InsertResultRow(TAECHANG_UI_RESULT_FILE, strFileName);
        if (!strFolder.IsEmpty())
            InsertResultRow(TAECHANG_UI_RESULT_FOLDER, strFolder);
        SetStatusText(TAECHANG_UI_COMPLETED);
    }
    else
    {
        CString strMessage = JsonExtractString(strResponseJson, L"message");
        InsertResultRow(TAECHANG_UI_RESULT_ERROR, strMessage);
        SetStatusText(TAECHANG_UI_FAILED);
    }
}

void CSageTaechangView::InsertResultRow(const CString& strField, const CString& strValue)
{
    int nIndex = m_wndResultList.InsertItem(m_wndResultList.GetItemCount(), strField);
    m_wndResultList.SetItemText(nIndex, 1, strValue);
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
