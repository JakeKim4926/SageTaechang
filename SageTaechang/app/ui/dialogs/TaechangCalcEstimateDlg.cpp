#include "pch.h"
#include "app/ui/dialogs/TaechangCalcEstimateDlg.h"
#include "TaechangDefine.h"
#include "app/infra/file/TaechangFileUtils.h"
#include "app/common/TaechangJson.h"

BEGIN_MESSAGE_MAP(TaechangCalcEstimateDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
    ON_EN_CHANGE(ID_CALC_ESTIMATE_DLG_YEAR_EDIT,  &TaechangCalcEstimateDlg::OnYearChanged)
    ON_EN_CHANGE(ID_CALC_ESTIMATE_DLG_MONTH_EDIT, &TaechangCalcEstimateDlg::OnMonthChanged)
END_MESSAGE_MAP()

TaechangCalcEstimateDlg::TaechangCalcEstimateDlg(
    const CString& strCompany,
    int nCopies, int nPages,
    int nUnitPrice, int nCoverPrice, int nFreight,
    const CString& strTemplatePath,
    const CString& strScriptPath,
    CWnd* pParent)
    : CDialog((UINT)0, pParent)
    , m_pDlgParent(pParent)
    , m_strCompany(strCompany)
    , m_nCopies(nCopies), m_nPages(nPages)
    , m_nUnitPrice(nUnitPrice), m_nCoverPrice(nCoverPrice), m_nFreight(nFreight)
    , m_strTemplatePath(strTemplatePath)
    , m_strScriptPath(strScriptPath) {
}

TaechangCalcEstimateDlg::~TaechangCalcEstimateDlg() {}

BYTE* TaechangCalcEstimateDlg::BuildDialogTemplate() {
    const WCHAR* szTitle = TAECHANG_UI_CALC_ESTIMATE_DLG_TITLE;
    const WCHAR* szFont = TAECHANG_CONTROL_FONT_FACE;
    const WORD wFontSize = TAECHANG_LOGIN_DLG_FONT_PT;

    size_t nTitleLen = wcslen(szTitle) + 1;
    size_t nFontLen = wcslen(szFont) + 1;
    size_t nBufSize = sizeof(DLGTEMPLATE)
        + sizeof(WORD) * 2
        + nTitleLen * sizeof(WCHAR)
        + sizeof(WORD) * 4
        + nFontLen * sizeof(WCHAR);

    BYTE* pBuf = new BYTE[nBufSize]();
    BYTE* p = pBuf;

    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_SETFONT | DS_CENTER;
    pDlg->dwExtendedStyle = 0;
    pDlg->cdit = 0;
    pDlg->x = 0; pDlg->y = 0;
    pDlg->cx = TAECHANG_CALC_ESTIMATE_DLG_TEMPLATE_CX;
    pDlg->cy = TAECHANG_CALC_ESTIMATE_DLG_TEMPLATE_CY;
    p += sizeof(DLGTEMPLATE);

    *(WORD*)p = 0; p += 2;
    *(WORD*)p = 0; p += 2;
    memcpy(p, szTitle, nTitleLen * sizeof(WCHAR));
    p += nTitleLen * sizeof(WCHAR);
    if (((ULONG_PTR)(p - pBuf)) % 2 != 0) p++;
    *(WORD*)p = wFontSize; p += 2;
    memcpy(p, szFont, nFontLen * sizeof(WCHAR));

    return pBuf;
}

INT_PTR TaechangCalcEstimateDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString TaechangCalcEstimateDlg::GetItemName() const {
    return m_strItemName;
}

CString TaechangCalcEstimateDlg::GetDate() const {
    return m_strDate;
}

BOOL TaechangCalcEstimateDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(TAECHANG_UI_CALC_ESTIMATE_DLG_TITLE);

    CRect rectClient, rectWindow;
    GetClientRect(&rectClient);
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0,
        TAECHANG_CALC_ESTIMATE_DLG_WIDTH + nFrameW,
        TAECHANG_CALC_ESTIMATE_DLG_HEIGHT + nFrameH,
        SWP_NOMOVE | SWP_NOZORDER);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    m_brushDivider.CreateSolidBrush(TAECHANG_COLOR_BORDER);

    CreateControls();
    ApplyFont();
    LayoutControls();

    m_wndYearEdit.SetFocus();
    return FALSE;
}

