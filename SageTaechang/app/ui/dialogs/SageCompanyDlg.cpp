#include "pch.h"
#include "app/ui/dialogs/SageCompanyDlg.h"
#include "SageDefine.h"
#include "app/ui/drawing/SageUiResources.h"

BEGIN_MESSAGE_MAP(SageCompanyDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

SageCompanyDlg::SageCompanyDlg(CWnd* pParent)
    : SageFramelessDialog(pParent) {
}

SageCompanyDlg::~SageCompanyDlg() {}

INT_PTR SageCompanyDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_PRICE_COMPANY_DLG_TITLE,
        SAGE_PRICE_COMPANY_DLG_TEMPLATE_CX, SAGE_PRICE_COMPANY_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString SageCompanyDlg::GetCompanyName() const {
    return m_strCompanyName;
}

BOOL SageCompanyDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(SAGE_UI_PRICE_COMPANY_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);

    CreateCaptionBar(SAGE_UI_PRICE_COMPANY_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_PRICE_COMPANY_DLG_WIDTH, LayoutControls());

    m_wndCompanyEdit.SetFocus();

    return FALSE;
}

BOOL SageCompanyDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN &&
        pMsg->hwnd == m_wndCompanyEdit.GetSafeHwnd()) {
        OnOK();
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void SageCompanyDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndLabel.Create(SAGE_UI_PRICE_COMPANY_DLG_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndCompanyEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PRICE_COMPANY_DLG_EDIT);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndHint.Create(SAGE_UI_PRICE_COMPANY_DLG_HINT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndOkBtn.Create(SAGE_UI_PRICE_COMPANY_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_PRICE_COMPANY_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndCompanyEdit.SetLimitText(SAGE_PRICE_COMPANY_MAX_LEN);
}

int SageCompanyDlg::LayoutControls() {
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
    m_wndCompanyEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, SAGE_INLINE_MSG_HEIGHT);
    m_wndHint.MoveWindow(nM, nHintTop, nEditW, SAGE_INLINE_MSG_HEIGHT);
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

    return nBtnTop + nBtnH + nM;
}

void SageCompanyDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);

    m_wndLabel.SetFont(&m_font);
    m_wndLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndCompanyEdit.SetFont(&m_font);
    m_wndHint.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndHint.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SageCompanyDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndCompanyEdit.SetState(SAGE_EDIT_ERROR);
    m_wndCompanyEdit.SetFocus();
}

void SageCompanyDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndCompanyEdit.SetState(SAGE_EDIT_NORMAL);

    CString strName;
    m_wndCompanyEdit.GetWindowText(strName);
    strName.Trim();

    if (strName.IsEmpty()) {
        ShowInputError(SAGE_UI_PRICE_COMPANY_REQUIRED);
        return;
    }

    if (strName.GetLength() > SAGE_PRICE_COMPANY_MAX_LEN) {
        ShowInputError(SAGE_UI_PRICE_COMPANY_TOO_LONG);
        m_wndCompanyEdit.SetSel(0, -1);
        return;
    }

    m_strCompanyName = strName;
    CDialog::OnOK();
}

void SageCompanyDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SageCompanyDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageEdit)))
        return hBrush;

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
