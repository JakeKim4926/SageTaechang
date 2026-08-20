#include "pch.h"
#include "app/ui/dialogs/SageCalcEstimateDlg.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "SageDefine.h"
#include "app/infra/file/SageFileUtils.h"
#include "app/common/SageJson.h"

BEGIN_MESSAGE_MAP(SageCalcEstimateDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
    ON_EN_CHANGE(ID_CALC_ESTIMATE_DLG_YEAR_EDIT,  &SageCalcEstimateDlg::OnYearChanged)
    ON_EN_CHANGE(ID_CALC_ESTIMATE_DLG_MONTH_EDIT, &SageCalcEstimateDlg::OnMonthChanged)
END_MESSAGE_MAP()

SageCalcEstimateDlg::SageCalcEstimateDlg(
    const CString& strCompany,
    int nCopies, int nPages,
    int nUnitPrice, int nCoverPrice, int nFreight,
    const CString& strTemplatePath,
    const CString& strScriptPath,
    CWnd* pParent)
    : SageFramelessDialog(pParent)
    , m_strCompany(strCompany)
    , m_nCopies(nCopies), m_nPages(nPages)
    , m_nUnitPrice(nUnitPrice), m_nCoverPrice(nCoverPrice), m_nFreight(nFreight)
    , m_strTemplatePath(strTemplatePath)
    , m_strScriptPath(strScriptPath) {
}

SageCalcEstimateDlg::~SageCalcEstimateDlg() {}

INT_PTR SageCalcEstimateDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_CALC_ESTIMATE_DLG_TITLE,
        SAGE_CALC_ESTIMATE_DLG_TEMPLATE_CX, SAGE_CALC_ESTIMATE_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString SageCalcEstimateDlg::GetItemName() const {
    return m_strItemName;
}

CString SageCalcEstimateDlg::GetDate() const {
    return m_strDate;
}

BOOL SageCalcEstimateDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(SAGE_UI_CALC_ESTIMATE_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushDivider.CreateSolidBrush(SAGE_COLOR_BORDER);

    CreateCaptionBar(SAGE_UI_CALC_ESTIMATE_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_CALC_ESTIMATE_DLG_WIDTH, LayoutControls());

    m_wndYearEdit.SetFocus();
    return FALSE;
}

void SageCalcEstimateDlg::CreateControls() {
    CRect r(0, 0, 0, 0);

    m_wndDateLabel.Create(SAGE_UI_CALC_ESTIMATE_DATE_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT, r, this);
    m_wndDateDivider.Create(L"", WS_CHILD | WS_VISIBLE, r, this);
    m_wndYearEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_YEAR_EDIT);
    m_wndDateSep1.Create(SAGE_UI_CALC_ESTIMATE_DATE_SEP,
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
    m_wndMonthEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_MONTH_EDIT);
    m_wndDateSep2.Create(SAGE_UI_CALC_ESTIMATE_DATE_SEP,
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
    m_wndDayEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_DAY_EDIT);
    m_wndItemLabel.Create(SAGE_UI_CALC_ESTIMATE_ITEM_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT, r, this);
    m_wndItemDivider.Create(L"", WS_CHILD | WS_VISIBLE, r, this);
    m_wndItemEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        r, this, ID_CALC_ESTIMATE_DLG_ITEM_EDIT);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.Create(SAGE_UI_CALC_ESTIMATE_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_CALC_ESTIMATE_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);

    m_wndYearEdit.SetLimitText(SAGE_CALC_YEAR_MAX_LEN);
    m_wndMonthEdit.SetLimitText(SAGE_CALC_MONTH_MAX_LEN);
    m_wndDayEdit.SetLimitText(SAGE_CALC_DAY_MAX_LEN);
    m_wndItemEdit.SetLimitText(SAGE_CALC_ITEM_MAX_LEN);

    m_wndYearEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)SAGE_UI_CALC_ESTIMATE_YEAR_CUE);
    m_wndMonthEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)SAGE_UI_CALC_ESTIMATE_MONTH_CUE);
    m_wndDayEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)SAGE_UI_CALC_ESTIMATE_DAY_CUE);
    m_wndItemEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)SAGE_UI_CALC_ESTIMATE_ITEM_CUE);
}

void SageCalcEstimateDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
    m_wndDateLabel.SetFont(&m_font);
    m_wndDateLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndDateLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndYearEdit.SetFont(&m_font);
    m_wndDateSep1.SetFont(&m_font);
    m_wndDateSep1.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndDateSep1.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndMonthEdit.SetFont(&m_font);
    m_wndDateSep2.SetFont(&m_font);
    m_wndDateSep2.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndDateSep2.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndDayEdit.SetFont(&m_font);
    m_wndItemLabel.SetFont(&m_font);
    m_wndItemLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndItemLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndItemEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SageCalcEstimateDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

int SageCalcEstimateDlg::LayoutControls() {
    int nM         = SAGE_MARGIN;
    int nEditH     = SAGE_EDIT_HEIGHT;
    int nBtnW      = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH      = SAGE_BUTTON_HEIGHT;
    int nBtnGap    = SAGE_ROW_GAP;
    int nLabelH    = SAGE_CALC_ESTIMATE_DLG_LABEL_H;
    int nLabelGap  = SAGE_CALC_ESTIMATE_DLG_LABEL_GAP;
    int nGroupGap  = SAGE_CALC_ESTIMATE_DLG_GROUP_GAP;
    int nDivGap    = SAGE_CALC_ESTIMATE_DLG_DIV_GAP;
    int nYearW     = SAGE_CALC_ESTIMATE_DLG_YEAR_W;
    int nMdW       = SAGE_CALC_ESTIMATE_DLG_MD_W;
    int nSepW      = SAGE_CALC_ESTIMATE_DLG_SEP_W;
    int nClientW   = SAGE_CALC_ESTIMATE_DLG_WIDTH;
    int nContentW  = nClientW - nM * 2;

    int nY = GetContentTop() + nM;

    m_wndDateLabel.MoveWindow(nM, nY, nContentW, nLabelH);
    nY += nLabelH + nDivGap;
    m_wndDateDivider.MoveWindow(nM, nY, nContentW, 1);
    nY += 1 + nDivGap;

    int nX = nM;
    m_wndYearEdit.MoveWindow(nX, nY, nYearW, nEditH);
    ApplyEditTextRect(m_wndYearEdit);
    nX += nYearW;
    m_wndDateSep1.MoveWindow(nX, nY, nSepW, nEditH);
    nX += nSepW;
    m_wndMonthEdit.MoveWindow(nX, nY, nMdW, nEditH);
    ApplyEditTextRect(m_wndMonthEdit);
    nX += nMdW;
    m_wndDateSep2.MoveWindow(nX, nY, nSepW, nEditH);
    nX += nSepW;
    m_wndDayEdit.MoveWindow(nX, nY, nMdW, nEditH);
    ApplyEditTextRect(m_wndDayEdit);
    nY += nEditH + nGroupGap;

    m_wndItemLabel.MoveWindow(nM, nY, nContentW, nLabelH);
    nY += nLabelH + nDivGap;
    m_wndItemDivider.MoveWindow(nM, nY, nContentW, 1);
    nY += 1 + nDivGap;

    m_wndItemEdit.MoveWindow(nM, nY, nContentW, nEditH);
    ApplyEditTextRect(m_wndItemEdit);
    nY += nEditH;

    m_wndError.MoveWindow(nM, nY, nContentW, SAGE_INLINE_MSG_HEIGHT);
    nY += SAGE_INLINE_MSG_HEIGHT + nBtnGap;

    int nBtnTop   = nY;
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nBtnGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

BOOL SageCalcEstimateDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
        HWND hFocus = ::GetFocus();
        if (hFocus == m_wndYearEdit.GetSafeHwnd()) {
            m_wndMonthEdit.SetFocus();
            return TRUE;
        }
        if (hFocus == m_wndMonthEdit.GetSafeHwnd()) {
            m_wndDayEdit.SetFocus();
            return TRUE;
        }
        if (hFocus == m_wndDayEdit.GetSafeHwnd()) {
            m_wndItemEdit.SetFocus();
            return TRUE;
        }
        if (hFocus == m_wndItemEdit.GetSafeHwnd()) {
            m_wndOkBtn.SetFocus();
            return TRUE;
        }
        if (hFocus == m_wndOkBtn.GetSafeHwnd()) {
            m_wndCancelBtn.SetFocus();
            return TRUE;
        }
        if (hFocus == m_wndCancelBtn.GetSafeHwnd()) {
            m_wndYearEdit.SetFocus();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SageCalcEstimateDlg::ShowInputError(CSageEdit& edit, const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    edit.SetState(SAGE_EDIT_ERROR);
    edit.SetFocus();
}

BOOL SageCalcEstimateDlg::ValidateInputs() {
    m_wndError.ClearMessage();
    m_wndYearEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndMonthEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndDayEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndItemEdit.SetState(SAGE_EDIT_NORMAL);

    CString strYear, strMonth, strDay;
    m_wndYearEdit.GetWindowText(strYear);
    m_wndMonthEdit.GetWindowText(strMonth);
    m_wndDayEdit.GetWindowText(strDay);
    strYear.Trim(); strMonth.Trim(); strDay.Trim();

    if (strYear.IsEmpty() || strMonth.IsEmpty() || strDay.IsEmpty()) {
        ShowInputError(m_wndYearEdit, SAGE_UI_CALC_ESTIMATE_DATE_REQUIRED);
        return FALSE;
    }

    int nYear  = _wtoi(strYear);
    int nMonth = _wtoi(strMonth);
    int nDay   = _wtoi(strDay);
    BOOL bDateOk = (nYear >= 1900 && nYear <= 9999)
                && (nMonth >= 1 && nMonth <= 12)
                && (nDay >= 1 && nDay <= 31);
    if (!bDateOk) {
        ShowInputError(m_wndYearEdit, SAGE_UI_CALC_ESTIMATE_DATE_INVALID);
        return FALSE;
    }

    CString strItem;
    m_wndItemEdit.GetWindowText(strItem);
    strItem.Trim();
    if (strItem.IsEmpty()) {
        ShowInputError(m_wndItemEdit, SAGE_UI_CALC_ESTIMATE_ITEM_REQUIRED);
        return FALSE;
    }
    if (strItem.GetLength() > SAGE_CALC_ITEM_MAX_LEN) {
        ShowInputError(m_wndItemEdit, SAGE_UI_CALC_ESTIMATE_ITEM_TOO_LONG);
        return FALSE;
    }

    m_strDate.Format(L"%04d-%02d-%02d", nYear, nMonth, nDay);
    m_strItemName = strItem;
    return TRUE;
}

BOOL SageCalcEstimateDlg::SelectOutputFolder(CString& strFolder) {
    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    BOOL bNeedUninitialize = (hrInit == S_OK || hrInit == S_FALSE) ? TRUE : FALSE;

    IFileOpenDialog* pDialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog));
    if (FAILED(hr) || pDialog == NULL) {
        if (bNeedUninitialize) CoUninitialize();
        return FALSE;
    }

    DWORD dwOptions = 0;
    pDialog->GetOptions(&dwOptions);
    pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    pDialog->SetTitle(SAGE_UI_SELECT_OUTPUT_TITLE);

    BOOL bSelected = FALSE;
    hr = pDialog->Show(GetSafeHwnd());
    if (SUCCEEDED(hr)) {
        IShellItem* pItem = NULL;
        hr = pDialog->GetResult(&pItem);
        if (SUCCEEDED(hr) && pItem != NULL) {
            PWSTR pszPath = NULL;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
            if (SUCCEEDED(hr) && pszPath != NULL) {
                strFolder = pszPath;
                CoTaskMemFree(pszPath);
                bSelected = TRUE;
            }
            pItem->Release();
        }
    }
    pDialog->Release();
    if (bNeedUninitialize) CoUninitialize();
    return bSelected;
}

