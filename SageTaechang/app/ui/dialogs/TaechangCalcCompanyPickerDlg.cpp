#include "pch.h"
#include "app/ui/dialogs/TaechangCalcCompanyPickerDlg.h"
#include "TaechangDefine.h"
#include "app/ui/drawing/SageUiResources.h"

BEGIN_MESSAGE_MAP(TaechangCalcCompanyPickerDlg, SageFramelessDialog)
    ON_WM_CTLCOLOR()
    ON_EN_CHANGE(ID_PICKER_DLG_SEARCH_EDIT, &TaechangCalcCompanyPickerDlg::OnSearchChanged)
    ON_LBN_DBLCLK(ID_PICKER_DLG_LIST, &TaechangCalcCompanyPickerDlg::OnListDblClick)
END_MESSAGE_MAP()

TaechangCalcCompanyPickerDlg::TaechangCalcCompanyPickerDlg(const CStringArray& arrNames, const CString& strInitialName, CWnd* pParent)
    : SageFramelessDialog(pParent), m_strInitialName(strInitialName) {
    m_arrAllNames.Copy(arrNames);
}

TaechangCalcCompanyPickerDlg::~TaechangCalcCompanyPickerDlg() {}

INT_PTR TaechangCalcCompanyPickerDlg::DoModal() {
    BYTE* pTemplate = BuildFramelessTemplate(TAECHANG_UI_PICKER_DLG_TITLE,
        TAECHANG_PICKER_DLG_TEMPLATE_CX, TAECHANG_PICKER_DLG_TEMPLATE_CY);
    InitModalIndirect((DLGTEMPLATE*)pTemplate, m_pDlgParent);
    INT_PTR nResult = CDialog::DoModal();
    delete[] pTemplate;
    return nResult;
}

CString TaechangCalcCompanyPickerDlg::GetSelectedName() const {
    return m_strSelectedName;
}

BOOL TaechangCalcCompanyPickerDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(TAECHANG_UI_PICKER_DLG_TITLE);

    m_brushBackground.CreateSolidBrush(TAECHANG_COLOR_APP_BACKGROUND);
    m_brushPanel.CreateSolidBrush(TAECHANG_COLOR_PANEL);

    CreateCaptionBar(TAECHANG_UI_PICKER_DLG_TITLE);
    CreateControls();
    ApplyFont();
    SizeFramelessClient(TAECHANG_PICKER_DLG_WIDTH, TAECHANG_PICKER_DLG_HEIGHT + GetContentTop());
    LayoutControls();
    ApplySearchEditTextRect();

    FilterList(CString());

    if (!m_strInitialName.IsEmpty()) {
        int nIdx = m_wndNameList.FindStringExact(-1, m_strInitialName);
        if (nIdx != LB_ERR) {
            m_wndNameList.SetCurSel(nIdx);
            m_wndNameList.SetTopIndex(nIdx);
        }
    }

    m_wndSearchEdit.SetFocus();

    return FALSE;
}

BOOL TaechangCalcCompanyPickerDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
        if (pMsg->hwnd == m_wndSearchEdit.GetSafeHwnd()) {
            m_wndNameList.SetFocus();
            if (m_wndNameList.GetCurSel() == LB_ERR && m_wndNameList.GetCount() > 0)
                m_wndNameList.SetCurSel(0);
            return TRUE;
        }
        if (pMsg->hwnd == m_wndNameList.GetSafeHwnd()) {
            OnOK();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

void TaechangCalcCompanyPickerDlg::CreateControls() {
    CRect rectEmpty(0, 0, 0, 0);

    m_wndSearchEdit.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
        rectEmpty, this, ID_PICKER_DLG_SEARCH_EDIT);
    m_wndNameList.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        rectEmpty, this, ID_PICKER_DLG_LIST);
    m_wndError.Create(NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, rectEmpty, this);
    m_wndMatchCount.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, rectEmpty, this);
    m_wndOkBtn.Create(TAECHANG_UI_PICKER_DLG_OK,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDOK);
    m_wndCancelBtn.Create(TAECHANG_UI_PICKER_DLG_CANCEL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rectEmpty, this, IDCANCEL);

    m_wndSearchEdit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)(LPCWSTR)TAECHANG_UI_PICKER_SEARCH_CUE);
}

