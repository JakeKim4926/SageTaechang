#include "pch.h"
#include "TaechangPasswordChangeDlg.h"
#include "TaechangDefine.h"
#include "TaechangAuthSession.h"
#include "TaechangUserService.h"
#include "SageDBMgr.h"

BEGIN_MESSAGE_MAP(TaechangPasswordChangeDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

TaechangPasswordChangeDlg::TaechangPasswordChangeDlg(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent) {
}

TaechangPasswordChangeDlg::~TaechangPasswordChangeDlg() {}

BYTE* TaechangPasswordChangeDlg::BuildDialogTemplate() {
    const WCHAR* szTitle = TAECHANG_UI_CHANGE_PW_TITLE;
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
    pDlg->cx = TAECHANG_LOGIN_DLG_TEMPLATE_CX;
    pDlg->cy = TAECHANG_LOGIN_DLG_TEMPLATE_CY;
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

INT_PTR TaechangPasswordChangeDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

BOOL TaechangPasswordChangeDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(TAECHANG_UI_CHANGE_PW_TITLE);

    CRect rectClient;
    GetClientRect(&rectClient);

    CRect rectWindow;
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0,
        TAECHANG_PASSWORD_DLG_WIDTH + nFrameW,
        TAECHANG_PASSWORD_DLG_HEIGHT + nFrameH,
        SWP_NOMOVE | SWP_NOZORDER);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateControls();
    ApplyFont();
    LayoutControls();

    m_wndCurrentEdit.SetFocus();
    return FALSE;
}

BOOL TaechangPasswordChangeDlg::PreTranslateMessage(MSG* pMsg) {
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

void TaechangPasswordChangeDlg::CreateControls() {
    CRect r(0, 0, 0, 0);

    m_wndCurrentLabel.Create(TAECHANG_UI_CHANGE_PW_CURRENT, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndNewLabel.Create(TAECHANG_UI_CHANGE_PW_NEW, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);
    m_wndConfirmLabel.Create(TAECHANG_UI_CHANGE_PW_CONFIRM, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, r, this);

    DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_PASSWORD | ES_AUTOHSCROLL;
    m_wndCurrentEdit.Create(dwEditStyle, r, this, ID_TAECHANG_PW_CURRENT_EDIT);
    m_wndNewEdit.Create(dwEditStyle, r, this, ID_TAECHANG_PW_NEW_EDIT);
    m_wndConfirmEdit.Create(dwEditStyle, r, this, ID_TAECHANG_PW_CONFIRM_EDIT);
    m_wndCurrentEdit.SetPasswordChar(L'*');
    m_wndNewEdit.SetPasswordChar(L'*');
    m_wndConfirmEdit.SetPasswordChar(L'*');

    m_wndOkBtn.Create(TAECHANG_UI_CHANGE_PW_OK, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_CHANGE_PW_CANCEL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, r, this, IDCANCEL);
}

void TaechangPasswordChangeDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nLabelW = TAECHANG_PASSWORD_DLG_LABEL_WIDTH;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PASSWORD_DLG_WIDTH;
    int nEditW = nClientW - nM * 2 - nLabelW - nGap;

    int nRowTop = nM;
    int nEditLeft = nM + nLabelW + nGap;

    m_wndCurrentLabel.MoveWindow(nM, nRowTop + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndCurrentEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndCurrentEdit);

    nRowTop += nEditH + nGap;
    m_wndNewLabel.MoveWindow(nM, nRowTop + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndNewEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndNewEdit);

    nRowTop += nEditH + nGap;
    m_wndConfirmLabel.MoveWindow(nM, nRowTop + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndConfirmEdit.MoveWindow(nEditLeft, nRowTop, nEditW, nEditH);
    ApplyEditTextRect(m_wndConfirmEdit);

    int nBtnTop = nRowTop + nEditH + nM;
    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangPasswordChangeDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndCurrentLabel.SetFont(&m_font);
    m_wndNewLabel.SetFont(&m_font);
    m_wndConfirmLabel.SetFont(&m_font);
    m_wndCurrentEdit.SetFont(&m_font);
    m_wndNewEdit.SetFont(&m_font);
    m_wndConfirmEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangPasswordChangeDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 4;
    rc.top += TAECHANG_EDIT_TEXT_TOP_PAD;
    rc.right -= 4;
    rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangPasswordChangeDlg::OnOK() {
    CString strCurrent;
    CString strNew;
    CString strConfirm;
    CString strError;

    m_wndCurrentEdit.GetWindowText(strCurrent);
    m_wndNewEdit.GetWindowText(strNew);
    m_wndConfirmEdit.GetWindowText(strConfirm);

    if (strCurrent.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_EMPTY_CURRENT, MB_ICONWARNING);
        m_wndCurrentEdit.SetFocus();
        return;
    }
    if (strNew.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_EMPTY_NEW, MB_ICONWARNING);
        m_wndNewEdit.SetFocus();
        return;
    }
    if (strNew.GetLength() < TAECHANG_USER_PW_MIN_LEN) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_TOO_SHORT, MB_ICONWARNING);
        m_wndNewEdit.SetSel(0, -1);
        m_wndNewEdit.SetFocus();
        return;
    }
    if (strConfirm.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_EMPTY_CONFIRM, MB_ICONWARNING);
        m_wndConfirmEdit.SetFocus();
        return;
    }
    if (strNew != strConfirm) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_MISMATCH, MB_ICONWARNING);
        m_wndNewEdit.SetSel(0, -1);
        m_wndNewEdit.SetFocus();
        return;
    }

    TaechangUserDto currentUser = taechangAuth.GetCurrentUser();
    TaechangUserDto loginDto;
    BOOL bSuccess = FALSE;
    if (sageDBMgr.GetUserService()->Login(currentUser.strLoginId, strCurrent, loginDto, bSuccess, strError) == FALSE) {
        AfxMessageBox(strError, MB_ICONERROR);
        return;
    }
    if (bSuccess == FALSE) {
        AfxMessageBox(TAECHANG_UI_CHANGE_PW_CURRENT_INVALID, MB_ICONWARNING);
        m_wndCurrentEdit.SetSel(0, -1);
        m_wndCurrentEdit.SetFocus();
        return;
    }

    if (sageDBMgr.GetUserService()->ChangePassword(currentUser.nUserId, strNew, strError) == FALSE) {
        AfxMessageBox(strError, MB_ICONWARNING);
        m_wndNewEdit.SetSel(0, -1);
        m_wndNewEdit.SetFocus();
        return;
    }

    currentUser.strPwHash = TaechangUserService::HashPassword(strNew);
    taechangAuth.SetLogin(currentUser);
    AfxMessageBox(TAECHANG_UI_CHANGE_PW_COMPLETED, MB_ICONINFORMATION);
    CDialog::OnOK();
}

void TaechangPasswordChangeDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangPasswordChangeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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

void TaechangPasswordChangeDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
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
        CBrush brBorder(TAECHANG_COLOR_PRIMARY);
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
