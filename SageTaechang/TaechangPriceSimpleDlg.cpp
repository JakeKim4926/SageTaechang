#include "pch.h"
#include "TaechangPriceSimpleDlg.h"
#include "TaechangDefine.h"

static BYTE* BuildSimpleDialogTemplate(LPCWSTR pszTitle) {
    const WCHAR* szFont = TAECHANG_CONTROL_FONT_FACE;
    const WORD wFontSize = TAECHANG_LOGIN_DLG_FONT_PT;

    size_t nTitleLen = wcslen(pszTitle) + 1;
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
    pDlg->x = 0;
    pDlg->y = 0;
    pDlg->cx = TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CX;
    pDlg->cy = TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CY;
    p += sizeof(DLGTEMPLATE);

    *(WORD*)p = 0; p += 2;
    *(WORD*)p = 0; p += 2;

    memcpy(p, pszTitle, nTitleLen * sizeof(WCHAR));
    p += nTitleLen * sizeof(WCHAR);

    if (((ULONG_PTR)(p - pBuf)) % 2 != 0)
        p++;

    *(WORD*)p = wFontSize; p += 2;
    memcpy(p, szFont, nFontLen * sizeof(WCHAR));

    return pBuf;
}

static BOOL ContainsNonAsciiSimple(const CString& str) {
    for (int i = 0; i < str.GetLength(); ++i) {
        if (str[i] > 127)
            return TRUE;
    }
    return FALSE;
}

static void DrawSimpleButton(CFont& font, int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    if (lpDrawItemStruct->CtlType != ODT_BUTTON)
        return;

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
    CFont* pOldFont = pDC->SelectObject(&font);
    rect.OffsetRect(0, TAECHANG_BUTTON_TEXT_TOP_OFFSET);
    pDC->DrawText(strText, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (pOldFont)
        pDC->SelectObject(pOldFont);
}

BEGIN_MESSAGE_MAP(TaechangCompanyRenameDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

TaechangCompanyRenameDlg::TaechangCompanyRenameDlg(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent) {}

TaechangCompanyRenameDlg::~TaechangCompanyRenameDlg() {}

BYTE* TaechangCompanyRenameDlg::BuildDialogTemplate() {
    return BuildSimpleDialogTemplate(TAECHANG_UI_PRICE_RENAME_DLG_TITLE);
}

INT_PTR TaechangCompanyRenameDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString TaechangCompanyRenameDlg::GetCompanyName() const {
    return m_strCompanyName;
}

BOOL TaechangCompanyRenameDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(TAECHANG_UI_PRICE_RENAME_DLG_TITLE);

    CRect rectClient;
    GetClientRect(&rectClient);
    CRect rectWindow;
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0,
        TAECHANG_PRICE_COMPANY_DLG_WIDTH + nFrameW,
        TAECHANG_PRICE_COMPANY_DLG_HEIGHT + nFrameH,
        SWP_NOMOVE | SWP_NOZORDER);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    CreateControls();
    ApplyFont();
    LayoutControls();
    m_wndEdit.SetFocus();
    return FALSE;
}

BOOL TaechangCompanyRenameDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangCompanyRenameDlg::CreateControls() {
    CRect r(0, 0, 0, 0);
    m_wndLabel.Create(TAECHANG_UI_PRICE_RENAME_DLG_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COMPANY_DLG_EDIT);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_RENAME_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);
}

void TaechangCompanyRenameDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = nM;
    int nEditTop = nLabelTop + nEditH;
    int nBtnTop = nEditTop + nEditH + nM;

    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangCompanyRenameDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCompanyRenameDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangCompanyRenameDlg::OnOK() {
    CString strName;
    m_wndEdit.GetWindowText(strName);
    strName.Trim();
    if (strName.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_REQUIRED, MB_ICONWARNING);
        m_wndEdit.SetFocus();
        return;
    }
    BOOL bHasKorean = ContainsNonAsciiSimple(strName);
    int nMaxLen = bHasKorean ? TAECHANG_PRICE_COMPANY_MAX_LEN_KO : TAECHANG_PRICE_COMPANY_MAX_LEN_EN;
    if (strName.GetLength() > nMaxLen) {
        AfxMessageBox(bHasKorean ? TAECHANG_UI_PRICE_COMPANY_TOO_LONG_KO : TAECHANG_UI_PRICE_COMPANY_TOO_LONG_EN, MB_ICONWARNING);
        m_wndEdit.SetSel(0, -1);
        m_wndEdit.SetFocus();
        return;
    }
    m_strCompanyName = strName;
    CDialog::OnOK();
}

void TaechangCompanyRenameDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCompanyRenameDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushBackground;
    }
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }
    pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
    return m_brushBackground;
}

void TaechangCompanyRenameDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    if (lpDrawItemStruct->CtlType != ODT_BUTTON) {
        CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
        return;
    }
    DrawSimpleButton(m_font, nIDCtl, lpDrawItemStruct);
}

BEGIN_MESSAGE_MAP(TaechangCoverPriceDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

TaechangCoverPriceDlg::TaechangCoverPriceDlg(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent), m_nCoverPrice(0) {}

TaechangCoverPriceDlg::~TaechangCoverPriceDlg() {}

BYTE* TaechangCoverPriceDlg::BuildDialogTemplate() {
    return BuildSimpleDialogTemplate(TAECHANG_UI_PRICE_COVER_DLG_TITLE);
}

INT_PTR TaechangCoverPriceDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

int TaechangCoverPriceDlg::GetCoverPrice() const {
    return m_nCoverPrice;
}

BOOL TaechangCoverPriceDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(TAECHANG_UI_PRICE_COVER_DLG_TITLE);
    CRect rectClient;
    GetClientRect(&rectClient);
    CRect rectWindow;
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0,
        TAECHANG_PRICE_COMPANY_DLG_WIDTH + nFrameW,
        TAECHANG_PRICE_COMPANY_DLG_HEIGHT + nFrameH,
        SWP_NOMOVE | SWP_NOZORDER);
    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    CreateControls();
    ApplyFont();
    LayoutControls();
    m_wndEdit.SetFocus();
    return FALSE;
}

BOOL TaechangCoverPriceDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangCoverPriceDlg::CreateControls() {
    CRect r(0, 0, 0, 0);
    m_wndLabel.Create(TAECHANG_UI_PRICE_COVER_DLG_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_DLG_EDIT);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_COVER_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(8);
}

void TaechangCoverPriceDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = nM;
    int nEditTop = nLabelTop + nEditH;
    int nBtnTop = nEditTop + nEditH + nM;
    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangCoverPriceDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCoverPriceDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangCoverPriceDlg::OnOK() {
    CString strCover;
    m_wndEdit.GetWindowText(strCover);
    strCover.Trim();
    if (strCover.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_COVER_REQUIRED, MB_ICONWARNING);
        m_wndEdit.SetFocus();
        return;
    }
    int nCoverPrice = _wtoi(strCover);
    if (nCoverPrice < 0 || nCoverPrice > TAECHANG_PRICE_AMOUNT_MAX) {
        AfxMessageBox(TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE, MB_ICONWARNING);
        m_wndEdit.SetSel(0, -1);
        m_wndEdit.SetFocus();
        return;
    }
    m_nCoverPrice = nCoverPrice;
    CDialog::OnOK();
}

void TaechangCoverPriceDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCoverPriceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
        return m_brushBackground;
    }
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }
    pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
    return m_brushBackground;
}

void TaechangCoverPriceDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    if (lpDrawItemStruct->CtlType != ODT_BUTTON) {
        CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
        return;
    }
    DrawSimpleButton(m_font, nIDCtl, lpDrawItemStruct);
}