static CString ReadScriptErrorMessage(const CString& strResultPath) {
    std::ifstream file(WideToUtf8(strResultPath), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return CString();
    std::ostringstream buf;
    buf << file.rdbuf();
    std::string strUtf8 = buf.str();
    if (strUtf8.size() >= 3 &&
        (unsigned char)strUtf8[0] == 0xEF &&
        (unsigned char)strUtf8[1] == 0xBB &&
        (unsigned char)strUtf8[2] == 0xBF) {
        strUtf8.erase(0, 3);
    }
    CString strJson = Utf8ToWide(strUtf8);
    strJson.Trim();
    return JsonExtractString(strJson, L"message");
}

BOOL SageCalcEstimateDlg::RunGenerate(const CString& strOutputFolder) {
    CString strResultPath = BuildTempJsonPath(L"tcc");

    CString strCopiesArg, strPagesArg, strUnitArg, strCoverArg, strFreightArg;
    strCopiesArg.Format(L"%d", m_nCopies);
    strPagesArg.Format(L"%d", m_nPages);
    strUnitArg.Format(L"%d", m_nUnitPrice);
    strCoverArg.Format(L"%d", m_nCoverPrice);
    strFreightArg.Format(L"%d", m_nFreight);

    CString strCommandLine = QuoteArgument(SAGE_POWERSHELL_PATH) +
        L" -NoProfile -ExecutionPolicy Bypass -File " + QuoteArgument(m_strScriptPath) +
        L" -TemplatePath " + QuoteArgument(m_strTemplatePath) +
        L" -OutputFolder " + QuoteArgument(strOutputFolder) +
        L" -ResultPath " + QuoteArgument(strResultPath) +
        L" -CompanyName " + QuoteArgument(m_strCompany) +
        L" -DateText " + QuoteArgument(m_strDate) +
        L" -ItemName " + QuoteArgument(m_strItemName) +
        L" -Copies " + strCopiesArg +
        L" -Pages " + strPagesArg +
        L" -UnitPrice " + strUnitArg +
        L" -CoverCost " + strCoverArg +
        L" -Freight " + strFreightArg;

    DWORD dwExitCode = 0;
    CString strError;
    BOOL bLaunched = RunProcessAndWait(strCommandLine, dwExitCode, strError);

    if (!bLaunched) {
        DeleteFileW(strResultPath);
        ShowSageMessageBox(strError.IsEmpty() ? (CString)SAGE_UI_CALC_ESTIMATE_PROCESS_FAILED : strError,
            MB_ICONERROR, this);
        return FALSE;
    }

    if (dwExitCode != 0) {
        CString strMsg = ReadScriptErrorMessage(strResultPath);
        DeleteFileW(strResultPath);
        ShowSageMessageBox(strMsg.IsEmpty() ? (CString)SAGE_UI_CALC_ESTIMATE_PROCESS_FAILED : strMsg,
            MB_ICONERROR, this);
        return FALSE;
    }

    DeleteFileW(strResultPath);
    ShowSageMessageBox(SAGE_UI_CALC_ESTIMATE_COMPLETED, MB_ICONINFORMATION, this);
    return TRUE;
}

void SageCalcEstimateDlg::OnOK() {
    if (!ValidateInputs())
        return;

    CString strOutputFolder;
    if (!SelectOutputFolder(strOutputFolder))
        return;

    if (!RunGenerate(strOutputFolder))
        return;

    CDialog::OnOK();
}

void SageCalcEstimateDlg::OnCancel() {
    CDialog::OnCancel();
}

void SageCalcEstimateDlg::OnYearChanged() {
    CString str;
    m_wndYearEdit.GetWindowText(str);
    if (str.GetLength() == SAGE_CALC_YEAR_MAX_LEN)
        m_wndMonthEdit.SetFocus();
}

void SageCalcEstimateDlg::OnMonthChanged() {
    CString str;
    m_wndMonthEdit.GetWindowText(str);
    if (str.GetLength() == SAGE_CALC_MONTH_MAX_LEN)
        m_wndDayEdit.SetFocus();
}

HBRUSH SageCalcEstimateDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageEdit)))
        return hBrush;
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushPanel;
    }
    if (nCtlColor == CTLCOLOR_STATIC) {
        HWND hWnd = pWnd->GetSafeHwnd();
        if (hWnd == m_wndDateDivider.GetSafeHwnd() ||
            hWnd == m_wndItemDivider.GetSafeHwnd()) {
            return m_brushDivider;
        }
        pDC->SetTextColor(SAGE_COLOR_SECONDARY_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushBackground;
    }

    pDC->SetBkColor(SAGE_COLOR_PANEL);
    return m_brushBackground;
}
