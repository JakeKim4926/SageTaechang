#include "pch.h"
#include "app/ui/dialogs/TaechangPriceSimpleDlg.h"
#include "TaechangDefine.h"
#include "app/ui/drawing/SageUiResources.h"

static CString FormatPriceText(int nPrice) {
    CString strText;
    strText.Format(L"%d", nPrice);
    for (int i = strText.GetLength() - 3; i > 0; i -= 3)
        strText.Insert(i, L',');
    return strText;
}

static CString RemovePriceSeparators(const CString& strText) {
    CString strResult = strText;
    strResult.Remove(L',');
    strResult.Trim();
    return strResult;
}

static int PriceTextToInt(const CString& strText) {
    CString strValue = RemovePriceSeparators(strText);
    return strValue.IsEmpty() ? 0 : _wtoi(strValue);
}

static BOOL ContainsNonAsciiSimple(const CString& str) {
    for (int i = 0; i < str.GetLength(); ++i) {
        if (str[i] > 127)
            return TRUE;
    }
    return FALSE;
}

BEGIN_MESSAGE_MAP(TaechangCompanyRenameDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

TaechangCompanyRenameDlg::TaechangCompanyRenameDlg(CWnd* pParent)
    : SageFramelessDialog(pParent), m_nPriceCount(0) {}

TaechangCompanyRenameDlg::~TaechangCompanyRenameDlg() {}

INT_PTR TaechangCompanyRenameDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(TAECHANG_UI_PRICE_RENAME_DLG_TITLE,
        TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CX, TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CY);
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

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    CreateCaptionBar(TAECHANG_UI_PRICE_RENAME_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(TAECHANG_PRICE_COMPANY_DLG_WIDTH, LayoutControls());
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
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndHint.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_RENAME_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(TAECHANG_PRICE_COMPANY_MAX_LEN_EN);

    if (!m_strInitialName.IsEmpty())
        m_wndEdit.SetWindowTextW(m_strInitialName);

    CString strHint;
    strHint.Format(TAECHANG_UI_PRICE_RENAME_DLG_HINT_FMT, m_nPriceCount);
    m_wndHint.SetWindowTextW(strHint);
}

void TaechangCompanyRenameDlg::SetCompanyContext(const CString& strCompanyName, int nPriceCount) {
    m_strInitialName = strCompanyName;
    m_nPriceCount = nPriceCount;
}

int TaechangCompanyRenameDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = GetContentTop() + nM;
    int nEditTop = nLabelTop + nEditH;
    int nErrorTop = nEditTop + nEditH;
    int nHintTop = nErrorTop + TAECHANG_INLINE_MSG_HEIGHT;
    int nBtnTop = nHintTop + TAECHANG_INLINE_MSG_HEIGHT + nGap;

    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);
    m_wndHint.MoveWindow(nM, nHintTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void TaechangCompanyRenameDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndEdit.SetFont(&m_font);
    m_wndHint.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCompanyRenameDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangCompanyRenameDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndEdit.SetState(SAGE_EDIT_ERROR);
    m_wndEdit.SetFocus();
}

void TaechangCompanyRenameDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndEdit.SetState(SAGE_EDIT_NORMAL);

    CString strName;
    m_wndEdit.GetWindowText(strName);
    strName.Trim();
    if (strName.IsEmpty()) {
        ShowInputError(TAECHANG_UI_PRICE_COMPANY_REQUIRED);
        return;
    }
    BOOL bHasKorean = ContainsNonAsciiSimple(strName);
    int nMaxLen = bHasKorean ? TAECHANG_PRICE_COMPANY_MAX_LEN_KO : TAECHANG_PRICE_COMPANY_MAX_LEN_EN;
    if (strName.GetLength() > nMaxLen) {
        ShowInputError(bHasKorean ? TAECHANG_UI_PRICE_COMPANY_TOO_LONG_KO : TAECHANG_UI_PRICE_COMPANY_TOO_LONG_EN);
        m_wndEdit.SetSel(0, -1);
        return;
    }
    m_strCompanyName = strName;
    CDialog::OnOK();
}

void TaechangCompanyRenameDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCompanyRenameDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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

BEGIN_MESSAGE_MAP(TaechangCoverPriceDlg, SageFramelessDialog)
    ON_EN_CHANGE(ID_PRICE_COVER_DLG_EDIT, &TaechangCoverPriceDlg::OnCoverPriceChanged)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

TaechangCoverPriceDlg::TaechangCoverPriceDlg(CWnd* pParent)
    : SageFramelessDialog(pParent), m_nCoverPrice(0), m_bFormattingCoverPrice(FALSE) {}

TaechangCoverPriceDlg::~TaechangCoverPriceDlg() {}

INT_PTR TaechangCoverPriceDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(TAECHANG_UI_PRICE_COVER_DLG_TITLE,
        TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CX, TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CY);
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
    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);
    CreateCaptionBar(TAECHANG_UI_PRICE_COVER_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(TAECHANG_PRICE_COMPANY_DLG_WIDTH, LayoutControls());
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
    m_wndEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_DLG_EDIT);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_COVER_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(10);
}

int TaechangCoverPriceDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = GetContentTop() + nM;
    int nEditTop = nLabelTop + nEditH;
    int nErrorTop = nEditTop + nEditH;
    int nBtnTop = nErrorTop + TAECHANG_INLINE_MSG_HEIGHT + nGap;
    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void TaechangCoverPriceDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCoverPriceDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangCoverPriceDlg::FormatPriceEditText() {
    if (m_bFormattingCoverPrice)
        return;

    CString strText;
    m_wndEdit.GetWindowTextW(strText);
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

    m_bFormattingCoverPrice = TRUE;
    m_wndEdit.SetWindowTextW(strFormatted);
    m_wndEdit.SetSel(strFormatted.GetLength(), strFormatted.GetLength());
    m_bFormattingCoverPrice = FALSE;
}

void TaechangCoverPriceDlg::OnCoverPriceChanged() {
    FormatPriceEditText();
}

void TaechangCoverPriceDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndEdit.SetState(SAGE_EDIT_ERROR);
    m_wndEdit.SetFocus();
}

void TaechangCoverPriceDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndEdit.SetState(SAGE_EDIT_NORMAL);

    CString strCover;
    m_wndEdit.GetWindowText(strCover);
    strCover.Trim();
    if (strCover.IsEmpty()) {
        ShowInputError(TAECHANG_UI_PRICE_COMPANY_COVER_REQUIRED);
        return;
    }
    int nCoverPrice = PriceTextToInt(strCover);
    if (nCoverPrice < 0 || nCoverPrice > TAECHANG_PRICE_AMOUNT_MAX) {
        ShowInputError(TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE);
        m_wndEdit.SetSel(0, -1);
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
