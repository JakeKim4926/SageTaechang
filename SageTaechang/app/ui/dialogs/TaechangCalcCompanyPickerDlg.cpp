#include "pch.h"
#include "app/ui/dialogs/TaechangCalcCompanyPickerDlg.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(TaechangCalcCompanyPickerDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
    ON_EN_CHANGE(ID_PICKER_DLG_SEARCH_EDIT, &TaechangCalcCompanyPickerDlg::OnSearchChanged)
    ON_LBN_DBLCLK(ID_PICKER_DLG_LIST, &TaechangCalcCompanyPickerDlg::OnListDblClick)
END_MESSAGE_MAP()

TaechangCalcCompanyPickerDlg::TaechangCalcCompanyPickerDlg(const CStringArray& arrNames, const CString& strInitialName, CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent), m_strInitialName(strInitialName) {
    m_arrAllNames.Copy(arrNames);
}

TaechangCalcCompanyPickerDlg::~TaechangCalcCompanyPickerDlg() {}

BYTE* TaechangCalcCompanyPickerDlg::BuildDialogTemplate() {
    const WCHAR* szTitle = TAECHANG_UI_PICKER_DLG_TITLE;
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
    pDlg->cx = TAECHANG_PICKER_DLG_TEMPLATE_CX;
    pDlg->cy = TAECHANG_PICKER_DLG_TEMPLATE_CY;
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

INT_PTR TaechangCalcCompanyPickerDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString TaechangCalcCompanyPickerDlg::GetSelectedName() const {
    return m_strSelectedName;
}

BOOL TaechangCalcCompanyPickerDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(TAECHANG_UI_PICKER_DLG_TITLE);

    CRect rectClient;
    GetClientRect(&rectClient);

    CRect rectWindow;
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0,
        TAECHANG_PICKER_DLG_WIDTH + nFrameW,
        TAECHANG_PICKER_DLG_HEIGHT + nFrameH,
        SWP_NOMOVE | SWP_NOZORDER);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateControls();
    ApplyFont();
    LayoutControls();
    ApplySearchEditTextRect();

    FilterList(CString());

    if (!m_strInitialName.IsEmpty()) {
        int nIdx = m_wndNameList.FindStringExact(-1, m_strInitialName);
        if (nIdx != LB_ERR) {
            m_wndNameList.SetCurSel(nIdx);
            m_wndNameList.SetTopIndex(nIdx);
        }
    }

    m_wndSearchEdit.SetFocus();

    return FALSE;
}

BOOL TaechangCalcCompanyPickerDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        if (pMsg->hwnd == m_wndSearchEdit.GetSafeHwnd()) {
            m_wndNameList.SetFocus();
            if (m_wndNameList.GetCurSel() == LB_ERR && m_wndNameList.GetCount() > 0)
                m_wndNameList.SetCurSel(0);
            return TRUE;
        }
        if (pMsg->hwnd == m_wndNameList.GetSafeHwnd()) {
            OnOK();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangCalcCompanyPickerDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndSearchEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PICKER_DLG_SEARCH_EDIT);
    m_wndNameList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        rectEmpty, this, ID_PICKER_DLG_LIST);
    m_wndOkBtn.Create(TAECHANG_UI_PICKER_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PICKER_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndSearchEdit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)(LPCWSTR)TAECHANG_UI_PICKER_SEARCH_CUE);
}

void TaechangCalcCompanyPickerDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PICKER_DLG_WIDTH;
    int nClientH = TAECHANG_PICKER_DLG_HEIGHT;
    int nContentW = nClientW - nM * 2;

    int nSearchTop = nM;
    int nListTop = nSearchTop + nEditH + nGap;
    int nBtnTop = nClientH - nM - nBtnH;
    int nListH = nBtnTop - nListTop - nGap;
    if (nListH < nEditH * 4)
        nListH = nEditH * 4;

    m_wndSearchEdit.MoveWindow(nM, nSearchTop, nContentW, nEditH);
    m_wndNameList.MoveWindow(nM, nListTop, nContentW, nListH);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangCalcCompanyPickerDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndSearchEdit.SetFont(&m_font);
    m_wndNameList.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCalcCompanyPickerDlg::ApplySearchEditTextRect() {
    CRect rectEdit;
    m_wndSearchEdit.GetClientRect(&rectEdit);
    rectEdit.left += 2;
    rectEdit.top += 4;
    rectEdit.right -= 2;
    rectEdit.bottom -= 2;
    m_wndSearchEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rectEdit));
}

void TaechangCalcCompanyPickerDlg::FilterList(const CString& strKeyword) {
    m_wndNameList.SetRedraw(FALSE);
    m_wndNameList.ResetContent();
    for (int i = 0; i < m_arrAllNames.GetSize(); i++) {
        if (strKeyword.IsEmpty() || m_arrAllNames[i].Find(strKeyword) >= 0)
            m_wndNameList.AddString(m_arrAllNames[i]);
    }
    m_wndNameList.SetRedraw(TRUE);
    if (m_wndNameList.GetCount() > 0)
        m_wndNameList.SetCurSel(0);
}

void TaechangCalcCompanyPickerDlg::OnSearchChanged() {
    CString strKeyword;
    m_wndSearchEdit.GetWindowText(strKeyword);
    strKeyword.Trim();
    FilterList(strKeyword);
}

void TaechangCalcCompanyPickerDlg::OnListDblClick() {
    OnOK();
}

void TaechangCalcCompanyPickerDlg::OnOK() {
    int nSel = m_wndNameList.GetCurSel();
    if (nSel == LB_ERR) {
        AfxMessageBox(TAECHANG_UI_PICKER_SELECT_REQUIRED, MB_ICONWARNING);
        return;
    }
    m_wndNameList.GetText(nSel, m_strSelectedName);
    CDialog::OnOK();
}

void TaechangCalcCompanyPickerDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCalcCompanyPickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }

    if (nCtlColor == CTLCOLOR_LISTBOX) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }

    pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
    return m_brushBackground;
}

void TaechangCalcCompanyPickerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
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