void TaechangCalcCompanyPickerDlg::LayoutControls() {
    int nM = TAECHANG_MARGIN;
    int nEditH = TAECHANG_EDIT_HEIGHT;
    int nBtnW = TAECHANG_LOGIN_DLG_BTN_WIDTH;
    int nBtnH = TAECHANG_BUTTON_HEIGHT;
    int nGap = TAECHANG_ROW_GAP;
    int nClientW = TAECHANG_PICKER_DLG_WIDTH;
    int nClientH = TAECHANG_PICKER_DLG_HEIGHT + GetContentTop();
    int nContentW = nClientW - nM * 2;

    int nSearchTop = GetContentTop() + nM;
    int nListTop = nSearchTop + nEditH + nGap;
    int nBtnTop = nClientH - nM - nBtnH;
    int nErrorTop = nBtnTop - TAECHANG_INLINE_MSG_HEIGHT;
    int nListH = nErrorTop - nListTop - nGap;
    if (nListH < nEditH * 4)
        nListH = nEditH * 4;

    m_wndSearchEdit.MoveWindow(nM, nSearchTop, nContentW, nEditH);
    m_wndNameList.MoveWindow(nM, nListTop, nContentW, nListH);
    m_wndError.MoveWindow(nM, nErrorTop, nContentW, TAECHANG_INLINE_MSG_HEIGHT);
    m_wndMatchCount.MoveWindow(nM, nBtnTop, nContentW, nBtnH);

    int nBtnRight = nClientW - nM;
    m_wndCancelBtn.MoveWindow(nBtnRight - nBtnW, nBtnTop, nBtnW, nBtnH);
    m_wndOkBtn.MoveWindow(nBtnRight - nBtnW * 2 - nGap, nBtnTop, nBtnW, nBtnH);
}

void TaechangCalcCompanyPickerDlg::ApplyFont() {
    m_font.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);

    m_wndSearchEdit.SetFont(&m_font);
    m_wndNameList.SetFont(&m_font);
    m_wndMatchCount.SetFont(SageUiResources::GetFont(SAGE_FONT_CAPTION));
    m_wndMatchCount.SetTextColorRole(SAGE_TEXT_SECONDARY);
    m_wndOkBtn.SetFont(&m_font);
    m_wndOkBtn.SetVariant(SAGE_BUTTON_PRIMARY);
    m_wndCancelBtn.SetFont(&m_font);
}

void TaechangCalcCompanyPickerDlg::ApplySearchEditTextRect() {
    CRect rectEdit;
    m_wndSearchEdit.GetClientRect(&rectEdit);
    rectEdit.left += 2;
    rectEdit.top += 4;
    rectEdit.right -= 2;
    rectEdit.bottom -= 2;
    m_wndSearchEdit.SendMessage(EM_SETRECTNP, 0, reinterpret_cast<LPARAM>(&rectEdit));
}

void TaechangCalcCompanyPickerDlg::FilterList(const CString& strKeyword) {
    m_wndNameList.SetRedraw(FALSE);
    m_wndNameList.ResetContent();
    for (int i = 0; i < m_arrAllNames.GetSize(); i++) {
        if (strKeyword.IsEmpty() || m_arrAllNames[i].Find(strKeyword) >= 0)
            m_wndNameList.AddString(m_arrAllNames[i]);
    }
    m_wndNameList.SetRedraw(TRUE);
    if (m_wndNameList.GetCount() > 0)
        m_wndNameList.SetCurSel(0);

    CString strCount;
    strCount.Format(TAECHANG_UI_PICKER_DLG_MATCH_FMT,
        static_cast<int>(m_arrAllNames.GetSize()), m_wndNameList.GetCount());
    m_wndMatchCount.SetWindowTextW(strCount);
}

void TaechangCalcCompanyPickerDlg::OnSearchChanged() {
    CString strKeyword;
    m_wndSearchEdit.GetWindowText(strKeyword);
    strKeyword.Trim();
    FilterList(strKeyword);
}

void TaechangCalcCompanyPickerDlg::OnListDblClick() {
    OnOK();
}

void TaechangCalcCompanyPickerDlg::OnOK() {
    m_wndError.ClearMessage();

    int nSel = m_wndNameList.GetCurSel();
    if (nSel == LB_ERR) {
        m_wndError.SetMessage(TAECHANG_UI_PICKER_SELECT_REQUIRED, SAGE_INLINE_ERROR);
        m_wndNameList.SetFocus();
        return;
    }
    m_wndNameList.GetText(nSel, m_strSelectedName);
    CDialog::OnOK();
}

void TaechangCalcCompanyPickerDlg::OnCancel() {
    CDialog::OnCancel();
}

HBRUSH TaechangCalcCompanyPickerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CSageLabel)))
        return hBrush;

    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }

    if (nCtlColor == CTLCOLOR_LISTBOX) {
        pDC->SetTextColor(TAECHANG_COLOR_TEXT);
        pDC->SetBkColor(TAECHANG_COLOR_PANEL);
        return m_brushPanel;
    }

    pDC->SetBkColor(TAECHANG_COLOR_APP_BACKGROUND);
    return m_brushBackground;
}
