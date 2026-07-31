#include "pch.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "TaechangDefine.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "app/infra/db/SageDBMgr.h"

BEGIN_MESSAGE_MAP(TaechangLoginDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()

TaechangLoginDlg::TaechangLoginDlg(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent) {
}

TaechangLoginDlg::~TaechangLoginDlg() {}

BYTE* TaechangLoginDlg::BuildDialogTemplate() {
    const WCHAR* szTitle = TAECHANG_UI_LOGIN_DLG_TITLE;
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

INT_PTR TaechangLoginDlg::DoModal() {
    BYTE* pTemplate = BuildDialogTemplate();
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

BOOL TaechangLoginDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(TAECHANG_UI_LOGIN_DLG_TITLE);

    CRect rectClient;
    GetClientRect(&rectClient);

    int nNewWidth = TAECHANG_LOGIN_DLG_WIDTH;
    int nNewHeight = TAECHANG_LOGIN_DLG_HEIGHT;

    CRect rectWindow;
    GetWindowRect(&rectWindow);
    int nFrameW = rectWindow.Width() - rectClient.Width();
    int nFrameH = rectWindow.Height() - rectClient.Height();
    SetWindowPos(NULL, 0, 0, nNewWidth + nFrameW, nNewHeight + nFrameH,
                 SWP_NOMOVE | SWP_NOZORDER);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateControls();
    ApplyFont();
    LayoutControls();

    m_wndIdEdit.SetFocus();

    return FALSE;
}

BOOL TaechangLoginDlg::PreTranslateMessage(MSG* pMsg) {
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

void TaechangLoginDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndIdLabel.Create(TAECHANG_UI_LOGIN_ID_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndPwLabel.Create(TAECHANG_UI_LOGIN_PW_LABEL,
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);

    m_wndIdEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_TAECHANG_LOGIN_ID_EDIT);
    m_wndPwEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | ES_PASSWORD,
        rectEmpty, this, ID_TAECHANG_LOGIN_PW_EDIT);
    m_wndPwEdit.SetPasswordChar(L'*');

    m_wndOkBtn.Create(TAECHANG_UI_LOGIN_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_LOGIN_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);
}

void TaechangLoginDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nLabelW = TAECHANG_LOGIN_DLG_LABEL_WIDTH;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_LOGIN_DLG_WIDTH;
    int nEditW = nClientW - nM * 2 - nLabelW - nGap;

    int nRow1Top = nM;
    int nRow2Top = nRow1Top + nEditH + nGap;
    int nBtnTop = nRow2Top + nEditH + nM;

    m_wndIdLabel.MoveWindow(nM, nRow1Top + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndIdEdit.MoveWindow(nM + nLabelW + nGap, nRow1Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndIdEdit);

    m_wndPwLabel.MoveWindow(nM, nRow2Top + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndPwEdit.MoveWindow(nM + nLabelW + nGap, nRow2Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndPwEdit);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangLoginDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndIdLabel.SetFont(&m_font);
    m_wndPwLabel.SetFont(&m_font);
    m_wndIdEdit.SetFont(&m_font);
    m_wndPwEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangLoginDlg::ApplyEditTextRect(CEdit& edit) {
    CRect rc;
    edit.GetClientRect(&rc);
    rc.left += 4;
    rc.top += TAECHANG_EDIT_TEXT_TOP_PAD;
    rc.right -= 4;
    rc.bottom -= 2;
    edit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rc));
}

void TaechangLoginDlg::OnOK() {
    CString strId;
    CString strPw;
    CString strError;

    m_wndIdEdit.GetWindowText(strId);
    m_wndPwEdit.GetWindowText(strPw);

    strId.Trim();

    if (strId.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_LOGIN_EMPTY_ID, MB_ICONWARNING);
        m_wndIdEdit.SetFocus();
        return;
    }

    if (strPw.IsEmpty()) {
        AfxMessageBox(TAECHANG_UI_LOGIN_EMPTY_PW, MB_ICONWARNING);
        m_wndPwEdit.SetFocus();
        return;
    }

    TaechangUserDto outDto;
    BOOL bSuccess;

    if (sageDBMgr.GetUserService()->Login(strId, strPw, outDto, bSuccess, strError) == FALSE) {
        AfxMessageBox(strError, MB_ICONERROR);
        return;
    }

    if (bSuccess == FALSE) {
        AfxMessageBox(TAECHANG_UI_LOGIN_FAILED, MB_ICONWARNING);
        m_wndPwEdit.SetSel(0, -1);
        m_wndPwEdit.SetFocus();
        return;
    }

    taechangAuth.SetLogin(outDto);

    CDialog::OnOK();
}

void TaechangLoginDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
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

void TaechangLoginDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
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
