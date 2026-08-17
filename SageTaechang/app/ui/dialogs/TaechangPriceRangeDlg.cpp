#include "pch.h"
#include "app/ui/dialogs/TaechangPriceRangeDlg.h"
#include "TaechangDefine.h"

namespace
{
    CString FormatPriceText(int nPrice) {
        CString strText;
        strText.Format(L"%d", nPrice);
        for (int i = strText.GetLength() - 3; i > 0; i -= 3)
            strText.Insert(i, L',');
        return strText;
    }

    CString RemovePriceSeparators(const CString& strText) {
        CString strResult = strText;
        strResult.Remove(L',');
        strResult.Trim();
        return strResult;
    }

    int PriceTextToInt(const CString& strText) {
        CString strValue = RemovePriceSeparators(strText);
        return strValue.IsEmpty() ? 0 : _wtoi(strValue);
    }
}

BEGIN_MESSAGE_MAP(TaechangPriceRangeDlg, SageFramelessDialog)
    ON_BN_CLICKED(ID_PRICE_RANGE_DLG_NO_MAX_CHECK, &TaechangPriceRangeDlg::OnNoMaxCheck)
    ON_BN_CLICKED(ID_PRICE_RANGE_DLG_SINGLE_CHECK, &TaechangPriceRangeDlg::OnSingleCheck)
    ON_EN_CHANGE(ID_PRICE_RANGE_DLG_PRINT_EDIT, &TaechangPriceRangeDlg::OnPrintPriceChanged)
    ON_EN_CHANGE(ID_PRICE_RANGE_DLG_COVER_EDIT, &TaechangPriceRangeDlg::OnCoverPriceChanged)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

TaechangPriceRangeDlg::TaechangPriceRangeDlg(CWnd* pParent)
    : SageFramelessDialog(pParent),
      m_nMinCopies(0),
      m_bHasMaxCopies(TRUE),
      m_nMaxCopies(0),
      m_nPrintPrice(0),
      m_nCoverPrice(0),
      m_bFormattingPrintPrice(FALSE),
      m_bFormattingCoverPrice(FALSE) {
}

TaechangPriceRangeDlg::~TaechangPriceRangeDlg() {}

