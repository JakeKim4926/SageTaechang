#include "pch.h"
#include "app/ui/dialogs/SageLoginDlg.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "SageDefine.h"
#include "app/core/auth/SageAuthSession.h"
#include "app/infra/db/SageDBMgr.h"

BEGIN_MESSAGE_MAP(SageLoginDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

SageLoginDlg::SageLoginDlg(CWnd* pParent)
    : SageFramelessDialog(pParent) {
}

SageLoginDlg::~SageLoginDlg() {}

INT_PTR SageLoginDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_LOGIN_DLG_TITLE,
        SAGE_LOGIN_DLG_TEMPLATE_CX, SAGE_LOGIN_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

BOOL SageLoginDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(SAGE_UI_LOGIN_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);

    CreateCaptionBar(SAGE_UI_LOGIN_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_LOGIN_DLG_WIDTH, LayoutControls());

    m_wndIdEdit.SetFocus();

    return FALSE;
}

BOOL SageLoginDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
        HWND hWnd = pMsg->hwnd;
        if (hWnd == m_wndIdEdit.GetSafeHwnd() && GetKeyState(VK_SHIFT) >= 0) {
            m_wndPwEdit.SetFocus();
            return TRUE;
        }
    }
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        HWND hWnd = pMsg->hwnd;
        if (hWnd == m_wndIdEdit.GetSafeHwnd() ||
            hWnd == m_wndPwEdit.GetSafeHwnd()) {
            OnOK();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SageLoginDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndIdLabel.Create(SAGE_UI_LOGIN_ID_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndPwLabel.Create(SAGE_UI_LOGIN_PW_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

    m_wndIdEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_SAGE_LOGIN_ID_EDIT);
    m_wndPwEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | ES_PASSWORD,
        rectEmpty, this, ID_SAGE_LOGIN_PW_EDIT);
    m_wndPwEdit.SetPasswordChar(L'*');

    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);

    m_wndOkBtn.Create(SAGE_UI_LOGIN_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_LOGIN_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);
}

int SageLoginDlg::LayoutControls() {
    int nM = SAGE_MARGIN;
    int nLabelW = SAGE_LOGIN_DLG_LABEL_WIDTH;
    int nEditH = SAGE_EDIT_HEIGHT;
    int nBtnW = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = SAGE_BUTTON_HEIGHT;
    int nGap = SAGE_ROW_GAP;
    int nClientW = SAGE_LOGIN_DLG_WIDTH;
    int nEditW = nClientW - nM * 2 - nLabelW - nGap;

    int nRow1Top = GetContentTop() + nM;
    int nRow2Top = nRow1Top + nEditH + nGap;
    int nErrorTop = nRow2Top + nEditH;
    int nBtnTop = nErrorTop + SAGE_INLINE_MSG_HEIGHT + nGap;

    m_wndIdLabel.MoveWindow(nM, nRow1Top, nLabelW, nEditH);
    m_wndIdEdit.MoveWindow(nM + nLabelW + nGap, nRow1Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndIdEdit);

    m_wndPwLabel.MoveWindow(nM, nRow2Top, nLabelW, nEditH);
    m_wndPwEdit.MoveWindow(nM + nLabelW + nGap, nRow2Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndPwEdit);

    m_wndError.MoveWindow(nM + nLabelW + nGap, nErrorTop, nEditW, SAGE_INLINE_MSG_HEIGHT);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void SageLoginDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);

    m_wndIdLabel.SetFont(&m_font);
    m_wndIdLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndIdLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndPwLabel.SetFont(&m_font);
    m_wndPwLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndPwLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndIdEdit.SetFont(&m_font);
    m_wndPwEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SageLoginDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 4;
    rc.top += SAGE_EDIT_TEXT_TOP_PAD;
    rc.right -= 4;
    rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void SageLoginDlg::ShowInputError(CSageEdit& edit, const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    edit.SetState(SAGE_EDIT_ERROR);
    edit.SetFocus();
}

void SageLoginDlg::OnOK() {
    CString strId;
    CString strPw;
    CString strError;

    m_wndError.ClearMessage();
    m_wndIdEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndPwEdit.SetState(SAGE_EDIT_NORMAL);

    m_wndIdEdit.GetWindowText(strId);
    m_wndPwEdit.GetWindowText(strPw);

    strId.Trim();

    if (strId.IsEmpty()) {
        ShowInputError(m_wndIdEdit, SAGE_UI_LOGIN_EMPTY_ID);
        return;
    }

    if (strPw.IsEmpty()) {
        ShowInputError(m_wndPwEdit, SAGE_UI_LOGIN_EMPTY_PW);
        return;
    }

    SageUserDto outDto;
    BOOL bSuccess;

    if (sageDBMgr.GetUserService()->Login(strId, strPw, outDto, bSuccess, strError) == FALSE) {
        ShowSageMessageBox(strError, MB_ICONERROR, this);
        return;
    }

    if (bSuccess == FALSE) {
        ShowInputError(m_wndPwEdit, SAGE_UI_LOGIN_FAILED);
        m_wndPwEdit.SetSel(0, -1);
        return;
    }

    sageAuth.SetLogin(outDto);

    CDialog::OnOK();
}

void SageLoginDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SageLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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
