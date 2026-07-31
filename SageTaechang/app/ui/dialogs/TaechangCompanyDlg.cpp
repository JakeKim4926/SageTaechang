#include "pch.h"
#include "app/ui/dialogs/TaechangCompanyDlg.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(TaechangCompanyDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

TaechangCompanyDlg::TaechangCompanyDlg(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent) {
}

TaechangCompanyDlg::~TaechangCompanyDlg() {}

BYTE* TaechangCompanyDlg::BuildDialogTemplate() {
    const WCHAR* szTitle = TAECHANG_UI_PRICE_COMPANY_DLG_TITLE;
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
    pDlg->x = 0;
    pDlg->y = 0;
    pDlg->cx = TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CX;
    pDlg->cy = TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CY;
    p += sizeof(DLGTEMPLATE);

    *(WORD*)p = 0; p += 2;
    *(WORD*)p = 0; p += 2;

    memcpy(p, szTitle, nTitleLen * sizeof(WCHAR));
    p += nTitleLen * sizeof(WCHAR);

    if (((ULONG_PTR)(p - pBuf)) % 2 != 0)
        p++;

    *(WORD*)p = wFontSize; p += 2;
    memcpy(p, szFont, nFontLen * sizeof(WCHAR));

    return pBuf;
}

INT_PTR TaechangCompanyDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString TaechangCompanyDlg::GetCompanyName() const {
    return m_strCompanyName;
}

BOOL TaechangCompanyDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(TAECHANG_UI_PRICE_COMPANY_DLG_TITLE);

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

    m_wndCompanyEdit.SetFocus();

    return FALSE;
}

BOOL TaechangCompanyDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndCompanyEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangCompanyDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndLabel.Create(TAECHANG_UI_PRICE_COMPANY_DLG_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndCompanyEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_COMPANY_DLG_EDIT);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndCompanyEdit.SetLimitText(TAECHANG_PRICE_COMPANY_MAX_LEN);
}

void TaechangCompanyDlg::LayoutControls() {
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
    m_wndCompanyEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    CRect rectEdit;
    m_wndCompanyEdit.GetClientRect(&rectEdit);
    rectEdit.left += 2;
    rectEdit.top += 4;
    rectEdit.right -= 2;
    rectEdit.bottom -= 2;
    m_wndCompanyEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rectEdit));

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangCompanyDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndLabel.SetFont(&m_font);
    m_wndCompanyEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCompanyDlg::OnOK() {
    CString strName;
    m_wndCompanyEdit.GetWindowText(strName);
    strName.Trim();

    if (strName.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_REQUIRED, MB_ICONWARNING);
        m_wndCompanyEdit.SetFocus();
        return;
    }

    if (strName.GetLength() > TAECHANG_PRICE_COMPANY_MAX_LEN) {
        AfxMessageBox(TAECHANG_UI_PRICE_COMPANY_TOO_LONG, MB_ICONWARNING);
        m_wndCompanyEdit.SetSel(0, -1);
        m_wndCompanyEdit.SetFocus();
        return;
    }

    m_strCompanyName = strName;
    CDialog::OnOK();
}

void TaechangCompanyDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCompanyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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

void TaechangCompanyDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
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
