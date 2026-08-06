#include "pch.h"
#include "app/ui/dialogs/SageFramelessDialog.h"
#include "app/ui/dialogs/SageDialogSizer.h"
#include "TaechangDefine.h"

BEGIN_MESSAGE_MAP(SageFramelessDialog, CDialog)
    ON_WM_NCHITTEST()
    ON_BN_CLICKED(ID_TAECHANG_DLG_CLOSE, &SageFramelessDialog::OnCaptionClose)
END_MESSAGE_MAP()

SageFramelessDialog::SageFramelessDialog(CWnd* pParent)
    : CDialog((UINT)0, pParent), m_pDlgParent(pParent) {
}

BYTE* SageFramelessDialog::BuildFramelessTemplate(LPCWSTR pszTitle, int nTemplateCx, int nTemplateCy) {
    const WCHAR* szFont = TAECHANG_CONTROL_FONT_FACE;
    const WORD wFontSize = TAECHANG_LOGIN_DLG_FONT_PT;

    size_t nTitleLen = wcslen(pszTitle) + 1;
    size_t nFontLen = wcslen(szFont) + 1;
    size_t nBufSize = sizeof(DLGTEMPLATE)
        + sizeof(WORD) * 2
        + nTitleLen * sizeof(WCHAR)
        + sizeof(WORD) * 4
        + nFontLen * sizeof(WCHAR);

    BYTE* pBuf = new BYTE[nBufSize]();
    BYTE* p = pBuf;

    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = WS_POPUP | WS_BORDER | DS_SETFONT | DS_CENTER;
    pDlg->dwExtendedStyle = 0;
    pDlg->cdit = 0;
    pDlg->x = 0;
    pDlg->y = 0;
    pDlg->cx = (short)nTemplateCx;
    pDlg->cy = (short)nTemplateCy;
    p += sizeof(DLGTEMPLATE);

    *(WORD*)p = 0; p += 2;
    *(WORD*)p = 0; p += 2;

    memcpy(p, pszTitle, nTitleLen * sizeof(WCHAR));
    p += nTitleLen * sizeof(WCHAR);

    if (((ULONG_PTR)(p - pBuf)) % 2 != 0)
        p++;

    *(WORD*)p = wFontSize; p += 2;
    memcpy(p, szFont, nFontLen * sizeof(WCHAR));

    return pBuf;
}

BOOL SageFramelessDialog::CreateCaptionBar(LPCWSTR pszTitle) {
    return m_wndCaption.Create(this, pszTitle, ID_TAECHANG_DLG_CLOSE);
}

int SageFramelessDialog::GetContentTop() const {
    return TAECHANG_DLG_CAPTION_HEIGHT;
}

void SageFramelessDialog::SizeFramelessClient(int nClientWidth, int nContentBottom) {
    SageDialogSizer::SizeToClient(*this, nClientWidth, nContentBottom);
    m_wndCaption.Layout(nClientWidth);
}

LRESULT SageFramelessDialog::OnNcHitTest(CPoint point) {
    LRESULT nHit = CDialog::OnNcHitTest(point);
    if (nHit != HTCLIENT)
        return nHit;

    CPoint ptClient = point;
    ScreenToClient(&ptClient);
    if (ptClient.y < TAECHANG_DLG_CAPTION_HEIGHT)
        return HTCAPTION;

    return nHit;
}

void SageFramelessDialog::OnCaptionClose() {
    SendMessage(WM_COMMAND, IDCANCEL);
}
