#include "pch.h"
#include "app/ui/dialogs/TaechangCompanyDlg.h"
#include "TaechangDefine.h"
#include "app/ui/drawing/SageUiResources.h"

BEGIN_MESSAGE_MAP(TaechangCompanyDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

TaechangCompanyDlg::TaechangCompanyDlg(CWnd* pParent)
    : SageFramelessDialog(pParent) {
}

TaechangCompanyDlg::~TaechangCompanyDlg() {}

INT_PTR TaechangCompanyDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(TAECHANG_UI_PRICE_COMPANY_DLG_TITLE,
        TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CX, TAECHANG_PRICE_COMPANY_DLG_TEMPLATE_CY);
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

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateCaptionBar(TAECHANG_UI_PRICE_COMPANY_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(TAECHANG_PRICE_COMPANY_DLG_WIDTH, LayoutControls());

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
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndHint.Create(TAECHANG_UI_PRICE_COMPANY_DLG_HINT,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndOkBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PRICE_COMPANY_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndCompanyEdit.SetLimitText(TAECHANG_PRICE_COMPANY_MAX_LEN);
}

int TaechangCompanyDlg::LayoutControls() {
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
    m_wndCompanyEdit.MoveWindow(nM, nEditTop, nEditW, nEditH);
    m_wndError.MoveWindow(nM, nErrorTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);
    m_wndHint.MoveWindow(nM, nHintTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);
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

void TaechangCompanyDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndLabel.SetFont(&m_font);
    m_wndLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndCompanyEdit.SetFont(&m_font);
    m_wndHint.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCompanyDlg::ShowInputError(const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    m_wndCompanyEdit.SetState(SAGE_EDIT_ERROR);
    m_wndCompanyEdit.SetFocus();
}

void TaechangCompanyDlg::OnOK() {
    m_wndError.ClearMessage();
    m_wndCompanyEdit.SetState(SAGE_EDIT_NORMAL);

    CString strName;
    m_wndCompanyEdit.GetWindowText(strName);
    strName.Trim();

    if (strName.IsEmpty()) {
        ShowInputError(TAECHANG_UI_PRICE_COMPANY_REQUIRED);
        return;
    }

    if (strName.GetLength() > TAECHANG_PRICE_COMPANY_MAX_LEN) {
        ShowInputError(TAECHANG_UI_PRICE_COMPANY_TOO_LONG);
        m_wndCompanyEdit.SetSel(0, -1);
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

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

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