INT_PTR TaechangPriceRangeDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(TAECHANG_UI_PRICE_RANGE_DLG_TITLE,
        TAECHANG_PRICE_RANGE_DLG_TEMPLATE_CX, TAECHANG_PRICE_RANGE_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

int TaechangPriceRangeDlg::GetMinCopies() const {
    return m_nMinCopies;
}

BOOL TaechangPriceRangeDlg::HasMaxCopies() const {
    return m_bHasMaxCopies;
}

int TaechangPriceRangeDlg::GetMaxCopies() const {
    return m_nMaxCopies;
}

int TaechangPriceRangeDlg::GetPrintPrice() const {
    return m_nPrintPrice;
}

int TaechangPriceRangeDlg::GetCoverPrice() const {
    return m_nCoverPrice;
}

void TaechangPriceRangeDlg::AddExistingRange(int nMinCopies, BOOL bHasMaxCopies, int nMaxCopies) {
    m_arrExistingMinCopies.Add(nMinCopies);
    m_arrExistingHasMaxCopies.Add(bHasMaxCopies ? TRUE : FALSE);
    m_arrExistingMaxCopies.Add(nMaxCopies);
}

BOOL TaechangPriceRangeDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(TAECHANG_UI_PRICE_RANGE_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateCaptionBar(TAECHANG_UI_PRICE_RANGE_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(TAECHANG_PRICE_RANGE_DLG_WIDTH, LayoutControls());

    m_wndMinEdit.SetFocus();
    return FALSE;
}

BOOL TaechangPriceRangeDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        (pMsg->hwnd == m_wndMinEdit.GetSafeHwnd() ||
         pMsg->hwnd == m_wndMaxEdit.GetSafeHwnd() ||
         pMsg->hwnd == m_wndPrintEdit.GetSafeHwnd() ||
         pMsg->hwnd == m_wndCoverEdit.GetSafeHwnd())) {
        OnOK();
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangPriceRangeDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndMinLabel.Create(TAECHANG_UI_PRICE_MIN_COPIES_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndMinEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_RANGE_DLG_MIN_EDIT);
    m_wndMinUnit.Create(TAECHANG_UI_PRICE_COPIES_UNIT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndSingleCheck.Create(TAECHANG_UI_PRICE_SINGLE_LABEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_PRICE_RANGE_DLG_SINGLE_CHECK);
    m_wndSingleCheck.SetFrameVisible(FALSE);
    m_wndMaxLabel.Create(TAECHANG_UI_PRICE_MAX_COPIES_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndMaxEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_NUMBER | ES_RIGHT | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_RANGE_DLG_MAX_EDIT);
    m_wndMaxUnit.Create(TAECHANG_UI_PRICE_COPIES_UNIT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndNoMaxCheck.Create(TAECHANG_UI_PRICE_RANGE_NO_MAX_LABEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, ID_PRICE_RANGE_DLG_NO_MAX_CHECK);
    m_wndNoMaxCheck.SetFrameVisible(FALSE);
    m_wndPrintLabel.Create(TAECHANG_UI_PRICE_PRINT_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndPrintEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_RIGHT | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_RANGE_DLG_PRINT_EDIT);
    m_wndPrintUnit.Create(TAECHANG_UI_PRICE_WON_UNIT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndCoverLabel.Create(TAECHANG_UI_PRICE_COVER_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndCoverEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_RIGHT | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_RANGE_DLG_COVER_EDIT);
    m_wndCoverUnit.Create(TAECHANG_UI_PRICE_WON_UNIT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_RANGE_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndMinEdit.SetLimitText(7);
    m_wndMaxEdit.SetLimitText(7);
    m_wndPrintEdit.SetLimitText(10);
    m_wndCoverEdit.SetLimitText(10);
}

int TaechangPriceRangeDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PRICE_RANGE_DLG_WIDTH;
    int nLabelW = TAECHANG_PRICE_RANGE_DLG_LABEL_WIDTH;
    int nCopiesW = TAECHANG_PRICE_RANGE_COPIES_EDIT_WIDTH;
    int nPriceW = TAECHANG_PRICE_RANGE_PRICE_EDIT_WIDTH;
    int nUnitW = TAECHANG_PRICE_RANGE_UNIT_WIDTH;
    int nFieldLeft = nM + nLabelW + nGap;

    int nY = GetContentTop() + nM;
    m_wndMinLabel.MoveWindow(nM, nY + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndMinEdit.MoveWindow(nFieldLeft, nY, nCopiesW, nEditH);
    ApplyEditTextRect(m_wndMinEdit);
    m_wndMinUnit.MoveWindow(nFieldLeft + nCopiesW + nGap, nY + TAECHANG_LABEL_VERT_OFFSET, nUnitW, nEditH);
    nY += nEditH + nGap;

    m_wndMaxLabel.MoveWindow(nM, nY + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndMaxEdit.MoveWindow(nFieldLeft, nY, nCopiesW, nEditH);
    ApplyEditTextRect(m_wndMaxEdit);
    m_wndMaxUnit.MoveWindow(nFieldLeft + nCopiesW + nGap, nY + TAECHANG_LABEL_VERT_OFFSET, nUnitW, nEditH);
    nY += nEditH + nGap;

    int nNoMaxW = m_wndNoMaxCheck.GetContentWidth();
    m_wndNoMaxCheck.MoveWindow(nFieldLeft, nY, nNoMaxW, nEditH);
    m_wndSingleCheck.MoveWindow(nFieldLeft + nNoMaxW + TAECHANG_PRICE_RANGE_CHECK_GAP, nY,
        m_wndSingleCheck.GetContentWidth(), nEditH);
    nY += nEditH + nGap;

    m_wndPrintLabel.MoveWindow(nM, nY + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndPrintEdit.MoveWindow(nFieldLeft, nY, nPriceW, nEditH);
    ApplyEditTextRect(m_wndPrintEdit);
    m_wndPrintUnit.MoveWindow(nFieldLeft + nPriceW + nGap, nY + TAECHANG_LABEL_VERT_OFFSET, nUnitW, nEditH);
    nY += nEditH + nGap;

    m_wndCoverLabel.MoveWindow(nM, nY + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndCoverEdit.MoveWindow(nFieldLeft, nY, nPriceW, nEditH);
    ApplyEditTextRect(m_wndCoverEdit);
    m_wndCoverUnit.MoveWindow(nFieldLeft + nPriceW + nGap, nY + TAECHANG_LABEL_VERT_OFFSET, nUnitW, nEditH);
    nY += nEditH;

    m_wndError.MoveWindow(nFieldLeft, nY, nClientW - nM - nFieldLeft, TAECHANG_INLINE_MSG_HEIGHT);
    nY += TAECHANG_INLINE_MSG_HEIGHT + nGap;

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nY, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nY, nBtnW, nBtnH);

    return nY + nBtnH + nM;
}

void TaechangPriceRangeDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndMinLabel.SetFont(&m_font);
    m_wndMinLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndMinLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndMinEdit.SetFont(&m_font);
    m_wndMinUnit.SetFont(&m_font);
    m_wndMinUnit.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndMinUnit.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndSingleCheck.SetFont(&m_font);
    m_wndMaxLabel.SetFont(&m_font);
    m_wndMaxLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndMaxLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndMaxEdit.SetFont(&m_font);
    m_wndMaxUnit.SetFont(&m_font);
    m_wndMaxUnit.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndMaxUnit.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndNoMaxCheck.SetFont(&m_font);
    m_wndPrintLabel.SetFont(&m_font);
    m_wndPrintLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndPrintLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndPrintEdit.SetFont(&m_font);
    m_wndPrintUnit.SetFont(&m_font);
    m_wndPrintUnit.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndPrintUnit.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndCoverLabel.SetFont(&m_font);
    m_wndCoverLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndCoverLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndCoverEdit.SetFont(&m_font);
    m_wndCoverUnit.SetFont(&m_font);
    m_wndCoverUnit.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndCoverUnit.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangPriceRangeDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rectEdit;
    edit.GetClientRect(&rectEdit);
    rectEdit.left += 2;
    rectEdit.top += 4;
    rectEdit.right -= 2;
    rectEdit.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rectEdit));
}

void TaechangPriceRangeDlg::ShowInputError(CSageEdit& edit, const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    edit.SetState(SAGE_EDIT_ERROR);
    edit.SetFocus();
}

void TaechangPriceRangeDlg::ClearInputError() {
    m_wndError.ClearMessage();
    m_wndMinEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndMaxEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndPrintEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndCoverEdit.SetState(SAGE_EDIT_NORMAL);
}

void TaechangPriceRangeDlg::OnNoMaxCheck() {
    BOOL bNoMax = m_wndNoMaxCheck.IsChecked();
    if (bNoMax && m_wndSingleCheck.IsChecked()) {
        m_wndError.SetMessage(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, SAGE_INLINE_WARNING);
        m_wndNoMaxCheck.SetChecked(FALSE);
        return;
    }
    m_wndError.ClearMessage();
    m_wndMaxEdit.EnableWindow(!bNoMax);
    if (bNoMax)
        m_wndMaxEdit.SetWindowTextW(L"");
}

void TaechangPriceRangeDlg::OnSingleCheck() {
    BOOL bSingle = m_wndSingleCheck.IsChecked();
    if (bSingle && m_wndNoMaxCheck.IsChecked()) {
        m_wndError.SetMessage(TAECHANG_UI_PRICE_SINGLE_AND_NO_MAX_CONFLICT, SAGE_INLINE_WARNING);
        m_wndSingleCheck.SetChecked(FALSE);
        return;
    }
    m_wndError.ClearMessage();
    m_wndMaxEdit.EnableWindow(!bSingle);
    if (bSingle)
        m_wndMaxEdit.SetWindowTextW(L"");
}

void TaechangPriceRangeDlg::FormatPriceEditText(CEdit& edit, BOOL& bFormatting) {
    if (bFormatting)
        return;

    CString strText;
    edit.GetWindowTextW(strText);
    CString strDigits = RemovePriceSeparators(strText);
    if (strDigits.IsEmpty())
        return;

    for (int i = 0; i < strDigits.GetLength(); ++i) {
        wchar_t ch = strDigits[i];
        if (ch < L'0' || ch > L'9')
            return;
    }

    CString strFormatted = FormatPriceText(_wtoi(strDigits));
    if (strFormatted == strText)
        return;

    bFormatting = TRUE;
    edit.SetWindowTextW(strFormatted);
    edit.SetSel(strFormatted.GetLength(), strFormatted.GetLength());
    bFormatting = FALSE;
}

void TaechangPriceRangeDlg::OnPrintPriceChanged() {
    FormatPriceEditText(m_wndPrintEdit, m_bFormattingPrintPrice);
}

void TaechangPriceRangeDlg::OnCoverPriceChanged() {
    FormatPriceEditText(m_wndCoverEdit, m_bFormattingCoverPrice);
}

BOOL TaechangPriceRangeDlg::IsCopiesRangeOverlap(int nMinA, BOOL bHasMaxA, int nMaxA, int nMinB, BOOL bHasMaxB, int nMaxB) const {
    int nEndA = bHasMaxA ? nMaxA : INT_MAX;
    int nEndB = bHasMaxB ? nMaxB : INT_MAX;
    return (nMinA <= nEndB && nMinB <= nEndA) ? TRUE : FALSE;
}

BOOL TaechangPriceRangeDlg::IsOverlappingExistingRange(int nMinCopies, BOOL bHasMaxCopies, int nMaxCopies) const {
    int nCount = static_cast<int>(m_arrExistingMinCopies.GetCount());
    for (int i = 0; i < nCount; ++i) {
        BOOL bExistingHasMax = m_arrExistingHasMaxCopies.GetAt(i) ? TRUE : FALSE;
        if (IsCopiesRangeOverlap(
            nMinCopies,
            bHasMaxCopies,
            nMaxCopies,
            m_arrExistingMinCopies.GetAt(i),
            bExistingHasMax,
            m_arrExistingMaxCopies.GetAt(i))) {
            return TRUE;
        }
    }
    return FALSE;
}

void TaechangPriceRangeDlg::OnOK() {
    ClearInputError();

    CString strMin, strMax, strPrint, strCover;
    m_wndMinEdit.GetWindowText(strMin);
    m_wndMaxEdit.GetWindowText(strMax);
    m_wndPrintEdit.GetWindowText(strPrint);
    m_wndCoverEdit.GetWindowText(strCover);
    strMin.Trim(); strMax.Trim(); strPrint.Trim(); strCover.Trim();

    int nMin = strMin.IsEmpty() ? 0 : _wtoi(strMin);
    if (nMin < 1 || nMin > TAECHANG_PRICE_COPIES_MAX) {
        ShowInputError(m_wndMinEdit, TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE);
        m_wndMinEdit.SetSel(0, -1);
        return;
    }

    BOOL bSingle = m_wndSingleCheck.IsChecked();
    BOOL bHasMax = m_wndNoMaxCheck.IsChecked() ? FALSE : TRUE;
    int nMax = 0;
    if (bSingle) {
        nMax = nMin;
    } else if (bHasMax) {
        nMax = strMax.IsEmpty() ? 0 : _wtoi(strMax);
        if (nMax < 1 || nMax > TAECHANG_PRICE_COPIES_MAX) {
            ShowInputError(m_wndMaxEdit, TAECHANG_UI_PRICE_COPIES_OUT_OF_RANGE);
            m_wndMaxEdit.SetSel(0, -1);
            return;
        }
        if (nMin >= nMax) {
            ShowInputError(m_wndMaxEdit, TAECHANG_UI_PRICE_MIN_LESS_THAN_MAX);
            m_wndMaxEdit.SetSel(0, -1);
            return;
        }
    }

    int nPrint = strPrint.IsEmpty() ? -1 : PriceTextToInt(strPrint);
    if (nPrint < 0 || nPrint > TAECHANG_PRICE_AMOUNT_MAX) {
        ShowInputError(m_wndPrintEdit, TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE);
        m_wndPrintEdit.SetSel(0, -1);
        return;
    }

    int nCover = strCover.IsEmpty() ? -1 : PriceTextToInt(strCover);
    if (nCover < 0 || nCover > TAECHANG_PRICE_AMOUNT_MAX) {
        ShowInputError(m_wndCoverEdit, TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE);
        m_wndCoverEdit.SetSel(0, -1);
        return;
    }

    if (IsOverlappingExistingRange(nMin, bHasMax, nMax)) {
        m_wndError.SetMessage(TAECHANG_UI_PRICE_RANGE_OVERLAP, SAGE_INLINE_WARNING);
        m_wndMinEdit.SetState(SAGE_EDIT_ERROR);
        m_wndMinEdit.SetSel(0, -1);
        m_wndMinEdit.SetFocus();
        return;
    }

    m_nMinCopies = nMin;
    m_bHasMaxCopies = bHasMax;
    m_nMaxCopies = nMax;
    m_nPrintPrice = nPrint;
    m_nCoverPrice = nCover;

    CDialog::OnOK();
}

void TaechangPriceRangeDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangPriceRangeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushBackground;
    }

    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }

    pDC->SetBkColor(TAECHANG_COLOR_PANEL);
    return m_brushBackground;
}