void TaechangCalcEstimateDlg::CreateControls() {
    CRect r(0, 0, 0, 0);

    m_wndDateLabel.Create(TAECHANG_UI_CALC_ESTIMATE_DATE_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT, r, this);
    m_wndDateDivider.Create(L"", WS_CHILD | WS_VISIBLE, r, this);
    m_wndYearEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_YEAR_EDIT);
    m_wndDateSep1.Create(TAECHANG_UI_CALC_ESTIMATE_DATE_SEP,
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
    m_wndMonthEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_MONTH_EDIT);
    m_wndDateSep2.Create(TAECHANG_UI_CALC_ESTIMATE_DATE_SEP,
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, r, this);
    m_wndDayEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_CENTER,
        r, this, ID_CALC_ESTIMATE_DLG_DAY_EDIT);
    m_wndItemLabel.Create(TAECHANG_UI_CALC_ESTIMATE_ITEM_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT, r, this);
    m_wndItemDivider.Create(L"", WS_CHILD | WS_VISIBLE, r, this);
    m_wndItemEdit.Create(
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        r, this, ID_CALC_ESTIMATE_DLG_ITEM_EDIT);
    m_wndOkBtn.Create(TAECHANG_UI_CALC_ESTIMATE_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_CALC_ESTIMATE_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);

    m_wndYearEdit.SetLimitText(TAECHANG_CALC_YEAR_MAX_LEN);
    m_wndMonthEdit.SetLimitText(TAECHANG_CALC_MONTH_MAX_LEN);
    m_wndDayEdit.SetLimitText(TAECHANG_CALC_DAY_MAX_LEN);
    m_wndItemEdit.SetLimitText(TAECHANG_CALC_ITEM_MAX_LEN);

    m_wndYearEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)TAECHANG_UI_CALC_ESTIMATE_YEAR_CUE);
    m_wndMonthEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)TAECHANG_UI_CALC_ESTIMATE_MONTH_CUE);
    m_wndDayEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)TAECHANG_UI_CALC_ESTIMATE_DAY_CUE);
    m_wndItemEdit.SendMessage(EM_SETCUEBANNER, TRUE,
        (LPARAM)(LPCWSTR)TAECHANG_UI_CALC_ESTIMATE_ITEM_CUE);
}

void TaechangCalcEstimateDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
    m_wndDateLabel.SetFont(&m_font);
    m_wndYearEdit.SetFont(&m_font);
    m_wndDateSep1.SetFont(&m_font);
    m_wndMonthEdit.SetFont(&m_font);
    m_wndDateSep2.SetFont(&m_font);
    m_wndDayEdit.SetFont(&m_font);
    m_wndItemLabel.SetFont(&m_font);
    m_wndItemEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCalcEstimateDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangCalcEstimateDlg::LayoutControls() {
    int nM         = TAECHANG_MARGIN;
    int nEditH     = TAECHANG_EDIT_HEIGHT;
    int nBtnW      = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH      = TAECHANG_BUTTON_HEIGHT;
    int nBtnGap    = TAECHANG_ROW_GAP;
    int nLabelH    = TAECHANG_CALC_ESTIMATE_DLG_LABEL_H;
    int nLabelGap  = TAECHANG_CALC_ESTIMATE_DLG_LABEL_GAP;
    int nGroupGap  = TAECHANG_CALC_ESTIMATE_DLG_GROUP_GAP;
    int nDivGap    = TAECHANG_CALC_ESTIMATE_DLG_DIV_GAP;
    int nYearW     = TAECHANG_CALC_ESTIMATE_DLG_YEAR_W;
    int nMdW       = TAECHANG_CALC_ESTIMATE_DLG_MD_W;
    int nSepW      = TAECHANG_CALC_ESTIMATE_DLG_SEP_W;
    int nClientW   = TAECHANG_CALC_ESTIMATE_DLG_WIDTH;
    int nContentW  = nClientW - nM * 2;

    int nY = nM;

    // 날짜 레이블 → 구분선 → 입력
    m_wndDateLabel.MoveWindow(nM, nY, nContentW, nLabelH);
    nY += nLabelH + nDivGap;
    m_wndDateDivider.MoveWindow(nM, nY, nContentW, 1);
    nY += 1 + nDivGap;

    int nX = nM;
    m_wndYearEdit.MoveWindow(nX, nY, nYearW, nEditH);
    ApplyEditTextRect(m_wndYearEdit);
    nX += nYearW;
    m_wndDateSep1.MoveWindow(nX, nY + TAECHANG_LABEL_VERT_OFFSET, nSepW, nEditH);
    nX += nSepW;
    m_wndMonthEdit.MoveWindow(nX, nY, nMdW, nEditH);
    ApplyEditTextRect(m_wndMonthEdit);
    nX += nMdW;
    m_wndDateSep2.MoveWindow(nX, nY + TAECHANG_LABEL_VERT_OFFSET, nSepW, nEditH);
    nX += nSepW;
    m_wndDayEdit.MoveWindow(nX, nY, nMdW, nEditH);
    ApplyEditTextRect(m_wndDayEdit);
    nY += nEditH + nGroupGap;

    // 품목명 레이블 → 구분선 → 입력
    m_wndItemLabel.MoveWindow(nM, nY, nContentW, nLabelH);
    nY += nLabelH + nDivGap;
    m_wndItemDivider.MoveWindow(nM, nY, nContentW, 1);
    nY += 1 + nDivGap;

    m_wndItemEdit.MoveWindow(nM, nY, nContentW, nEditH);
    ApplyEditTextRect(m_wndItemEdit);

    // 버튼
    int nBtnTop   = TAECHANG_CALC_ESTIMATE_DLG_HEIGHT - nM - nBtnH;
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nBtnGap, nBtnTop, nBtnW, nBtnH);
}

BOOL TaechangCalcEstimateDlg::PreTranslateMessage(MSG* pMsg) {
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

BOOL TaechangCalcEstimateDlg::ValidateInputs() {
    CString strYear, strMonth, strDay;
    m_wndYearEdit.GetWindowText(strYear);
    m_wndMonthEdit.GetWindowText(strMonth);
    m_wndDayEdit.GetWindowText(strDay);
    strYear.Trim(); strMonth.Trim(); strDay.Trim();

    if (strYear.IsEmpty() || strMonth.IsEmpty() || strDay.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_DATE_REQUIRED, MB_ICONWARNING);
        m_wndYearEdit.SetFocus();
        return FALSE;
    }

    int nYear  = _wtoi(strYear);
    int nMonth = _wtoi(strMonth);
    int nDay   = _wtoi(strDay);
    BOOL bDateOk = (nYear >= 1900 && nYear <= 9999)
                && (nMonth >= 1 && nMonth <= 12)
                && (nDay >= 1 && nDay <= 31);
    if (!bDateOk) {
        AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_DATE_INVALID, MB_ICONWARNING);
        m_wndYearEdit.SetFocus();
        return FALSE;
    }

    CString strItem;
    m_wndItemEdit.GetWindowText(strItem);
    strItem.Trim();
    if (strItem.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_ITEM_REQUIRED, MB_ICONWARNING);
        m_wndItemEdit.SetFocus();
        return FALSE;
    }
    if (strItem.GetLength() > TAECHANG_CALC_ITEM_MAX_LEN) {
        AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_ITEM_TOO_LONG, MB_ICONWARNING);
        m_wndItemEdit.SetFocus();
        return FALSE;
    }

    m_strDate.Format(L"%04d-%02d-%02d", nYear, nMonth, nDay);
    m_strItemName = strItem;
    return TRUE;
}

