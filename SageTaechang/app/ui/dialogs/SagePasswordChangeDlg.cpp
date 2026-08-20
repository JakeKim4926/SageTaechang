#include "pch.h"
#include "app/ui/dialogs/SagePasswordChangeDlg.h"
#include "app/ui/dialogs/SageMessageBoxDlg.h"
#include "SageDefine.h"
#include "app/core/auth/SageAuthSession.h"
#include "app/core/auth/SageUserService.h"
#include "app/infra/db/SageDBMgr.h"
#include "app/ui/drawing/SageUiResources.h"

BEGIN_MESSAGE_MAP(SagePasswordChangeDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

SagePasswordChangeDlg::SagePasswordChangeDlg(CWnd* pParent)
    : SageFramelessDialog(pParent) {
}

SagePasswordChangeDlg::~SagePasswordChangeDlg() {}

INT_PTR SagePasswordChangeDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(SAGE_UI_CHANGE_PW_TITLE,
        SAGE_LOGIN_DLG_TEMPLATE_CX, SAGE_LOGIN_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

BOOL SagePasswordChangeDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(SAGE_UI_CHANGE_PW_TITLE);

    m_brushBackground.CreateSolidBrush(SAGE_COLOR_PANEL);
    m_brushPanel.CreateSolidBrush(SAGE_COLOR_PANEL);

    CreateCaptionBar(SAGE_UI_CHANGE_PW_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(SAGE_PASSWORD_DLG_WIDTH, LayoutControls());

    m_wndCurrentEdit.SetFocus();
    return FALSE;
}

BOOL SagePasswordChangeDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB && GetKeyState(VK_SHIFT) >= 0) {
        HWND hWnd = pMsg->hwnd;
        if (hWnd == m_wndCurrentEdit.GetSafeHwnd()) {
            m_wndNewEdit.SetFocus();
            return TRUE;
        }
        if (hWnd == m_wndNewEdit.GetSafeHwnd()) {
            m_wndConfirmEdit.SetFocus();
            return TRUE;
        }
    }
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        HWND hWnd = pMsg->hwnd;
        if (hWnd == m_wndCurrentEdit.GetSafeHwnd() ||
            hWnd == m_wndNewEdit.GetSafeHwnd() ||
            hWnd == m_wndConfirmEdit.GetSafeHwnd()) {
            OnOK();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void SagePasswordChangeDlg::CreateControls() {
    CRect r(0, 0, 0, 0);

    m_wndCurrentLabel.Create(SAGE_UI_CHANGE_PW_CURRENT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndNewLabel.Create(SAGE_UI_CHANGE_PW_NEW, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndConfirmLabel.Create(SAGE_UI_CHANGE_PW_CONFIRM, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndHint.Create(SAGE_UI_CHANGE_PW_HINT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);

    DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_PASSWORD | ES_AUTOHSCROLL;
    m_wndCurrentEdit.Create(dwEditStyle, r, this, ID_SAGE_PW_CURRENT_EDIT);
    m_wndNewEdit.Create(dwEditStyle, r, this, ID_SAGE_PW_NEW_EDIT);
    m_wndConfirmEdit.Create(dwEditStyle, r, this, ID_SAGE_PW_CONFIRM_EDIT);
    m_wndCurrentEdit.SetPasswordChar(L'*');
    m_wndNewEdit.SetPasswordChar(L'*');
    m_wndConfirmEdit.SetPasswordChar(L'*');
    m_wndNewEdit.SetLimitText(SAGE_USER_PW_MAX_LEN);
    m_wndConfirmEdit.SetLimitText(SAGE_USER_PW_MAX_LEN);

    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, r, this);
    m_wndError.SetBackgroundRole(SAGE_BG_PANEL);

    m_wndOkBtn.Create(SAGE_UI_CHANGE_PW_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(SAGE_UI_CHANGE_PW_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
}

int SagePasswordChangeDlg::LayoutControls() {
    int nM = SAGE_MARGIN;
    int nLabelW = SAGE_PASSWORD_DLG_LABEL_WIDTH;
    int nEditH = SAGE_EDIT_HEIGHT;
    int nBtnW = SAGE_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = SAGE_BUTTON_HEIGHT;
    int nGap = SAGE_ROW_GAP;
    int nClientW = SAGE_PASSWORD_DLG_WIDTH;
    int nEditW = nClientW - nM * 2 - nLabelW - nGap;

    int nRowTop = GetContentTop() + nM;
    int nEditLeft = nM + nLabelW + nGap;

    m_wndCurrentLabel.MoveWindow(nM, nRowTop, nLabelW, nEditH);
    m_wndCurrentEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndCurrentEdit);

    nRowTop += nEditH + nGap;
    m_wndNewLabel.MoveWindow(nM, nRowTop, nLabelW, nEditH);
    m_wndNewEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndNewEdit);

    nRowTop += nEditH + nGap;
    m_wndConfirmLabel.MoveWindow(nM, nRowTop, nLabelW, nEditH);
    m_wndConfirmEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndConfirmEdit);

    int nErrorTop = nRowTop + nEditH;
    m_wndError.MoveWindow(nEditLeft, nErrorTop, nEditW, SAGE_INLINE_MSG_HEIGHT);

    int nHintTop = nErrorTop + SAGE_INLINE_MSG_HEIGHT;
    m_wndHint.MoveWindow(nEditLeft, nHintTop, nEditW, SAGE_INLINE_MSG_HEIGHT);

    int nBtnTop = nHintTop + SAGE_INLINE_MSG_HEIGHT + nGap;
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void SagePasswordChangeDlg::ApplyFont() {
    m_font.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);

    m_wndCurrentLabel.SetFont(&m_font);
    m_wndCurrentLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndCurrentLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndNewLabel.SetFont(&m_font);
    m_wndNewLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndNewLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndConfirmLabel.SetFont(&m_font);
    m_wndConfirmLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndConfirmLabel.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndHint.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndHint.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndHint.SetBackgroundRole(SAGE_BG_PANEL);
    m_wndCurrentEdit.SetFont(&m_font);
    m_wndNewEdit.SetFont(&m_font);
    m_wndConfirmEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void SagePasswordChangeDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 4;
    rc.top += SAGE_EDIT_TEXT_TOP_PAD;
    rc.right -= 4;
    rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void SagePasswordChangeDlg::ShowInputError(CSageEdit& edit, const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    edit.SetState(SAGE_EDIT_ERROR);
    edit.SetFocus();
}

void SagePasswordChangeDlg::OnOK() {
    CString strCurrent;
    CString strNew;
    CString strConfirm;
    CString strError;

    m_wndError.ClearMessage();
    m_wndCurrentEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndNewEdit.SetState(SAGE_EDIT_NORMAL);
    m_wndConfirmEdit.SetState(SAGE_EDIT_NORMAL);

    m_wndCurrentEdit.GetWindowText(strCurrent);
    m_wndNewEdit.GetWindowText(strNew);
    m_wndConfirmEdit.GetWindowText(strConfirm);

    if (strCurrent.IsEmpty()) {
        ShowInputError(m_wndCurrentEdit, SAGE_UI_CHANGE_PW_EMPTY_CURRENT);
        return;
    }
    if (strNew.IsEmpty()) {
        ShowInputError(m_wndNewEdit, SAGE_UI_CHANGE_PW_EMPTY_NEW);
        return;
    }
    if (strConfirm.IsEmpty()) {
        ShowInputError(m_wndConfirmEdit, SAGE_UI_CHANGE_PW_EMPTY_CONFIRM);
        return;
    }
    if (strNew != strConfirm) {
        ShowInputError(m_wndConfirmEdit, SAGE_UI_CHANGE_PW_MISMATCH);
        m_wndConfirmEdit.SetSel(0, -1);
        return;
    }

    SageUserDto currentUser = sageAuth.GetCurrentUser();
    SageUserDto loginDto;
    BOOL bSuccess = FALSE;
    if (sageDBMgr.GetUserService()->Login(currentUser.strLoginId, strCurrent, loginDto, bSuccess, strError) == FALSE) {
        ShowSageMessageBox(strError, MB_ICONERROR, this);
        return;
    }
    if (bSuccess == FALSE) {
        ShowInputError(m_wndCurrentEdit, SAGE_UI_CHANGE_PW_CURRENT_INVALID);
        m_wndCurrentEdit.SetSel(0, -1);
        return;
    }

    if (sageDBMgr.GetUserService()->ChangePassword(currentUser.nUserId, strNew, strError) == FALSE) {
        ShowInputError(m_wndNewEdit, strError);
        m_wndNewEdit.SetSel(0, -1);
        return;
    }

    currentUser.strPwHash = SageUserService::HashPassword(strNew);
    sageAuth.SetLogin(currentUser);
    ShowSageMessageBox(SAGE_UI_CHANGE_PW_COMPLETED, MB_ICONINFORMATION, this);
    CDialog::OnOK();
}

void SagePasswordChangeDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH SagePasswordChangeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageEdit)))
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
