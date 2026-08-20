#include "pch.h"
#include "app/ui/dialogs/SagePriceSimpleDlg.h"
#include "SageDefine.h"
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

BEGIN_MESSAGE_MAP(SageCompanyRenameDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

SageCompanyRenameDlg::SageCompanyRenameDlg(CWnd* pParent)
    : SageFramelessDialog(pParent), m_nPriceCount(0) {}

SageCompanyRenameDlg::~SageCompanyRenameDlg() {}

INT_PTR SageCompanyRenameDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_PRICE_RENAME_DLG_TITLE,
        SAGE_PRICE_COMPANY_DLG_TEMPLATE_CX, SAGE_PRICE_COMPANY_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString SageCompanyRenameDlg::GetCompanyName() const {
    return m_strCompanyName;
}

BOOL SageCompanyRenameDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(SAGE_UI_PRICE_RENAME_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);
    CreateCaptionBar(SAGE_UI_PRICE_RENAME_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_PRICE_COMPANY_DLG_WIDTH, LayoutControls());
    m_wndEdit.SetFocus();
    return FALSE;
}

BOOL SageCompanyRenameDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SageCompanyRenameDlg::CreateControls() {
    CRect r(0, 0, 0, 0);
    m_wndLabel.Create(SAGE_UI_PRICE_RENAME_DLG_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COMPANY_DLG_EDIT);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndHint.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndOkBtn.Create(SAGE_UI_PRICE_RENAME_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(SAGE_PRICE_COMPANY_MAX_LEN_EN);

    if (!m_strInitialName.IsEmpty())
        m_wndEdit.SetWindowTextW(m_strInitialName);

    CString strHint;
    strHint.Format(SAGE_UI_PRICE_RENAME_DLG_HINT_FMT, m_nPriceCount);
    m_wndHint.SetWindowTextW(strHint);
}

void SageCompanyRenameDlg::SetCompanyContext(const CString& strCompanyName, int nPriceCount) {
    m_strInitialName = strCompanyName;
    m_nPriceCount = nPriceCount;
}

int SageCompanyRenameDlg::LayoutControls() {
    int nM = SAGE_MARGIN;
    int nEditH = SAGE_EDIT_HEIGHT;
    int nBtnW = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = SAGE_BUTTON_HEIGHT;
    int nGap = SAGE_ROW_GAP;
    int nClientW = SAGE_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = GetContentTop() + nM;
    int nEditTop = nLabelTop + nEditH;
    int nErrorTop = nEditTop + nEditH;
    int nHintTop = nErrorTop + SAGE_INLINE_MSG_HEIGHT;
    int nBtnTop = nHintTop + SAGE_INLINE_MSG_HEIGHT + nGap;

    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, SAGE_INLINE_MSG_HEIGHT);
    m_wndHint.MoveWindow(nM, nHintTop, nEditW, SAGE_INLINE_MSG_HEIGHT);
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void SageCompanyRenameDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndEdit.SetFont(&m_font);
    m_wndHint.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndHint.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SageCompanyRenameDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void SageCompanyRenameDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndEdit.SetState(SAGE_EDIT_ERROR);
    m_wndEdit.SetFocus();
}

void SageCompanyRenameDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndEdit.SetState(SAGE_EDIT_NORMAL);

    CString strName;
    m_wndEdit.GetWindowText(strName);
    strName.Trim();
    if (strName.IsEmpty()) {
        ShowInputError(SAGE_UI_PRICE_COMPANY_REQUIRED);
        return;
    }
    BOOL bHasKorean = ContainsNonAsciiSimple(strName);
    int nMaxLen = bHasKorean ? SAGE_PRICE_COMPANY_MAX_LEN_KO : SAGE_PRICE_COMPANY_MAX_LEN_EN;
    if (strName.GetLength() > nMaxLen) {
        ShowInputError(bHasKorean ? SAGE_UI_PRICE_COMPANY_TOO_LONG_KO : SAGE_UI_PRICE_COMPANY_TOO_LONG_EN);
        m_wndEdit.SetSel(0, -1);
        return;
    }
    m_strCompanyName = strName;
    CDialog::OnOK();
}

void SageCompanyRenameDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SageCompanyRenameDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushBackground;
    }
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushPanel;
    }
    pDC->SetBkColor(SAGE_COLOR_PANEL);
    return m_brushBackground;
}