BOOL TaechangCalcEstimateDlg::SelectOutputFolder(CString& strFolder) {
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
    pDialog->SetTitle(TAECHANG_UI_SELECT_OUTPUT_TITLE);

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

BOOL TaechangCalcEstimateDlg::RunGenerate(const CString& strOutputFolder) {
    CString strResultPath = BuildTempJsonPath(L"tcc");

    CString strCopiesArg, strPagesArg, strUnitArg, strCoverArg, strFreightArg;
    strCopiesArg.Format(L"%d", m_nCopies);
    strPagesArg.Format(L"%d", m_nPages);
    strUnitArg.Format(L"%d", m_nUnitPrice);
    strCoverArg.Format(L"%d", m_nCoverPrice);
    strFreightArg.Format(L"%d", m_nFreight);

    CString strCommandLine = QuoteArgument(TAECHANG_POWERSHELL_PATH) +
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
        AfxMessageBox(strError.IsEmpty() ? (CString)TAECHANG_UI_CALC_ESTIMATE_PROCESS_FAILED : strError,
            MB_ICONERROR);
        return FALSE;
    }

    if (dwExitCode != 0) {
        CString strMsg = ReadScriptErrorMessage(strResultPath);
        DeleteFileW(strResultPath);
        AfxMessageBox(strMsg.IsEmpty() ? (CString)TAECHANG_UI_CALC_ESTIMATE_PROCESS_FAILED : strMsg,
            MB_ICONERROR);
        return FALSE;
    }

    DeleteFileW(strResultPath);
    AfxMessageBox(TAECHANG_UI_CALC_ESTIMATE_COMPLETED, MB_ICONINFORMATION);
    return TRUE;
}

void TaechangCalcEstimateDlg::OnOK() {
    if (!ValidateInputs())
        return;

    CString strOutputFolder;
    if (!SelectOutputFolder(strOutputFolder))
        return;

    if (!RunGenerate(strOutputFolder))
        return;

    CDialog::OnOK();
}

void TaechangCalcEstimateDlg::OnCancel() {
    CDialog::OnCancel();
}

void TaechangCalcEstimateDlg::OnYearChanged() {
    CString str;
    m_wndYearEdit.GetWindowText(str);
    if (str.GetLength() == TAECHANG_CALC_YEAR_MAX_LEN)
        m_wndMonthEdit.SetFocus();
}

void TaechangCalcEstimateDlg::OnMonthChanged() {
    CString str;
    m_wndMonthEdit.GetWindowText(str);
    if (str.GetLength() == TAECHANG_CALC_MONTH_MAX_LEN)
        m_wndDayEdit.SetFocus();
}

HBRUSH TaechangCalcEstimateDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }
    if (nCtlColor == CTLCOLOR_STATIC) {
        HWND hWnd = pWnd->GetSafeHwnd();
        if (hWnd == m_wndDateDivider.GetSafeHwnd() ||
            hWnd == m_wndItemDivider.GetSafeHwnd()) {
            return m_brushDivider;
        }
        pDC->SetTextColor(TAECHANG_COLOR_SECONDARY_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushBackground;
    }

    pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
    return m_brushBackground;
}

void TaechangCalcEstimateDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    if (lpDrawItemStruct->CtlType != ODT_BUTTON) {
        CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
        return;
    }

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rect = lpDrawItemStruct->rcItem;
    BOOL bPressed = (lpDrawItemStruct->itemState & ODS_SELECTED) != 0;
    BOOL bDisabled = (lpDrawItemStruct->itemState & ODS_DISABLED) != 0;
    BOOL bPrimary = (nIDCtl == IDOK);

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
    CFont* pOldFont = pDC->SelectObject(&m_font);
    rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
    pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (pOldFont)
        pDC->SelectObject(pOldFont);
}
