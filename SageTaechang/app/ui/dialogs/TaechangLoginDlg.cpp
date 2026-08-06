#include "pch.h"
#include "app/ui/dialogs/TaechangLoginDlg.h"
#include "TaechangDefine.h"
#include "app/core/auth/TaechangAuthSession.h"
#include "app/infra/db/SageDBMgr.h"

BEGIN_MESSAGE_MAP(TaechangLoginDlg, CDialog)
    ON_WM_CTLCOLOR()
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

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateControls();
    ApplyFont();
    SageDialogSizer::SizeToClient(*this, TAECHANG_LOGIN_DLG_WIDTH, LayoutControls());

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

    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);

    m_wndOkBtn.Create(TAECHANG_UI_LOGIN_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_LOGIN_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);
}

int TaechangLoginDlg::LayoutControls() {
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
    int nErrorTop = nRow2Top + nEditH;
    int nBtnTop = nErrorTop + TAECHANG_INLINE_MSG_HEIGHT + nGap;

    m_wndIdLabel.MoveWindow(nM, nRow1Top + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndIdEdit.MoveWindow(nM + nLabelW + nGap, nRow1Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndIdEdit);

    m_wndPwLabel.MoveWindow(nM, nRow2Top + TAECHANG_LABEL_VERT_OFFSET, nLabelW, nEditH);
    m_wndPwEdit.MoveWindow(nM + nLabelW + nGap, nRow2Top, nEditW, nEditH);
    ApplyEditTextRect(m_wndPwEdit);

    m_wndError.MoveWindow(nM + nLabelW + nGap, nErrorTop, nEditW, TAECHANG_INLINE_MSG_HEIGHT);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);

    return nBtnTop + nBtnH + nM;
}

void TaechangLoginDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndIdLabel.SetFont(&m_font);
    m_wndIdLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndPwLabel.SetFont(&m_font);
    m_wndPwLabel.SetTextColorRole(SAGE_TEXT_MUTED);
    m_wndIdEdit.SetFont(&m_font);
    m_wndPwEdit.SetFont(&m_font);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
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

void TaechangLoginDlg::ShowInputError(CSageEdit& edit, const CString& strMessage) {
    m_wndError.SetMessage(strMessage, SAGE_INLINE_ERROR);
    edit.SetState(SAGE_EDIT_ERROR);
    edit.SetFocus();
}

void TaechangLoginDlg::OnOK() {
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
        ShowInputError(m_wndIdEdit, TAECHANG_UI_LOGIN_EMPTY_ID);
        return;
    }

    if (strPw.IsEmpty()) {
        ShowInputError(m_wndPwEdit, TAECHANG_UI_LOGIN_EMPTY_PW);
        return;
    }

    TaechangUserDto outDto;
    BOOL bSuccess;

    if (sageDBMgr.GetUserService()->Login(strId, strPw, outDto, bSuccess, strError) == FALSE) {
        AfxMessageBox(strError, MB_ICONERROR);
        return;
    }

    if (bSuccess == FALSE) {
        ShowInputError(m_wndPwEdit, TAECHANG_UI_LOGIN_FAILED);
        m_wndPwEdit.SetSel(0, -1);
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