BEGIN_MESSAGE_MAP(SageCoverPriceDlg, SageFramelessDialog)
    ON_EN_CHANGE(ID_PRICE_COVER_DLG_EDIT, &SageCoverPriceDlg::OnCoverPriceChanged)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

SageCoverPriceDlg::SageCoverPriceDlg(CWnd* pParent)
    : SageFramelessDialog(pParent), m_nCoverPrice(0), m_bFormattingCoverPrice(FALSE) {}

SageCoverPriceDlg::~SageCoverPriceDlg() {}

INT_PTR SageCoverPriceDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_PRICE_COVER_DLG_TITLE,
        SAGE_PRICE_COMPANY_DLG_TEMPLATE_CX, SAGE_PRICE_COMPANY_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

int SageCoverPriceDlg::GetCoverPrice() const {
    return m_nCoverPrice;
}

BOOL SageCoverPriceDlg::OnInitDialog() {
    CDialog::OnInitDialog();
    SetWindowText(SAGE_UI_PRICE_COVER_DLG_TITLE);
    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);
    CreateCaptionBar(SAGE_UI_PRICE_COVER_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_PRICE_COMPANY_DLG_WIDTH, LayoutControls());
    m_wndEdit.SetFocus();
    return FALSE;
}

BOOL SageCoverPriceDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SageCoverPriceDlg::CreateControls() {
    CRect r(0, 0, 0, 0);
    m_wndLabel.Create(SAGE_UI_PRICE_COVER_DLG_LABEL, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL, r, this, ID_PRICE_COVER_DLG_EDIT);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.Create(SAGE_UI_PRICE_COVER_DLG_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_PRICE_COMPANY_DLG_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
    m_wndEdit.SetLimitText(10);
}

int SageCoverPriceDlg::LayoutControls() {
    int nM = SAGE_MARGIN;
    int nEditH = SAGE_EDIT_HEIGHT;
    int nBtnW = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = SAGE_BUTTON_HEIGHT;
    int nGap = SAGE_ROW_GAP;
    int nClientW = SAGE_PRICE_COMPANY_DLG_WIDTH;
    int nEditW = nClientW - nM * 2;
    int nLabelTop = GetContentTop() + nM;
    int nEditTop = nLabelTop + nEditH;
    int nErrorTop = nEditTop + nEditH;
    int nBtnTop = nErrorTop + SAGE_INLINE_MSG_HEIGHT + nGap;
    m_wndLabel.MoveWindow(nM, nLabelTop, nEditW, nEditH);
    m_wndEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    ApplyEditTextRect();
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, SAGE_INLINE_MSG_HEIGHT);
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void SageCoverPriceDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
    m_wndLabel.SetFont(&m_font);
    m_wndLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SageCoverPriceDlg::ApplyEditTextRect() {
    CRect rc;
    m_wndEdit.GetClientRect(&rc);
    rc.left += 2; rc.top += 4; rc.right -= 2; rc.bottom -= 2;
    m_wndEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void SageCoverPriceDlg::FormatPriceEditText() {
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

void SageCoverPriceDlg::OnCoverPriceChanged() {
    FormatPriceEditText();
}

void SageCoverPriceDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndEdit.SetState(SAGE_EDIT_ERROR);
    m_wndEdit.SetFocus();
}

void SageCoverPriceDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndEdit.SetState(SAGE_EDIT_NORMAL);

    CString strCover;
    m_wndEdit.GetWindowText(strCover);
    strCover.Trim();
    if (strCover.IsEmpty()) {
        ShowInputError(SAGE_UI_PRICE_COMPANY_COVER_REQUIRED);
        return;
    }
    int nCoverPrice = PriceTextToInt(strCover);
    if (nCoverPrice < 0 || nCoverPrice > SAGE_PRICE_AMOUNT_MAX) {
        ShowInputError(SAGE_UI_PRICE_AMOUNT_OUT_OF_RANGE);
        m_wndEdit.SetSel(0, -1);
        return;
    }
    m_nCoverPrice = nCoverPrice;
    CDialog::OnOK();
}

void SageCoverPriceDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SageCoverPriceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushBackground;
    }
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(SAGE_COLOR_TEXT);
        pDC->SetBkColor(SAGE_COLOR_PANEL);
        return m_brushPanel;
    }
    pDC->SetBkColor(SAGE_COLOR_PANEL);
    return m_brushBackground;
}
